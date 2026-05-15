import logging
import os
import tempfile
import time
from contextlib import asynccontextmanager, suppress
from pathlib import Path
from typing import Any, Dict, Optional

import whisper
from fastapi import FastAPI, File, HTTPException, UploadFile


logger = logging.getLogger("whisper_api")
logging.basicConfig(
    level=os.getenv("LOG_LEVEL", "INFO").upper(),
    format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
)

WHISPER_MODEL_NAME = os.getenv("WHISPER_MODEL", "small")
WHISPER_LANGUAGE = os.getenv("WHISPER_LANGUAGE", "fr")
MAX_UPLOAD_SIZE_MB = int(os.getenv("WHISPER_MAX_UPLOAD_SIZE_MB", "25"))
MAX_UPLOAD_SIZE_BYTES = MAX_UPLOAD_SIZE_MB * 1024 * 1024
UPLOAD_CHUNK_SIZE = 1024 * 1024
ALLOWED_EXTENSIONS = {".wav", ".mp3", ".m4a", ".flac"}
ALLOWED_CONTENT_TYPES = {
    "audio/flac",
    "audio/mp3",
    "audio/mpeg",
    "audio/mp4",
    "audio/m4a",
    "audio/wav",
    "audio/x-flac",
    "audio/x-m4a",
    "audio/x-wav",
    "application/octet-stream",
}


@asynccontextmanager
async def lifespan(app: FastAPI):
    app.state.model = None
    app.state.model_name = WHISPER_MODEL_NAME
    app.state.model_load_error = None

    logger.info("Loading Whisper model '%s'...", WHISPER_MODEL_NAME)
    started_at = time.perf_counter()

    try:
        app.state.model = whisper.load_model(WHISPER_MODEL_NAME)
        load_duration = time.perf_counter() - started_at
        logger.info(
            "Whisper model '%s' loaded in %.2fs.",
            WHISPER_MODEL_NAME,
            load_duration,
        )
    except Exception as exc:
        app.state.model_load_error = str(exc)
        logger.exception("Unable to load Whisper model '%s'.", WHISPER_MODEL_NAME)

    yield


app = FastAPI(title="Whisper STT API", version="1.1.0", lifespan=lifespan)


def _get_extension(filename: Optional[str]) -> str:
    if not filename:
        return ""
    return Path(filename).suffix.lower()


def _validate_upload(file: UploadFile) -> str:
    extension = _get_extension(file.filename)
    if extension not in ALLOWED_EXTENSIONS:
        raise HTTPException(
            status_code=400,
            detail="Unsupported file type. Use WAV, MP3, M4A, or FLAC.",
        )

    if file.content_type and file.content_type.lower() not in ALLOWED_CONTENT_TYPES:
        raise HTTPException(
            status_code=400,
            detail=(
                "Unsupported media type. Expected an audio upload in WAV, MP3, M4A, or FLAC format."
            ),
        )

    return extension


async def _save_upload_to_temp(file: UploadFile, extension: str) -> tuple[str, int]:
    total_bytes = 0
    temp_path = None

    try:
        with tempfile.NamedTemporaryFile(delete=False, suffix=extension) as temp_file:
            temp_path = temp_file.name

            while chunk := await file.read(UPLOAD_CHUNK_SIZE):
                total_bytes += len(chunk)
                if total_bytes > MAX_UPLOAD_SIZE_BYTES:
                    raise HTTPException(
                        status_code=413,
                        detail=f"File too large. Maximum allowed size is {MAX_UPLOAD_SIZE_MB} MB.",
                    )
                temp_file.write(chunk)
    except Exception:
        if temp_path:
            with suppress(FileNotFoundError):
                os.unlink(temp_path)
        raise

    if total_bytes == 0:
        raise HTTPException(status_code=400, detail="Uploaded file is empty.")

    return temp_path, total_bytes


def _build_health_payload(app: FastAPI) -> Dict[str, Any]:
    model_ready = app.state.model is not None
    payload: Dict[str, Any] = {
        "status": "ok" if model_ready else "degraded",
        "model": app.state.model_name,
        "model_loaded": model_ready,
        "max_upload_size_mb": MAX_UPLOAD_SIZE_MB,
    }
    if app.state.model_load_error:
        payload["model_load_error"] = app.state.model_load_error
    return payload


@app.get("/health")
async def health_check():
    payload = _build_health_payload(app)
    if payload["model_loaded"]:
        return payload
    raise HTTPException(status_code=503, detail=payload)


@app.post("/transcribe")
async def transcribe_audio(file: UploadFile = File(...)):
    if app.state.model is None:
        raise HTTPException(
            status_code=503,
            detail={
                "message": "Whisper model is not ready.",
                "model": app.state.model_name,
                "model_load_error": app.state.model_load_error,
            },
        )

    extension = _validate_upload(file)
    temp_path = None
    file_size = 0

    try:
        temp_path, file_size = await _save_upload_to_temp(file, extension)

        logger.info(
            "Starting transcription for '%s' (%d bytes).",
            file.filename or "upload",
            file_size,
        )
        started_at = time.perf_counter()
        result = app.state.model.transcribe(temp_path, language=WHISPER_LANGUAGE)
        duration = time.perf_counter() - started_at
        text = result.get("text", "")

        logger.info(
            "Completed transcription for '%s' in %.2fs.",
            file.filename or "upload",
            duration,
        )
        return {"text": text}
    except HTTPException:
        raise
    except RuntimeError as exc:
        logger.exception("Whisper runtime error while transcribing '%s'.", file.filename)
        raise HTTPException(
            status_code=500,
            detail="Transcription failed because the Whisper runtime reported an error.",
        ) from exc
    except Exception as exc:
        logger.exception("Unexpected transcription failure for '%s'.", file.filename)
        raise HTTPException(
            status_code=500,
            detail="Unexpected server error during transcription.",
        ) from exc
    finally:
        await file.close()
        if temp_path:
            with suppress(FileNotFoundError):
                os.unlink(temp_path)