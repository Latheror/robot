# LLM Setup Guide (vLLM)

## Overview

The robot uses **vLLM** as the local language-model inference backend. vLLM exposes an OpenAI-compatible HTTP API, which lets n8n and Open WebUI call the local model through standard `/v1` endpoints.

**Location**: `LLM/docker-compose.yml`

## Runtime Services

| Service | Image | Purpose | Host Port |
|---------|-------|---------|-----------|
| `vllm` | `vllm/vllm-openai:latest` | OpenAI-compatible LLM inference | `8001` |
| `openwebui` | `ghcr.io/open-webui/open-webui:main` | Optional web UI connected to vLLM | `3000` |

The Compose file uses a persistent `vllm-cache` volume for Hugging Face model files and a health check on `/health`.

For n8n AI Agent tool calls, vLLM must be started with auto tool choice enabled.
The default parser in this workspace is `hermes`, which matches the Qwen2.5
instruction model used here.

## Model Selection

Default model:

```env
VLLM_MODEL=Qwen/Qwen2.5-3B-Instruct
VLLM_SERVED_MODEL_NAME=robot-llm
```

This replaces the previous local `llama3.2` workflow model with a similar-size, broadly supported instruction model that works well with vLLM. The served name `robot-llm` is intentionally stable so n8n does not need edits when the underlying Hugging Face model changes.

Override the model by copying `LLM/.env.example` to `LLM/.env` and changing `VLLM_MODEL`.

## Start the Stack

```bash
docker compose -f LLM/docker-compose.yml pull
docker compose -f LLM/docker-compose.yml up -d
```

If you change the model, keep the tool parser aligned with the model family.
For example, the current Qwen2.5 setup uses:

```env
VLLM_TOOL_CALL_PARSER=hermes
```

## API Usage

Base URL from the host:

```text
http://localhost:8001/v1
```

Base URL from containers that can reach the host:

```text
http://host.docker.internal:8001/v1
```

Base URL from services inside `LLM/docker-compose.yml`:

```text
http://vllm:8000/v1
```

### Health Check

```bash
curl http://localhost:8001/health
```

### List Models

```bash
curl http://localhost:8001/v1/models
```

### Chat Completion

```bash
curl http://localhost:8001/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
    "model": "robot-llm",
    "messages": [
      {"role": "user", "content": "Réponds en trois mots: bonjour robot"}
    ],
    "max_tokens": 32,
    "temperature": 0.2
  }'
```

### Python Example

```python
import requests

VLLM_URL = "http://localhost:8001/v1"
VLLM_MODEL = "robot-llm"

def chat_with_vllm(prompt: str) -> str:
    response = requests.post(
        f"{VLLM_URL}/chat/completions",
        json={
            "model": VLLM_MODEL,
            "messages": [
                {"role": "system", "content": "Tu es Margot, un robot assistant amical."},
                {"role": "user", "content": prompt},
            ],
            "max_tokens": 120,
            "temperature": 0.7,
        },
        timeout=120,
    )
    response.raise_for_status()
    return response.json()["choices"][0]["message"]["content"].strip()
```

## Integration with n8n

The workflow in `N8N/n8n_workflow.json` now uses the n8n **OpenAI Chat Model** node with:

```text
Base URL: http://host.docker.internal:8001/v1
Model: robot-llm
Responses API: disabled
```

If you see the error `"auto" tool choice requires --enable-auto-tool-choice and --tool-call-parser to be set`,
restart vLLM with the updated Compose file above.

The API key can be any non-empty placeholder for local vLLM unless API-key enforcement is enabled in vLLM.

## Open WebUI

Open WebUI is configured with:

```env
ENABLE_OLLAMA_API=false
OPENAI_API_BASE_URL=http://vllm:8000/v1
OPENAI_API_KEY=not-needed-local-vllm
```

Access it at:

```text
http://localhost:3000
```

## Troubleshooting

### vLLM container is unhealthy

1. Inspect logs: `docker logs vllm --tail 200`
2. Confirm GPU availability: `docker run --rm --gpus all nvidia/cuda:12.4.1-base-ubuntu22.04 nvidia-smi`
3. Lower memory pressure in `LLM/.env`:
   ```env
  VLLM_GPU_MEMORY_UTILIZATION=0.65
   VLLM_MAX_MODEL_LEN=2048
   ```

### Model download fails

1. Confirm internet access from Docker.
2. For gated models, set `HUGGING_FACE_HUB_TOKEN` in `LLM/.env`.
3. Use a public model such as `Qwen/Qwen2.5-3B-Instruct`.

### n8n cannot connect

1. Confirm `curl http://localhost:8001/health` succeeds from the host.
2. Confirm `host.docker.internal` works from the n8n container.
3. Confirm the n8n OpenAI credential uses any non-empty API key and the node option `baseURL` remains `http://host.docker.internal:8001/v1`.

## Resources

- **vLLM**: https://docs.vllm.ai
- **OpenAI-compatible server**: https://docs.vllm.ai/en/latest/serving/openai_compatible_server.html
- **Hugging Face models**: https://huggingface.co/models

---

Last updated: May 2026
