# ERIC API Service

A REST API service for submitting advanced VAT returns (UStVA) to ELSTER using the ERIC library. This service processes XML files, validates them, submits them with certificate authentication, and returns PDF confirmations.

## Features

- Submit advanced VAT returns (UStVA) to ELSTER
- Validate XML before submission
- Certificate-based authentication (base64 encoded .pfx files)
- Automatic PDF generation
- Automatic extraction of data type version from XML
- Comprehensive error handling

## Setup

### Prerequisites

- Docker and Docker Compose
- ERIC libraries installed in `Linux-x86_64/lib`
- Valid ELSTER certificate (.pfx file)

### Installation

1. Build and start the Docker container:

```bash
docker-compose up -d --build
```

2. Check if the server is running:

```bash
curl http://localhost:5000/health
```

3. View logs:

```bash
docker-compose logs -f eric-api
```

4. Stop the server:

```bash
docker-compose down
```

## Running

Start the API with Docker:

```bash
docker-compose up -d
```

The API will start on `http://localhost:5000`

To rebuild after code changes:

```bash
docker-compose up -d --build
```

## API Endpoints

### POST /submit

Submit an advanced VAT return (UStVA) to ELSTER.

**Request Body (JSON):**

```json
{
  "xml": "<xml content>",
  "cert_base64": "base64 encoded .pfx file",
  "password": "certificate password",
  "datenartversion": "UStVA_2025" (optional, auto-detected if not provided),
  "return_pdf": true (optional, default: true)
}
```

**Response:**

- If `return_pdf=true` (default): Returns PDF file with submission metadata in headers:
  - `X-Transfer-Handle`: Transfer handle from ELSTER
  - `X-Submission-Status`: Submission status
  - `X-Server-Response`: Server response (truncated)
- If `return_pdf=false`: Returns JSON with:
  - `status`: "success"
  - `transfer_handle`: Transfer handle
  - `pdf_base64`: PDF as base64 encoded string
  - `server_response`: Server response XML
  - `message`: Success message

**Example:**

```bash
curl -X POST http://localhost:5000/submit \
  -H "Content-Type: application/json" \
  -d '{
    "xml": "<?xml version=\"1.0\" encoding=\"UTF-8\"?>...",
    "cert_base64": "MIIKpAIBAzCCCl4GCSqGSIb3...",
    "password": "your_password"
  }' \
  --output submission_result.pdf
```

### POST /validate

Validate XML without submitting (useful for testing).

**Request Body (JSON):**

```json
{
  "xml": "<xml content>",
  "datenartversion": "UStVA_2025" (optional)
}
```

**Response:**

```json
{
  "valid": true,
  "message": "Validation successful",
  "validation_result": "..."
}
```

**Example:**

```bash
curl -X POST http://localhost:5000/validate \
  -H "Content-Type: application/json" \
  -d '{
    "xml": "<?xml version=\"1.0\" encoding=\"UTF-8\"?>..."
  }'
```

### GET /health

Health check endpoint.

**Response:**

```json
{
  "status": "ok"
}
```

## Python Client Example

```python
import requests
import base64

# Read certificate file
with open('certificate.pfx', 'rb') as f:
    cert_base64 = base64.b64encode(f.read()).decode('utf-8')

# Read XML file
with open('ustva_2025.xml', 'r') as f:
    xml_content = f.read()

# Submit VAT return
response = requests.post(
    'http://localhost:5000/submit',
    json={
        'xml': xml_content,
        'cert_base64': cert_base64,
        'password': 'your_password'
    }
)

if response.status_code == 200:
    # Save PDF
    with open('submission_result.pdf', 'wb') as f:
        f.write(response.content)

    # Get transfer handle from headers
    transfer_handle = response.headers.get('X-Transfer-Handle')
    print(f'Submission successful! Transfer Handle: {transfer_handle}')
else:
    print(f'Error: {response.json()}')
```

## Error Handling

The API returns appropriate HTTP status codes:

- `200`: Success
- `400`: Bad request (missing/invalid parameters)
- `500`: Server error (ERIC processing failed)

Error responses include:

```json
{
  "error": "Error description",
  "error_code": 610001001,
  "error_message": "Detailed error message",
  "server_response": "..."
}
```

## Configuration

The API configuration is set via environment variables in `docker-compose.yml`:

- `ERIC_HOME_DIR`: Path to ERIC libraries (default: `/app/Linux-x86_64/lib`)
- `ERIC_LOG_DIR`: Path for ERIC log files (default: `/app/logs`)

Port mapping is configured in `docker-compose.yml` (default: `5000:5000`)

## Notes

- The API automatically extracts the `datenartversion` from the XML if not provided
- For UStVA, it looks for the year in the XML structure
- Certificates are temporarily stored during processing and cleaned up afterwards
- The ERIC instance is reused across requests for better performance
- All temporary files are cleaned up after processing

## Changelog

### 2025-01-XX - Initial Release

- Created REST API for submitting advanced VAT returns (UStVA) to ELSTER
- Implemented `/submit` endpoint for authenticated submission with certificate
- Implemented `/validate` endpoint for XML validation without submission
- Implemented `/health` endpoint for health checks
- Added automatic extraction of `datenartversion` from XML
- Added support for base64-encoded certificate files
- Added PDF generation and return functionality
- Added comprehensive error handling with ERIC error codes
- Added automatic cleanup of temporary files
- Rewrote API to use ericapi directly without demo dependencies
- Implemented standalone buffer and certificate management with context managers
- Docker-only deployment (no local Python setup required)
- Improved `extract_datenart_version` to handle `Anmeldungssteuern` tag with version attribute
- Added support for multiple XML structure patterns (UStVA_2025 and Anmeldungssteuern)
- Fixed PDF generation to always capture PDF via callback (works with both return_pdf true/false)

## Deployment

For production deployment to Hetzner Cloud, see [DEPLOYMENT.md](DEPLOYMENT.md) for step-by-step instructions.

**Recommended Server**: CX23 (Cost-Optimized) - €3.56/month

- 2 vCPU, 4 GB RAM, 40 GB SSD
- Perfect for 1000 users with monthly submissions
- 50% cheaper than regular performance servers

## Production Deployment & Maintenance

### Initial Deployment

See [DEPLOYMENT.md](DEPLOYMENT.md) for complete initial setup instructions.

### Deploying New Code

When you have code changes to deploy:

#### Option 1: Using rsync (from local machine)

```bash
# From your local project directory
cd /path/to/solobooks-eric-api

# Transfer updated files (exclude unnecessary files)
rsync -avz --progress -e "ssh -i ~/.ssh/id_rsa_solobooks" \
    --exclude '.git' \
    --exclude '__pycache__' \
    --exclude '*.pyc' \
    --exclude 'logs' \
    --exclude '*.log' \
    --exclude '.env' \
    --exclude 'docker-compose.yml' \
    --exclude 'Dockerfile' \
    ./ root@YOUR_SERVER_IP:/opt/eric-api/
```

#### Option 2: Using Git (if code is in repository)

```bash
# On the server
cd /opt/eric-api
git pull origin main  # or your branch name
```

### Restarting Services After Code Changes

After deploying new code, rebuild and restart:

```bash
# Navigate to project directory
cd /opt/eric-api

# Load environment variables
export $(cat .env.prod | xargs)

# Rebuild and restart containers
docker compose -f docker-compose.prod.yml up -d --build

# Check status
docker compose -f docker-compose.prod.yml ps

# View logs to verify everything started correctly
docker compose -f docker-compose.prod.yml logs -f
```

### Quick Restart (No Code Changes)

If you just need to restart services without code changes:

```bash
cd /opt/eric-api
docker compose -f docker-compose.prod.yml restart

# Or restart specific service
docker compose -f docker-compose.prod.yml restart eric-api
docker compose -f docker-compose.prod.yml restart nginx
```

### Viewing Logs

```bash
# All services
docker compose -f docker-compose.prod.yml logs -f

# Specific service
docker compose -f docker-compose.prod.yml logs -f eric-api

# Last 100 lines
docker compose -f docker-compose.prod.yml logs --tail=100 eric-api
```

### Stopping Services

```bash
# Stop all services
docker compose -f docker-compose.prod.yml down

# Stop specific service
docker compose -f docker-compose.prod.yml stop eric-api
```

### Common Maintenance Tasks

#### Update SSL Certificate (Let's Encrypt Auto-Renewal)

Certificates auto-renew, but you may need to copy them:

```bash
# Stop nginx
docker stop eric-api-nginx

# Renew certificate (if needed)
certbot renew

# Copy updated certificates
cp /etc/letsencrypt/live/eric-api.yourdomain.com/fullchain.pem /opt/eric-api/nginx/ssl/cert.pem
cp /etc/letsencrypt/live/eric-api.yourdomain.com/privkey.pem /opt/eric-api/nginx/ssl/key.pem

# Restart nginx
docker start eric-api-nginx
```

#### Check Container Status

```bash
docker compose -f docker-compose.prod.yml ps
docker stats  # Resource usage
```

#### Access Container Shell

```bash
# Access eric-api container
docker compose -f docker-compose.prod.yml exec eric-api bash

# Access nginx container
docker compose -f docker-compose.prod.yml exec nginx sh
```

## License

See the ERIC license documentation in `Linux-x86_64/lizenz.pdf`
