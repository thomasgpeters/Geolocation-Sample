# FranchiseAI Deployment Guide

## Building, Running, and Deploying the FranchiseAI Platform

---

## Table of Contents

1. [Repository Setup](#1-repository-setup)
2. [Prerequisites](#2-prerequisites)
3. [Building from Source](#3-building-from-source)
4. [Database Setup](#4-database-setup)
5. [ApiLogicServer Setup](#5-apilogicserver-setup)
6. [Configuration](#6-configuration)
7. [Running the Application](#7-running-the-application)
8. [Production Deployment](#8-production-deployment)
9. [Migrating to a New Repository](#9-migrating-to-a-new-repository)
10. [SSL / HTTPS](#10-ssl--https)
11. [Monitoring & Health Checks](#11-monitoring--health-checks)
12. [Backup & Recovery](#12-backup--recovery)
13. [Troubleshooting](#13-troubleshooting)

---

## 1. Repository Setup

### Cloning

```bash
git clone https://github.com/thomasgpeters/Geolocation-Franchise.git
cd Geolocation-Franchise
```

### Directory Structure (Deployment-Relevant)

```
├── CMakeLists.txt                 # Build configuration
├── wt_config.xml                  # Wt server configuration
├── config/
│   ├── app_config.json            # Local config (git-ignored)
│   └── app_config.sample.json     # Config template
├── database/
│   ├── schema.sql                 # PostgreSQL schema + seed data
│   └── README.md                  # Database setup details
├── resources/
│   ├── wt_config.xml              # Copied to build dir by CMake
│   ├── css/style.css              # Application styles
│   └── scripts/leaflet.js         # Leaflet map library
├── src/                           # Application source code
└── tests/                         # Test framework
```

---

## 2. Prerequisites

### System Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| **OS** | Ubuntu 20.04 LTS / macOS 12+ | Ubuntu 22.04 LTS |
| **CPU** | 2 cores | 4+ cores |
| **RAM** | 2 GB | 4+ GB |
| **Disk** | 1 GB | 5+ GB |

### Build Dependencies

| Dependency | Version | Purpose |
|------------|---------|---------|
| **C++ Compiler** | GCC 8+ / Clang 7+ / MSVC 2019+ | C++17 support required |
| **CMake** | 3.16+ | Build system |
| **Wt (Witty)** | 4.x | Web framework with HTTP connector |
| **CURL** | Any | HTTP client for external APIs |
| **ncurses** | Any (optional) | Test runner UI |

### Runtime Dependencies

| Dependency | Version | Purpose |
|------------|---------|---------|
| **PostgreSQL** | 14+ | Primary database |
| **Python** | 3.10+ | ApiLogicServer |
| **pip** | Latest | Python package manager |

### Installing Build Dependencies

**Ubuntu/Debian:**

```bash
# Build tools
sudo apt-get update
sudo apt-get install build-essential cmake libcurl4-openssl-dev libncurses5-dev

# Wt framework
sudo apt-get install witty witty-dev

# PostgreSQL
sudo apt-get install postgresql postgresql-contrib libpq-dev

# Python (for ApiLogicServer)
sudo apt-get install python3 python3-pip python3-venv
```

**macOS (Homebrew):**

```bash
brew install cmake curl ncurses wt postgresql@14 python@3
```

**Wt from Source (if packages are unavailable or outdated):**

```bash
git clone https://github.com/emweb/wt.git
cd wt
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

---

## 3. Building from Source

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
| `franchise_ai_search` | `make franchise_ai_search` | Main application |
| `test_als_client` | `make test_als_client` | ApiLogicServer client tests |
| `test_runner` | `make test_runner` | ncurses-based test runner |
| `run` | `make run` | Build and launch the application |
| `run_tests` | `make run_tests` | Run test UI |

### Verify Build

```bash
# From build directory
ls -la franchise_ai_search
./franchise_ai_search --help
```

---

## 4. Database Setup

### Create the Database

```bash
# Connect to PostgreSQL
sudo -u postgres psql

# Create database and user
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

---

## 5. ApiLogicServer Setup

ApiLogicServer generates a complete REST API (JSON:API compliant) from the PostgreSQL schema.

### Install

```bash
python3 -m venv venv
source venv/bin/activate      # macOS/Linux
# venv\Scripts\activate       # Windows

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

## 6. Configuration

### Configuration Precedence

The application loads configuration in this order (first value wins):

1. **Environment variables** (highest precedence)
2. **`config/app_config.json`** (local file, git-ignored)
3. **Compiled defaults** in source code

### Create Local Config

```bash
cp config/app_config.sample.json config/app_config.json
```

Edit `config/app_config.json`:

```json
{
  "api_logic_server": {
    "host": "localhost",
    "port": 5656,
    "protocol": "http",
    "api_prefix": "/api",
    "timeout_ms": 30000
  },
  "api_keys": {
    "openai_api_key": "sk-your-openai-key",
    "openai_model": "gpt-4o",
    "openai_max_tokens": 2000,
    "gemini_api_key": "your-gemini-key",
    "google_api_key": "your-google-key",
    "bbb_api_key": "your-bbb-key",
    "census_api_key": "your-census-key"
  },
  "geocoding": {
    "provider": "nominatim",
    "nominatim_endpoint": "https://nominatim.openstreetmap.org",
    "user_agent": "FranchiseAI/1.0"
  }
}
```

### Environment Variables (Recommended for Production)

```bash
# ApiLogicServer connection
export API_LOGIC_SERVER_HOST="localhost"
export API_LOGIC_SERVER_PORT="5656"
export API_LOGIC_SERVER_PROTOCOL="http"

# AI providers
export OPENAI_API_KEY="sk-your-openai-key"
export GEMINI_API_KEY="your-gemini-key"

# Data source APIs
export GOOGLE_API_KEY="your-google-key"
export BBB_API_KEY="your-bbb-key"
export CENSUS_API_KEY="your-census-key"
```

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

The `wt_config.xml` controls Wt framework behavior:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<server>
    <application-settings location="*">
        <progressive-bootstrap>false</progressive-bootstrap>
        <session-id-cookie>true</session-id-cookie>
        <session-timeout>1800</session-timeout>
        <idle-timeout>600</idle-timeout>
        <properties>
            <property name="leafletJSURL">scripts/leaflet.js</property>
            <property name="leafletCSSURL">css/leaflet.css</property>
        </properties>
    </application-settings>
</server>
```

| Setting | Value | Description |
|---------|-------|-------------|
| `progressive-bootstrap` | `false` | Enables clean HTML5 history URLs |
| `session-id-cookie` | `true` | Session via cookie, not URL parameter |
| `session-timeout` | `1800` (30 min) | Maximum session lifetime |
| `idle-timeout` | `600` (10 min) | Disconnect after inactivity |

---

## 7. Running the Application

### Development

```bash
# Using make target (recommended)
cd build
make run

# Manual execution
./franchise_ai_search \
    --docroot ./resources \
    --approot ./resources \
    --http-address 0.0.0.0 \
    --http-port 8080
```

Open `http://localhost:8080` in your browser.

### Command Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `--docroot <path>` | Document root for static resources | `./resources` |
| `--approot <path>` | Application root (wt_config.xml location) | `./resources` |
| `--http-address <addr>` | HTTP server bind address | `0.0.0.0` |
| `--http-port <port>` | HTTP server port | `8080` |
| `--https-address <addr>` | HTTPS server bind address | — |
| `--https-port <port>` | HTTPS server port | — |
| `--ssl-certificate <file>` | SSL certificate file | — |
| `--ssl-private-key <file>` | SSL private key file | — |
| `--accesslog <file>` | Access log file path | — |

### Startup Sequence

When the application starts, it performs:

```
1. Load app_config.json (local API keys)
2. Initialize ApiLogicServer client
3. Load AppConfig entries from ALS into memory cache
4. Load current Franchisee from ALS
5. Load current StoreLocation from ALS
6. Initialize UI and routing
```

### Verify Configuration Loading

Check the server startup log:

```
# Correct — loading from approot:
config: reading Wt config file: /path/to/build/resources/wt_config.xml

# Incorrect — falling back to system config:
config: reading Wt config file: /etc/wt/wt_config.xml
```

If you see the system path, verify that `--approot` points to your `resources/` directory.

---

## 8. Production Deployment

### Startup Order

Services must start in this order:

```
1. PostgreSQL         (database)
2. ApiLogicServer     (REST API layer)
3. FranchiseAI        (web application)
```

### Systemd Service (FranchiseAI)

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
WorkingDirectory=/opt/franchiseai/build
ExecStart=/opt/franchiseai/build/franchise_ai_search \
    --docroot /opt/franchiseai/build/resources \
    --approot /opt/franchiseai/build/resources \
    --http-address 0.0.0.0 \
    --http-port 8080
Restart=on-failure
RestartSec=5
EnvironmentFile=/etc/franchiseai/env

[Install]
WantedBy=multi-user.target
```

Create `/etc/franchiseai/env`:

```bash
OPENAI_API_KEY=sk-your-production-key
GOOGLE_API_KEY=your-production-key
BBB_API_KEY=your-production-key
CENSUS_API_KEY=your-production-key
API_LOGIC_SERVER_HOST=localhost
API_LOGIC_SERVER_PORT=5656
API_LOGIC_SERVER_PROTOCOL=http
```

Enable and start:

```bash
sudo systemctl daemon-reload
sudo systemctl enable franchiseai
sudo systemctl start franchiseai
sudo systemctl status franchiseai
```

### Systemd Service (ApiLogicServer)

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

### Nginx Reverse Proxy

For production, place Nginx in front of the Wt application:

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
        alias /opt/franchiseai/build/resources/;
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

## 9. Migrating to a New Repository

When starting a new project from this codebase (e.g., creating `Geolocation-Franchise` from `Geolocation-Sample`), there are two approaches.

### Option A: Keep Full Git History (Recommended)

Creates an independent repository with the complete commit history intact. No fork relationship.

```bash
# 1. Clone into a new directory
git clone https://github.com/thomasgpeters/Geolocation-Sample.git Geolocation-Franchise
cd Geolocation-Franchise

# 2. Remove the old remote
git remote remove origin

# 3. Create the new repo on GitHub and push
gh repo create thomasgpeters/Geolocation-Franchise --private --source=. --push
```

### Option B: Fresh Start (No History)

Creates a clean repository with only the current code as the initial commit.

```bash
# 1. Copy files without git history
cp -r Geolocation-Sample Geolocation-Franchise
cd Geolocation-Franchise
rm -rf .git

# 2. Initialize fresh repository
git init
git add .
git commit -m "Initial commit: FranchiseAI platform v2.0"

# 3. Create remote and push
gh repo create thomasgpeters/Geolocation-Franchise --private --source=. --push
```

### Post-Migration Checklist

After creating the new repository:

- [ ] Update `README.md` with new repository name and URLs
- [ ] Verify `config/app_config.json` is git-ignored (contains API keys)
- [ ] Update any hardcoded references to the old repository name
- [ ] Set up branch protection rules on the new repository
- [ ] Rotate API keys if the old repository was public
- [ ] Archive the old repository (GitHub Settings > Danger Zone > Archive)

---

## 10. SSL / HTTPS

### Option A: Let's Encrypt with Certbot (Free)

```bash
# Install certbot
sudo apt-get install certbot python3-certbot-nginx

# Obtain certificate
sudo certbot --nginx -d franchiseai.example.com

# Auto-renewal is configured automatically
sudo certbot renew --dry-run
```

### Option B: Wt Built-in SSL

Pass SSL parameters directly to the application:

```bash
./franchise_ai_search \
    --docroot ./resources \
    --approot ./resources \
    --https-address 0.0.0.0 \
    --https-port 443 \
    --ssl-certificate /path/to/cert.pem \
    --ssl-private-key /path/to/key.pem
```

### Option C: Self-Signed (Development Only)

```bash
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes \
    -subj "/CN=localhost"
```

---

## 11. Monitoring & Health Checks

### Process Monitoring

```bash
# Check service status
sudo systemctl status franchiseai
sudo systemctl status franchiseai-api

# View logs
sudo journalctl -u franchiseai -f
sudo journalctl -u franchiseai-api -f
```

### PostgreSQL Monitoring

```bash
# Check connection
pg_isready -h localhost -p 5432

# Active connections
psql -U franchiseai_user -d franchiseai -c "SELECT count(*) FROM pg_stat_activity WHERE datname = 'franchiseai';"

# Database size
psql -U franchiseai_user -d franchiseai -c "SELECT pg_size_pretty(pg_database_size('franchiseai'));"
```

### ApiLogicServer Health

```bash
# Check API is responding
curl -s http://localhost:5656/api/AppConfig | head -c 200

# Check specific endpoint
curl -s -o /dev/null -w "%{http_code}" http://localhost:5656/api/Franchisee
```

### Application Health

```bash
# Check web server is responding
curl -s -o /dev/null -w "%{http_code}" http://localhost:8080

# Check port is listening
ss -tlnp | grep 8080
```

---

## 12. Backup & Recovery

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

Add to crontab (`crontab -e`):

```cron
# Daily backup at 2 AM, keep 30 days
0 2 * * * pg_dump -U franchiseai_user -d franchiseai -F c -f /opt/backups/franchiseai_$(date +\%Y\%m\%d).dump && find /opt/backups -name "franchiseai_*.dump" -mtime +30 -delete
```

### Restore from Backup

```bash
# Drop and recreate database
sudo -u postgres psql -c "DROP DATABASE franchiseai;"
sudo -u postgres psql -c "CREATE DATABASE franchiseai OWNER franchiseai_user;"

# Restore from dump
pg_restore -U franchiseai_user -d franchiseai backup_20260204.dump

# Or from SQL
psql -U franchiseai_user -d franchiseai -f schema_backup.sql
psql -U franchiseai_user -d franchiseai -f data_backup.sql
```

### Configuration Backup

```bash
# Back up configuration files (exclude API keys from version control)
tar czf config_backup.tar.gz \
    config/app_config.json \
    /etc/franchiseai/env \
    resources/wt_config.xml
```

---

## 13. Troubleshooting

### Build Issues

**Wt not found by CMake:**

```bash
# Specify Wt location
cmake .. -DWT_INCLUDE_DIR=/usr/local/include -DWT_LIBRARY=/usr/local/lib/libwt.so
```

**C++17 not supported:**

```bash
# Check compiler version
g++ --version    # Need GCC 8+
clang++ --version  # Need Clang 7+

# Specify compiler
cmake .. -DCMAKE_CXX_COMPILER=g++-11
```

### Runtime Issues

**"Connection refused" to ApiLogicServer:**

```bash
# Verify ALS is running
curl http://localhost:5656/api

# Check port
ss -tlnp | grep 5656

# Check config
grep -r "api_logic_server" config/app_config.json
```

**"Connection refused" to PostgreSQL:**

```bash
# Check PostgreSQL is running
pg_isready -h localhost -p 5432

# Check database exists
psql -U postgres -c "\l" | grep franchiseai

# Check permissions
psql -U franchiseai_user -d franchiseai -c "SELECT 1;"
```

**wt_config.xml not loading (URLs have `?wtd=` parameters):**

```bash
# Verify approot path
ls -la resources/wt_config.xml

# Check startup log for config path
./franchise_ai_search --docroot ./resources --approot ./resources --http-port 8080 2>&1 | grep "config:"
```

**Session expires immediately:**

Check `wt_config.xml` session and idle timeout values. Default is 30 minutes session / 10 minutes idle.

**Blank page or "Internal Server Error":**

```bash
# Check application logs
sudo journalctl -u franchiseai --since "5 minutes ago"

# Run in foreground for detailed output
./franchise_ai_search --docroot ./resources --approot ./resources --http-port 8080
```

### Performance Issues

**Slow search results (> 5 seconds):**

- Verify network connectivity to OpenStreetMap Overpass API
- Check API timeout settings in source (`connectTimeoutMs`, `requestTimeoutMs`)
- Ensure caching is enabled in `OSMAPIConfig`

**High memory usage:**

- Check PostgreSQL `shared_buffers` and `work_mem` settings
- Monitor active Wt sessions: high idle timeouts keep sessions in memory longer
- Reduce `session-timeout` and `idle-timeout` in `wt_config.xml`

---

*End of Deployment Guide*
