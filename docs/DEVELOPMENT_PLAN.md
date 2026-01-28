# FranchiseAI Development Plan

This document tracks pending features and their implementation status for the FranchiseAI Prospect Search Application.

---

## Table of Contents

1. [Overview](#overview)
2. [Priority Legend](#priority-legend)
3. [Critical - Unimplemented Methods](#critical---unimplemented-methods)
4. [High Priority - API Integrations](#high-priority---api-integrations)
5. [High Priority - UI Pages](#high-priority---ui-pages)
6. [Medium Priority - Event Handlers](#medium-priority---event-handlers)
7. [Medium Priority - Cache Management](#medium-priority---cache-management)
8. [Low Priority - UI Components](#low-priority---ui-components)
9. [Completed Features](#completed-features)

---

## Overview

| Priority | Category | Count | Status |
|----------|----------|-------|--------|
| CRITICAL | Unimplemented methods | 2 | Pending |
| HIGH | Demo data APIs | 16 | Pending |
| HIGH | Placeholder UI pages | 4 | Pending |
| MEDIUM | Stub event handlers | 3 | Pending |
| MEDIUM | Empty cache methods | 5 | Pending |
| LOW | Incomplete UI components | 1 | Pending |

**Total Pending Features: 31**

---

## Priority Legend

| Priority | Description |
|----------|-------------|
| CRITICAL | Blocks core functionality; must be implemented for production |
| HIGH | Essential for full feature set; needed before release |
| MEDIUM | Improves user experience; can be deferred |
| LOW | Nice to have; minimal impact if delayed |

---

## Critical - Unimplemented Methods

These methods are declared in headers but have **no implementation** in the source files.

### GoogleMyBusinessAPI JSON Parsing

| Method | File | Line | Status | Description |
|--------|------|------|--------|-------------|
| `parseBusinessFromJson()` | `src/services/GoogleMyBusinessAPI.h` | 118 | **NOT IMPLEMENTED** | Parse single business from Google Places API JSON response |
| `parseSearchResults()` | `src/services/GoogleMyBusinessAPI.h` | 119 | **NOT IMPLEMENTED** | Parse search results array from Google Places API response |

#### Implementation Notes
```cpp
// Expected signature (from header):
Models::BusinessInfo parseBusinessFromJson(const std::string& json);
std::vector<Models::BusinessInfo> parseSearchResults(const std::string& json);
```

**Dependencies**: Required for real Google Places API integration.

---

## High Priority - API Integrations

All API services currently return demo/mock data instead of making real API calls.

### GoogleMyBusinessAPI

**File**: `src/services/GoogleMyBusinessAPI.cpp`

| Method | Lines | Current State | Required Implementation |
|--------|-------|---------------|------------------------|
| `searchBusinesses()` | 25-38 | Returns fake business data | Integrate with Google Places Nearby Search API |
| `getPlaceDetails()` | 56-71 | Minimal placeholder object | Integrate with Google Places Details API |
| `getAutocomplete()` | 87-102 | Hardcoded suggestions | Integrate with Google Places Autocomplete API |

#### API Endpoints Needed
- Nearby Search: `https://maps.googleapis.com/maps/api/place/nearbysearch/json`
- Place Details: `https://maps.googleapis.com/maps/api/place/details/json`
- Autocomplete: `https://maps.googleapis.com/maps/api/place/autocomplete/json`

#### Required API Key
- Google Cloud Platform API key with Places API enabled

---

### BBBAPI (Better Business Bureau)

**File**: `src/services/BBBAPI.cpp`

| Method | Lines | Current State | Required Implementation |
|--------|-------|---------------|------------------------|
| `searchBusinesses()` | 23-31 | Generates fake BBB results | Integrate with BBB API |
| `searchByName()` | 33-45 | Routes to demo generation | Implement name-based search |
| `searchAccreditedBusinesses()` | 47-57 | Fake accredited businesses | Filter by accreditation status |
| `getBusinessProfile()` | 59-72 | Minimal placeholder | Fetch complete business profile |
| `getComplaintHistory()` | 74-82 | Fake complaint data | Retrieve actual complaint records |
| `checkAccreditation()` | 84-91 | Always returns `true` | Verify actual BBB accreditation |

#### API Endpoints Needed
- BBB API: `https://api.bbb.org/api` (requires partnership/license)

#### Notes
- BBB API access may require business partnership agreement
- Consider fallback to web scraping if API unavailable

---

### DemographicsAPI

**File**: `src/services/DemographicsAPI.cpp`

| Method | Lines | Current State | Required Implementation |
|--------|-------|---------------|------------------------|
| `getByZipCode()` | 24-32 | Demo data generation | Integrate with Census Bureau API |
| `getByCity()` | 34-67 | Random generated data | Integrate with Census Bureau API |
| `getMultipleZipCodes()` | 69-83 | Uses `generateDemoData` | Batch Census API requests |
| `getZipCodesInRadius()` | 85-97 | Demo area data | Geographic radius calculation + Census data |
| `findHighPotentialAreas()` | 99-125 | Filters demo data only | Algorithm using real demographic data |
| `getBusinessDensity()` | 127-135 | Returns demo data | Integrate with Census Business Patterns |
| `getEmploymentBySector()` | 137-146 | Random employment data | Integrate with BLS API |

#### API Endpoints Needed
- Census Bureau: `https://api.census.gov/data`
- Bureau of Labor Statistics: `https://api.bls.gov/publicAPI/v2/timeseries/data`
- ZIP Code Database: Consider ZIPCodeAPI or similar service

#### Required API Keys
- Census Bureau API key (free)
- BLS API key (free, optional but recommended)

---

## High Priority - UI Pages

These pages show placeholder content instead of functional interfaces.

### My Prospects Page

**File**: `src/FranchiseApp.cpp`
**Method**: `showProspectsPage()` (Lines 358-388)

| Current State | Required Implementation |
|---------------|------------------------|
| Shows empty state message only | Full prospect management system |

#### Features Needed
- [ ] Prospect list display with sorting/filtering
- [ ] Prospect detail view
- [ ] Add/remove prospect functionality
- [ ] Prospect status tracking (New, Contacted, Follow-up, Converted)
- [ ] Notes and activity log per prospect
- [ ] Persistent storage (database or file-based)
- [ ] Search within saved prospects

---

### Demographics Page

**File**: `src/FranchiseApp.cpp`
**Method**: `showDemographicsPage()` (Lines 390-419)

| Current State | Required Implementation |
|---------------|------------------------|
| Shows "Coming Soon" message | Demographics visualization dashboard |

#### Features Needed
- [ ] Interactive map visualization
- [ ] Demographic data overlays (income, population, business density)
- [ ] Area comparison tool
- [ ] Market potential scoring visualization
- [ ] Export demographic reports
- [ ] Filter by demographic criteria

#### Technical Considerations
- Consider integrating Leaflet.js or Google Maps for mapping
- Wt has WGoogleMap widget available

---

### Reports Page

**File**: `src/FranchiseApp.cpp`
**Method**: `showReportsPage()` (Lines 421-445)

| Current State | Required Implementation |
|---------------|------------------------|
| Shows empty placeholder | Analytics and reporting dashboard |

#### Features Needed
- [ ] Search activity analytics
- [ ] Prospect conversion metrics
- [ ] Geographic coverage reports
- [ ] Time-based activity charts
- [ ] CSV/PDF export functionality
- [ ] Scheduled report generation

---

### Settings Page

**File**: `src/FranchiseApp.cpp`
**Method**: `showSettingsPage()` (Lines 447-494)

| Current State | Required Implementation |
|---------------|------------------------|
| Shows section titles only, no inputs | Functional settings management |

#### Features Needed
- [ ] API key input fields (Google, BBB, Census, OpenAI, Gemini)
- [ ] API key validation/testing
- [ ] Search preference settings (default radius, result limits)
- [ ] Franchise profile configuration
- [ ] AI provider selection (OpenAI vs Gemini)
- [ ] Notification preferences
- [ ] Data export/import settings
- [ ] Settings persistence (config file or database)

---

## Medium Priority - Event Handlers

These handlers show message boxes instead of performing actual operations.

### View Details Handler

**File**: `src/FranchiseApp.cpp`
**Method**: `onViewDetails()` (Lines 208-223)

| Current State | Required Implementation |
|---------------|------------------------|
| Shows message box with basic info | Full business detail modal/panel |

#### Features Needed
- [ ] Modal dialog with complete business information
- [ ] Contact information display
- [ ] Google Maps embed for location
- [ ] BBB rating details
- [ ] AI analysis breakdown
- [ ] Quick actions (add to prospects, export, share)

---

### Add to Prospects Handler

**File**: `src/FranchiseApp.cpp`
**Method**: `onAddToProspects()` (Lines 225-240)

| Current State | Required Implementation |
|---------------|------------------------|
| Shows confirmation dialog only | Actual prospect storage |

#### Features Needed
- [ ] Save prospect to persistent storage
- [ ] Duplicate detection
- [ ] Add notes on save
- [ ] Categorization/tagging
- [ ] Success/error feedback
- [ ] Undo capability

---

### Export Results Handler

**File**: `src/FranchiseApp.cpp`
**Method**: `onExportResults()` (Lines 242-251)

| Current State | Required Implementation |
|---------------|------------------------|
| Shows message about export | Actual file generation and download |

#### Features Needed
- [ ] CSV export with all business fields
- [ ] PDF report generation
- [ ] Excel format support
- [ ] Custom field selection
- [ ] File download trigger
- [ ] Export history tracking

---

## Medium Priority - Cache Management

All cache methods are empty stubs that don't perform actual caching operations.

### Cache Methods Status

| Service | Method | File | Lines | Status |
|---------|--------|------|-------|--------|
| GoogleMyBusinessAPI | `clearCache()` | GoogleMyBusinessAPI.cpp | 116-118 | Empty body |
| GoogleMyBusinessAPI | `getCacheSize()` | GoogleMyBusinessAPI.cpp | 120-122 | Returns hardcoded 0 |
| BBBAPI | `clearCache()` | BBBAPI.cpp | 103-105 | Empty body |
| DemographicsAPI | `clearCache()` | DemographicsAPI.cpp | 159-161 | Empty body |
| DemographicsAPI | `getCacheSize()` | DemographicsAPI.cpp | 163-165 | Returns hardcoded 0 |

#### Implementation Notes
- Consider using `std::unordered_map` with timestamp for TTL-based caching
- Cache configuration already defined in API config structs
- AI engines already have caching implemented - can use as reference

---

## Low Priority - UI Components

### ResultCard Update Method

**File**: `src/widgets/ResultCard.cpp`
**Method**: `updateData()` (Line 326)

| Current State | Required Implementation |
|---------------|------------------------|
| Updates data member only | Full UI rebuild on data change |

```cpp
// Current implementation:
void ResultCard::updateData(const Models::SearchResultItem& item) {
    item_ = item;
    // Would need to rebuild UI - simplified for now
}
```

#### Features Needed
- [ ] Rebuild all UI elements with new data
- [ ] Animate transitions if data changes
- [ ] Handle null/empty data gracefully

---

## Completed Features

### AI Engine Integration (Completed)

| Feature | Status | Date |
|---------|--------|------|
| AIEngine abstract interface | Done | 2024-01 |
| OpenAI GPT integration | Done | 2024-01 |
| Google Gemini integration | Done | 2024-01 |
| AI-powered business analysis | Done | 2024-01 |
| AI-powered market analysis | Done | 2024-01 |
| Response caching for AI engines | Done | 2024-01 |
| Fallback to local analysis | Done | 2024-01 |

### Core Application (Completed)

| Feature | Status |
|---------|--------|
| Wt web framework integration | Done |
| Sidebar navigation | Done |
| Search panel with filters | Done |
| Results display grid | Done |
| Result cards with scoring | Done |
| Multi-source data aggregation | Done |
| Business scoring algorithm | Done |

---

## Development Notes

### Environment Setup

Required dependencies:
- CMake 3.16+
- C++17 compiler
- Wt 4.x library
- libcurl (for HTTP requests)

### API Keys Required for Full Functionality

| Service | Environment Variable | Required For |
|---------|---------------------|--------------|
| Google Places | `GOOGLE_API_KEY` | Business search, autocomplete |
| BBB | `BBB_API_KEY` | Accreditation data |
| Census Bureau | `CENSUS_API_KEY` | Demographics |
| OpenAI | `OPENAI_API_KEY` | AI analysis (GPT models) |
| Google Gemini | `GEMINI_API_KEY` | AI analysis (Gemini models) |

### Build Instructions

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./franchise_ai_search --docroot ./resources --http-address 0.0.0.0 --http-port 8080
```

---

## Contributing

When implementing pending features:

1. Update this document to mark features as "In Progress"
2. Follow existing code patterns and naming conventions
3. Add appropriate error handling
4. Update tests if applicable
5. Mark as "Done" with completion date when finished

---

*Last Updated: January 2025*
