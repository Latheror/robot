# Whisper STT API

A simple API for speech-to-text using OpenAI Whisper.

## Usage

1. Build and run the container:
   ```bash
   docker-compose up --build
   ```

2. The API will be available at `http://localhost:8000`

3. Endpoint: `POST /transcribe`
   - Upload a WAV, MP3, M4A, or FLAC file.
   - Returns JSON: `{"text": "transcribed text"}`

## Example with curl

```bash
curl -X POST "http://localhost:8000/transcribe" -F "file=@your_audio.wav"
```