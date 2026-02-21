from fastapi import FastAPI, File, UploadFile, HTTPException
import whisper
import tempfile
import os

app = FastAPI(title="Whisper STT API")

# Load the model (small model for speed)
model = whisper.load_model("small")

@app.post("/transcribe")
async def transcribe_audio(file: UploadFile = File(...)):
    if not file.filename.endswith(('.wav', '.mp3', '.m4a', '.flac')):
        raise HTTPException(status_code=400, detail="Unsupported file type. Use WAV, MP3, M4A, or FLAC.")

    # Save uploaded file to temp
    with tempfile.NamedTemporaryFile(delete=False, suffix=os.path.splitext(file.filename)[1]) as temp_file:
        temp_file.write(await file.read())
        temp_path = temp_file.name

    try:
        # Transcribe
        result = model.transcribe(temp_path)
        text = result["text"]
        return {"text": text}
    finally:
        # Clean up
        os.unlink(temp_path)