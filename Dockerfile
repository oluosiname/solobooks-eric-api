FROM --platform=linux/amd64 python:3.11-slim

# Install system dependencies that ERIC might need
RUN apt-get update && apt-get install -y \
    libc6 \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy ERIC libraries and Python modules
COPY Linux-x86_64/lib /app/Linux-x86_64/lib
COPY Linux-x86_64/Beispiel/ericdemo-python /app/Linux-x86_64/Beispiel/ericdemo-python

# Copy application files
COPY requirements.txt /app/
COPY api.py /app/
COPY models.py /app/
COPY eric_client.py /app/

# Install Python dependencies
RUN pip install --no-cache-dir -r requirements.txt

# Create logs directory
RUN mkdir -p /app/logs

# Expose port
EXPOSE 5000

# Run the API
CMD ["python", "api.py"]

