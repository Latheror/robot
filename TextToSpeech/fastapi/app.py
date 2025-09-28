from fastapi import FastAPI, Query
from fastapi.responses import FileResponse
from pathlib import Path
from chatterbox_run import get_device, load_tts_model, synthesize_speech, save_audio, MODEL_REPO, CHECKPOINT_FILENAME
from chatterbox.tts import ChatterboxTTS
import torch
import tempfile

app = FastAPI()

# Load the TTS model once at startup
device = get_device()
tts_model = load_tts_model(MODEL_REPO, CHECKPOINT_FILENAME, device)

@app.get("/synthesize")
def synthesize(text: str = Query(..., description="Text to synthesize")):
    """
    Generate a WAV file from the given text.
    Returns the WAV file.
    """
    # Create a temporary file for the WAV
    with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as tmp_file:
        wav_path = Path(tmp_file.name)

    # Generate speech
    wav_tensor = synthesize_speech(
        tts_model,
        text,
        audio_prompt_path=None,
        exaggeration=0.4,
        temperature=0.4,
        cfg_weight=0.5
    )

    # Save the WAV file
    save_audio(wav_tensor, str(wav_path), tts_model.sr)

    # Return the WAV file
    return FileResponse(path=str(wav_path), filename="output.wav", media_type="audio/wav")
