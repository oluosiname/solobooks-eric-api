# Hetzner Deployment Guide

Step-by-step guide to deploy the ERIC API to Hetzner Cloud using Docker.

## Prerequisites

- Hetzner Cloud account ✓
- Domain name (optional, but recommended for HTTPS)
- SSH key pair for server access
- Local machine with Docker and Git installed

## Step 1: Create Hetzner Cloud Server

### 1.1 Create a New Server

1. Log in to [Hetzner Cloud Console](https://console.hetzner.cloud/)
2. Click **"Add Server"** or **"New Project"** → **"Add Server"**
3. **Select "Cost-Optimized" tab** (important - this is 50% cheaper!)
4. Configure server:
   - **Image**: Ubuntu 22.04
   - **Type**: **CX23** (2 vCPU, 4 GB RAM, 40 GB SSD) - €3.56/month
   - **Architecture**: x86 (Intel®/AMD)
   - **Location**: Choose closest to your users (e.g., Nuremberg, Falkenstein)
   - **SSH Keys**: Add your SSH public key
   - **Name**: `eric-api-production`
5. Click **"Create & Buy Now"**
6. Note the **IP address** (e.g., `123.45.67.89`)

**Note**: CX23 uses older hardware but is 50% cheaper than regular performance servers. Perfect for your monthly submission workload with 1000 users.

### 1.2 Test SSH Connection

```bash
ssh -i /path/to/your/ssh/key root@YOUR_SERVER_IP
```

**Note**: Replace `/path/to/your/ssh/key` with the actual path to your SSH private key file.

## Step 2: Server Initial Setup

### 2.1 Update System

```bash
apt-get update && apt-get upgrade -y
```

### 2.2 Install Docker

```bash
# Install Docker
curl -fsSL https://get.docker.com -o get-docker.sh
sh get-docker.sh

# Install Docker Compose
apt-get install -y docker-compose-plugin

# Verify installation
docker --version
docker compose version
```

### 2.3 Configure Firewall

```bash
# Install UFW (Uncomplicated Firewall)
apt-get install -y ufw

# Allow SSH
ufw allow 22/tcp

# Allow HTTP and HTTPS
ufw allow 80/tcp
ufw allow 443/tcp

# Enable firewall
ufw enable

# Check status
ufw status
```

### 2.4 Create Application Directory

```bash
mkdir -p /opt/eric-api
cd /opt/eric-api
```

## Step 3: Prepare Production Files

### 3.1 Create Production Dockerfile

On your **local machine**, create `Dockerfile.prod`:

```dockerfile
FROM --platform=linux/amd64 python:3.11-slim

# Install system dependencies
RUN apt-get update && apt-get install -y \
    libc6 \
    curl \
    && rm -rf /var/lib/apt/lists/*

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
RUN pip install gunicorn  # Production WSGI server

# Create logs directory
RUN mkdir -p /app/logs

# Create non-root user
RUN useradd -m -u 1000 appuser && chown -R appuser:appuser /app
USER appuser

EXPOSE 5000

# Use gunicorn for production
CMD ["gunicorn", "--bind", "0.0.0.0:5000", "--workers", "2", "--threads", "2", "--timeout", "120", "--access-logfile", "-", "--error-logfile", "-", "api:app"]
```

### 3.2 Create Production Docker Compose

Create `docker-compose.prod.yml`:

```yaml
services:
  eric-api:
    build:
      context: .
      dockerfile: Dockerfile.prod
    container_name: eric-api
    ports:
      - "127.0.0.1:5000:5000" # Only bind to localhost
    volumes:
      - ./logs:/app/logs
      - ./Linux-x86_64:/app/Linux-x86_64:ro # Mount ERIC libraries as read-only
    environment:
      - FLASK_ENV=production
      - API_KEY=${API_KEY}
      - ERIC_HOME_DIR=/app/Linux-x86_64/lib
      - ERIC_LOG_DIR=/app/logs
    restart: unless-stopped
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:5000/health"]
      interval: 30s
      timeout: 10s
      retries: 3
      start_period: 40s
    deploy:
      resources:
        limits:
          cpus: "2"
          memory: 3G
        reservations:
          cpus: "1"
          memory: 1G

  nginx:
    image: nginx:alpine
    container_name: eric-api-nginx
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./nginx/nginx.conf:/etc/nginx/nginx.conf:ro
      - ./nginx/ssl:/etc/nginx/ssl:ro
    depends_on:
      eric-api:
        condition: service_healthy
    restart: unless-stopped
```

### 3.3 Create Nginx Configuration

Create directory and config file:

```bash
mkdir -p nginx/ssl
```

Create `nginx/nginx.conf`:

```nginx
events {
    worker_connections 1024;
}

http {
    # Rate limiting
    limit_req_zone $binary_remote_addr zone=api_limit:10m rate=10r/s;
    limit_req_zone $binary_remote_addr zone=submit_limit:10m rate=2r/s;

    # Upstream
    upstream eric_api {
        server eric-api:5000;
    }

    # HTTP to HTTPS redirect
    server {
        listen 80;
        server_name _;

        # Let's Encrypt challenge
        location /.well-known/acme-challenge/ {
            root /var/www/certbot;
        }

        # Redirect everything else to HTTPS
        location / {
            return 301 https://$host$request_uri;
        }
    }

    # HTTPS server
    server {
        listen 443 ssl http2;
        server_name _;

        # SSL certificates (will be added by Certbot)
        ssl_certificate /etc/nginx/ssl/cert.pem;
        ssl_certificate_key /etc/nginx/ssl/key.pem;

        # SSL configuration
        ssl_protocols TLSv1.2 TLSv1.3;
        ssl_ciphers HIGH:!aNULL:!MD5;
        ssl_prefer_server_ciphers on;

        # Security headers
        add_header X-Frame-Options "SAMEORIGIN" always;
        add_header X-Content-Type-Options "nosniff" always;
        add_header X-XSS-Protection "1; mode=block" always;
        add_header Strict-Transport-Security "max-age=31536000; includeSubDomains" always;

        # Health check (no rate limit)
        location /health {
            proxy_pass http://eric_api;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;
        }

        # Submit endpoint (stricter rate limit)
        location /submit {
            limit_req zone=submit_limit burst=5 nodelay;

            proxy_pass http://eric_api;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;

            # Timeouts for long-running requests
            proxy_connect_timeout 60s;
            proxy_send_timeout 120s;
            proxy_read_timeout 120s;
        }

        # All other endpoints
        location / {
            limit_req zone=api_limit burst=20 nodelay;

            proxy_pass http://eric_api;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;

            proxy_connect_timeout 60s;
            proxy_send_timeout 60s;
            proxy_read_timeout 60s;
        }
    }
}
```

### 3.4 Create Environment File

Create `.env.prod` (keep this secure, don't commit to Git):

```bash
# API Key for authentication (OPTIONAL - generate now for future use)
# See API_KEY_IMPLEMENTATION_PLAN.md for when to implement authentication
API_KEY=your-super-secret-api-key-change-this-to-random-string

# Flask environment
FLASK_ENV=production
```

Generate a secure API key (optional but recommended):

```bash
# On your local machine
openssl rand -hex 32
```

Copy the output and use it as your `API_KEY`.

**Note**: API key authentication is not yet implemented. You can generate the key now and store it in `.env.prod` so it's ready when you implement authentication (see `API_KEY_IMPLEMENTATION_PLAN.md`). The API will work without authentication until then.

### 3.5 API Key Authentication (Future)

API key authentication is planned but not yet implemented. See `API_KEY_IMPLEMENTATION_PLAN.md` for the implementation plan. For now, you can skip to Step 4.

## Step 4: Deploy to Server

### 4.1 Transfer Files to Server

From your **local machine**:

**Using rsync with SSH key file:**
```bash
# Create deployment package (exclude unnecessary files)
rsync -avz --progress -e "ssh -i /path/to/your/ssh/key" \
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

**Or use SCP with SSH key file:**
```bash
scp -i /path/to/your/ssh/key -r \
    --exclude='.git' \
    --exclude='__pycache__' \
    --exclude='logs' \
    ./ root@YOUR_SERVER_IP:/opt/eric-api/
```

**Note**: Replace `/path/to/your/ssh/key` with the actual path to your SSH private key file (e.g., `~/.ssh/id_rsa` or `~/.ssh/id_rsa_solobooks`).

### 4.2 Set Up Environment on Server

SSH into your server:

```bash
ssh -i /path/to/your/ssh/key root@YOUR_SERVER_IP
cd /opt/eric-api
```

**Note**: Replace `/path/to/your/ssh/key` with the actual path to your SSH private key file.

Create `.env.prod` file:

```bash
nano .env.prod
```

Paste your environment variables:

```
API_KEY=your-generated-api-key-here
FLASK_ENV=production
```

Save and exit (Ctrl+X, then Y, then Enter).

### 4.3 Create Self-Signed SSL Certificate (for testing)

```bash
mkdir -p nginx/ssl
cd nginx/ssl

openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
    -keyout key.pem \
    -out cert.pem \
    -subj "/C=DE/ST=State/L=City/O=Organization/CN=localhost"

chmod 600 key.pem
chmod 644 cert.pem
cd /opt/eric-api
```

### 4.4 Build and Start Services

```bash
cd /opt/eric-api

# Load environment variables
export $(cat .env.prod | xargs)

# Build and start services
docker compose -f docker-compose.prod.yml up -d --build

# Check logs
docker compose -f docker-compose.prod.yml logs -f
```

Wait for services to start (about 30-60 seconds).

### 4.5 Verify Services Are Running

```bash
# Check container status
docker compose -f docker-compose.prod.yml ps

# Test health endpoint
curl http://localhost:5000/health

# Check logs
docker compose -f docker-compose.prod.yml logs eric-api
```

## Step 5: Set Up SSL/HTTPS (Optional but Recommended)

### 5.1 Install Certbot

```bash
apt-get update
apt-get install -y certbot python3-certbot-nginx
```

### 5.2 Get SSL Certificate

**Option A: With Domain Name**

```bash
# Stop Docker nginx container (required - certbot needs port 80)
docker stop eric-api-nginx

# Get certificate
certbot certonly --standalone -d eric-api.yourdomain.com

# Copy certificates to nginx directory
cp /etc/letsencrypt/live/eric-api.yourdomain.com/fullchain.pem /opt/eric-api/nginx/ssl/cert.pem
cp /etc/letsencrypt/live/eric-api.yourdomain.com/privkey.pem /opt/eric-api/nginx/ssl/key.pem
chmod 644 /opt/eric-api/nginx/ssl/cert.pem
chmod 600 /opt/eric-api/nginx/ssl/key.pem

# Start nginx container again
docker start eric-api-nginx
```

**Note**: Use `docker stop eric-api-nginx` instead of `docker compose stop nginx` to ensure the container is fully stopped and port 80 is free for certbot.

**Option B: Self-Signed Certificate (for testing)**

Already created in Step 4.3 - skip this step.

### 5.3 Set Up Auto-Renewal (for Let's Encrypt)

```bash
# Test renewal
certbot renew --dry-run

# Add to crontab
crontab -e

# Add this line (runs twice daily)
0 0,12 * * * certbot renew --quiet && cp /etc/letsencrypt/live/your-domain.com/fullchain.pem /opt/eric-api/nginx/ssl/cert.pem && cp /etc/letsencrypt/live/your-domain.com/privkey.pem /opt/eric-api/nginx/ssl/key.pem && docker compose -f /opt/eric-api/docker-compose.prod.yml restart nginx
```

## Step 6: Add API Key Authentication (Optional - Future)

API key authentication is not yet implemented. See `API_KEY_IMPLEMENTATION_PLAN.md` for the complete implementation plan. You can skip this step for now and deploy without authentication.

When ready to implement, follow the plan in `API_KEY_IMPLEMENTATION_PLAN.md`.

## Step 7: Test Deployment

### 7.1 Test Health Endpoint

```bash
curl https://YOUR_SERVER_IP/health
# or
curl https://your-domain.com/health
```

### 7.2 Test with API Key (When Implemented)

Once API key authentication is implemented (see `API_KEY_IMPLEMENTATION_PLAN.md`), test with:

```bash
curl -X POST https://your-domain.com/validate \
  -H "Content-Type: application/json" \
  -H "X-API-Key: your-api-key-here" \
  -d '{
    "xml": "<?xml version=\"1.0\" encoding=\"UTF-8\"?>..."
  }'
```

For now, you can test without the API key header since authentication is not yet implemented.

## Step 8: Monitoring and Maintenance

### 8.1 View Logs

```bash
# All services
docker compose -f docker-compose.prod.yml logs -f

# Specific service
docker compose -f docker-compose.prod.yml logs -f eric-api

# Last 100 lines
docker compose -f docker-compose.prod.yml logs --tail=100 eric-api
```

### 8.2 Monitor Resources

```bash
# Container stats
docker stats

# Disk usage
df -h

# Memory usage
free -h
```

### 8.3 Update Application

#### Deploying New Code

**Option 1: Using rsync (from local machine)**

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

**Option 2: Using Git (if code is in repository)**

```bash
# On the server
cd /opt/eric-api
git pull origin main  # or your branch name
```

#### Rebuild and Restart After Code Changes

```bash
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

#### Quick Restart (No Code Changes)

```bash
# Restart all services
docker compose -f docker-compose.prod.yml restart

# Or restart specific service
docker compose -f docker-compose.prod.yml restart eric-api
docker compose -f docker-compose.prod.yml restart nginx
```

### 8.4 Backup Logs

```bash
# Create backup script
cat > /opt/eric-api/backup-logs.sh << 'EOF'
#!/bin/bash
DATE=$(date +%Y%m%d)
tar -czf /opt/eric-api/logs-backup-${DATE}.tar.gz /opt/eric-api/logs
# Keep only last 7 days
find /opt/eric-api -name "logs-backup-*.tar.gz" -mtime +7 -delete
EOF

chmod +x /opt/eric-api/backup-logs.sh

# Add to crontab (daily at 2 AM)
crontab -e
# Add: 0 2 * * * /opt/eric-api/backup-logs.sh
```

## Step 9: Security Hardening

### 9.1 Disable Root SSH Login (Optional but Recommended)

```bash
# Create new user
adduser deploy
usermod -aG sudo deploy
usermod -aG docker deploy

# Copy SSH key
mkdir -p /home/deploy/.ssh
cp ~/.ssh/authorized_keys /home/deploy/.ssh/
chown -R deploy:deploy /home/deploy/.ssh
chmod 700 /home/deploy/.ssh
chmod 600 /home/deploy/.ssh/authorized_keys

# Test login as deploy user
# Then edit /etc/ssh/sshd_config:
# PermitRootLogin no
# Then: systemctl restart sshd
```

### 9.2 Set Up Fail2Ban

```bash
apt-get install -y fail2ban

# Configure for SSH
cat > /etc/fail2ban/jail.local << 'EOF'
[sshd]
enabled = true
port = 22
maxretry = 3
bantime = 3600
EOF

systemctl enable fail2ban
systemctl start fail2ban
```

## Troubleshooting

### Service Won't Start

```bash
# Check logs
docker compose -f docker-compose.prod.yml logs eric-api

# Check if port is in use
netstat -tulpn | grep 5000

# Restart services
docker compose -f docker-compose.prod.yml restart
```

### Out of Memory

```bash
# Check memory usage
free -h
docker stats

# If needed, upgrade server or reduce Gunicorn workers
# Edit Dockerfile.prod: --workers 1
```

### SSL Certificate Issues

```bash
# Check certificate
openssl x509 -in /opt/eric-api/nginx/ssl/cert.pem -text -noout

# Test SSL
openssl s_client -connect your-domain.com:443
```

## Quick Reference Commands

```bash
# Start services
docker compose -f docker-compose.prod.yml up -d

# Stop services
docker compose -f docker-compose.prod.yml down

# Restart services
docker compose -f docker-compose.prod.yml restart

# View logs
docker compose -f docker-compose.prod.yml logs -f

# Rebuild after code changes
docker compose -f docker-compose.prod.yml up -d --build

# Check status
docker compose -f docker-compose.prod.yml ps

# Execute command in container
docker compose -f docker-compose.prod.yml exec eric-api bash
```

## Cost Summary

- **CX23 Server (Cost-Optimized)**: €3.56/month
- **IPv4 Address**: €0.60/month (included in some regions, may vary)
- **Domain**: €1-2/month (optional)
- **SSL Certificate**: Free (Let's Encrypt)
- **Total**: ~€4-6/month (~€48-72/year)

**Savings**: 50% cheaper than regular performance servers (CPX22), perfect for monthly submission workload!

## Resource Specifications

**CX23 Server:**

- 2 vCPU (Intel®/AMD)
- 4 GB RAM
- 40 GB SSD
- 20 TB traffic
- Cost-Optimized (older hardware, variable CPU performance)

**Suitable for:**

- Up to 1000 users
- Monthly VAT submissions
- Low to medium CPU usage
- Variable workloads

## Next Steps

1. Set up monitoring (optional): Consider adding Prometheus/Grafana
2. Set up alerts: Configure email/Slack notifications for errors
3. Regular backups: Automate log backups
4. Documentation: Update API documentation with production URL

---

**Need Help?** Check logs first: `docker compose -f docker-compose.prod.yml logs -f`
