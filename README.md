# FranchiseAI - Prospect Search Application

A modular C++ application using the Wt (Witty) web framework that enables franchise owners to discover potential catering clients through AI-powered search across multiple data sources.

## Features

- **AI-Powered Search**: Intelligent search that aggregates data from multiple sources
- **Multi-Source Data Integration**:
  - Google My Business - Business listings and ratings
  - Better Business Bureau (BBB) - Accreditation and complaint data
  - Demographics Data - Population, income, and business statistics
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
│   │   └── DemographicsAPI.cpp/h      # Demographics API
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
Explore demographic data and market potential (placeholder).

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
