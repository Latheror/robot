# Whisper STT API

A lightweight FastAPI wrapper around OpenAI Whisper for local speech-to-text.

## What changed

- explicit startup logging for model loading
- `/health` endpoint for readiness checks
- upload validation by extension, media type, emptiness, and max size
- clearer HTTP errors when the model is unavailable or transcription fails

## Usage

1. Build and run the container:
   ```bash
   docker compose up --build
   ```

2. The API will be available at `http://localhost:8000`

3. Endpoint: `POST /transcribe`
   - Upload a WAV, MP3, M4A, or FLAC file.
   - Returns JSON: `{"text": "transcribed text"}`

4. Readiness endpoint: `GET /health`
   - Returns `200` when the Whisper model is loaded.
   - Returns `503` with diagnostic details when startup failed.

## Configuration

The container reads these optional environment variables:

- `WHISPER_MODEL` - Whisper model name to load, default `small`
- `WHISPER_MAX_UPLOAD_SIZE_MB` - max upload size, default `25`

## Example with curl

```bash
curl -X POST "http://localhost:8000/transcribe" -F "file=@your_audio.wav"
```

## Health check example

```bash
curl http://localhost:8000/health
```