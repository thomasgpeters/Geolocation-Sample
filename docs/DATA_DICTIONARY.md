# FranchiseAI Data Dictionary

## API-to-Storage Data Flow Documentation

**Version:** 1.0
**Last Updated:** 2026-02-01
**Purpose:** Comprehensive mapping of data sources, transformations, and persistence

---

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [Data Source APIs](#data-source-apis)
3. [Application Data Models](#application-data-models)
4. [Persistent Storage Schema](#persistent-storage-schema)
5. [Data Flow Architecture](#data-flow-architecture)
6. [Field Mapping Tables](#field-mapping-tables)
7. [Transient vs Permanent Data](#transient-vs-permanent-data)
8. [Data Lifecycle](#data-lifecycle)

---

## Executive Summary

The FranchiseAI Prospect Discovery Platform aggregates data from multiple external APIs, transforms it through application data models, and persists selected data to a PostgreSQL database for long-term storage and analysis.

### Data Flow Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           EXTERNAL DATA SOURCES                              │
├─────────────────┬─────────────────┬─────────────────┬───────────────────────┤
│  Google Places  │      BBB        │  Demographics   │   OpenStreetMap       │
│     API         │     API         │     API         │      API              │
└────────┬────────┴────────┬────────┴────────┬────────┴───────────┬───────────┘
         │                 │                 │                    │
         ▼                 ▼                 ▼                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                        APPLICATION LAYER (C++)                               │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐ │
│  │BusinessInfo │  │ SearchResult│  │Demographic  │  │    GeoLocation      │ │
│  │   Model     │  │   Model     │  │Data Model   │  │      Model          │ │
│  └─────────────┘  └─────────────┘  └─────────────┘  └─────────────────────┘ │
│                           │                                                  │
│                           ▼                                                  │
│               ┌─────────────────────┐                                        │
│               │   AISearchService   │ ◄── Aggregation & AI Analysis          │
│               └──────────┬──────────┘                                        │
└──────────────────────────┼──────────────────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                    PERSISTENCE LAYER (PostgreSQL)                            │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐ │
│  │  prospects  │  │prospect_    │  │ territory_  │  │   audit_log         │ │
│  │   table     │  │scores table │  │demographics │  │     table           │ │
│  └─────────────┘  └─────────────┘  └─────────────┘  └─────────────────────┘ │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Data Source APIs

### 1. Google Places API

**Endpoint:** `https://maps.googleapis.com/maps/api/place`
**Authentication:** API Key (GOOGLE_API_KEY)
**Rate Limit:** 100 QPS (paid tier)

| API Field | Data Type | Description | Maps To |
|-----------|-----------|-------------|---------|
| `place_id` | string | Unique Google identifier | `BusinessInfo.id` |
| `name` | string | Business name | `BusinessInfo.name` |
| `formatted_address` | string | Full address | `BusinessInfo.address.getFullAddress()` |
| `vicinity` | string | Short address | `BusinessInfo.address.street1` |
| `geometry.location.lat` | double | Latitude | `BusinessInfo.address.latitude` |
| `geometry.location.lng` | double | Longitude | `BusinessInfo.address.longitude` |
| `rating` | float | Google rating (1-5) | `BusinessInfo.googleRating` |
| `user_ratings_total` | int | Total reviews | `BusinessInfo.googleReviewCount` |
| `types[]` | string[] | Place types | `BusinessInfo.type` (inferred) |
| `business_status` | string | Open/closed status | N/A (filtered) |
| `formatted_phone_number` | string | Phone number | `BusinessInfo.contact.primaryPhone` |
| `website` | string | Website URL | `BusinessInfo.contact.website` |
| `opening_hours.weekday_text[]` | string[] | Operating hours | `BusinessInfo.hours.*` |
| `price_level` | int | Price indicator (0-4) | N/A |

**Business Type Mapping (Google → Application):**

| Google Place Type | BusinessType Enum |
|-------------------|-------------------|
| `accounting`, `bank`, `insurance_agency` | `FINANCIAL_SERVICES` |
| `lawyer` | `LAW_FIRM` |
| `doctor`, `hospital`, `physiotherapist` | `MEDICAL_FACILITY` |
| `school`, `secondary_school`, `university` | `EDUCATIONAL_INSTITUTION` |
| `lodging` | `HOTEL` |
| `local_government_office`, `city_hall`, `courthouse` | `GOVERNMENT_OFFICE` |
| `gym`, `stadium` | `CONFERENCE_CENTER` |
| `storage`, `moving_company` | `WAREHOUSE` |
| Default | `CORPORATE_OFFICE` or `OTHER` |

---

### 2. Better Business Bureau (BBB) API

**Endpoint:** `https://api.bbb.org/api`
**Authentication:** API Key (BBB_API_KEY)
**Rate Limit:** 50 requests/minute

| API Field | Data Type | Description | Maps To |
|-----------|-----------|-------------|---------|
| `bbb_id` | string | BBB identifier | `BusinessInfo.id` (secondary) |
| `business_name` | string | Business name | `BusinessInfo.name` |
| `address` | object | Address components | `BusinessInfo.address.*` |
| `phone` | string | Phone number | `BusinessInfo.contact.primaryPhone` |
| `website` | string | Website | `BusinessInfo.contact.website` |
| `rating` | string | Letter grade (A+ to F) | `BusinessInfo.bbbRating` |
| `is_accredited` | boolean | BBB accreditation | `BusinessInfo.bbbAccredited` |
| `complaint_count` | int | Total complaints | `BusinessInfo.bbbComplaintCount` |
| `years_in_business` | int | Years operating | `BusinessInfo.yearEstablished` |
| `category` | string | Business category | `BusinessInfo.category` |

**BBB Rating Mapping:**

| BBB Rating String | BBBRating Enum |
|-------------------|----------------|
| "A+" | `A_PLUS` |
| "A" | `A` |
| "A-" | `A_MINUS` |
| "B+" | `B_PLUS` |
| "B" | `B` |
| "B-" | `B_MINUS` |
| "C+" | `C_PLUS` |
| "C" | `C` |
| "C-" | `C_MINUS` |
| "D+" | `D_PLUS` |
| "D" | `D` |
| "D-" | `D_MINUS` |
| "F" | `F` |
| null/empty | `NOT_RATED` |
| Not found | `NOT_ACCREDITED` |

---

### 3. Demographics API (Census Bureau / BLS)

**Census Endpoint:** `https://api.census.gov/data`
**BLS Endpoint:** `https://api.bls.gov`
**Authentication:** API Key (CENSUS_API_KEY)
**Cache TTL:** 24 hours (data is relatively static)

| API Field | Data Type | Description | Maps To |
|-----------|-----------|-------------|---------|
| `zip_code` | string | ZIP code | `DemographicData.zipCode` |
| `city` | string | City name | `DemographicData.city` |
| `state` | string | State code | `DemographicData.state` |
| `county` | string | County name | `DemographicData.county` |
| `total_population` | int | Total population | `DemographicData.totalPopulation` |
| `working_age_pop` | int | Ages 18-65 | `DemographicData.workingAgePopulation` |
| `median_age` | double | Median age | `DemographicData.medianAge` |
| `median_household_income` | double | Median income | `DemographicData.medianHouseholdIncome` |
| `average_household_income` | double | Average income | `DemographicData.averageHouseholdIncome` |
| `per_capita_income` | double | Per capita income | `DemographicData.perCapitaIncome` |
| `unemployment_rate` | double | Unemployment % | `DemographicData.unemploymentRate` |
| `total_businesses` | int | Business count | `DemographicData.totalBusinesses` |
| `office_buildings` | int | Office count | `DemographicData.officeBuildings` |
| `warehouses` | int | Warehouse count | `DemographicData.warehouses` |
| `conference_venues` | int | Venue count | `DemographicData.conferenceVenues` |
| `corporate_headquarters` | int | HQ count | `DemographicData.corporateHeadquarters` |
| `employment_by_sector` | map | Sector breakdown | `DemographicData.employmentBySector` |
| `population_growth_rate` | double | Growth % | `DemographicData.populationGrowthRate` |
| `business_growth_rate` | double | Business growth % | `DemographicData.businessGrowthRate` |

**Industry Sector Keys:**

| Sector Key | IndustrySector Constant |
|------------|-------------------------|
| `technology` | `TECHNOLOGY` |
| `healthcare` | `HEALTHCARE` |
| `finance` | `FINANCE` |
| `manufacturing` | `MANUFACTURING` |
| `retail` | `RETAIL` |
| `professional` | `PROFESSIONAL` |
| `education` | `EDUCATION` |
| `government` | `GOVERNMENT` |
| `logistics` | `LOGISTICS` |
| `hospitality` | `HOSPITALITY` |

---

### 4. OpenStreetMap API (Overpass / Nominatim)

**Overpass Endpoint:** `https://overpass-api.de/api/interpreter`
**Nominatim Endpoint:** `https://nominatim.openstreetmap.org`
**Authentication:** None (User-Agent required)
**Rate Limit:** 1 request/second
**Cache TTL:** 24 hours

| API Field | Data Type | Description | Maps To |
|-----------|-----------|-------------|---------|
| `id` | int64 | OSM node/way ID | `OSMPoi.osmId` |
| `type` | string | node/way/relation | `OSMPoi.osmType` |
| `lat` | double | Latitude | `OSMPoi.latitude` |
| `lon` | double | Longitude | `OSMPoi.longitude` |
| `tags.name` | string | POI name | `OSMPoi.name` |
| `tags.amenity` | string | Amenity type | `OSMPoi.amenity` |
| `tags.building` | string | Building type | `OSMPoi.building` |
| `tags.office` | string | Office type | `OSMPoi.office` |
| `tags.shop` | string | Shop type | `OSMPoi.shop` |
| `tags.tourism` | string | Tourism type | `OSMPoi.tourism` |
| `tags.healthcare` | string | Healthcare type | `OSMPoi.healthcare` |
| `tags.addr:street` | string | Street address | `OSMPoi.street` |
| `tags.addr:housenumber` | string | House number | `OSMPoi.houseNumber` |
| `tags.addr:city` | string | City | `OSMPoi.city` |
| `tags.addr:postcode` | string | Postal code | `OSMPoi.postcode` |
| `tags.phone` | string | Phone number | `OSMPoi.phone` |
| `tags.website` | string | Website URL | `OSMPoi.website` |
| `tags.email` | string | Email address | `OSMPoi.email` |
| `tags.opening_hours` | string | Hours of operation | `OSMPoi.openingHours` |

**OSM Tag to BusinessType Mapping:**

| OSM Tag | Tag Value | BusinessType |
|---------|-----------|--------------|
| `office` | `company`, `corporate` | `CORPORATE_OFFICE` |
| `office` | `coworking` | `COWORKING_SPACE` |
| `office` | `lawyer`, `legal` | `LAW_FIRM` |
| `office` | `financial`, `insurance` | `FINANCIAL_SERVICES` |
| `office` | `government` | `GOVERNMENT_OFFICE` |
| `office` | `ngo`, `foundation` | `NONPROFIT` |
| `building` | `warehouse`, `industrial` | `WAREHOUSE` |
| `building` | `office` | `CORPORATE_OFFICE` |
| `building` | `hotel` | `HOTEL` |
| `building` | `hospital` | `MEDICAL_FACILITY` |
| `building` | `school`, `university` | `EDUCATIONAL_INSTITUTION` |
| `amenity` | `conference_centre` | `CONFERENCE_CENTER` |
| `amenity` | `hospital`, `clinic` | `MEDICAL_FACILITY` |
| `amenity` | `school`, `college`, `university` | `EDUCATIONAL_INSTITUTION` |
| `amenity` | `bank` | `FINANCIAL_SERVICES` |
| `tourism` | `hotel`, `motel` | `HOTEL` |

---

### 5. AI Analysis APIs

#### OpenAI API

**Endpoint:** `https://api.openai.com/v1/chat/completions`
**Model:** GPT-4 / gpt-3.5-turbo
**Authentication:** API Key (OPENAI_API_KEY)

| Input Field | Source | Description |
|-------------|--------|-------------|
| Business name | `BusinessInfo.name` | Target business |
| Business type | `BusinessInfo.type` | Category |
| Employee count | `BusinessInfo.employeeCount` | Size indicator |
| Location | `BusinessInfo.address` | Geographic context |
| Ratings | `BusinessInfo.googleRating`, `bbbRating` | Reputation |

| Output Field | Maps To |
|--------------|---------|
| `summary` | `BusinessInfo.aiInsights` |
| `highlights[]` | `SearchResultItem.keyHighlights` |
| `recommendations[]` | `BusinessInfo.suggestedApproach` |
| `catering_score` | `BusinessInfo.cateringPotentialScore` |
| `confidence` | `SearchResultItem.aiConfidenceScore` |

#### Google Gemini API

**Endpoint:** `https://generativelanguage.googleapis.com/v1beta/models`
**Model:** gemini-pro
**Authentication:** API Key (GEMINI_API_KEY)

Same input/output mapping as OpenAI.

---

## Application Data Models

### BusinessInfo Model (`src/models/BusinessInfo.h`)

The central data model for business/prospect information. Aggregates data from multiple sources.

```cpp
class BusinessInfo {
    // Identification
    std::string id;                    // Source-specific ID
    std::string name;                  // Business name
    std::string description;           // Business description
    BusinessType type;                 // Enum: CORPORATE_OFFICE, HOTEL, etc.
    DataSource source;                 // Enum: GOOGLE_MY_BUSINESS, BBB, etc.

    // Location
    Address address;                   // Full address with lat/lon

    // Contact
    ContactInfo contact;               // Phone, email, website, contact person

    // Business Details
    std::string category;              // Primary category
    std::vector<std::string> subcategories;
    int employeeCount;                 // Employee count
    int yearEstablished;               // Year founded
    double annualRevenue;              // Revenue estimate

    // Ratings & Reviews
    double googleRating;               // Google rating (1-5)
    int googleReviewCount;             // Google review count
    BBBRating bbbRating;               // BBB letter grade
    bool bbbAccredited;                // BBB accreditation status
    int bbbComplaintCount;             // BBB complaint count

    // Operating Hours
    BusinessHours hours;               // Daily hours

    // Catering Indicators
    bool hasConferenceRoom;            // Has meeting space
    bool hasEventSpace;                // Has event facilities
    bool regularMeetings;              // Holds regular meetings
    int estimatedEmployeesOnSite;      // On-site employees

    // AI Insights
    int cateringPotentialScore;        // 0-100 score
    std::string aiInsights;            // AI-generated insights
    std::vector<std::string> suggestedApproach;

    // Metadata
    std::time_t lastUpdated;           // Last update timestamp
    std::time_t dateAdded;             // Date added to system
    bool isVerified;                   // Manual verification flag
};
```

### DemographicData Model (`src/models/DemographicData.h`)

```cpp
struct DemographicData {
    // Location
    std::string zipCode;
    std::string city;
    std::string state;
    std::string county;

    // Population Metrics
    int totalPopulation;
    int workingAgePopulation;          // 18-65
    double medianAge;

    // Economic Metrics
    double medianHouseholdIncome;
    double averageHouseholdIncome;
    double perCapitaIncome;
    double unemploymentRate;

    // Business Metrics
    int totalBusinesses;
    int officeBuildings;
    int warehouses;
    int conferenceVenues;
    int corporateHeadquarters;

    // Employment by Sector
    std::map<std::string, int> employmentBySector;

    // Growth Indicators
    double populationGrowthRate;
    double businessGrowthRate;
    double economicGrowthIndex;

    // Scoring
    int marketPotentialScore;          // 0-100

    // Context
    double distanceFromFranchise;      // Miles from store
};
```

### SearchResultItem Model (`src/models/SearchResult.h`)

```cpp
class SearchResultItem {
    std::string id;
    SearchResultType resultType;       // BUSINESS, DEMOGRAPHIC_AREA, COMBINED

    // Scoring
    double relevanceScore;             // 0.0-1.0
    double aiConfidenceScore;          // 0.0-1.0
    int overallScore;                  // 0-100

    // Associated Data
    std::shared_ptr<BusinessInfo> business;
    std::shared_ptr<DemographicData> demographic;

    // AI-Generated Content
    std::string matchReason;
    std::string aiSummary;
    std::vector<std::string> keyHighlights;
    std::vector<std::string> recommendedActions;

    // Source Tracking
    std::vector<DataSource> sources;   // Which APIs contributed

    // Distance
    double distanceMiles;              // From search center
};
```

### GeoLocation Model (`src/models/GeoLocation.h`)

```cpp
struct GeoLocation {
    // Coordinates
    double latitude;
    double longitude;

    // Address Components
    std::string formattedAddress;
    std::string street;
    std::string city;
    std::string state;
    std::string postalCode;
    std::string country;

    // Metadata
    std::string source;                // "nominatim", "google"
    double accuracy;                   // Accuracy in meters
    bool isValid;
};

struct SearchArea {
    GeoLocation center;
    double radiusKm;
    double radiusMiles;
};
```

---

## Persistent Storage Schema

### Core Tables

#### `prospects` Table

**Purpose:** Store prospective catering clients discovered through search.

| Column | Type | Source API | Model Field | Nullable |
|--------|------|------------|-------------|----------|
| `id` | UUID | Generated | - | No |
| `territory_id` | UUID | Internal | - | Yes |
| `franchisee_id` | UUID | Internal | - | Yes |
| `assigned_to_user_id` | UUID | Internal | - | Yes |
| `business_name` | VARCHAR(300) | Google/BBB/OSM | `BusinessInfo.name` | No |
| `dba_name` | VARCHAR(300) | BBB | `BusinessInfo.name` (alt) | Yes |
| `legal_name` | VARCHAR(300) | BBB | - | Yes |
| `industry_id` | UUID | Mapped | `BusinessInfo.category` | Yes |
| `industry_naics` | VARCHAR(10) | BBB | - | Yes |
| `business_type` | VARCHAR(100) | All | `BusinessInfo.type` | Yes |
| `employee_count` | INTEGER | Google/BBB | `BusinessInfo.employeeCount` | Yes |
| `employee_count_range` | ENUM | Derived | `BusinessInfo.employeeCount` | Yes |
| `annual_revenue` | DECIMAL | BBB | `BusinessInfo.annualRevenue` | Yes |
| `year_established` | INTEGER | BBB | `BusinessInfo.yearEstablished` | Yes |
| `address_line1` | VARCHAR(200) | All | `BusinessInfo.address.street1` | Yes |
| `address_line2` | VARCHAR(100) | All | `BusinessInfo.address.street2` | Yes |
| `city` | VARCHAR(100) | All | `BusinessInfo.address.city` | Yes |
| `state_province` | VARCHAR(50) | All | `BusinessInfo.address.state` | Yes |
| `postal_code` | VARCHAR(20) | All | `BusinessInfo.address.zipCode` | Yes |
| `country_code` | CHAR(2) | Default | `BusinessInfo.address.country` | Yes |
| `latitude` | DECIMAL(10,8) | All | `BusinessInfo.address.latitude` | Yes |
| `longitude` | DECIMAL(11,8) | All | `BusinessInfo.address.longitude` | Yes |
| `geocode_accuracy` | VARCHAR(20) | Geocoding | `GeoLocation.accuracy` | Yes |
| `primary_phone` | VARCHAR(30) | All | `BusinessInfo.contact.primaryPhone` | Yes |
| `secondary_phone` | VARCHAR(30) | BBB | `BusinessInfo.contact.secondaryPhone` | Yes |
| `email` | VARCHAR(255) | OSM/BBB | `BusinessInfo.contact.email` | Yes |
| `website` | VARCHAR(500) | All | `BusinessInfo.contact.website` | Yes |
| `linkedin_url` | VARCHAR(500) | Manual | - | Yes |
| `facebook_url` | VARCHAR(500) | Manual | - | Yes |
| `status` | ENUM | Internal | - | No |
| `status_changed_at` | TIMESTAMP | Internal | - | Yes |
| `data_source` | VARCHAR(100) | All | `DataSource` enum name | Yes |
| `source_record_id` | VARCHAR(200) | All | Original API ID | Yes |
| `is_verified` | BOOLEAN | Internal | `BusinessInfo.isVerified` | No |
| `is_duplicate` | BOOLEAN | Internal | - | No |
| `duplicate_of_id` | UUID | Internal | - | Yes |
| `do_not_contact` | BOOLEAN | Internal | - | No |
| `created_at` | TIMESTAMP | Generated | `BusinessInfo.dateAdded` | No |
| `updated_at` | TIMESTAMP | Generated | `BusinessInfo.lastUpdated` | No |

#### `prospect_scores` Table

**Purpose:** Store AI and manual lead scoring for prospects.

| Column | Type | Source | Model Field | Nullable |
|--------|------|--------|-------------|----------|
| `id` | UUID | Generated | - | No |
| `prospect_id` | UUID | FK | - | No |
| `total_score` | INTEGER | AI/Calculated | `SearchResultItem.overallScore` | No |
| `score_grade` | CHAR(1) | Calculated | - | Yes |
| `fit_score` | INTEGER | AI | - | Yes |
| `engagement_score` | INTEGER | AI | - | Yes |
| `intent_score` | INTEGER | AI | - | Yes |
| `catering_potential_score` | INTEGER | AI | `BusinessInfo.cateringPotentialScore` | Yes |
| `order_frequency_score` | INTEGER | AI | - | Yes |
| `budget_score` | INTEGER | AI | - | Yes |
| `proximity_score` | INTEGER | Calculated | Distance-based | Yes |
| `market_density_score` | INTEGER | Demographics | - | Yes |
| `score_source` | ENUM | Internal | "ai_model", "manual", etc. | No |
| `model_version` | VARCHAR(50) | AI | - | Yes |
| `confidence` | DECIMAL(5,4) | AI | `SearchResultItem.aiConfidenceScore` | Yes |
| `score_factors` | JSONB | AI | Detailed factors | Yes |
| `scored_at` | TIMESTAMP | Generated | - | No |
| `expires_at` | TIMESTAMP | Calculated | - | Yes |

#### `territory_demographics` Table

**Purpose:** Store demographic data for territories (from Demographics API).

| Column | Type | Source API | Model Field | Nullable |
|--------|------|------------|-------------|----------|
| `id` | UUID | Generated | - | No |
| `territory_id` | UUID | FK | - | No |
| `total_population` | INTEGER | Census | `DemographicData.totalPopulation` | Yes |
| `population_density` | DECIMAL | Census | Calculated | Yes |
| `median_age` | DECIMAL | Census | `DemographicData.medianAge` | Yes |
| `total_households` | INTEGER | Census | - | Yes |
| `median_household_income` | DECIMAL | Census | `DemographicData.medianHouseholdIncome` | Yes |
| `average_household_size` | DECIMAL | Census | - | Yes |
| `income_under_25k_pct` | DECIMAL | Census | - | Yes |
| `income_25k_50k_pct` | DECIMAL | Census | - | Yes |
| `income_50k_75k_pct` | DECIMAL | Census | - | Yes |
| `income_75k_100k_pct` | DECIMAL | Census | - | Yes |
| `income_100k_150k_pct` | DECIMAL | Census | - | Yes |
| `income_over_150k_pct` | DECIMAL | Census | - | Yes |
| `total_businesses` | INTEGER | Census | `DemographicData.totalBusinesses` | Yes |
| `total_employees` | INTEGER | Census | - | Yes |
| `business_density` | DECIMAL | Calculated | - | Yes |
| `daytime_population` | INTEGER | Census | - | Yes |
| `commuter_inflow` | INTEGER | Census | - | Yes |
| `commuter_outflow` | INTEGER | Census | - | Yes |
| `data_year` | INTEGER | Census | - | No |
| `data_source` | VARCHAR(100) | Internal | "US Census Bureau" | Yes |

#### `store_locations` Table

**Purpose:** Store franchise store locations with geocoded coordinates.

| Column | Type | Source | Model Field | Nullable |
|--------|------|--------|-------------|----------|
| `id` | UUID | Generated | `Franchisee.storeId` | No |
| `franchisee_id` | UUID | FK | - | Yes |
| `store_name` | VARCHAR(200) | User Input | `Franchisee.storeName` | No |
| `store_code` | VARCHAR(50) | Generated | - | Yes |
| `address_line1` | VARCHAR(200) | User Input | `Franchisee.address` | No |
| `address_line2` | VARCHAR(100) | User Input | - | Yes |
| `city` | VARCHAR(100) | Geocoding | `GeoLocation.city` | Yes |
| `state_province` | VARCHAR(50) | Geocoding | `GeoLocation.state` | Yes |
| `postal_code` | VARCHAR(20) | Geocoding | `GeoLocation.postalCode` | Yes |
| `country_code` | CHAR(2) | Default | "US" | Yes |
| `latitude` | DECIMAL(10,8) | Geocoding | `GeoLocation.latitude` | Yes |
| `longitude` | DECIMAL(11,8) | Geocoding | `GeoLocation.longitude` | Yes |
| `geocode_source` | VARCHAR(50) | Geocoding | `GeoLocation.source` | Yes |
| `geocoded_at` | TIMESTAMP | Geocoding | - | Yes |
| `default_search_radius_miles` | DECIMAL | User Input | `Franchisee.defaultSearchRadiusMiles` | Yes |
| `phone` | VARCHAR(30) | User Input | `Franchisee.phone` | Yes |
| `email` | VARCHAR(255) | User Input | `Franchisee.email` | Yes |
| `is_active` | BOOLEAN | Internal | - | No |
| `is_primary` | BOOLEAN | Internal | - | No |

#### `audit_log` Table

**Purpose:** Track all user actions for compliance.

| Column | Type | Source | Description |
|--------|------|--------|-------------|
| `id` | UUID | Generated | Unique identifier |
| `user_id` | UUID | Session | User who performed action |
| `event_type` | VARCHAR(50) | Internal | Action type |
| `event_details` | JSONB | Internal | Action details |
| `ip_address` | VARCHAR(45) | Request | Client IP |
| `user_agent` | TEXT | Request | Browser/client info |
| `created_at` | TIMESTAMP | Generated | Event timestamp |

---

## Data Flow Architecture

### Search Flow: API to Display

```
┌──────────────────────────────────────────────────────────────────────────┐
│                         USER SEARCH REQUEST                               │
│   Location: "Denver, CO"    Radius: 25 miles    Types: [CORPORATE_OFFICE]│
└─────────────────────────────────┬────────────────────────────────────────┘
                                  │
                                  ▼
┌──────────────────────────────────────────────────────────────────────────┐
│                         AISearchService.search()                          │
│                                                                           │
│  1. Geocode location → GeoLocation (lat/lon)                             │
│  2. Create SearchArea with radius                                         │
│  3. Query all APIs in parallel:                                           │
└────────┬────────────────┬────────────────┬────────────────┬──────────────┘
         │                │                │                │
         ▼                ▼                ▼                ▼
┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐
│Google Places│  │    BBB      │  │Demographics │  │OpenStreetMap│
│    API      │  │    API      │  │    API      │  │    API      │
└──────┬──────┘  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘
       │                │                │                │
       ▼                ▼                ▼                ▼
┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐
│GooglePlace[]│  │BusinessInfo│  │Demographic  │  │ OSMPoi[]    │
│ (raw data)  │  │    []       │  │Data[]       │  │ (raw data)  │
└──────┬──────┘  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘
       │                │                │                │
       ▼                ▼                ▼                ▼
┌──────────────────────────────────────────────────────────────────────────┐
│                    DATA TRANSFORMATION LAYER                              │
│                                                                           │
│  GooglePlace → BusinessInfo (placeToBusinessInfo)                        │
│  OSMPoi → BusinessInfo (poiToBusinessInfo)                               │
│                                                                           │
│  Merge duplicates by name + address similarity                            │
│  Enrich with cross-source data (BBB ratings + Google reviews)            │
└─────────────────────────────────┬────────────────────────────────────────┘
                                  │
                                  ▼
┌──────────────────────────────────────────────────────────────────────────┐
│                       RESULT AGGREGATION                                  │
│                                                                           │
│  BusinessInfo[] + DemographicData[]                                       │
│       ↓                                                                   │
│  SearchResultItem[] (with relevance scoring)                              │
│       ↓                                                                   │
│  SearchResults (sorted, paginated)                                        │
└─────────────────────────────────┬────────────────────────────────────────┘
                                  │
                                  ▼
┌──────────────────────────────────────────────────────────────────────────┐
│                         AI ANALYSIS                                       │
│                                                                           │
│  For each SearchResultItem:                                               │
│  • Generate match reason                                                  │
│  • Calculate catering potential score                                     │
│  • Generate recommended actions                                           │
│  • AI summary (OpenAI/Gemini)                                            │
│                                                                           │
│  Overall Analysis:                                                        │
│  • Market summary                                                         │
│  • Top recommendations                                                    │
└─────────────────────────────────┬────────────────────────────────────────┘
                                  │
                                  ▼
┌──────────────────────────────────────────────────────────────────────────┐
│                      UI DISPLAY (TRANSIENT)                               │
│                                                                           │
│  SearchResults → UI Widget                                                │
│  User can browse, filter, sort results                                    │
└──────────────────────────────────────────────────────────────────────────┘
```

### Save Flow: Display to Persistence

```
┌──────────────────────────────────────────────────────────────────────────┐
│                      USER ACTION: "Add to Prospects"                      │
│                                                                           │
│  Select SearchResultItem with BusinessInfo                                │
└─────────────────────────────────┬────────────────────────────────────────┘
                                  │
                                  ▼
┌──────────────────────────────────────────────────────────────────────────┐
│                       FranchiseApp.onAddToProspects()                     │
│                                                                           │
│  1. Convert BusinessInfo to Prospect JSON                                 │
│  2. Add franchisee_id, territory_id                                       │
│  3. Add source tracking (data_source, source_record_id)                   │
└─────────────────────────────────┬────────────────────────────────────────┘
                                  │
                                  ▼
┌──────────────────────────────────────────────────────────────────────────┐
│                 ApiLogicServerClient.createResource("Prospect")           │
│                                                                           │
│  POST /api/Prospect                                                       │
│  Body: { business_name, address, scores, source, ... }                   │
└─────────────────────────────────┬────────────────────────────────────────┘
                                  │
                                  ▼
┌──────────────────────────────────────────────────────────────────────────┐
│                      PostgreSQL (PERMANENT)                               │
│                                                                           │
│  INSERT INTO prospects (...) VALUES (...)                                 │
│  INSERT INTO prospect_scores (...) VALUES (...)                           │
│  INSERT INTO audit_log (...) VALUES (...)                                 │
└──────────────────────────────────────────────────────────────────────────┘
```

---

## Field Mapping Tables

### Google Places → BusinessInfo → prospects

| Google Places API | BusinessInfo | prospects table |
|-------------------|--------------|-----------------|
| `place_id` | `id` | `source_record_id` |
| `name` | `name` | `business_name` |
| `formatted_address` | `address.getFullAddress()` | `address_line1` |
| `vicinity` | `address.street1` | `address_line1` |
| `geometry.location.lat` | `address.latitude` | `latitude` |
| `geometry.location.lng` | `address.longitude` | `longitude` |
| `rating` | `googleRating` | - (in prospect_scores) |
| `user_ratings_total` | `googleReviewCount` | - |
| `types[]` | `type` (inferred) | `business_type` |
| `formatted_phone_number` | `contact.primaryPhone` | `primary_phone` |
| `website` | `contact.website` | `website` |
| - | `source` = GOOGLE_MY_BUSINESS | `data_source` = "Google My Business" |

### BBB API → BusinessInfo → prospects

| BBB API | BusinessInfo | prospects table |
|---------|--------------|-----------------|
| `bbb_id` | `id` | `source_record_id` |
| `business_name` | `name` | `business_name` |
| `address.street` | `address.street1` | `address_line1` |
| `address.city` | `address.city` | `city` |
| `address.state` | `address.state` | `state_province` |
| `address.zip` | `address.zipCode` | `postal_code` |
| `phone` | `contact.primaryPhone` | `primary_phone` |
| `website` | `contact.website` | `website` |
| `rating` | `bbbRating` | - (indirect) |
| `is_accredited` | `bbbAccredited` | - |
| `complaint_count` | `bbbComplaintCount` | - |
| `years_in_business` | `yearEstablished` | `year_established` |
| `category` | `category` | `business_type` |
| - | `source` = BBB | `data_source` = "Better Business Bureau" |

### OpenStreetMap → BusinessInfo → prospects

| OSM (Overpass) | BusinessInfo | prospects table |
|----------------|--------------|-----------------|
| `id` (node/way) | `id` | `source_record_id` |
| `tags.name` | `name` | `business_name` |
| `lat` | `address.latitude` | `latitude` |
| `lon` | `address.longitude` | `longitude` |
| `tags.addr:street` | `address.street1` | `address_line1` |
| `tags.addr:housenumber` | `address.street1` (prefix) | `address_line1` |
| `tags.addr:city` | `address.city` | `city` |
| `tags.addr:postcode` | `address.zipCode` | `postal_code` |
| `tags.phone` | `contact.primaryPhone` | `primary_phone` |
| `tags.website` | `contact.website` | `website` |
| `tags.email` | `contact.email` | `email` |
| `tags.amenity/office/building` | `type` (inferred) | `business_type` |
| - | `source` = OPENSTREETMAP | `data_source` = "OpenStreetMap" |

### Demographics API → DemographicData → territory_demographics

| Census/BLS API | DemographicData | territory_demographics |
|----------------|-----------------|------------------------|
| `zip_code` | `zipCode` | - (linked via territory) |
| `total_population` | `totalPopulation` | `total_population` |
| `working_age_population` | `workingAgePopulation` | - |
| `median_age` | `medianAge` | `median_age` |
| `median_household_income` | `medianHouseholdIncome` | `median_household_income` |
| `total_businesses` | `totalBusinesses` | `total_businesses` |
| `office_buildings` | `officeBuildings` | - |
| `unemployment_rate` | `unemploymentRate` | - |
| `population_growth_rate` | `populationGrowthRate` | - |
| `business_growth_rate` | `businessGrowthRate` | - |

---

## Transient vs Permanent Data

### Transient Data (In-Memory Only)

Data that exists only during runtime and is not persisted to the database:

| Data Type | Location | Lifecycle | Purpose |
|-----------|----------|-----------|---------|
| `GooglePlace` | `GooglePlacesAPI` | Per-search | Raw API response |
| `OSMPoi` | `OpenStreetMapAPI` | Per-search | Raw API response |
| `SearchResults` | `AISearchService` | Per-search | Aggregated results for display |
| `SearchResultItem` | `AISearchService` | Per-search | Individual result with scores |
| `SearchProgress` | `AISearchService` | Per-search | Progress tracking |
| `GeoLocation` | Geocoding services | Per-operation | Coordinates for search |
| `SearchArea` | `AISearchService` | Per-search | Search boundary |
| `GeoBoundingBox` | API calls | Per-request | API query parameter |
| `OSMAreaStats` | `OpenStreetMapAPI` | Per-search | Area statistics |
| `BatchGeocodeResult` | `GoogleGeocodingAPI` | Per-batch | Batch geocoding results |
| `AIEngineResult` | `AIEngine` | Per-analysis | AI analysis output |
| `ThreadPoolMetrics` | `ThreadPool` | Runtime | Performance metrics |
| `GooglePlacesStats` | `GooglePlacesAPI` | Runtime | API usage statistics |

**Cache Data (Transient with TTL):**

| Cache Type | TTL | Location | Purpose |
|------------|-----|----------|---------|
| Geocoding cache | 24 hours | `GeocodingService` | Reduce API calls |
| Places cache | 1 hour | `GooglePlacesAPI` | Reduce API calls |
| Demographics cache | 24 hours | `DemographicsAPI` | Static data caching |
| OSM POI cache | 24 hours | `OpenStreetMapAPI` | Static data caching |
| Place details cache | 1 hour | `GooglePlacesAPI` | Reduce API calls |

### Permanent Data (PostgreSQL)

Data that is persisted to the database for long-term storage:

| Table | Purpose | Source | Retention |
|-------|---------|--------|-----------|
| `prospects` | Saved prospect records | User saves from search | Indefinite |
| `prospect_scores` | Lead scores per prospect | AI analysis at save | Indefinite |
| `prospect_contacts` | Contact persons | User input | Indefinite |
| `prospect_outreach` | Outreach history | User logs | Indefinite |
| `prospect_notes` | Notes per prospect | User input | Indefinite |
| `prospect_tags` | Tags per prospect | User tagging | Indefinite |
| `franchisees` | Franchise owners | Admin input | Indefinite |
| `store_locations` | Franchise stores | User input + geocoding | Indefinite |
| `territories` | Sales territories | Admin input | Indefinite |
| `territory_demographics` | Demographics per territory | Demographics API (saved) | Yearly refresh |
| `regions` | Geographic regions | Admin input | Indefinite |
| `industries` | Industry classifications | Seed data | Indefinite |
| `users` | Application users | Registration | Indefinite |
| `user_sessions` | Active sessions | Login | Session expiry |
| `audit_log` | Audit trail | All actions | 7+ years (compliance) |
| `app_config` | Application settings | Admin input | Indefinite |
| `saved_searches` | Saved search queries | User saves | Indefinite |
| `data_source_sync` | Sync history | System | 90 days |
| `tags` | Tag definitions | Admin input | Indefinite |

### Data Transition: Transient → Permanent

The following actions cause transient data to become permanent:

| User Action | Transient Data | Permanent Storage |
|-------------|----------------|-------------------|
| "Add to Prospects" | `SearchResultItem.business` | `prospects` + `prospect_scores` |
| "Save Search" | `SearchQuery` | `saved_searches.search_params` (JSONB) |
| "Add Note" | User text | `prospect_notes` |
| "Log Outreach" | User input | `prospect_outreach` |
| "Add Contact" | User input | `prospect_contacts` |
| "Save Store Location" | `Franchisee` + `GeoLocation` | `store_locations` |
| Any action | Session info | `audit_log` |

---

## Data Lifecycle

### Search Result Lifecycle

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  API Call   │ ──▶ │  Transform  │ ──▶ │  Display    │ ──▶ │   Discard   │
│  (External) │     │  (Memory)   │     │  (UI)       │     │  (GC)       │
└─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘
                          │
                          │ User Action: "Add to Prospects"
                          ▼
                    ┌─────────────┐     ┌─────────────┐
                    │  Persist    │ ──▶ │  Database   │
                    │  (API)      │     │  (Permanent)│
                    └─────────────┘     └─────────────┘
```

### Prospect Lifecycle

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│    New      │ ──▶ │  Contacted  │ ──▶ │  Qualified  │ ──▶ │   Won/Lost  │
│  (saved)    │     │  (outreach) │     │  (scored)   │     │  (closed)   │
└─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘
      │                   │                   │                   │
      ▼                   ▼                   ▼                   ▼
 ┌─────────┐        ┌─────────┐        ┌─────────┐        ┌─────────┐
 │prospects│        │outreach │        │ scores  │        │ status  │
 │  table  │        │  table  │        │  table  │        │ updated │
 └─────────┘        └─────────┘        └─────────┘        └─────────┘
```

### Cache Lifecycle

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  API Call   │ ──▶ │   Cache     │ ──▶ │   Serve     │ ──▶ │   Expire    │
│  (Fresh)    │     │   (Store)   │     │   (Hits)    │     │   (TTL)     │
└─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘
                          │                   │
                          │                   │ Cache Miss
                          ▼                   ▼
                    ┌─────────────┐     ┌─────────────┐
                    │  LRU Evict  │     │  Refetch    │
                    │  (Full)     │     │  (Stale)    │
                    └─────────────┘     └─────────────┘
```

---

## Appendix: Enum Definitions

### BusinessType Enum

```cpp
enum class BusinessType {
    CORPORATE_OFFICE,        // 0
    WAREHOUSE,               // 1
    CONFERENCE_CENTER,       // 2
    HOTEL,                   // 3
    COWORKING_SPACE,         // 4
    MEDICAL_FACILITY,        // 5
    EDUCATIONAL_INSTITUTION, // 6
    GOVERNMENT_OFFICE,       // 7
    MANUFACTURING,           // 8
    TECH_COMPANY,            // 9
    FINANCIAL_SERVICES,      // 10
    LAW_FIRM,                // 11
    NONPROFIT,               // 12
    OTHER                    // 13
};
```

### DataSource Enum

```cpp
enum class DataSource {
    GOOGLE_MY_BUSINESS,  // 0
    BBB,                 // 1
    DEMOGRAPHICS,        // 2
    OPENSTREETMAP,       // 3
    MANUAL_ENTRY,        // 4
    IMPORTED             // 5
};
```

### Database ENUMs

```sql
-- prospect_status
CREATE TYPE prospect_status AS ENUM (
    'new', 'contacted', 'qualified', 'proposal_sent',
    'negotiating', 'won', 'lost', 'inactive'
);

-- business_size
CREATE TYPE business_size AS ENUM (
    'micro',      -- 1-9 employees
    'small',      -- 10-49 employees
    'medium',     -- 50-249 employees
    'large',      -- 250+ employees
    'enterprise'  -- 1000+ employees
);

-- score_source
CREATE TYPE score_source AS ENUM (
    'ai_model', 'manual', 'rule_based', 'imported'
);
```

---

## Document Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-02-01 | AI Analysis | Initial comprehensive data dictionary |
