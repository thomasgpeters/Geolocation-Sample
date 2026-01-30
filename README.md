# FranchiseAI - Prospect Search Application

A modular C++ application using the Wt (Witty) web framework that enables franchise owners to discover potential catering clients through AI-powered search across multiple data sources.

## Features

- **AI-Powered Search**: Intelligent search that aggregates data from multiple sources
- **Multi-Source Data Integration**:
  - Google My Business - Business listings and ratings
  - Better Business Bureau (BBB) - Accreditation and complaint data
  - Demographics Data - Population, income, and business statistics
  - OpenStreetMap - Free, open-source geolocation and POI data
- **Smart Scoring**: AI-driven catering potential scoring for each prospect
- **Modern UI**: Responsive sidebar navigation with a professional dashboard
- **Prospect Management**: Save, export, and manage discovered prospects

## Target Market

The application helps franchise owners find potential clients interested in catering services for:
- Corporate offices and conference rooms
- Warehouses and distribution centers
- Coworking spaces
- Hotels and conference centers
- Manufacturing facilities
- Any business with regular meeting and event catering needs

## Project Structure

```
├── CMakeLists.txt              # Build configuration
├── src/
│   ├── main.cpp                # Application entry point
│   ├── FranchiseApp.cpp/h      # Main Wt application class
│   ├── models/                 # Data models
│   │   ├── BusinessInfo.cpp/h      # Business data model
│   │   ├── DemographicData.h       # Demographics data model
│   │   └── SearchResult.cpp/h      # Search result models
│   ├── services/               # API services
│   │   ├── AISearchService.cpp/h       # AI search aggregator
│   │   ├── GoogleMyBusinessAPI.cpp/h   # Google Places API
│   │   ├── BBBAPI.cpp/h               # BBB API service
│   │   ├── DemographicsAPI.cpp/h      # Demographics API
│   │   └── OpenStreetMapAPI.cpp/h     # OpenStreetMap/Overpass API
│   └── widgets/                # UI components
│       ├── Sidebar.cpp/h           # Navigation sidebar
│       ├── Navigation.cpp/h        # Top navigation bar
│       ├── SearchPanel.cpp/h       # Search form panel
│       ├── ResultCard.cpp/h        # Individual result card
│       └── ResultsDisplay.cpp/h    # Results container
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

### Dashboard
Overview of prospect discovery statistics with quick action buttons.

### AI Search
Main search interface with:
- Location input (city, state, ZIP code)
- Search radius slider
- Business type filters
- Data source selection
- Advanced sorting options

### My Prospects
Manage saved prospects (placeholder for future implementation).

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
Configure API keys and preferences (placeholder).

## API Configuration

For production use, configure your API keys in the settings page or via environment variables:

```bash
export GOOGLE_API_KEY="your-google-api-key"
export BBB_API_KEY="your-bbb-api-key"
export CENSUS_API_KEY="your-census-api-key"
```

Note: The application includes demo data generation for testing without API keys.

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

## Architecture

### Models Layer
- `BusinessInfo`: Comprehensive business data model with contact info, ratings, and catering potential scoring
- `DemographicData`: Area demographics including population, income, and business statistics
- `SearchResult`: Container for search results with AI analysis

### Services Layer
- `AISearchService`: Orchestrates searches across all data sources and performs AI analysis
- `GoogleMyBusinessAPI`: Interface to Google Places API
- `BBBAPI`: Interface to Better Business Bureau API
- `DemographicsAPI`: Interface to Census and economic data APIs
- `OpenStreetMapAPI`: Interface to OpenStreetMap Overpass API for geolocation and POI data

### UI Layer (Wt Widgets)
- `Sidebar`: Main navigation with collapsible menu
- `Navigation`: Top bar with quick search and user actions
- `SearchPanel`: Form for configuring search parameters
- `ResultsDisplay`: Container for displaying search results
- `ResultCard`: Individual prospect card with metrics and actions

## License

MIT License - See LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request
