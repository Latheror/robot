FROM python:3.10-slim

WORKDIR /app

RUN apt-get update && apt-get install -y gcc g++ && rm -rf /var/lib/apt/lists/*

RUN pip install -U pip setuptools wheel

RUN pip install torch

RUN pip install "numpy>=1.24,<1.26"

COPY requirements.txt .

RUN pip install --no-cache-dir -r requirements.txt

RUN pip install --no-deps chatterbox-tts

COPY app.py .

EXPOSE 8000

CMD ["python", "app.py"]