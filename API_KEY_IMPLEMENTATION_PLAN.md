# API Key Authentication Implementation Plan

## Overview

This document outlines the plan for implementing API key authentication to protect the ERIC API endpoints.

## Current Status

- ✅ API key generation documented in deployment guide
- ✅ Environment variable `API_KEY` configured in `docker-compose.prod.yml`
- ❌ Authentication decorator not yet implemented in `api.py`
- ❌ Endpoints not yet protected

## Implementation Steps

### Step 1: Add Authentication Decorator

Add the `require_api_key` decorator to `api.py`:

**Location**: After imports, before route definitions

```python
from functools import wraps
import os

def require_api_key(f):
    @wraps(f)
    def decorated_function(*args, **kwargs):
        api_key = request.headers.get('X-API-Key') or request.args.get('api_key')
        expected_key = os.environ.get('API_KEY')

        if not expected_key:
            return jsonify({'error': 'API key not configured'}), 500

        if not api_key or api_key != expected_key:
            return jsonify({'error': 'Invalid or missing API key'}), 401

        return f(*args, **kwargs)
    return decorated_function
```

### Step 2: Apply Decorator to Protected Endpoints

Apply `@require_api_key` decorator to:

- `/validate` endpoint
- `/submit` endpoint

**Keep public** (no decorator):

- `/health` endpoint (needed for health checks)

**Example**:

```python
@app.route('/validate', methods=['POST'])
@require_api_key
def validate_xml():
    # ... existing code

@app.route('/submit', methods=['POST'])
@require_api_key
def submit_vat_return():
    # ... existing code
```

### Step 3: Test Authentication

#### Test without API key (should fail):

```bash
curl -X POST https://your-domain.com/validate \
  -H "Content-Type: application/json" \
  -d '{"xml": "..."}'
# Expected: 401 Unauthorized
```

#### Test with invalid API key (should fail):

```bash
curl -X POST https://your-domain.com/validate \
  -H "Content-Type: application/json" \
  -H "X-API-Key: wrong-key" \
  -d '{"xml": "..."}'
# Expected: 401 Unauthorized
```

#### Test with valid API key (should succeed):

```bash
curl -X POST https://your-domain.com/validate \
  -H "Content-Type: application/json" \
  -H "X-API-Key: your-actual-api-key" \
  -d '{"xml": "..."}'
# Expected: 200 OK with validation result
```

#### Test health endpoint (should work without key):

```bash
curl https://your-domain.com/health
# Expected: 200 OK (no authentication required)
```

### Step 4: Update Documentation

After implementation:

- [ ] Update `README.md` with API key authentication instructions
- [ ] Add API key examples to API documentation
- [ ] Update deployment guide to mark Step 6 as completed

## API Key Usage

### Header Method (Recommended)

```bash
curl -H "X-API-Key: your-api-key" ...
```

### Query Parameter Method (Alternative)

```bash
curl "https://your-domain.com/validate?api_key=your-api-key" ...
```

## Security Considerations

1. **Environment Variable**: API key is stored in `.env.prod` (not committed to Git)
2. **HTTPS Required**: API keys should only be sent over HTTPS in production
3. **Key Rotation**: Plan for periodic key rotation
4. **Rate Limiting**: Already implemented in Nginx (complements API key auth)

## Rollback Plan

If issues arise:

1. Remove `@require_api_key` decorators from endpoints
2. Restart containers: `docker compose -f docker-compose.prod.yml restart eric-api`
3. API will work without authentication again

## Future Enhancements

- [ ] Multiple API keys support (for different clients)
- [ ] API key scoping (read-only vs. full access)
- [ ] API key expiration dates
- [ ] API key usage logging/analytics
- [ ] JWT-based authentication (more advanced)

## Notes

- The `API_KEY` environment variable is already configured in `docker-compose.prod.yml`
- Generating the API key during deployment is recommended so it's ready when needed
- The API will work fine without authentication until this is implemented
