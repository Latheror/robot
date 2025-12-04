import torch
import soundfile as sf
from chatterbox.tts import ChatterboxTTS
from huggingface_hub import hf_hub_download
from safetensors.torch import load_file
from fastapi import FastAPI, HTTPException
from fastapi.responses import FileResponse
from pydantic import BaseModel
import os
import tempfile

app = FastAPI()

MODEL_REPO = "Thomcles/Chatterbox-TTS-French"
CHECKPOINT_FILENAME = "t3_cfg.safetensors"

model = None
device = None

class SynthesisRequest(BaseModel):
    text: str
    exaggeration: float = 0.5
    temperature: float = 0.6
    cfg_weight: float = 0.3

def get_device() -> str:
    return "cuda" if torch.cuda.is_available() else "cpu"

def download_checkpoint(repo: str, filename: str) -> str:
    return hf_hub_download(repo_id=repo, filename=filename)

def load_tts_model(repo: str, checkpoint_file: str, device: str) -> ChatterboxTTS:
    model = ChatterboxTTS.from_pretrained(device=device)
    checkpoint_path = download_checkpoint(repo, checkpoint_file)
    t3_state = load_file(checkpoint_path, device="cpu")
    model.t3.load_state_dict(t3_state)
    return model

@app.on_event("startup")
async def startup_event():
    global model, device
    device = get_device()
    print(f"Loading model on {device}...")
    model = load_tts_model(MODEL_REPO, CHECKPOINT_FILENAME, device)
    print("Model loaded.")

@app.post("/synthesize")
async def synthesize(request: SynthesisRequest):
    if model is None:
        raise HTTPException(status_code=500, detail="Model not loaded")
    
    try:
        with torch.inference_mode():
            wav = model.generate(
                text=request.text,
                audio_prompt_path=None,
                exaggeration=request.exaggeration,
                temperature=request.temperature,
                cfg_weight=request.cfg_weight
            )
        
        # Save to temporary file
        with tempfile.NamedTemporaryFile(delete=False, suffix=".wav") as tmp_file:
            sf.write(tmp_file.name, wav.squeeze().cpu().numpy(), model.sr)
            tmp_path = tmp_file.name
        
        return FileResponse(tmp_path, media_type="audio/wav", filename="output.wav")
    
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)