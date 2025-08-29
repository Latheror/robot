# Quick Start (Official Instructions)

1. **Clone the repository**

```
git clone https://github.com/ggml-org/whisper.cpp.git
```

2. **Navigate into the directory**

```
cd whisper.cpp
```

3. **Download a Whisper model in ggml format**

For example:
```
sh ./models/download-ggml-model.sh base.en
```

4. **Build the whisper-cli example**

```
cmake -B build
cmake --build build -j --config Release
```

5. **Transcribe an audio file**

```
./build/bin/whisper-cli -f samples/jfk.wav
```

---
# Whisper.cpp Setup and Usage

## 1. Download the Model


The required model for whisper.cpp is `ggml-small.bin`.

Download it from the official source:

[https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-small.bin](https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-small.bin)

**Place the file in:**
```
Backend/whisper.cpp/models/ggml-small.bin
```

## 2. Prepare an Audio File

- The audio file must be in WAV format, mono channel, 16 kHz sample rate.
- You can convert your audio using [Audacity](https://www.audacityteam.org/) or the following command (requires ffmpeg):

```
ffmpeg -i input.mp3 -ar 16000 -ac 1 output.wav
```

## 3. Run Transcription (French Audio)

Use the following command to transcribe a French audio file:

```
Backend/whisper.cpp/examples/main.exe -m Backend/whisper.cpp/models/ggml-small.bin -f path/to/your_audio.wav -l fr
```

- `-m` specifies the model file.
- `-f` specifies your WAV audio file.
- `-l fr` sets the language to French.

## 4. Notes

- The `small` model is fast and reasonably accurate for most tasks. For higher accuracy, consider using larger models (see the [official repo](https://github.com/ggerganov/whisper.cpp)).
- For best results, ensure your audio is clear and properly formatted.
- You can find more options and details in the [whisper.cpp documentation](https://github.com/ggerganov/whisper.cpp).
