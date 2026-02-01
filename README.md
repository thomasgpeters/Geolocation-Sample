# FranchiseAI - Prospect Search Application

A modular C++ application using the Wt (Witty) web framework that enables franchise owners to discover potential catering clients through AI-powered search across multiple data sources.

## Features

- **AI-Powered Search**: Intelligent search that aggregates data from multiple sources
- **OpenAI Integration**: Deep AI analysis of prospects using GPT-4o for catering potential assessment
- **Multi-Source Data Integration**:
  - Google My Business - Business listings and ratings
  - Better Business Bureau (BBB) - Accreditation and complaint data
  - Demographics Data - Population, income, and business statistics
  - OpenStreetMap - Free, open-source geolocation and POI data
- **Smart Scoring**: AI-driven catering potential scoring for each prospect
- **Modern UI**: Responsive sidebar navigation with a professional dashboard
- **Prospect Management**: Save prospects with on-demand AI analysis, view and manage in My Prospects

## Target Market

The application helps franchise owners find potential clients interested in catering services for:
- Corporate offices and conference rooms
- Warehouses and distribution centers
- Coworking spaces
- Hotels and conference centers
- Manufacturing facilities
- Any business with regular meeting and event catering needs

## ⚠️ Authentication (Temporary Implementation)

> **Note:** The current authentication system is a temporary mock implementation for development and testing purposes. A more robust, enterprise-class authentication system with full database integration will be implemented in a future sprint.

### Current Mock Users

The application currently uses hardcoded credentials for authentication:

| Email | Password | Role |
|-------|----------|------|
| `admin@franchiseai.com` | `admin123` | admin |
| `mike@pittsburghcatering.com` | `mike123` | franchisee |
| `thomas.g.peters@imagery-business-systems.com` | `password123` | admin |

### Role-Based Access

- **admin**: Full access to all features including the Audit Trail
- **franchisee**: Access to standard features (search, prospects, settings)

### Sidebar Menu

```
📊 Dashboard
🔍 AI Search
📍 Open Street Map
─────────────────
👥 My Prospects
📈 Reports
📋 Audit Trail (admin only)
─────────────────
⚙️ Settings
```

### Planned Enterprise Features (Future Sprint)

The production authentication system will include:

- Secure password hashing (bcrypt/Argon2)
- JWT or session-based token management
- Integration with ApiLogicServer for persistent user storage
- Password reset functionality
- Account lockout after failed attempts
- Multi-factor authentication (MFA) support
- Role-based access control (RBAC) with granular permissions
- Audit logging for all authentication events
- Session timeout and refresh token handling

### Technical Notes

- Authentication state is stored in memory only (sessions do not persist across server restarts)
- No external database calls are made during authentication
- The `AuthService` class handles all authentication logic in `src/services/AuthService.cpp`

---

## Project Structure

```
├── CMakeLists.txt              # Build configuration
├── config/
│   ├── app_config.json         # User configuration (git-ignored)
│   └── app_config.sample.json  # Sample configuration template
├── src/
│   ├── main.cpp                # Application entry point
│   ├── AppConfig.h             # Global configuration singleton
│   ├── FranchiseApp.cpp/h      # Main Wt application class
│   ├── models/                 # Data models
│   │   ├── BusinessInfo.cpp/h      # Business data model
│   │   ├── DemographicData.h       # Demographics data model
│   │   └── SearchResult.cpp/h      # Search result models
│   ├── services/               # API services
│   │   ├── AISearchService.cpp/h       # AI search aggregator
│   │   ├── AIEngine.h                  # AI engine interface
│   │   ├── OpenAIEngine.cpp/h          # OpenAI GPT integration
│   │   ├── GeminiEngine.cpp/h          # Google Gemini integration
│   │   ├── GoogleMyBusinessAPI.cpp/h   # Google Places API
│   │   ├── GoogleGeocodingAPI.cpp/h    # Google Geocoding API for address resolution
│   │   ├── GooglePlacesAPI.cpp/h       # Google Places API for POI search
│   │   ├── ThreadPool.cpp/h            # Multi-threaded task execution pool
│   │   ├── BBBAPI.cpp/h               # BBB API service
│   │   ├── DemographicsAPI.cpp/h      # Demographics API
│   │   ├── OpenStreetMapAPI.cpp/h     # OpenStreetMap/Overpass API
│   │   └── ApiLogicServerClient.cpp/h # ALS REST client (Franchisee, StoreLocation, AppConfig)
│   └── widgets/                # UI components
│       ├── Sidebar.cpp/h           # Navigation sidebar
│       ├── Navigation.cpp/h        # Top navigation bar
│       ├── SearchPanel.cpp/h       # Search form panel
│       ├── ResultCard.cpp/h        # Individual result card
│       └── ResultsDisplay.cpp/h    # Results container
├── tests/                      # Test framework
│   ├── TestOrchestrator.cpp/h      # Test suite management and execution
│   └── test_runner_ui.cpp          # ncurses-based test runner UI
└── resources/
    └── css/
        └── style.css           # Application styles
```

## Prerequisites

- CMake 3.16 or higher
- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2019+)
- Wt (Witty) library 4.x with HTTP connector

### Installing Wt

**Ubuntu/Debian:**
```bash
sudo apt-get install witty witty-dev
```

**macOS (Homebrew):**
```bash
brew install wt
```

**From Source:**
```bash
git clone https://github.com/emweb/wt.git
cd wt
mkdir build && cd build
cmake ..
make && sudo make install
```

## Building

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
make -j$(nproc)
```

## Running

```bash
# From the build directory
./franchise_ai_search --docroot ./resources --http-address 0.0.0.0 --http-port 8080
```

Then open your browser to: http://localhost:8080

### Command Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `--docroot <path>` | Document root for static resources | ./resources |
| `--http-address <addr>` | HTTP server bind address | 0.0.0.0 |
| `--http-port <port>` | HTTP server port | 8080 |
| `--help` | Show help message | - |

## Application Pages

### Sidebar Navigation
The sidebar provides the main navigation with the following structure:

| # | Menu Item | Description |
|---|-----------|-------------|
| 1 | **Dashboard** | Overview and quick stats |
| 2 | **AI Search** | Find new prospects |
| 3 | **Demographics** | Explore area data on map |
| - | ─────────── | *Divider* |
| 4 | **My Prospects** | Saved prospects list |
| 5 | **Reports** | Analytics and reports |
| 6 | **Settings** | Store setup, API keys, preferences |

The divider visually separates discovery tools (Dashboard, AI Search, Demographics) from management tools (My Prospects, Reports, Settings).

#### Franchise Info Popup
The sidebar header includes a clickable user section that displays a franchise information popup:

**Compact User Section:**
- User avatar (image or fallback icon)
- Owner name and franchise name
- Dropdown arrow indicator (▼)

**Popup Contents (on click):**
- **Header**: Large avatar, owner name, franchise name, store ID badge, edit button (✏️)
- **Contact Details**: Address, phone, and email with icons
- **Actions**: View My Profile, Logout

The popup uses smooth slide-in animation and matches the application's dark theme styling.

### Dashboard
Overview of prospect discovery statistics with quick action buttons.

### AI Search
Main search interface with:
- Location input (city, state, ZIP code)
- Search radius slider
- Minimum catering score filter
- Advanced sorting options (relevance, catering potential, distance, employee count, rating)

**Note**: Business type filters and data source selection have been moved to **Settings > Marketing** tab for centralized configuration. These preferences are automatically applied to all searches.

#### Performance Optimizations
- **Multi-threaded geocoding**: Uses a configurable thread pool for parallel address resolution
- **Google APIs integration**: Leverages Google Geocoding and Places APIs for faster, more accurate results
- **Intelligent caching**: Reduces redundant API calls for repeated searches

### My Prospects
View and manage saved prospects with AI-powered analysis.

#### Features
- **Prospect Cards**: Each saved prospect displays:
  - Business name and catering potential score (color-coded badge)
  - Address and business type information
  - AI Analysis summary (generated when prospect was saved)
  - Key highlights and recommended actions
- **Score Badges**:
  - **Green (70%+)**: High catering potential
  - **Yellow (40-69%)**: Medium potential
  - **Red (<40%)**: Lower potential
- **Remove**: Delete prospects from your saved list
- **Find More**: Quick link to return to AI Search

#### AI Analysis Workflow
AI analysis is performed **on-demand** when you add a prospect:
1. Search results use fast local scoring (instant results)
2. When you click "Add to Prospects", the system calls OpenAI to analyze that specific business
3. The AI summary, highlights, and recommendations are saved with the prospect
4. This approach keeps searches fast while providing deep analysis for prospects you care about

### Demographics
Explore area data powered by OpenStreetMap with an interactive map and category-based POI visualization.

#### Layout
- **Interactive Map** (left): Leaflet-powered map loaded via CDN showing the search area and POI markers
- **Category Pill Tray** (right): Floating sidebar for selecting and configuring POI categories
- **Area Summary Footer**: Horizontal stats bar showing total POIs, business density, location, and radius

#### Search Controls
- **Location Input**: Enter any city, address, or location name
- **Radius Dropdown**: Select search radius (5, 10, 25, 40, or 50 km)
- **Analyze Area Button**: Triggers geocoding, map update, and POI marker refresh
- **Location Blur Event**: Automatically recenters map when location input loses focus

#### Category Pill Tray
The pill tray provides a modern interface for selecting which POI categories to display on the map:

1. **Category Dropdown**: Select from 12 business categories:
   - Offices, Hotels, Conference Venues, Restaurants, Cafes
   - Hospitals, Universities, Schools
   - Industrial, Warehouses, Banks, Government

2. **Pill Cards**: Each selected category appears as a colored card with:
   - **Post-it Note Styling**: Soft pastel background colors (peach, mint, blue, yellow, pink, purple, teal, orange, lavender, lime, cream, indigo)
   - **Category Name & Count**: Shows the category name and total POIs available
   - **POI Slider Control**: Audio mixer-style fader to control how many POIs to display (0 to max)
   - **Real-time Value Display**: Slider value updates instantly while dragging
   - **Remove Button (×)**: Click to remove the category from the tray

3. **Color-Coded Markers**: Each category's POI markers on the map use a deeper, more vivid version of the pill's pastel color for easy visual correlation

4. **Clear All**: Button in the footer to remove all category pills at once

#### Slider Control Features
The POI limit slider has an audio mixer fader appearance:
- **Track**: Recessed groove with purple gradient center
- **Fader Thumb**: Metallic rectangular cap with 3D shading
- **Real-time Updates**: Value display updates as you drag (JavaScript `input` event)
- **Server Sync**: `sliderMoved()` signal keeps backend state synchronized

#### Market Potential Score
Displayed in the navigation header as a color-coded badge:
- **Green (80-100)**: High potential market
- **Yellow (50-79)**: Medium potential market
- **Red (0-49)**: Lower potential market
- **Hidden**: When no search is active

#### Technical Implementation
- **Map Library**: Leaflet.js loaded from unpkg.com CDN
- **Custom Markers**: `L.divIcon` with colored circular divs matching category colors
- **State Management**: `CategoryPillData` struct with shared_ptr for lambda captures
- **Responsive Layout**: Flexbox with `calc(100vh - offset)` for full-height map

### Reports
View analytics and generate reports (placeholder).

### Settings
Unified configuration hub with a tabbed interface for managing all application settings.

#### Tab Interface
The Settings page uses a modern tab navigation with four sections:

| Tab | Purpose |
|-----|---------|
| **Store Setup** | Configure your franchise location and contact details |
| **Marketing** | Configure target business types, employee ranges, and data sources for AI Search |
| **AI Configuration** | Set up OpenAI or Gemini for prospect analysis |
| **Data Sources** | Configure API keys for business data providers |

#### Tab 1: Store Setup
Configure your franchise store details and contact information.

**Store Information:**
- **Store Name**: Your franchise location name (e.g., "Vocelli Pizza - Downtown")
- **Store Address**: Full address used as the center point for prospect searches
- **Owner/Manager Name**: Contact person for the franchise
- **Store Phone**: Business contact number
- **Store Email**: Contact email address

#### Tab 2: Marketing
Configure your target market preferences for AI Search. These settings are automatically applied to all searches.

**Target Business Types:**
Checkboxes for 12 business categories to target:
- Corporate Offices, Conference Centers, Hotels
- Medical Facilities, Educational Institutions, Manufacturing/Industrial
- Warehouses/Distribution, Government Offices, Tech Companies
- Financial Services, Coworking Spaces, Non-profits

**Target Employee Ranges:**
Checkboxes to filter by organization size:
- Small (1-50 employees)
- Medium (51-200 employees)
- Large (201-500 employees)
- Enterprise (500+ employees)

**Data Sources:**
Select which data sources to use for searches:
- Google My Business
- Better Business Bureau (BBB)
- Demographics Data
- OpenStreetMap

#### Tab 3: AI Configuration
Configure AI providers for intelligent prospect analysis.

- **OpenAI API Key**: Enter your OpenAI API key for GPT-powered analysis
- **OpenAI Model**: Select from available models:
  - gpt-4o (Recommended) - Best quality analysis
  - gpt-4o-mini - Faster, lower cost
  - gpt-4-turbo, gpt-4, gpt-3.5-turbo
- **Google Gemini API Key**: Alternative AI provider (used if OpenAI not configured)
- **Status Indicator**: Shows current AI engine configuration status with color coding

#### Tab 4: Data Sources
Configure API keys for business data providers.

- **Google Places API Key**: For Google My Business integration
- **BBB API Key**: For Better Business Bureau accreditation data
- **Census/Demographics API Key**: For population and economic data

**Note**: OpenStreetMap integration requires no API key and is always available.

#### Saving Settings
- Click **"Save All Settings"** to save changes across all tabs
- Success/error messages appear below the save button
- API key fields are masked after saving for security
- Store information is immediately reflected in the sidebar

## API Configuration

API keys can be configured in three ways (in order of precedence):

### 1. Environment Variables (Recommended for Production)

```bash
# AI Providers
export OPENAI_API_KEY="sk-your-openai-api-key"
export GEMINI_API_KEY="your-gemini-api-key"

# Data Sources
export GOOGLE_API_KEY="your-google-api-key"
export BBB_API_KEY="your-bbb-api-key"
export CENSUS_API_KEY="your-census-api-key"
```

### 2. Configuration File

Copy the sample configuration and add your keys:

```bash
cp config/app_config.sample.json config/app_config.json
```

Edit `config/app_config.json`:
```json
{
  "openai_api_key": "sk-your-openai-api-key",
  "openai_model": "gpt-4o",
  "google_api_key": "your-google-api-key",
  "bbb_api_key": "your-bbb-api-key",
  "census_api_key": "your-census-api-key",
  "gemini_api_key": "your-gemini-api-key"
}
```

**Note**: The `config/app_config.json` file is git-ignored to protect your API keys.

### 3. Settings Page

Navigate to Settings in the application to enter API keys through the UI. Keys entered here are saved to the configuration file.

### Configuration Loading

At startup, the application:
1. Loads environment variables first
2. Loads from `config/app_config.json` (only fills in missing values)
3. Prints configuration status to console showing which keys are configured

**Note**: The application works without API keys using OpenStreetMap (free, no key required) for location data and local scoring for prospect analysis.

### OpenStreetMap Integration

The OpenStreetMap integration uses the free Overpass API and requires no API key. It provides:

- **POI Search**: Find businesses and points of interest by location and radius
- **Geocoding**: Convert addresses to coordinates via Nominatim
- **Area Statistics**: Aggregate counts of business types in an area
- **Business Type Mapping**: Automatic mapping of OSM tags to application business types

**Supported OSM Tags → Business Types:**

| OSM Tag | Business Type |
|---------|---------------|
| `office=company`, `office=corporate` | Corporate Office |
| `office=it`, `office=telecommunication` | Tech Company |
| `office=financial`, `office=insurance` | Financial Services |
| `office=lawyer` | Law Firm |
| `office=government` | Government Office |
| `amenity=conference_centre` | Conference Center |
| `tourism=hotel` | Hotel |
| `amenity=hospital`, `healthcare=*` | Medical Facility |
| `amenity=university`, `amenity=college` | Educational Institution |
| `building=warehouse` | Warehouse |
| `building=industrial` | Manufacturing |
| `amenity=coworking_space` | Coworking Space |

**Configuration Options (OSMAPIConfig):**

```cpp
struct OSMAPIConfig {
    std::string overpassEndpoint = "https://overpass-api.de/api/interpreter";
    std::string nominatimEndpoint = "https://nominatim.openstreetmap.org";
    int requestTimeoutMs = 30000;
    bool enableCaching = true;
    int cacheDurationMinutes = 1440;  // 24 hours
    int maxResultsPerQuery = 100;
    std::string userAgent = "FranchiseAI/1.0";
};
```

## ApiLogicServer Integration

The application integrates with [ApiLogicServer](https://apilogicserver.github.io/Docs/) for persistent storage of franchisee data, store locations, and application configuration. This enables data to survive application restarts and be shared across instances.

### Startup Sequence

When the application starts, it performs the following initialization:

```
1. Load app_config.json (local API keys)
2. Initialize ApiLogicServer client
3. Load AppConfig entries from ALS into memory cache
4. Load current Franchisee from ALS (using current_franchisee_id)
5. Load current StoreLocation from ALS (using current_store_id)
6. Initialize UI
```

**Console Output Example:**
```
[ALS] Loading all AppConfig entries...
[ALS] Cached: current_franchisee_id    = 'abc123-...'  (id: ...)
[ALS] Cached: current_store_id         = 'def456-...'  (id: ...)
[ALS] Cached: default_search_radius    = '5'           (id: ...)
[ALS] Loaded 14 config entries
[App] Loading franchisee from ALS: abc123-...
[App] Loaded franchisee: Vocelli Pizza
[App] Loading store location from ALS: def456-...
```

### AppConfig Persistence

Application settings are stored in the `app_config` table via ALS. The client maintains an in-memory cache for fast lookups while ensuring persistence across restarts.

**Key AppConfig Entries:**

| Config Key | Description | Example Value |
|------------|-------------|---------------|
| `current_franchisee_id` | UUID of the active franchisee | `c1000000-0000-...` |
| `current_store_id` | UUID of the active store location | `d1000000-0000-...` |
| `default_search_radius` | Default search radius in miles | `5` |
| `theme` | UI theme preference | `light` |
| `enable_ai_search` | Feature flag for AI search | `true` |
| `enable_demographics` | Feature flag for demographics | `true` |

**Cache Operations:**
```cpp
// Read from cache (instant)
std::string value = alsClient->getAppConfigValue("current_store_id");

// Write to cache and ALS (persisted)
alsClient->setAppConfigValue("current_store_id", newStoreId);
```

### Franchisee Management

Franchisees represent the business entity that owns one or more store locations.

**FranchiseeDTO Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `id` | UUID | Unique identifier (auto-generated for new) |
| `businessName` | string | Legal business name |
| `dbaName` | string | "Doing Business As" name |
| `franchiseNumber` | string | Franchise identifier (unique) |
| `ownerFirstName` | string | Owner's first name |
| `ownerLastName` | string | Owner's last name |
| `email` | string | Contact email |
| `phone` | string | Contact phone |
| `addressLine1/2` | string | Business address |
| `city/stateProvince/postalCode` | string | Location details |
| `latitude/longitude` | double | Geocoded coordinates |
| `isActive` | bool | Active status |

**API Operations:**
```cpp
// Create new franchisee (UUID auto-generated)
FranchiseeDTO dto;
dto.businessName = "Vocelli Pizza";
dto.ownerFirstName = "John";
auto response = alsClient->saveFranchisee(dto);
// POST /api/Franchisee with generated UUID

// Update existing franchisee
dto.id = "existing-uuid";
alsClient->saveFranchisee(dto);
// PATCH /api/Franchisee/{id}

// Load all franchisees (for dropdown)
auto franchisees = loadAvailableFranchisees();
```

### Store Location Management

Store locations represent physical franchise locations linked to a franchisee.

**StoreLocationDTO Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `id` | UUID | Unique identifier (auto-generated for new) |
| `franchiseeId` | UUID | Link to parent franchisee |
| `storeName` | string | Store display name |
| `storeCode` | string | Store identifier (unique) |
| `addressLine1/2` | string | Store address |
| `city/stateProvince/postalCode` | string | Location details |
| `latitude/longitude` | double | Geocoded coordinates |
| `geocodeSource` | string | Source of coordinates (nominatim, google, manual) |
| `defaultSearchRadiusMiles` | double | Default search radius |
| `phone/email` | string | Store contact info |
| `isActive/isPrimary` | bool | Status flags |

**API Operations:**
```cpp
// Create new store location
StoreLocationDTO dto;
dto.storeName = "Downtown Location";
dto.franchiseeId = currentFranchiseeId;  // Link to franchisee
auto response = alsClient->saveStoreLocation(dto);
// POST /api/StoreLocation with generated UUID

// Update existing store
dto.id = "existing-uuid";
alsClient->saveStoreLocation(dto);
// PATCH /api/StoreLocation/{id}

// Load from dropdown selection
selectStoreById(selectedId);
// Loads store data and saves to current_store_id AppConfig
```

### Client-Side UUID Generation

For new records (Franchisee and StoreLocation), UUIDs are generated client-side before POSTing to ALS. This ensures the client knows the ID immediately and avoids issues with PostgreSQL UUID generation.

**UUID v4 Format:**
```
xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
         │    │    │
         │    │    └── Variant bits (8, 9, a, or b)
         │    └─────── Version 4 (random)
         └──────────── Random hex digits
```

**Example:**
```
[ALS] Creating new StoreLocation with generated UUID: 9407a827-1fd1-46ce-a5ab-d55f43f3044c
[ALS] POST http://localhost:5657/api/StoreLocation
```

### Save All Settings Flow

When the user clicks "Save All Settings" in the Settings page:

```
1. saveFranchiseeToALS()
   ├── Build FranchiseeDTO from form data
   ├── If no ID: generate UUID, POST to /api/Franchisee
   ├── If has ID: PATCH to /api/Franchisee/{id}
   └── Save current_franchisee_id to AppConfig

2. saveStoreLocationToALS()
   ├── Build StoreLocationDTO from form data
   ├── Set franchiseeId to link to current franchisee
   ├── If no ID: generate UUID, POST to /api/StoreLocation
   ├── If has ID: PATCH to /api/StoreLocation/{id}
   └── Save current_store_id to AppConfig
```

### ALS Endpoint Configuration

The ApiLogicServer endpoint is configured in `config/app_config.json`:

```json
{
  "api_logic_server_endpoint": "http://localhost:5657/api"
}
```

Or via environment variable:
```bash
export API_LOGIC_SERVER_ENDPOINT="http://localhost:5657/api"
```

## Architecture

### Models Layer
- `BusinessInfo`: Comprehensive business data model with contact info, ratings, and catering potential scoring
- `DemographicData`: Area demographics including population, income, and business statistics
- `SearchResult`: Container for search results with AI analysis

### Services Layer
- `AISearchService`: Orchestrates searches across all data sources and performs AI analysis
- `GoogleMyBusinessAPI`: Interface to Google Places API
- `GoogleGeocodingAPI`: High-performance address-to-coordinate resolution using Google Geocoding API
- `GooglePlacesAPI`: POI search and business details using Google Places API
- `ThreadPool`: Configurable thread pool for parallel geocoding and API requests
- `BBBAPI`: Interface to Better Business Bureau API
- `DemographicsAPI`: Interface to Census and economic data APIs
- `OpenStreetMapAPI`: Interface to OpenStreetMap Overpass API for geolocation and POI data
- `ApiLogicServerClient`: REST client for ApiLogicServer (Franchisee, StoreLocation, AppConfig CRUD operations)

### UI Layer (Wt Widgets)
- `Sidebar`: Main navigation with collapsible menu
- `Navigation`: Top bar with quick search and user actions
- `SearchPanel`: Form for configuring search parameters
- `ResultsDisplay`: Container for displaying search results
- `ResultCard`: Individual prospect card with metrics and actions

## Testing

### Test Orchestrator
The project includes a comprehensive test framework with an ncurses-based UI for interactive test execution.

#### Components
- **TestOrchestrator**: Core class managing test suites, test cases, and execution
- **TestSuite**: Groups related tests with enable/disable functionality
- **TestCase**: Individual test with executor function, description, and result tracking
- **TestResult**: Contains pass/fail status, message, duration, and logs

#### ncurses Test Runner UI
A terminal-based test runner with visual interface:

**Features:**
- **Suite/Test Tree View**: Hierarchical display of test suites and individual tests
- **Checkbox Selection**: Enable/disable individual tests or entire suites
- **Keyboard Navigation**:
  - `↑/↓`: Navigate between items
  - `Space`: Toggle selection
  - `Enter`: Run selected tests
  - `r`: Run all enabled tests
  - `a`: Select all tests
  - `n`: Deselect all tests
  - `q`: Quit
- **Progress Display**: Real-time progress bar during test execution
- **Result Summary**: Pass/fail counts with color-coded status

#### Building the Test Runner
```bash
# Requires ncurses library
cmake .. -DBUILD_TESTS=ON
make test_runner
./test_runner
```

## License

MIT License - See LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request
