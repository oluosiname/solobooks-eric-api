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

## License

See the ERIC license documentation in `Linux-x86_64/lizenz.pdf`
