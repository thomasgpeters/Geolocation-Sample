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
├── wt_config.xml               # Wt server configuration (clean URLs, sessions)
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
    ├── wt_config.xml           # Wt configuration (copied here for --approot)
    ├── css/
    │   └── style.css           # Application styles
    └── scripts/
        └── leaflet.js          # Leaflet map library (local)
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
| `--approot <path>` | Application root (where wt_config.xml is located) | ./resources |
| `--http-address <addr>` | HTTP server bind address | 0.0.0.0 |
| `--http-port <port>` | HTTP server port | 8080 |
| `--help` | Show help message | - |

## Wt Server Configuration

The application uses Wt's (Witty) web framework with specific configuration for clean URLs and session management.

### Configuration File: wt_config.xml

The `wt_config.xml` file is located in the `resources/` directory and is automatically loaded when `--approot` points to that directory.

**Location:** `resources/wt_config.xml`

```xml
<?xml version="1.0" encoding="UTF-8"?>
<server>
    <application-settings location="*">
        <!-- Disable progressive bootstrap - required for HTML5 history-based URLs -->
        <progressive-bootstrap>false</progressive-bootstrap>

        <!-- Use cookies for session ID instead of URL parameter -->
        <session-id-cookie>true</session-id-cookie>

        <!-- Session timeout in seconds (30 minutes) -->
        <session-timeout>1800</session-timeout>

        <!-- Idle timeout - disconnect idle sessions after 10 minutes -->
        <idle-timeout>600</idle-timeout>

        <!-- Leaflet Map Configuration - Local Resources -->
        <properties>
            <property name="leafletJSURL">scripts/leaflet.js</property>
            <property name="leafletCSSURL">css/leaflet.css</property>
        </properties>
    </application-settings>
</server>
```

### Clean URL Configuration

By default, Wt uses query parameters like `?_=/dashboard` or `?wtd=abc123` for internal routing and session tracking. The configuration above eliminates these for cleaner URLs:

| Setting | Purpose | Effect |
|---------|---------|--------|
| `progressive-bootstrap: false` | Disables hash-based URL fallback | Enables HTML5 History API for clean paths |
| `session-id-cookie: true` | Stores session ID in cookie | Removes `wtd=` parameter from URLs |

**URL Comparison:**

| Without Config | With Config |
|----------------|-------------|
| `/?_=/dashboard` | `/dashboard` |
| `/search?_=abc123&wtd=xyz` | `/search` |
| `/prospects?wtd=session123` | `/prospects` |

### Configuration Loading Order

Wt searches for `wt_config.xml` in this order:

1. `--config` or `-c` command line parameter
2. `$WT_CONFIG_XML` environment variable
3. `appRoot/wt_config.xml` (where appRoot is set via `--approot`) ✓ **Used by this app**
4. `/etc/wt/wt_config.xml` (system fallback)

### Running with Configuration

The `make run` target automatically sets up the correct paths:

```bash
# Using make run (recommended)
cd build
cmake ..
make run

# Manual execution with approot
./franchise_ai_search \
    --docroot ./resources \
    --approot ./resources \
    --http-address 0.0.0.0 \
    --http-port 8080
```

### Verifying Configuration Loading

Check the server startup log for confirmation:

```
# Correct (loading from approot):
config: reading Wt config file: /path/to/build/resources/wt_config.xml

# Incorrect (falling back to system config):
config: reading Wt config file: /etc/wt/wt_config.xml
```

If you see the system path, ensure:
1. `wt_config.xml` exists in the `resources/` directory
2. `--approot` parameter points to the `resources/` directory

### Session and Timeout Settings

| Setting | Value | Description |
|---------|-------|-------------|
| `session-timeout` | 1800 (30 min) | Maximum session lifetime |
| `idle-timeout` | 600 (10 min) | Disconnect after inactivity |

Sessions are maintained via cookies (`session-id-cookie: true`), so users can:
- Bookmark pages directly (e.g., `/dashboard`, `/search`)
- Use browser back/forward navigation naturally
- Share clean URLs without session parameters

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
- **Header**: Large avatar, owner name, franchise name, store ID badge
- **Contact Details**: Address, phone, and email with icons (synced from Settings)
- **Actions**:
  - **Edit Profile** (✏️) - Navigates to Settings page to edit franchisee details
  - **View Profile** (👤) - Navigates to Settings page
  - **Logout** (🚪) - Returns to login page

**Synchronization:**
The popup details (address, phone, email) are automatically updated when:
- Settings are saved via the Settings page
- A different store is selected from the store dropdown
- The app loads franchisee data from ApiLogicServer on startup

The popup uses smooth slide-in animation and matches the application's dark theme styling.

### Dashboard
Overview of prospect discovery statistics with quick action buttons.

#### Hot Prospects Section
The Dashboard features a **Hot Prospects** section that displays your top 5 highest-scoring prospects at a glance:

**Features:**
- **Top 5 Display**: Shows the five highest-scoring prospects from your most recent search
- **Score Badges**: Color-coded badges indicate prospect quality
- **Quick Preview**: Click any prospect to open a detailed preview dialog
- **One-Click Add**: Add prospects directly to your saved list without leaving the Dashboard
- **Auto-Refresh**: Automatically pulls from your most recent search or saved prospects

**Score Badge Colors:**

| Score Range | Color | Label |
|-------------|-------|-------|
| 80-100 | Green | Excellent |
| 60-79 | Blue | Good |
| 40-59 | Amber | Fair |
| 0-39 | Gray | Low |

**Score Legend:**
A color-coded legend in the Hot Prospects header helps users quickly understand what each badge color represents.

**Layout:**
- Limited to 4 visible rows with smooth scrolling for additional prospects
- Compact card design with score badge, business name, and action buttons

### AI Search
Main search interface with:
- **Single location input**: Enter any address, city, or location description (e.g., "Denver, CO" or "123 Main St, Pittsburgh, PA")
- Search radius slider
- Minimum catering score filter
- Advanced sorting options (relevance, catering potential, distance, employee count, rating)

**Simplified Address Entry**: The AI Search uses a single text field for location input, allowing flexible address formats. City, State, and Zip fields have been moved to Settings for store configuration only.

**Note**: Business type filters and data source selection have been moved to **Settings > Marketing** tab for centralized configuration. These preferences are automatically applied to all searches.

#### Progressive Loading & Score Optimization
Search results now appear **instantly** while score optimization happens in the background:

**How It Works:**
1. User clicks "Search"
2. Results appear immediately (1-3 seconds) with base scores
3. "Optimizing scores..." indicator appears (pulsing badge)
4. Scoring Engine processes results in background
5. Cards re-sort automatically with final adjusted scores
6. Indicator disappears when complete

**Visual Indicators:**

| State | Indicator |
|-------|-----------|
| Loading | Spinner with "Searching..." text |
| Results Ready | Results display with base scores |
| Optimizing | Pulsing badge: "Optimizing scores..." |
| Complete | Final sorted results, no indicator |

**Benefits:**
- **Instant Feedback**: See results in 1-3 seconds instead of waiting for full processing
- **Non-Blocking**: Users can browse and interact with results during optimization
- **Transparency**: Clear indication when scoring is still in progress

#### Configurable Scoring Engine
Franchisees can customize how prospects are scored through the Settings page. The scoring engine applies penalty and bonus rules to calculate a final catering potential score for each prospect.

**Key Features:**
- **Penalty Rules**: Subtract points for negative indicators (missing data, poor ratings)
- **Bonus Rules**: Add points for positive indicators (verified businesses, high ratings)
- **Customizable**: Adjust point values with sliders, enable/disable rules with checkboxes
- **Persistent**: Rules are saved to PostgreSQL via ApiLogicServer and applied automatically

For a complete list of available scoring rules and configuration options, see [Settings > Tab 5: Scoring Rules](#tab-5-scoring-rules).

#### Performance Optimizations
- **Bounding box queries**: Uses bbox instead of radius `around:` queries (5-10x faster)
- **Faster Overpass endpoint**: Uses lz4.overpass-api.de mirror with compression
- **Combined node/way queries**: Uses `nw` shorthand to halve the number of query clauses
- **Quadtile output sorting**: Uses `qt` modifier for optimized result ordering
- **Compression enabled**: All HTTP requests accept gzip/deflate for faster transfer
- **TCP optimizations**: TCP_NODELAY and TCP_KEEPALIVE for responsive connections
- **Multi-threaded geocoding**: Configurable thread pool for parallel address resolution
- **Google APIs integration**: Leverages Google Geocoding and Places APIs when configured
- **Intelligent caching**: 24-hour cache reduces redundant API calls
- **Aggressive timeouts**: Fast-fail on slow connections to provide quick feedback

#### API Timeout Configuration

All external API calls use aggressive timeout settings for responsive feedback:

| API Service | Connection Timeout | Request Timeout | Notes |
|-------------|-------------------|-----------------|-------|
| OpenStreetMap Overpass | 3s | 8s | Uses lz4 mirror with compression |
| Nominatim Geocoding | 3s | 8s | Fast-fail on network issues |
| Geocoding Service | 2s | 5s | Geocoding is typically fast |
| Google Places | 3s | 8s | Reliable with connection timeout |
| Google Geocoding | 3s | 5s | Fast responses expected |

#### Error Handling & Fallbacks

The search system includes robust error handling to ensure results even when some APIs fail:

1. **Geocoding Fallback Chain**:
   - Try Google Geocoding (if API key configured)
   - Fall back to Nominatim (OpenStreetMap)
   - Fall back to Denver, CO as default location

2. **Coordinate Validation**:
   - Rejects invalid (0,0) coordinates before searching
   - Validates coordinate ranges (-90 to 90 lat, -180 to 180 lon)

3. **API Error Detection**:
   - Properly detects and reports Overpass API errors/timeouts
   - Extracts error messages from API responses
   - Prevents silent failures from empty results

4. **Known Locations Cache**:
   - Common US cities geocoded locally (instant results)
   - Includes: New York, Los Angeles, Chicago, Houston, Phoenix, Seattle, Denver, etc.

### My Prospects
View and manage saved prospects with AI-powered analysis.

#### Two-Column Grid Layout
The My Prospects page displays saved prospects in a responsive two-column grid:
- **Desktop (>1200px)**: Two prospect cards side-by-side per row
- **Tablet/Mobile (<1200px)**: Single column layout for readability

#### Prospect Card Structure
Each prospect card features a professional layout with distinct sections:

**Card Header:**
- **Business Type Icon**: Greyscale emoji representing the business type (🏢 Corporate, 🏨 Hotel, 🏭 Warehouse, etc.)
- **Business Name**: Primary heading with company name
- **Address**: Full formatted address below the name
- **AI Score Bubble**: Color-coded clickable score badge in the top-right corner

**AI Score Popover:**
Clicking the AI Score bubble reveals a details popover showing:
- **Optimized Score**: The final adjusted score with applied rules
- **Original Score**: The base score before rule adjustments
- **Applied Rules**: When scores differ, explains which rules caused the change (e.g., "Large workforce (+10), Conference facilities (+5)")

**Demographics Section (👥):**
- Section header with icon and "DEMOGRAPHICS" label
- Stat badges for: Business type, Employee count, Google rating
- Feature badges: Conference Room, Event Space, BBB Accredited

**Data Sources Section (📊):**
- Section header with icon and "DATA SOURCES" label
- Source badges showing where data originated (OpenStreetMap, Google, BBB, etc.)

**Recommended Actions:**
- Collapsible yellow box with exposure triangle (▶/▼)
- Shows action count in header (e.g., "Recommended Actions (3)")
- Numbered list of AI-generated recommended actions when expanded
- Compact styling when collapsed for minimal visual footprint

**Card Footer:**
- Remove button to delete prospect from saved list

#### Score Badge Colors
| Score Range | Color | Meaning |
|-------------|-------|---------|
| 80-100 | Green | Excellent potential |
| 60-79 | Yellow/Orange | Good potential |
| 40-59 | Orange | Fair potential |
| 0-39 | Red | Lower potential |

#### Business Type Icons
| Icon | Business Type |
|------|---------------|
| 🏢 | Corporate Office |
| 🏭 | Warehouse / Manufacturing |
| 🏛️ | Conference Center / Government |
| 🏨 | Hotel |
| 💼 | Coworking Space |
| 🏥 | Medical Facility |
| 🎓 | Educational Institution |
| 💻 | Tech Company |
| 🏦 | Financial Services |
| ⚖️ | Law Firm |
| ❤️ | Non-Profit |

#### Prospect-Franchisee Linking
Saved prospects are **linked to the current franchisee/store location**:

**How It Works:**
1. When you save a prospect, it's linked to your current store via `store_location_id`
2. Prospects are persisted to ApiLogicServer (PostgreSQL database)
3. When you switch to a different store, prospects are reloaded for that store
4. Each franchisee sees only their own saved prospects

**Data Persistence:**
- **SavedProspectDTO** stores full business details, scores, and AI analysis
- Prospects survive application restarts
- Delete action removes from both in-memory list and database

**Synchronization Flow:**
```
Add to Prospects → saveProspectToALS() → POST /api/SavedProspect
                                              ↓
Switch Store → selectStoreById() → loadProspectsFromALS()
                                              ↓
                                   GET /api/SavedProspect?filter[store_location_id]=...
```

#### Non-Blocking Add to Prospects Workflow
When adding prospects, FranchiseAI uses a **non-blocking toast notification** system:

**How It Works:**
1. Click "+ Add to Prospects" on any search result
2. A floating toast notification appears in the corner showing:
   - Business name and score badge
   - Brief AI analysis excerpt
   - Optional "View" link to My Prospects
3. Toast auto-fades after 6 seconds
4. Multiple toasts stack if adding prospects rapidly

**Benefits:**
- **Uninterrupted Flow**: Continue browsing and adding prospects without modal dialogs blocking interaction
- **Visual Confirmation**: Clear feedback that prospect was saved successfully
- **Efficient Bulk Discovery**: Add multiple prospects in quick succession without clicking "OK" repeatedly

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
The Settings page uses a modern tab navigation with five sections:

| Tab | Purpose |
|-----|---------|
| **Franchisee** | Configure your franchise store, contact details, and select from saved stores |
| **Marketing** | Configure target business types, employee ranges, and search preferences |
| **AI Configuration** | Set up OpenAI or Gemini for prospect analysis |
| **Data Sources** | Configure API keys for business data providers |
| **Scoring Rules** | Customize prospect scoring penalties and bonuses |

#### Tab 1: Franchisee Information
Configure your franchise store details and contact information.

**Store Selection:**
- **Store Dropdown**: Select from previously saved stores or create a new one
  - "-- New Store --" option to create a new store location
  - Selecting an existing store populates all fields automatically
  - Store data is loaded from ApiLogicServer

**Store Information:**
- **Store Name**: Your franchise location name (e.g., "Vocelli Pizza - Downtown")
- **Street Address**: Street address line with optional suite/apt number (e.g., "123 Main St, Suite 200")
- **City**: City name (e.g., "Denver")
- **State**: State dropdown with full state names displayed (stores state code, e.g., "Colorado" → "CO")
- **Zip Code**: Postal code (e.g., "80202")
- **Owner/Manager Name**: Contact person for the franchise
- **Store Phone**: Business contact number

**Address Layout:**
- Street Address occupies its own full-width row for longer addresses with suite/apartment numbers
- City, State, and Zip are displayed on a second row in a standard address format
- State uses a dropdown with all 50 US states showing full names (displays "California" but stores "CA")

**Address Handling:**
- Separate fields for Street, City, State, and Zip ensure accurate data storage
- Full address is automatically assembled for geocoding (e.g., "123 Main St, Denver, CO 80202")
- Individual fields are persisted to ApiLogicServer for proper database storage

**Synchronization:**
When a store is selected or saved:
- Sidebar header updates with store name and location
- Sidebar popover updates with full contact details (address, phone, email)
- AI Search uses the store location as the default search center
- Open Street Map centers on the store location
- All three views display the same formatted address via `getFullAddress()`

#### Tab 2: Marketing
Configure your target market preferences for AI Search. These settings are automatically applied to all searches.

**Search Preferences:**
- **Default Search Radius**: Miles from your store to search (saved to `franchisee_.defaultSearchRadiusMiles`)

**Target Business Types:**
Checkboxes for 12 business categories to target:
- Corporate Offices, Conference Centers, Hotels
- Medical Facilities, Educational Institutions, Manufacturing/Industrial
- Warehouses/Distribution, Government Offices, Tech Companies
- Financial Services, Coworking Spaces, Non-profits

**Saved Preferences:** When the Settings page loads, checkboxes are initialized from your saved `franchisee_.searchCriteria.businessTypes`. If no preferences are saved, sensible defaults are used.

**Target Organization Size:**
Dropdown to filter by employee count:
- Any Size
- Small (1-50 employees)
- Medium (51-200 employees)
- Large (201-500 employees)
- Enterprise (500+ employees)

**Saved Preferences:** The dropdown initializes to your saved `franchisee_.searchCriteria.minEmployees` and `maxEmployees` values.

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

#### Tab 5: Scoring Rules
Customize how prospects are scored to match your business priorities.

**Two-Panel Layout:**
The Scoring Rules tab uses a side-by-side GridBag layout with two distinct panels:

| Panel | Header Icon | Description |
|-------|-------------|-------------|
| **Penalties** | ↓ (white down arrow in red circle) | Rules that subtract points for negative indicators |
| **Bonuses** | ↑ (white up arrow in green circle) | Rules that add points for positive indicators |

**Penalty Rules (Left Panel):**
Adjust point deductions for missing or incomplete data:
- **Missing Address** (default -15): No street address available
- **Missing Phone Number** (default -10): No phone number listed
- **Missing Website** (default -5): No website listed
- **Potentially Closed** (default -25): Business shows signs of being inactive
- **Incomplete Hours** (default -5): Business hours not fully listed
- **Few Reviews** (default -8): Very few customer reviews
- **Poor Rating** (default -12): Below average customer rating

**Bonus Rules (Right Panel):**
Adjust point bonuses for desirable attributes:
- **Verified Business** (default +10): Confirmed accurate information
- **Complete Profile** (default +8): All contact information listed
- **Many Reviews** (default +12): Substantial number of reviews
- **Excellent Rating** (default +15): Above average customer rating
- **Established Business** (default +10): Operating for multiple years
- **Active Online Presence** (default +7): Actively maintains online profiles
- **Ideal Employee Count** (default +10): Matches target employee range
- **Target Industry** (default +15): Operates in preferred industry category

**Controls:**
- **Sliders**: Drag to adjust point values (-50 to 0 for penalties, 0 to +50 for bonuses)
- **Checkboxes**: Enable or disable individual rules
- **Persistence**: Rules are saved to PostgreSQL via ApiLogicServer and persist across sessions
- **Taglines**: Each rule displays its description below the rule name

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
    // Uses lz4 mirror - faster response with compression
    std::string overpassEndpoint = "https://lz4.overpass-api.de/api/interpreter";
    std::string nominatimEndpoint = "https://nominatim.openstreetmap.org";
    int requestTimeoutMs = 8000;        // 8 seconds - bbox queries are fast
    int connectTimeoutMs = 3000;        // 3 seconds connection timeout
    bool enableCaching = true;
    int cacheDurationMinutes = 1440;    // 24 hours - OSM data is relatively static
    int maxResultsPerQuery = 50;        // Limit results for faster response
    std::string userAgent = "FranchiseAI/1.0";
};
```

**Overpass Query Optimization:**

The Overpass API query has been heavily optimized for faster results:

| Optimization | Before | After | Impact |
|--------------|--------|-------|--------|
| Query type | `around:` radius | `bbox` bounding box | 5-10x faster |
| Endpoint | overpass-api.de | lz4.overpass-api.de | Compressed responses |
| Node/Way queries | Separate (10 clauses) | Combined `nw` (5 clauses) | 50% fewer clauses |
| Output format | `out center` | `out center qt` | Quadtile-sorted, faster |
| Query timeout | 10s | 6s | Faster failure feedback |
| Compression | None | gzip, deflate | Smaller payload |
| TCP options | Default | NODELAY + KEEPALIVE | Lower latency |

**Focused on high-value catering prospects:**
- Named offices (`office` + `name`)
- Company/Corporation offices
- Hotels
- Conference centers
- Hospitals
- Universities and colleges

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

#### Synchronization Across Views

The application maintains a single `franchisee_` member variable that is synchronized across all views:

**Settings Page → Sidebar:**
- Saving settings calls `updateHeaderWithFranchisee()` which updates both the header and popover
- Store selection via dropdown immediately syncs all franchisee data

**Sidebar Popover:**
- Displays franchisee contact details (address, phone, email)
- "Edit Profile" action navigates to Settings page
- Details auto-update when franchisee data changes

**AI Search Page:**
- Uses `franchisee_.getFullAddress()` as default search location
- Uses `franchisee_.searchCriteria` for business types and employee ranges
- Shows franchisee badge with "Change" button to navigate to Settings

**Open Street Map Page:**
- Centers map on `franchisee_.location` coordinates
- Uses `franchisee_.searchCriteria.radiusMiles` for default search radius
- Location input shows `franchisee_.getFullAddress()` (full formatted address)
- Shows red pin marker at franchisee location with popup details

**Location Synchronization:**
When the franchisee location changes (Settings save, store selection, or app startup):
- `currentSearchLocation_` is updated with the full formatted address
- `currentSearchArea_` is updated with the franchisee's search area
- All three views (Settings, AI Search, Open Street Map) display consistent location

**getFullAddress() Method:**
Returns a properly formatted address string combining:
- Street address + City + State + Zip Code
- Example: "123 Main St, Denver, CO 80202"

**Synchronization Flow:**
```
Settings Save → franchisee_ updated → updateHeaderWithFranchisee()
                                            ↓
                                    sidebar_->setUserInfo()
                                    sidebar_->setFranchiseDetails()
                                            ↓
                              currentSearchLocation_ = getFullAddress()
                              currentSearchArea_ = createSearchArea()
                                            ↓
                            All views read from franchisee_ on render
```

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

### Saved Prospect Management

Saved prospects are linked to store locations and persist to the database.

**SavedProspectDTO Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `id` | UUID | Unique identifier (auto-generated for new) |
| `storeLocationId` | UUID | Link to parent store location |
| `businessName` | string | Prospect business name |
| `businessCategory` | string | Business type/category |
| `addressLine1/2` | string | Business address |
| `city/stateProvince/postalCode` | string | Location details |
| `latitude/longitude` | double | Geocoded coordinates |
| `phone/email/website` | string | Contact information |
| `employeeCount` | int | Number of employees |
| `cateringPotentialScore` | int | Calculated catering score |
| `relevanceScore` | double | Search relevance score |
| `distanceMiles` | double | Distance from franchisee |
| `aiSummary` | string | AI-generated analysis summary |
| `matchReason` | string | Why this prospect matches criteria |
| `keyHighlights` | string | Pipe-separated list of highlights |
| `recommendedActions` | string | Pipe-separated list of actions |
| `dataSource` | string | Where the data came from (OpenStreetMap, etc.) |
| `savedAt` | string | ISO timestamp when saved |
| `isContacted` | bool | Has been contacted |
| `isConverted` | bool | Has converted to customer |
| `notes` | string | User notes |

**API Operations:**
```cpp
// Save new prospect (UUID auto-generated)
SavedProspectDTO dto = prospectItemToDTO(item);
dto.storeLocationId = currentStoreLocationId_;
alsClient->saveProspect(dto);
// POST /api/SavedProspect

// Load prospects for current store
auto response = alsClient->getProspectsForStore(currentStoreLocationId_);
// GET /api/SavedProspect?filter[store_location_id]=...

// Delete prospect
alsClient->deleteSavedProspect(prospectId);
// DELETE /api/SavedProspect/{id}
```

**Prospect Loading Flow:**
```
App Startup → loadStoreLocationFromALS() → loadProspectsFromALS()
                                                    ↓
Store Switch → selectStoreById() → loadProspectsFromALS()
                                                    ↓
                              savedProspects_ populated with store's prospects
```

### API Request Pattern (POST/PATCH)

Save operations use **POST for creating new records** and **PATCH for updating existing records** with client-generated UUIDs:

```cpp
// All save methods follow this pattern:
ApiResponse saveFranchisee(const FranchiseeDTO& franchisee) {
    FranchiseeDTO dto = franchisee;
    if (dto.id.empty()) {
        dto.id = generateUUID();  // Client-side UUID generation
        std::string json = dto.toJson();
        return httpPost("/Franchisee", json);  // POST for new records
    } else {
        std::string json = dto.toJson();
        return httpPatch("/Franchisee/" + dto.id, json);  // PATCH for updates
    }
}
```

**HTTP Methods:**
- **POST** to collection endpoint (`/Franchisee`) for creating new records
- **PATCH** to resource endpoint (`/Franchisee/{id}`) for updating existing records
- Client generates UUID before POST to ensure the client knows the ID immediately

**Affected Endpoints:**

| Operation | Method | Endpoint |
|-----------|--------|----------|
| Create franchisee | `POST` | `/Franchisee` |
| Update franchisee | `PATCH` | `/Franchisee/{id}` |
| Create store location | `POST` | `/StoreLocation` |
| Update store location | `PATCH` | `/StoreLocation/{id}` |
| Create scoring rule | `POST` | `/ScoringRule` |
| Update scoring rule | `PATCH` | `/ScoringRule/{id}` |
| Create saved prospect | `POST` | `/SavedProspect` |
| Update saved prospect | `PATCH` | `/SavedProspect/{id}` |

### Client-Side UUID Generation

For new records (Franchisee and StoreLocation), UUIDs are generated client-side before sending to ALS. This ensures the client knows the ID immediately and avoids issues with PostgreSQL UUID generation.

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

### Scoring Rules API

The Scoring Rules system allows franchisees to customize prospect scoring. Rules are stored in the database and accessed via the `/ScoringRule` API endpoint.

**Naming Convention:**
| Layer | Name | Notes |
|-------|------|-------|
| API Endpoint | `/ScoringRule` | Singular PascalCase for JSON:API |
| JSON Type | `"type": "ScoringRule"` | Matches endpoint name |
| Database Table | `scoring_rules` | snake_case for PostgreSQL |

**ScoringRuleDTO Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `id` | UUID | Unique identifier (client-generated) |
| `ruleId` | string | Rule identifier (e.g., 'no_address', 'verified_business') |
| `name` | string | Display name |
| `description` | string | Rule description |
| `isPenalty` | bool | True for penalties, false for bonuses |
| `enabled` | bool | Whether rule is active |
| `defaultPoints` | int | Default point adjustment |
| `currentPoints` | int | Current configured adjustment |
| `minPoints` | int | Minimum allowed value (default -50) |
| `maxPoints` | int | Maximum allowed value (default 50) |
| `franchiseeId` | UUID | NULL = global rule, otherwise franchisee-specific |

**API Operations:**

```cpp
// Create scoring rule (new)
ScoringRuleDTO rule;
rule.ruleId = "no_address";
rule.name = "Missing Address";
rule.isPenalty = true;
rule.currentPoints = -15;
alsClient->saveScoringRule(rule);
// POST /api/ScoringRule (for new records)

// Update existing scoring rule
rule.id = "existing-uuid";
alsClient->saveScoringRule(rule);
// PATCH /api/ScoringRule/{id} (for updates)

// Get all rules
auto response = alsClient->getScoringRules();
// GET /api/ScoringRule

// Get rules for specific franchisee
auto response = alsClient->getScoringRulesForFranchisee(franchiseeId);
// GET /api/ScoringRule?filter[franchisee_id]=...

// Delete rule
alsClient->deleteScoringRule(ruleId);
// DELETE /api/ScoringRule/{id}
```

**Database Schema (PostgreSQL):**

```sql
CREATE TABLE scoring_rules (
    id                  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    rule_id             VARCHAR(50) NOT NULL,
    name                VARCHAR(100) NOT NULL,
    description         TEXT,
    is_penalty          BOOLEAN NOT NULL DEFAULT FALSE,
    enabled             BOOLEAN NOT NULL DEFAULT TRUE,
    default_points      INTEGER NOT NULL DEFAULT 0,
    current_points      INTEGER NOT NULL DEFAULT 0,
    min_points          INTEGER NOT NULL DEFAULT -50,
    max_points          INTEGER NOT NULL DEFAULT 50,
    franchisee_id       UUID,
    created_at          TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at          TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT fk_scoring_rules_franchisee
        FOREIGN KEY (franchisee_id) REFERENCES franchisees(id) ON DELETE CASCADE,
    CONSTRAINT uq_scoring_rules_rule_franchisee
        UNIQUE (rule_id, franchisee_id)
);

-- Indexes
CREATE INDEX idx_scoring_rules_franchisee_id ON scoring_rules(franchisee_id);
CREATE INDEX idx_scoring_rules_enabled ON scoring_rules(enabled) WHERE enabled = TRUE;
CREATE INDEX idx_scoring_rules_rule_id ON scoring_rules(rule_id);
```

**Default Rules (Sample Data):**

| Rule ID | Name | Type | Default Points |
|---------|------|------|----------------|
| `no_address` | Missing Address | Penalty | -15 |
| `no_phone` | Missing Phone Number | Penalty | -10 |
| `no_website` | Missing Website | Penalty | -5 |
| `closed_business` | Potentially Closed | Penalty | -25 |
| `verified_business` | Verified Business | Bonus | +10 |
| `complete_profile` | Complete Profile | Bonus | +8 |
| `high_reviews` | Many Reviews | Bonus | +12 |
| `excellent_rating` | Excellent Rating | Bonus | +15 |
| `target_industry` | Target Industry | Bonus | +15 |

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

## Deployment

For complete deployment instructions including production setup, database configuration, Nginx reverse proxy, SSL, systemd services, monitoring, backup procedures, and repository migration, see the **[Deployment Guide](docs/DEPLOYMENT_GUIDE.md)**.

## License

MIT License - See LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request
