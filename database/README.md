# Database & ApiLogicServer Setup

This folder contains the PostgreSQL database schema and ApiLogicServer middleware configuration for the FranchiseAI application.

## Files

| File | Description |
|------|-------------|
| `schema.sql` | Complete PostgreSQL schema with all tables, indexes, triggers, views, and seed data |
| `scoring_rule.sql` | Standalone SQLite-compatible scoring rules schema (for reference/testing) |

## ApiLogicServer Setup

ApiLogicServer generates a complete REST API from your database schema, including:
- JSON:API compliant endpoints
- Authentication & authorization
- Business logic hooks
- Admin UI

### Prerequisites

- Python 3.10+
- PostgreSQL 14+ (running and accessible)
- pip (Python package manager)

### Step 1: Create Python Virtual Environment

```bash
# Create virtual environment
python3 -m venv venv

# Activate virtual environment
# On macOS/Linux:
source venv/bin/activate

# On Windows:
venv\Scripts\activate
```

### Step 2: Install ApiLogicServer

```bash
pip install ApiLogicServer
```

### Step 3: Create the Database

```bash
# Connect to PostgreSQL and create the database
psql -U postgres -c "CREATE DATABASE franchiseai;"

# Run the schema
psql -U postgres -d franchiseai -f schema.sql
```

### Step 4: Generate ApiLogicServer Project

```bash
# Navigate to parent directory (outside database folder)
cd ..

# Create ApiLogicServer project from database
ApiLogicServer create \
    --project_name=api \
    --db_url="postgresql://postgres:password@localhost:5432/franchiseai"
```

Replace `password` with your PostgreSQL password.

### Step 5: Configure the API

Edit `api/config/config.py` to customize:

```python
# API settings
APILOGICPROJECT_PORT = 5656
APILOGICPROJECT_HOST = "0.0.0.0"

# Database connection
SQLALCHEMY_DATABASE_URI = "postgresql://postgres:password@localhost:5432/franchiseai"

# CORS settings (for C++ client)
CORS_ORIGINS = ["http://localhost:8080", "http://localhost:9090"]
```

### Step 6: Start the API Server

```bash
cd api

# Using the startup script
python api_logic_server_run.py

# Or using the CLI
ApiLogicServer run --project_name=. --host=0.0.0.0 --port=5656
```

The API will be available at:
- **API Endpoints**: `http://localhost:5656/api`
- **Admin UI**: `http://localhost:5656/admin-app/`
- **Swagger Docs**: `http://localhost:5656/api/swagger`

## API Endpoints

Once running, the following endpoints are available:

### Core Entities

| Endpoint | Description |
|----------|-------------|
| `GET /api/Franchisee` | List all franchisees |
| `GET /api/StoreLocation` | List all store locations |
| `GET /api/ScoringRule` | List all scoring rules |
| `GET /api/User` | List all users |
| `GET /api/AppConfig` | List all app configuration entries |

### Scoring Rules API

```bash
# Get all scoring rules
curl http://localhost:5656/api/ScoringRule

# Get rules for a specific franchisee
curl "http://localhost:5656/api/ScoringRule?filter[franchisee_id]=<uuid>"

# Update a scoring rule
curl -X PATCH http://localhost:5656/api/ScoringRule/<uuid> \
    -H "Content-Type: application/json" \
    -d '{"data": {"attributes": {"current_points": -15, "enabled": true}}}'
```

## Database Schema Overview

### Main Tables

- **franchisees** - Franchise owner information
- **store_locations** - Physical store locations with geocoding
- **scoring_rules** - Configurable prospect scoring rules
- **users** - Application users with authentication
- **app_config** - Application configuration key-value store
- **prospects** - Saved catering prospects
- **audit_log** - Activity audit trail

### Views

- **v_active_scoring_rules** - Active scoring rules ordered by impact

### Key Relationships

```
franchisees (1) ─────< (N) store_locations
franchisees (1) ─────< (N) scoring_rules
franchisees (1) ─────< (N) users
franchisees (1) ─────< (N) prospects
```

## Environment Variables

For production, use environment variables instead of hardcoding:

```bash
export APILOGICPROJECT_DATABASE_URI="postgresql://user:pass@host:5432/franchiseai"
export APILOGICPROJECT_PORT=5656
export APILOGICPROJECT_SECRET_KEY="your-secret-key"
```

## Troubleshooting

### Connection Refused
```bash
# Check PostgreSQL is running
pg_isready -h localhost -p 5432

# Check the database exists
psql -U postgres -c "\l" | grep franchiseai
```

### Permission Denied
```bash
# Grant permissions to user
psql -U postgres -c "GRANT ALL PRIVILEGES ON DATABASE franchiseai TO your_user;"
```

### Schema Changes
After modifying `schema.sql`, regenerate the API:

```bash
# Drop and recreate
psql -U postgres -c "DROP DATABASE franchiseai;"
psql -U postgres -c "CREATE DATABASE franchiseai;"
psql -U postgres -d franchiseai -f schema.sql

# Regenerate API (backup customizations first!)
ApiLogicServer rebuild-from-database --project_name=api
```

## C++ Client Integration

The C++ application connects to ApiLogicServer via `ApiLogicServerClient`:

```cpp
// Configuration in AppConfig or environment
std::string alsEndpoint = "http://localhost:5656/api";

// Client usage
ApiLogicServerClient client;
client.setEndpoint(alsEndpoint);

// Load scoring rules
auto response = client.getScoringRules();
auto rules = ApiLogicServerClient::parseScoringRules(response);
```

See `src/services/ApiLogicServerClient.h` for the complete API.
