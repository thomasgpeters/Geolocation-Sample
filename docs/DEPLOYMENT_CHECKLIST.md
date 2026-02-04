# FranchiseAI Deployment Checklist

> Companion to [DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md) — use the guide for detailed instructions, this document for step-by-step verification.

---

## Table of Contents

1. [Common Prerequisites (All Deployment Types)](#1-common-prerequisites-all-deployment-types)
2. [Database Setup](#2-database-setup)
3. [ApiLogicServer Setup](#3-apilogicserver-setup)
4. [Deployment Type A: Native (Binary on Host)](#4-deployment-type-a-native-binary-on-host)
5. [Deployment Type B: Container (Docker)](#5-deployment-type-b-container-docker)
6. [Deployment Type C: Cloud](#6-deployment-type-c-cloud)
7. [Post-Deployment Verification](#7-post-deployment-verification)
8. [SSL / HTTPS](#8-ssl--https)
9. [Monitoring Setup](#9-monitoring-setup)
10. [Backup Configuration](#10-backup-configuration)
11. [Repository Migration](#11-repository-migration)

---

## 1. Common Prerequisites (All Deployment Types)

### DEV Environment (Build Host)

- [ ] C++ compiler installed (GCC 8+ / Clang 7+)
- [ ] CMake 3.16+ installed
- [ ] Wt 4.x development libraries installed (`witty witty-dev`)
- [ ] CURL development library installed (`libcurl4-openssl-dev`)
- [ ] ncurses installed (optional, for test runner)
- [ ] PostgreSQL 14+ installed and running
- [ ] Python 3.10+ installed with venv support
- [ ] Repository cloned and accessible

### Build and Package

- [ ] Build completes successfully: `mkdir build && cd build && cmake .. && make -j$(nproc)`
- [ ] Binary exists: `ls build/franchise_ai_search`
- [ ] Tests pass: `make test`
- [ ] Release package created: `./package-release.sh <version>`
- [ ] Package tarball exists: `ls dist/franchiseai-<version>.tar.gz`
- [ ] Verify package contains NO source files: `tar tzf dist/franchiseai-<version>.tar.gz | grep -E '\.(cpp|h)$'` (should return nothing)

### Runtime Host (TEST / UAT / PROD)

- [ ] Wt runtime libraries installed (`libwt-dev libwthttp-dev`)
- [ ] CURL runtime library installed (`libcurl4`)
- [ ] PostgreSQL client installed (`postgresql-client`)
- [ ] Python 3.10+ installed with venv support
- [ ] Non-root service user created (`franchiseai`)

---

## 2. Database Setup

- [ ] PostgreSQL 14+ installed and running
- [ ] Database created: `CREATE DATABASE franchiseai;`
- [ ] Database user created: `CREATE USER franchiseai_user WITH ENCRYPTED PASSWORD '...';`
- [ ] Privileges granted: `GRANT ALL PRIVILEGES ON DATABASE franchiseai TO franchiseai_user;`
- [ ] Schema loaded: `psql -U franchiseai_user -d franchiseai -f database/schema.sql`
- [ ] Tables verified: `psql -U franchiseai_user -d franchiseai -c "\dt"` (should list all tables)
- [ ] Seed data present (franchisees, scoring_rules, app_config)
- [ ] Connection tested from application host: `pg_isready -h <db-host> -p 5432`

---

## 3. ApiLogicServer Setup

- [ ] Python virtual environment created: `python3 -m venv venv`
- [ ] ApiLogicServer installed: `pip install ApiLogicServer`
- [ ] API project generated: `ApiLogicServer create --project_name=api --db_url="postgresql://..."`
- [ ] Config updated (`api/config/config.py`): port, host, database URI, CORS origins
- [ ] API server starts: `cd api && python api_logic_server_run.py`
- [ ] API responds: `curl http://localhost:5656/api` returns 200
- [ ] Admin UI accessible: `http://localhost:5656/admin-app/`
- [ ] Swagger docs accessible: `http://localhost:5656/api/swagger`

---

## 4. Deployment Type A: Native (Binary on Host)

### 4a. DEV (Source + Build)

- [ ] Repository cloned
- [ ] Build completed successfully
- [ ] `config/app_config.json` created from `config/app_config.sample.json`
- [ ] API keys populated in config
- [ ] PostgreSQL running locally
- [ ] ApiLogicServer running on port 5656
- [ ] Application starts: `make run`
- [ ] Application accessible at `http://localhost:8080`
- [ ] Login works
- [ ] Search returns results

### 4b. TEST / UAT / PROD (Binary Package)

#### Transfer and Extract

- [ ] Release package uploaded to target host
- [ ] Package extracted: `cd /opt && tar xzf franchiseai-<version>.tar.gz`
- [ ] Binary is executable: `ls -la /opt/franchiseai-<version>/bin/franchise_ai_search`
- [ ] Runtime deps satisfied: `ldd /opt/franchiseai-<version>/bin/franchise_ai_search | grep "not found"` (should return nothing)

#### Configuration

- [ ] Config file created: `cp config/app_config.sample.json config/app_config.json`
- [ ] Environment file created: `cp config/env.sample config/env`
- [ ] `config/app_config.json` edited with environment-specific values
- [ ] `config/env` edited with API keys and ALS host
- [ ] Database connection string points to correct environment database

#### Systemd Services

- [ ] `franchiseai.service` unit file created in `/etc/systemd/system/`
- [ ] `franchiseai-api.service` unit file created in `/etc/systemd/system/`
- [ ] `systemctl daemon-reload` executed
- [ ] API service enabled: `systemctl enable franchiseai-api`
- [ ] App service enabled: `systemctl enable franchiseai`
- [ ] API service started: `systemctl start franchiseai-api`
- [ ] App service started: `systemctl start franchiseai`
- [ ] API service running: `systemctl status franchiseai-api` shows active
- [ ] App service running: `systemctl status franchiseai` shows active

#### Nginx Reverse Proxy (UAT / PROD)

- [ ] Nginx installed and running
- [ ] Nginx config created for `franchiseai.example.com`
- [ ] WebSocket proxy headers configured (`Upgrade`, `Connection`)
- [ ] Static resources location configured (`/resources/`)
- [ ] Nginx config tested: `nginx -t`
- [ ] Nginx reloaded: `systemctl reload nginx`

#### Firewall

- [ ] Port 80/tcp allowed (HTTP)
- [ ] Port 443/tcp allowed (HTTPS)
- [ ] Port 8080/tcp blocked from external access
- [ ] Port 5656/tcp blocked from external access
- [ ] Port 5432/tcp blocked from external access (unless remote DB)

---

## 5. Deployment Type B: Container (Docker)

### 5a. Image Build

- [ ] Docker and Docker Compose installed
- [ ] `Dockerfile` (application) present in repository root
- [ ] `Dockerfile.api` (ApiLogicServer) present in repository root
- [ ] Application image builds: `docker compose build app`
- [ ] API image builds: `docker compose build api`
- [ ] No source code in final app image: `docker run --rm <app-image> find /opt/franchiseai -name "*.cpp" -o -name "*.h" -o -name "CMakeLists.txt"` (returns nothing)

### 5b. Docker Compose Startup

- [ ] `.env` file created with `DB_PASSWORD`, `OPENAI_API_KEY`, etc.
- [ ] All services start: `docker compose up -d`
- [ ] PostgreSQL healthy: `docker compose ps postgres` shows healthy
- [ ] API service running: `docker compose ps api` shows running
- [ ] App service running: `docker compose ps app` shows running
- [ ] Schema initialized (from `docker-entrypoint-initdb.d`)

### 5c. Verification

- [ ] Application responds: `curl http://localhost:8080`
- [ ] API responds: `curl http://localhost:5656/api`
- [ ] Database accessible: `docker compose exec postgres pg_isready`
- [ ] Logs are clean: `docker compose logs --tail=50`

### 5d. Registry Push (for Remote Deployment)

- [ ] Container registry accessible (ECR / ACR / GCR / Docker Hub)
- [ ] Images tagged with version: `docker tag <image> <registry>/<image>:<version>`
- [ ] Images pushed: `docker push <registry>/<image>:<version>`
- [ ] Images pullable from target environment

---

## 6. Deployment Type C: Cloud

### 6a. AWS (EC2 + RDS)

- [ ] VPC configured with public and private subnets
- [ ] EC2 instance launched (Ubuntu 22.04, t3.medium+)
- [ ] Security groups configured (80, 443 inbound; 8080, 5656 internal only)
- [ ] RDS PostgreSQL 14 instance created
- [ ] RDS accessible from EC2 (same VPC / security group)
- [ ] Runtime dependencies installed on EC2 (no compiler)
- [ ] Release package uploaded and extracted on EC2
- [ ] `config/env` points to RDS endpoint
- [ ] ApiLogicServer configured for RDS
- [ ] ALB created with HTTPS listener
- [ ] ACM certificate provisioned and attached to ALB
- [ ] ALB target group points to EC2:8080
- [ ] Application accessible via ALB DNS / domain

### 6b. AWS (ECS / Fargate)

- [ ] ECR repositories created for app and API images
- [ ] Images pushed to ECR
- [ ] ECS cluster created
- [ ] Task definitions created with correct environment variables
- [ ] Services created with desired count
- [ ] ALB configured with target groups for ECS services
- [ ] RDS accessible from ECS tasks (VPC / security group)
- [ ] CloudWatch log groups receiving logs

### 6c. Azure (VM or Container Instances)

- [ ] Resource group created
- [ ] VM launched or Container Instance created
- [ ] Azure Database for PostgreSQL provisioned
- [ ] Network security group configured
- [ ] Release package deployed (VM) or image pulled (Container)
- [ ] Application Gateway or Load Balancer configured
- [ ] SSL certificate attached

### 6d. GCP (Compute Engine or Cloud Run)

- [ ] Compute Engine VM launched or Cloud Run service created
- [ ] Cloud SQL PostgreSQL instance provisioned
- [ ] VPC connector configured (for Cloud Run to reach Cloud SQL)
- [ ] Release package deployed (VM) or image pushed to GCR (Cloud Run)
- [ ] Cloud Load Balancing configured
- [ ] Google-managed SSL certificate attached

---

## 7. Post-Deployment Verification

Run these checks after every deployment, regardless of deployment type.

### Application Health

- [ ] HTTP 200 on home page: `curl -s -o /dev/null -w "%{http_code}" http://<host>:8080`
- [ ] Login page renders
- [ ] Authentication works (login with test/admin credentials)
- [ ] Dashboard loads with stats
- [ ] AI Search page renders with search panel
- [ ] Search executes and returns results
- [ ] "Add to Prospects" saves a prospect
- [ ] My Prospects page displays saved prospects
- [ ] Contact information visible on prospect cards
- [ ] Demographics map loads with Leaflet
- [ ] Sidebar navigation works for all pages

### API Health

- [ ] API root responds: `curl http://<host>:5656/api`
- [ ] Franchisee endpoint: `curl http://<host>:5656/api/Franchisee`
- [ ] AppConfig endpoint: `curl http://<host>:5656/api/AppConfig`
- [ ] Admin UI accessible: `http://<host>:5656/admin-app/`

### Database Connectivity

- [ ] Application connects to database (no errors in logs)
- [ ] Data persists across application restart
- [ ] Session management works (session survives page refresh)

### Performance Baseline

- [ ] Home page loads in < 2 seconds
- [ ] Search completes in < 10 seconds
- [ ] No memory leaks apparent after 30 minutes of use

---

## 8. SSL / HTTPS

### Option A: Let's Encrypt (Nginx)

- [ ] Certbot installed: `apt-get install certbot python3-certbot-nginx`
- [ ] Certificate obtained: `certbot --nginx -d franchiseai.example.com`
- [ ] Auto-renewal tested: `certbot renew --dry-run`
- [ ] HTTP redirects to HTTPS
- [ ] Certificate valid: `curl -vI https://franchiseai.example.com 2>&1 | grep "SSL certificate verify ok"`

### Option B: Wt Built-in SSL

- [ ] Certificate and key files in place
- [ ] Application started with `--https-address` and `--ssl-certificate` flags
- [ ] HTTPS port accessible

### Option C: Cloud-Managed

- [ ] Certificate provisioned (ACM / Azure / GCP)
- [ ] Certificate attached to load balancer
- [ ] HTTPS listener configured
- [ ] HTTP-to-HTTPS redirect in place

### Option D: Self-Signed (DEV / TEST Only)

- [ ] Self-signed cert generated: `openssl req -x509 ...`
- [ ] Certificate and key configured in application or Nginx

---

## 9. Monitoring Setup

### Process Monitoring

- [ ] Systemd services configured to restart on failure (`Restart=on-failure`)
- [ ] Application logs accessible: `journalctl -u franchiseai -f` or `docker compose logs -f`
- [ ] API logs accessible: `journalctl -u franchiseai-api -f`

### Health Check Endpoints

- [ ] Application: `curl http://localhost:8080` returns 200
- [ ] API: `curl http://localhost:5656/api` returns 200
- [ ] PostgreSQL: `pg_isready -h localhost -p 5432` returns "accepting connections"

### Alerting (PROD)

- [ ] Uptime monitoring configured (UptimeRobot, Pingdom, CloudWatch, etc.)
- [ ] Alert notifications configured (email, Slack, PagerDuty)
- [ ] Disk space monitoring active
- [ ] Log rotation configured

---

## 10. Backup Configuration

### Database Backups

- [ ] Backup script created and tested: `pg_dump -U franchiseai_user -d franchiseai -F c -f backup.dump`
- [ ] Backup restores successfully: `pg_restore -U franchiseai_user -d franchiseai backup.dump`
- [ ] Cron job configured for automated daily backups
- [ ] Backup retention policy set (e.g., 30 days)
- [ ] Backups stored offsite or in cloud storage (S3, Azure Blob, GCS)

### Configuration Backups

- [ ] `config/app_config.json` backed up
- [ ] `config/env` backed up
- [ ] `resources/wt_config.xml` backed up
- [ ] Nginx configuration backed up
- [ ] Systemd unit files backed up

### Recovery Testing

- [ ] Database restore tested from backup
- [ ] Application restarts cleanly after restore
- [ ] Data integrity verified after restore

---

## 11. Repository Migration

Use when creating `Geolocation-Franchise` from `Geolocation-Sample`.

### Option A: Keep History

- [ ] Clone source repo: `git clone <source> <new-name>`
- [ ] Remove old origin: `git remote remove origin`
- [ ] Create new repo: `gh repo create <org>/<new-name> --private --source=. --push`

### Option B: Fresh Start

- [ ] Copy source directory: `cp -r <source> <new-name>`
- [ ] Remove git history: `rm -rf .git`
- [ ] Initialize new repo: `git init && git add . && git commit -m "Initial commit"`
- [ ] Create new repo: `gh repo create <org>/<new-name> --private --source=. --push`

### Post-Migration

- [ ] `README.md` updated with new repository name and URLs
- [ ] `config/app_config.json` is git-ignored (contains API keys)
- [ ] Hardcoded references to old repo name updated
- [ ] Branch protection rules configured on new repository
- [ ] API keys rotated if old repository was public
- [ ] Old repository archived

---

*Companion document to [DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md) — refer there for detailed instructions, configurations, and architecture diagrams.*
