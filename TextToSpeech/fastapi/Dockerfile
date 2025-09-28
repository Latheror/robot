# Use Python 3.11 slim for compatibility
FROM python:3.11-slim

WORKDIR /app

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    libsndfile1 \
    git \
    && rm -rf /var/lib/apt/lists/*

# Upgrade pip, setuptools, wheel
RUN pip install --upgrade pip setuptools wheel

# Install numpy first to prevent build errors (needed by pkuseg)
RUN pip install numpy

# CPU-only PyTorch + torchaudio
RUN pip install torch==2.2.0+cpu torchaudio==2.2.0+cpu -f https://download.pytorch.org/whl/torch_stable.html

# Copy requirements.txt and install the rest
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy the application code
COPY . .

# Expose port for FastAPI
EXPOSE 8000

# Run FastAPI
CMD ["uvicorn", "app:app", "--host", "0.0.0.0", "--port", "8000"]
