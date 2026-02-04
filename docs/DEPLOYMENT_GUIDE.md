# FranchiseAI Deployment Guide

## Building, Packaging, and Deploying the FranchiseAI Platform

---

## Table of Contents

1. [Environment Model](#1-environment-model)
2. [Prerequisites](#2-prerequisites)
3. [Building from Source](#3-building-from-source)
4. [Creating a Release Package](#4-creating-a-release-package)
5. [Database Setup](#5-database-setup)
6. [ApiLogicServer Setup](#6-apilogicserver-setup)
7. [Configuration](#7-configuration)
8. [Deployment Target: Native](#8-deployment-target-native)
9. [Deployment Target: Container (Docker)](#9-deployment-target-container-docker)
10. [Deployment Target: Cloud](#10-deployment-target-cloud)
11. [SSL / HTTPS](#11-ssl--https)
12. [Monitoring & Health Checks](#12-monitoring--health-checks)
13. [Backup & Recovery](#13-backup--recovery)
14. [Migrating to a New Repository](#14-migrating-to-a-new-repository)
15. [Troubleshooting](#15-troubleshooting)

---

## 1. Environment Model

FranchiseAI uses a four-environment promotion model. Source code is only present in DEV. All other environments receive binary-only release packages.

### Environment Overview

| Environment | Purpose | What Gets Deployed | Source Code? |
|-------------|---------|-------------------|--------------|
| **DEV** | Active development and testing | Full repository (source + build) | Yes |
| **TEST** | Integration testing, QA | Release package (binary + resources + config) | No |
| **UAT** | User acceptance testing, stakeholder demos | Release package (binary + resources + config) | No |
| **PROD** | Live production | Release package (binary + resources + config) | No |

### Promotion Flow

```
┌───────────┐     ┌───────────┐     ┌───────────┐     ┌───────────┐
│    DEV    │────▶│   TEST    │────▶│    UAT    │────▶│   PROD    │
│           │     │           │     │           │     │           │
│ Source +  │     │ Binary +  │     │ Binary +  │     │ Binary +  │
│ Build     │     │ Resources │     │ Resources │     │ Resources │
│ Compile & │     │ Config    │     │ Config    │     │ Config    │
│ Test      │     │           │     │           │     │           │
└───────────┘     └───────────┘     └───────────┘     └───────────┘
                        │                 │                 │
                   make package      same package      same package
                   (binary only)     (env config        (env config
                                      differs)           differs)
```

### Key Principle

**Source code never leaves DEV.** The build process produces a self-contained release package containing only:

- Compiled binary (`franchise_ai_search`)
- Runtime resources (`resources/` — CSS, JS, wt_config.xml)
- Configuration template (`config/app_config.sample.json`)
- Database schema (`database/schema.sql`)
- Shared libraries manifest (documents required runtime libs)

Source files (`.cpp`, `.h`), CMakeLists.txt, test code, build directories, and git history are excluded from release packages.

---

## 2. Prerequisites

### Build Host (DEV Only)

| Dependency | Version | Purpose |
|------------|---------|---------|
| **C++ Compiler** | GCC 8+ / Clang 7+ / MSVC 2019+ | C++17 support required |
| **CMake** | 3.16+ | Build system |
| **Wt (Witty)** | 4.x | Web framework with HTTP connector |
| **CURL** | Any | HTTP client for external APIs |
| **ncurses** | Any (optional) | Test runner UI |

### Runtime Host (All Environments)

| Dependency | Version | Purpose |
|------------|---------|---------|
| **Wt shared libraries** | 4.x | `libwt.so`, `libwthttp.so` |
| **CURL shared library** | Any | `libcurl.so` |
| **PostgreSQL** | 14+ | Primary database |
| **Python** | 3.10+ | ApiLogicServer |

### System Requirements

| Component | DEV / TEST | UAT / PROD |
|-----------|-----------|------------|
| **OS** | Ubuntu 22.04 LTS / macOS 12+ | Ubuntu 22.04 LTS |
| **CPU** | 2 cores | 4+ cores |
| **RAM** | 2 GB | 4+ GB |
| **Disk** | 5 GB (source + build) | 1 GB (binary + resources) |

### Installing Build Dependencies (DEV Only)

**Ubuntu/Debian:**

```bash
sudo apt-get update
sudo apt-get install build-essential cmake libcurl4-openssl-dev libncurses5-dev
sudo apt-get install witty witty-dev
sudo apt-get install postgresql postgresql-contrib libpq-dev
sudo apt-get install python3 python3-pip python3-venv
```

**macOS (Homebrew):**

```bash
brew install cmake curl ncurses wt postgresql@14 python@3
```

**Wt from Source (if packages are unavailable):**

```bash
git clone https://github.com/emweb/wt.git
cd wt && mkdir build && cd build
cmake ..
make -j$(nproc) && sudo make install
```

### Installing Runtime Dependencies (TEST / UAT / PROD)

```bash
# Runtime libraries only — no compiler, no dev headers
sudo apt-get update
sudo apt-get install libwt-dev libwthttp-dev libcurl4 libncurses6
sudo apt-get install postgresql-client
sudo apt-get install python3 python3-pip python3-venv
```

---

## 3. Building from Source

Building happens exclusively on the DEV environment.

### Standard Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Build with Tests

```bash
mkdir build && cd build
cmake .. -DBUILD_TESTS=ON
make -j$(nproc)
```

### Build Targets

| Target | Command | Description |
|--------|---------|-------------|
| `franchise_ai_search` | `make franchise_ai_search` | Main application binary |
| `test_als_client` | `make test_als_client` | ApiLogicServer client tests |
| `test_runner` | `make test_runner` | ncurses-based interactive test runner |
| `run` | `make run` | Build and launch the application |
| `run_tests` | `make run_tests` | Run test UI |

### Verify Build

```bash
ls -la franchise_ai_search
./franchise_ai_search --help
```

---

## 4. Creating a Release Package

The release package is what gets deployed to TEST, UAT, and PROD. It contains no source code.

### Package Script

Run from the repository root after building:

```bash
#!/bin/bash
# package-release.sh — Creates a binary-only release package
# Usage: ./package-release.sh [version]

VERSION=${1:-$(date +%Y%m%d-%H%M%S)}
PACKAGE_NAME="franchiseai-${VERSION}"
PACKAGE_DIR="dist/${PACKAGE_NAME}"

echo "=== Creating release package: ${PACKAGE_NAME} ==="

# Ensure build exists
if [ ! -f build/franchise_ai_search ]; then
    echo "ERROR: build/franchise_ai_search not found. Run 'make' first."
    exit 1
fi

# Clean and create package directory
rm -rf "${PACKAGE_DIR}"
mkdir -p "${PACKAGE_DIR}/bin"
mkdir -p "${PACKAGE_DIR}/resources"
mkdir -p "${PACKAGE_DIR}/config"
mkdir -p "${PACKAGE_DIR}/database"
mkdir -p "${PACKAGE_DIR}/logs"

# --- Binary ---
cp build/franchise_ai_search "${PACKAGE_DIR}/bin/"
chmod +x "${PACKAGE_DIR}/bin/franchise_ai_search"

# --- Runtime resources (CSS, JS, wt_config.xml) ---
cp -r build/resources/css "${PACKAGE_DIR}/resources/"
cp -r build/resources/scripts "${PACKAGE_DIR}/resources/"
cp build/resources/wt_config.xml "${PACKAGE_DIR}/resources/"

# --- Configuration template (NOT the real config with API keys) ---
cp config/app_config.sample.json "${PACKAGE_DIR}/config/app_config.sample.json"

# --- Database schema (for fresh installs and migrations) ---
cp database/schema.sql "${PACKAGE_DIR}/database/"

# --- Runtime dependencies manifest ---
cat > "${PACKAGE_DIR}/RUNTIME_DEPS.md" << 'DEPS'
# Runtime Dependencies

The following shared libraries must be present on the target host:

- libwt.so (Wt 4.x)
- libwthttp.so (Wt HTTP connector)
- libcurl.so (CURL)
- libncurses (optional, for test runner only)

Install on Ubuntu/Debian:
    sudo apt-get install libwt-dev libwthttp-dev libcurl4

Verify with:
    ldd bin/franchise_ai_search
DEPS

# --- Startup script ---
cat > "${PACKAGE_DIR}/bin/start.sh" << 'STARTUP'
#!/bin/bash
# FranchiseAI startup script
# Usage: ./start.sh [port]

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
APP_DIR="$(dirname "$SCRIPT_DIR")"
PORT=${1:-8080}

# Load environment config if present
if [ -f "${APP_DIR}/config/env" ]; then
    set -a
    source "${APP_DIR}/config/env"
    set +a
fi

exec "${SCRIPT_DIR}/franchise_ai_search" \
    --docroot "${APP_DIR}/resources" \
    --approot "${APP_DIR}/resources" \
    --http-address 0.0.0.0 \
    --http-port "${PORT}" \
    --accesslog "${APP_DIR}/logs/access.log"
STARTUP
chmod +x "${PACKAGE_DIR}/bin/start.sh"

# --- Environment config template ---
cat > "${PACKAGE_DIR}/config/env.sample" << 'ENV'
# FranchiseAI Environment Configuration
# Copy to 'env' and fill in values for your environment.
# This file is sourced by start.sh at startup.

# ApiLogicServer connection
API_LOGIC_SERVER_HOST=localhost
API_LOGIC_SERVER_PORT=5656
API_LOGIC_SERVER_PROTOCOL=http

# AI providers
OPENAI_API_KEY=
GEMINI_API_KEY=

# Data source APIs
GOOGLE_API_KEY=
BBB_API_KEY=
CENSUS_API_KEY=
ENV

# --- Create tarball ---
mkdir -p dist
tar -czf "dist/${PACKAGE_NAME}.tar.gz" -C dist "${PACKAGE_NAME}"

echo ""
echo "=== Release package created ==="
echo "  Package: dist/${PACKAGE_NAME}.tar.gz"
echo "  Contents:"
find "${PACKAGE_DIR}" -type f | sed "s|${PACKAGE_DIR}/|    |" | sort
echo ""
echo "  Deploy with:"
echo "    scp dist/${PACKAGE_NAME}.tar.gz user@host:/opt/"
echo "    ssh user@host 'cd /opt && tar xzf ${PACKAGE_NAME}.tar.gz'"
```

### Package Contents

```
franchiseai-20260204/
├── bin/
│   ├── franchise_ai_search          # Compiled binary
│   └── start.sh                     # Startup script
├── resources/
│   ├── css/style.css                # Application styles
│   ├── scripts/leaflet.js           # Leaflet map library
│   └── wt_config.xml               # Wt server configuration
├── config/
│   ├── app_config.sample.json       # Config template (copy to app_config.json)
│   └── env.sample                   # Environment variables template (copy to env)
├── database/
│   └── schema.sql                   # PostgreSQL schema + seed data
├── logs/                            # Log output directory
└── RUNTIME_DEPS.md                  # Required shared libraries
```

**What is NOT in the package:**

- No `.cpp` or `.h` source files
- No `CMakeLists.txt` or build configuration
- No `src/`, `tests/`, or `docs/` directories
- No `.git` history
- No `node_modules` or build artifacts
- No API keys or credentials

### Deploying a Release Package

```bash
# On the target host (TEST / UAT / PROD)
cd /opt
tar xzf franchiseai-20260204.tar.gz

# Create environment-specific config
cd franchiseai-20260204/config
cp app_config.sample.json app_config.json
cp env.sample env
# Edit app_config.json and env with environment-specific values

# Verify runtime dependencies
ldd /opt/franchiseai-20260204/bin/franchise_ai_search

# Start
/opt/franchiseai-20260204/bin/start.sh
```

---

## 5. Database Setup

### Create the Database

```bash
sudo -u postgres psql

CREATE DATABASE franchiseai;
CREATE USER franchiseai_user WITH ENCRYPTED PASSWORD 'your-secure-password';
GRANT ALL PRIVILEGES ON DATABASE franchiseai TO franchiseai_user;
\q
```

### Run the Schema

```bash
psql -U franchiseai_user -d franchiseai -f database/schema.sql
```

The schema creates all required tables with seed data:

| Table | Purpose |
|-------|---------|
| `franchisees` | Franchise owner information |
| `store_locations` | Physical store locations with geocoding |
| `scoring_rules` | Configurable prospect scoring rules |
| `users` | Application users with authentication |
| `user_sessions` | Session management |
| `app_config` | Key-value application configuration |
| `prospects` / `saved_prospects` | Saved prospect records |
| `audit_log` | Activity audit trail |

### Verify Schema

```bash
psql -U franchiseai_user -d franchiseai -c "\dt"
```

### Per-Environment Database Strategy

| Environment | Database | Notes |
|-------------|----------|-------|
| **DEV** | Local PostgreSQL | Full seed data, frequent schema changes |
| **TEST** | Dedicated instance | Reset between test cycles; schema matches DEV |
| **UAT** | Dedicated instance | Realistic data volume; client-accessible |
| **PROD** | Managed / HA instance | Automated backups; connection pooling recommended |

---

## 6. ApiLogicServer Setup

ApiLogicServer generates a complete REST API (JSON:API compliant) from the PostgreSQL schema.

### Install

```bash
python3 -m venv venv
source venv/bin/activate

pip install ApiLogicServer
```

### Generate API Project

```bash
ApiLogicServer create \
    --project_name=api \
    --db_url="postgresql://franchiseai_user:your-secure-password@localhost:5432/franchiseai"
```

### Configure

Edit `api/config/config.py`:

```python
APILOGICPROJECT_PORT = 5656
APILOGICPROJECT_HOST = "0.0.0.0"
SQLALCHEMY_DATABASE_URI = "postgresql://franchiseai_user:your-secure-password@localhost:5432/franchiseai"
CORS_ORIGINS = ["http://localhost:8080"]
```

### Start the API Server

```bash
cd api
python api_logic_server_run.py
```

Verify at:

| URL | Purpose |
|-----|---------|
| `http://localhost:5656/api` | API endpoints |
| `http://localhost:5656/admin-app/` | Admin UI |
| `http://localhost:5656/api/swagger` | Swagger docs |

---

## 7. Configuration

### Configuration Precedence

The application loads configuration in this order (first value wins):

1. **Environment variables** (highest precedence)
2. **`config/app_config.json`** (local file, git-ignored)
3. **Compiled defaults** in source code

### Per-Environment Configuration

| Setting | DEV | TEST | UAT | PROD |
|---------|-----|------|-----|------|
| `API_LOGIC_SERVER_HOST` | `localhost` | `test-db-host` | `uat-db-host` | `prod-db-host` |
| `API_LOGIC_SERVER_PORT` | `5656` | `5656` | `5656` | `5656` |
| `OPENAI_API_KEY` | Dev key | Test key | UAT key | Production key |
| `GOOGLE_API_KEY` | Dev key | Test key | UAT key | Production key |
| Session timeout | 30 min | 30 min | 30 min | 30 min |
| Idle timeout | 10 min | 10 min | 10 min | 10 min |

### Environment Variables (Recommended for TEST / UAT / PROD)

Create `config/env` from the `config/env.sample` template:

```bash
# ApiLogicServer connection
API_LOGIC_SERVER_HOST=localhost
API_LOGIC_SERVER_PORT=5656
API_LOGIC_SERVER_PROTOCOL=http

# AI providers
OPENAI_API_KEY=sk-your-key
GEMINI_API_KEY=your-gemini-key

# Data source APIs
GOOGLE_API_KEY=your-google-key
BBB_API_KEY=your-bbb-key
CENSUS_API_KEY=your-census-key
```

The `start.sh` script sources this file automatically at startup.

### API Key Requirements

| API | Required? | Free Tier | Notes |
|-----|-----------|-----------|-------|
| **OpenStreetMap** | No key needed | Unlimited (fair use) | Always available |
| **OpenAI** | Optional | No | Required for AI prospect analysis |
| **Google Gemini** | Optional | Yes | Fallback AI provider |
| **Google Places** | Optional | $200/month credit | Enhanced business data |
| **Google Geocoding** | Optional | $200/month credit | Better address resolution |
| **BBB** | Optional | No | Business accreditation data |
| **Census** | Optional | Yes | Demographics data |

The application works without any API keys using OpenStreetMap for location data and local scoring for prospect analysis.

### Wt Server Configuration

The `wt_config.xml` in `resources/` controls Wt framework behavior:

| Setting | Value | Description |
|---------|-------|-------------|
| `progressive-bootstrap` | `false` | Enables clean HTML5 history URLs |
| `session-id-cookie` | `true` | Session via cookie, not URL parameter |
| `session-timeout` | `1800` (30 min) | Maximum session lifetime |
| `idle-timeout` | `600` (10 min) | Disconnect after inactivity |

---

## 8. Deployment Target: Native

Direct deployment of the binary and resources onto a Linux host. This is the simplest deployment model.

### DEV (Source + Build)

```bash
# Clone, build, and run — full development workflow
git clone https://github.com/thomasgpeters/Geolocation-Franchise.git
cd Geolocation-Franchise
mkdir build && cd build
cmake ..
make -j$(nproc)
make run
```

Open `http://localhost:8080`.

### TEST / UAT / PROD (Binary Package Only)

```bash
# Deploy release package
scp dist/franchiseai-20260204.tar.gz deploy@target-host:/opt/
ssh deploy@target-host

cd /opt
tar xzf franchiseai-20260204.tar.gz
cd franchiseai-20260204

# Configure for this environment
cp config/app_config.sample.json config/app_config.json
cp config/env.sample config/env
# Edit both files with environment-specific values

# Start
./bin/start.sh
```

### Systemd Service (TEST / UAT / PROD)

Create `/etc/systemd/system/franchiseai.service`:

```ini
[Unit]
Description=FranchiseAI Prospect Discovery Platform
After=network.target postgresql.service
Wants=postgresql.service

[Service]
Type=simple
User=franchiseai
Group=franchiseai
WorkingDirectory=/opt/franchiseai
ExecStart=/opt/franchiseai/bin/start.sh
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

Create `/etc/systemd/system/franchiseai-api.service`:

```ini
[Unit]
Description=FranchiseAI ApiLogicServer
After=network.target postgresql.service
Wants=postgresql.service

[Service]
Type=simple
User=franchiseai
Group=franchiseai
WorkingDirectory=/opt/franchiseai/api
ExecStart=/opt/franchiseai/venv/bin/python api_logic_server_run.py
Restart=on-failure
RestartSec=5
Environment=APILOGICPROJECT_PORT=5656
Environment=APILOGICPROJECT_HOST=0.0.0.0

[Install]
WantedBy=multi-user.target
```

Enable and start:

```bash
sudo systemctl daemon-reload
sudo systemctl enable franchiseai franchiseai-api
sudo systemctl start franchiseai-api
sudo systemctl start franchiseai
```

### Nginx Reverse Proxy

For UAT and PROD, place Nginx in front of the application:

```nginx
upstream franchiseai {
    server 127.0.0.1:8080;
}

server {
    listen 80;
    server_name franchiseai.example.com;
    return 301 https://$host$request_uri;
}

server {
    listen 443 ssl http2;
    server_name franchiseai.example.com;

    ssl_certificate     /etc/letsencrypt/live/franchiseai.example.com/fullchain.pem;
    ssl_private_key     /etc/letsencrypt/live/franchiseai.example.com/privkey.pem;

    # WebSocket support (required by Wt)
    location / {
        proxy_pass http://franchiseai;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_read_timeout 86400;
    }

    # Static resources (serve directly for performance)
    location /resources/ {
        alias /opt/franchiseai/resources/;
        expires 7d;
        add_header Cache-Control "public, immutable";
    }
}
```

### Firewall Rules

```bash
# Allow HTTP/HTTPS
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp

# Block direct access to application and API ports from outside
sudo ufw deny 8080/tcp
sudo ufw deny 5656/tcp
```

---

## 9. Deployment Target: Container (Docker)

Containerized deployment packages the binary, resources, and runtime dependencies into Docker images. No source code is included in the images.

### Dockerfile (Application)

Place this in the repository root. It uses a multi-stage build: the first stage compiles from source, the second stage copies only the binary and runtime files.

```dockerfile
# ============================================================
# Stage 1: Build (DEV only — discarded after compilation)
# ============================================================
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential cmake libcurl4-openssl-dev \
    witty witty-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY CMakeLists.txt .
COPY src/ src/
COPY tests/ tests/
COPY resources/ resources/
COPY wt_config.xml .

RUN mkdir build && cd build && cmake .. && make -j$(nproc)

# ============================================================
# Stage 2: Runtime (this is what gets deployed — no source code)
# ============================================================
FROM ubuntu:22.04 AS runtime

RUN apt-get update && apt-get install -y \
    libwt-dev libwthttp-dev libcurl4 \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -r -s /bin/false franchiseai

WORKDIR /opt/franchiseai

# Copy ONLY the binary and runtime resources — no source code
COPY --from=builder /build/build/franchise_ai_search bin/
COPY --from=builder /build/build/resources/ resources/
COPY config/app_config.sample.json config/app_config.sample.json
COPY database/schema.sql database/

RUN mkdir -p logs && chown -R franchiseai:franchiseai /opt/franchiseai

USER franchiseai

EXPOSE 8080

CMD ["./bin/franchise_ai_search", \
     "--docroot", "./resources", \
     "--approot", "./resources", \
     "--http-address", "0.0.0.0", \
     "--http-port", "8080"]
```

### Dockerfile (ApiLogicServer)

```dockerfile
FROM python:3.11-slim

RUN pip install ApiLogicServer

WORKDIR /opt/api

# ALS project is generated at build time or mounted at runtime
COPY api/ .

EXPOSE 5656

CMD ["python", "api_logic_server_run.py"]
```

### docker-compose.yml

```yaml
version: '3.8'

services:
  postgres:
    image: postgres:14
    environment:
      POSTGRES_DB: franchiseai
      POSTGRES_USER: franchiseai_user
      POSTGRES_PASSWORD: ${DB_PASSWORD:-changeme}
    volumes:
      - pgdata:/var/lib/postgresql/data
      - ./database/schema.sql:/docker-entrypoint-initdb.d/schema.sql
    ports:
      - "5432:5432"
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U franchiseai_user -d franchiseai"]
      interval: 10s
      timeout: 5s
      retries: 5

  api:
    build:
      context: .
      dockerfile: Dockerfile.api
    environment:
      APILOGICPROJECT_PORT: 5656
      APILOGICPROJECT_HOST: 0.0.0.0
      SQLALCHEMY_DATABASE_URI: "postgresql://franchiseai_user:${DB_PASSWORD:-changeme}@postgres:5432/franchiseai"
    ports:
      - "5656:5656"
    depends_on:
      postgres:
        condition: service_healthy

  app:
    build:
      context: .
      dockerfile: Dockerfile
    environment:
      API_LOGIC_SERVER_HOST: api
      API_LOGIC_SERVER_PORT: 5656
      API_LOGIC_SERVER_PROTOCOL: http
      OPENAI_API_KEY: ${OPENAI_API_KEY:-}
      GOOGLE_API_KEY: ${GOOGLE_API_KEY:-}
      BBB_API_KEY: ${BBB_API_KEY:-}
      CENSUS_API_KEY: ${CENSUS_API_KEY:-}
    ports:
      - "8080:8080"
    depends_on:
      - api

volumes:
  pgdata:
```

### Build and Run

```bash
# Build images (source code is used in Stage 1 only, discarded in final image)
docker compose build

# Start all services
docker compose up -d

# Verify
docker compose ps
curl http://localhost:8080
```

### Verify No Source Code in Image

```bash
# Inspect the final image — no .cpp, .h, or CMake files should exist
docker run --rm franchiseai-app find /opt/franchiseai -name "*.cpp" -o -name "*.h" -o -name "CMakeLists.txt"
# Should return no results
```

---

## 10. Deployment Target: Cloud

### AWS (EC2 + RDS)

**Architecture:**

```
┌──────────────────────────────┐
│        AWS VPC               │
│                              │
│  ┌────────────────────────┐  │
│  │   EC2 Instance         │  │
│  │   ┌─────────────────┐  │  │
│  │   │ FranchiseAI     │  │  │
│  │   │ (binary only)   │  │  │
│  │   └─────────────────┘  │  │
│  │   ┌─────────────────┐  │  │
│  │   │ ApiLogicServer  │  │  │
│  │   └─────────────────┘  │  │
│  │   ┌─────────────────┐  │  │
│  │   │ Nginx           │  │  │
│  │   └─────────────────┘  │  │
│  └────────────────────────┘  │
│              │               │
│  ┌───────────▼────────────┐  │
│  │   RDS PostgreSQL 14    │  │
│  │   (Multi-AZ optional)  │  │
│  └────────────────────────┘  │
│                              │
│  ┌────────────────────────┐  │
│  │   ALB (HTTPS)          │  │
│  └────────────────────────┘  │
└──────────────────────────────┘
```

**Deployment steps:**

```bash
# 1. Launch EC2 (Ubuntu 22.04, t3.medium or larger)
# 2. Install runtime dependencies (no compiler needed)
sudo apt-get install libwt-dev libwthttp-dev libcurl4 nginx python3 python3-venv

# 3. Upload release package (binary only, no source)
scp dist/franchiseai-20260204.tar.gz ec2-user@<ec2-ip>:/opt/
ssh ec2-user@<ec2-ip> 'cd /opt && tar xzf franchiseai-20260204.tar.gz'

# 4. Configure for RDS endpoint
cat > /opt/franchiseai/config/env << EOF
API_LOGIC_SERVER_HOST=localhost
API_LOGIC_SERVER_PORT=5656
API_LOGIC_SERVER_PROTOCOL=http
OPENAI_API_KEY=sk-prod-key
GOOGLE_API_KEY=prod-google-key
EOF

# 5. Set up ApiLogicServer pointing to RDS
# 6. Configure Nginx + ALB + ACM certificate
# 7. Start services via systemd
```

### AWS (ECS / Fargate)

Use the Docker images from Section 9. Push to ECR, deploy via ECS task definitions.

```bash
# Push to ECR
aws ecr get-login-password | docker login --username AWS --password-stdin <account>.dkr.ecr.<region>.amazonaws.com
docker tag franchiseai-app:latest <account>.dkr.ecr.<region>.amazonaws.com/franchiseai-app:latest
docker push <account>.dkr.ecr.<region>.amazonaws.com/franchiseai-app:latest
```

### Azure (App Service or VM)

Same binary package approach as AWS EC2, or use Docker images with Azure Container Instances / App Service.

```bash
# Azure VM: upload release package
scp dist/franchiseai-20260204.tar.gz azureuser@<vm-ip>:/opt/

# Azure Container Instances: use Docker image
az container create \
    --resource-group franchiseai-rg \
    --name franchiseai \
    --image <acr>.azurecr.io/franchiseai-app:latest \
    --ports 8080 \
    --environment-variables API_LOGIC_SERVER_HOST=api OPENAI_API_KEY=sk-key
```

### GCP (Compute Engine or Cloud Run)

```bash
# Compute Engine: same as EC2 — upload release package
# Cloud Run: use Docker image
gcloud run deploy franchiseai \
    --image gcr.io/<project>/franchiseai-app:latest \
    --port 8080 \
    --set-env-vars API_LOGIC_SERVER_HOST=api,OPENAI_API_KEY=sk-key
```

### Cloud Deployment Summary

| Cloud Provider | Compute | Database | Load Balancer | SSL |
|---------------|---------|----------|---------------|-----|
| **AWS** | EC2 / ECS Fargate | RDS PostgreSQL | ALB | ACM |
| **Azure** | VM / Container Instances / App Service | Azure Database for PostgreSQL | Application Gateway | App Service Managed |
| **GCP** | Compute Engine / Cloud Run | Cloud SQL PostgreSQL | Cloud Load Balancing | Google-managed |

In all cases, the deployed artifact is either:
- A **release package** (binary + resources, no source) for VM-based deployment
- A **Docker image** (multi-stage build, final image has no source) for container-based deployment

---

## 11. SSL / HTTPS

### Option A: Let's Encrypt with Certbot (Free)

```bash
sudo apt-get install certbot python3-certbot-nginx
sudo certbot --nginx -d franchiseai.example.com
sudo certbot renew --dry-run
```

### Option B: Wt Built-in SSL

```bash
./bin/franchise_ai_search \
    --docroot ./resources \
    --approot ./resources \
    --https-address 0.0.0.0 \
    --https-port 443 \
    --ssl-certificate /path/to/cert.pem \
    --ssl-private-key /path/to/key.pem
```

### Option C: Cloud-Managed (AWS ACM / Azure / GCP)

Use cloud-native certificate management with the load balancer. SSL termination happens at the ALB/Application Gateway/Cloud LB, and traffic to the application is HTTP internally.

### Option D: Self-Signed (DEV / TEST Only)

```bash
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes \
    -subj "/CN=localhost"
```

---

## 12. Monitoring & Health Checks

### Process Monitoring

```bash
# Systemd
sudo systemctl status franchiseai
sudo systemctl status franchiseai-api

# Logs
sudo journalctl -u franchiseai -f
sudo journalctl -u franchiseai-api -f

# Docker
docker compose ps
docker compose logs -f app
```

### PostgreSQL

```bash
pg_isready -h localhost -p 5432
psql -U franchiseai_user -d franchiseai -c "SELECT count(*) FROM pg_stat_activity WHERE datname = 'franchiseai';"
psql -U franchiseai_user -d franchiseai -c "SELECT pg_size_pretty(pg_database_size('franchiseai'));"
```

### ApiLogicServer

```bash
curl -s http://localhost:5656/api/AppConfig | head -c 200
curl -s -o /dev/null -w "%{http_code}" http://localhost:5656/api/Franchisee
```

### Application

```bash
curl -s -o /dev/null -w "%{http_code}" http://localhost:8080
ss -tlnp | grep 8080
```

---

## 13. Backup & Recovery

### Database Backup

```bash
# Full backup
pg_dump -U franchiseai_user -d franchiseai -F c -f backup_$(date +%Y%m%d_%H%M%S).dump

# Schema only
pg_dump -U franchiseai_user -d franchiseai --schema-only -f schema_backup.sql

# Data only
pg_dump -U franchiseai_user -d franchiseai --data-only -f data_backup.sql
```

### Automated Daily Backup

```cron
# Daily backup at 2 AM, keep 30 days
0 2 * * * pg_dump -U franchiseai_user -d franchiseai -F c -f /opt/backups/franchiseai_$(date +\%Y\%m\%d).dump && find /opt/backups -name "franchiseai_*.dump" -mtime +30 -delete
```

### Restore from Backup

```bash
sudo -u postgres psql -c "DROP DATABASE franchiseai;"
sudo -u postgres psql -c "CREATE DATABASE franchiseai OWNER franchiseai_user;"
pg_restore -U franchiseai_user -d franchiseai backup_20260204.dump
```

### Configuration Backup

```bash
tar czf config_backup.tar.gz \
    config/app_config.json \
    config/env \
    resources/wt_config.xml
```

---

## 14. Migrating to a New Repository

When starting a new project from this codebase (e.g., creating `Geolocation-Franchise` from `Geolocation-Sample`).

### Option A: Keep Full Git History (Recommended)

```bash
git clone https://github.com/thomasgpeters/Geolocation-Sample.git Geolocation-Franchise
cd Geolocation-Franchise
git remote remove origin
gh repo create thomasgpeters/Geolocation-Franchise --private --source=. --push
```

### Option B: Fresh Start (No History)

```bash
cp -r Geolocation-Sample Geolocation-Franchise
cd Geolocation-Franchise
rm -rf .git
git init
git add .
git commit -m "Initial commit: FranchiseAI platform v2.0"
gh repo create thomasgpeters/Geolocation-Franchise --private --source=. --push
```

### Post-Migration Checklist

- [ ] Update `README.md` with new repository name and URLs
- [ ] Verify `config/app_config.json` is git-ignored (contains API keys)
- [ ] Update any hardcoded references to the old repository name
- [ ] Set up branch protection rules on the new repository
- [ ] Rotate API keys if the old repository was public
- [ ] Archive the old repository (GitHub Settings > Danger Zone > Archive)

---

## 15. Troubleshooting

### Build Issues (DEV Only)

**Wt not found by CMake:**

```bash
cmake .. -DWT_INCLUDE_DIR=/usr/local/include -DWT_LIBRARY=/usr/local/lib/libwt.so
```

**C++17 not supported:**

```bash
g++ --version      # Need GCC 8+
clang++ --version  # Need Clang 7+
cmake .. -DCMAKE_CXX_COMPILER=g++-11
```

### Runtime Issues (All Environments)

**Missing shared library:**

```bash
# Check which libraries are missing
ldd bin/franchise_ai_search | grep "not found"

# Install missing runtime libraries
sudo apt-get install libwt-dev libwthttp-dev libcurl4
```

**"Connection refused" to ApiLogicServer:**

```bash
curl http://localhost:5656/api
ss -tlnp | grep 5656
```

**"Connection refused" to PostgreSQL:**

```bash
pg_isready -h localhost -p 5432
psql -U postgres -c "\l" | grep franchiseai
```

**wt_config.xml not loading (URLs have `?wtd=` parameters):**

Verify `--approot` points to the directory containing `wt_config.xml`. Check startup log for `config: reading Wt config file:`.

**Blank page or "Internal Server Error":**

```bash
# Check logs
sudo journalctl -u franchiseai --since "5 minutes ago"

# Or run in foreground
./bin/franchise_ai_search --docroot ./resources --approot ./resources --http-port 8080
```

### Performance Issues

**Slow search results (> 5 seconds):**

- Verify network connectivity to OpenStreetMap Overpass API
- Ensure caching is enabled in OSM config
- Check API timeout settings

**High memory usage:**

- Reduce `session-timeout` and `idle-timeout` in `wt_config.xml`
- Check PostgreSQL `shared_buffers` and `work_mem` settings

---

*End of Deployment Guide*
