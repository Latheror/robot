import torch
import soundfile as sf
import argparse
from chatterbox.tts import ChatterboxTTS
from huggingface_hub import hf_hub_download
from safetensors.torch import load_file

# Configuration
MODEL_REPO = "Thomcles/Chatterbox-TTS-French"
CHECKPOINT_FILENAME = "t3_cfg.safetensors"
OUTPUT_PATH = "output_cloned_voice.wav"

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

def synthesize_speech(model: ChatterboxTTS, text: str, audio_prompt_path:str, **kwargs) -> torch.Tensor:
    with torch.inference_mode():
        return model.generate(
            text=text, 
            audio_prompt_path=audio_prompt_path, 
            **kwargs
        )

def save_audio(waveform: torch.Tensor, path: str, sample_rate: int):
    sf.write(path, waveform.squeeze().cpu().numpy(), sample_rate)

def main():
    parser = argparse.ArgumentParser(description="Run Chatterbox TTS")
    parser.add_argument(
        "--text", 
        type=str, 
        default="Ceci est un test du modèle Chatterbox TTS.", 
        help="Texte à synthétiser."
    )
    parser.add_argument(
        "--output",
        type=str,
        default=OUTPUT_PATH,
        help="Chemin du fichier audio de sortie."
    )
    args = parser.parse_args()

    print("Loading model...")
    device = get_device()
    model = load_tts_model(MODEL_REPO, CHECKPOINT_FILENAME, device)

    print(f"Generating speech on {device}...")
    wav = synthesize_speech(
        model,
        args.text,
        audio_prompt_path=None,
        exaggeration=0.4,
        temperature=0.4,
        cfg_weight=0.5
    )

    print(f"Saving output to: {args.output}")
    save_audio(wav, args.output, model.sr)
    print("Done.")

if __name__ == "__main__":
    main()
