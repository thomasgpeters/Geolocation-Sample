# Generic Prospecting Platform Architecture

## Executive Summary

This document analyzes the feasibility of pivoting the current FranchiseAI platform from a catering-specific prospecting tool into a **generic, seed-data-driven prospecting platform** that can be configured for any business prospecting use case through configuration and seed data alone — without code changes.

**Verdict: Highly feasible.** The existing architecture already has several of the right abstractions in place. The main work is extracting hardcoded domain assumptions into a configuration layer we're calling a **Prospecting Profile**.

---

## What Already Works (Generic Building Blocks)

The codebase already has these domain-agnostic foundations:

| Component | Why It's Already Generic |
|-----------|------------------------|
| `company_types` table | 9 types already defined (food service, retail, services, manufacturing, real estate) with configurable `default_search_radius_miles` and `default_prospect_industries` |
| `franchisees` → `company_type_id` FK | Franchisees are already typed — the system already knows "what kind of business is this user" |
| `scoring_rules` table | Fully configurable penalty/bonus system with enable/disable, adjustable points, per-franchisee overrides |
| `ScoringEngine` | Rule-based engine that loads rules from database — architecture is domain-agnostic |
| `app_config` table | Feature flags, display settings, business rules — all database-driven |
| `AIEngine` abstract interface | Strategy pattern already supports OpenAI and Gemini — adding prompt configuration is natural |
| `territories` / `regions` / `demographics` | Geographic infrastructure is industry-neutral |
| `prospect_status` enum | Pipeline stages (new → contacted → qualified → won) are universal |
| `outreach_type` enum | Already includes email, phone, linkedin, referral, event |
| Multi-tenant architecture | Franchisee-scoped data isolation already exists |
| `saved_searches` with JSONB params | Search parameters already stored as flexible JSON |
| `data_source_sync` tracking | Source-agnostic import tracking |

**The foundation is strong.** The core data model, scoring engine, multi-tenancy, and geographic infrastructure are all usable across domains.

---

## What's Currently Hardcoded (Needs Abstraction)

A thorough code audit identified these domain-specific couplings:

### 1. AI Prompts (Critical)
**Files**: `OpenAIEngine.cpp`, `GeminiEngine.cpp`, `AIEngine.cpp`

All AI system prompts are hardcoded strings like:
- *"You are an expert business analyst specializing in corporate catering market analysis"*
- *"Analyze businesses for their potential as catering clients"*
- *"You are a market research analyst specializing in the food service industry"*

**Impact**: The AI brain of the platform is locked to catering. This is the single most impactful change.

### 2. Search Queries (Critical)
**Files**: `OpenStreetMapAPI.cpp`, `GooglePlacesAPI.cpp`

OSM Overpass queries search for: offices, hotels, conference centres, hospitals, universities.
Google Places queries search for: corporate_office, accounting, bank, hospital, lodging.

These are hardcoded arrays, not configurable.

### 3. Model Field Names (Medium)
**Files**: `BusinessInfo.h/cpp`, `SearchResult.cpp`, `ApiLogicServerClient.cpp`

The field `cateringPotentialScore` appears 50+ times across the codebase. The method `calculateCateringPotential()` contains domain-specific scoring logic.

### 4. UI Text (Medium)
**Files**: `FranchiseApp.cpp`, `SearchPanel.cpp`, `ResultsDisplay.cpp`

Hardcoded strings like:
- *"Find potential catering clients in your area"*
- *"Enter your Vocelli Pizza store details"*
- *"Select the types of businesses you want to target for catering services"*
- Marketing insights per business type all reference catering

### 5. Default Business Type Checkboxes (Low)
**File**: `FranchiseApp.cpp`

Default search filters pre-check catering-relevant categories (Corporate Offices, Hotels, Conference Centers).

---

## The Prospecting Profile Concept

The key architectural idea is a **Prospecting Profile** — a configuration object that defines everything about a specific prospecting use case. The platform loads a profile at startup (or per-tenant), and all behavior adapts accordingly.

### Profile Structure

```
prospecting_profiles
├── profile_id (PK)
├── code                    -- "franchise_catering", "franchise_buyers", "hotel_conference"
├── name                    -- "Franchise Catering Prospects"
├── description             -- Human-readable purpose
│
├── ── Target Definition ──
├── prospect_noun           -- "prospect" / "candidate" / "lead" / "buyer"
├── prospect_noun_plural    -- "prospects" / "candidates" / "leads" / "buyers"
├── target_description      -- "businesses needing catering" / "individuals seeking franchise ownership"
├── target_entity_type      -- "business" / "individual" / "organization"
│
├── ── AI Configuration ──
├── ai_system_prompt        -- Full system prompt for AI analysis
├── ai_analysis_prompt      -- Prompt template for individual prospect analysis
├── ai_market_prompt        -- Prompt template for market/area analysis
├── ai_summary_prompt       -- Prompt template for search results summary
│
├── ── Search Configuration ──
├── search_description      -- "Find potential catering clients in your area"
├── default_search_radius   -- 10.0
├── osm_query_tags          -- JSONB: [{"key":"office","value":"*"}, {"key":"tourism","value":"hotel"}]
├── google_place_types      -- JSONB: ["corporate_office", "hospital", "university"]
├── linkedin_search_config  -- JSONB: {"titles":[], "industries":[], "keywords":[]}
├── data_sources_enabled    -- JSONB: ["osm", "google_places", "bbb", "demographics"]
│
├── ── Scoring Configuration ──
├── relevance_field_name    -- "catering_potential" / "investment_readiness" / "venue_fit"
├── scoring_dimensions      -- JSONB: ["fit","engagement","intent","proximity"]
├── score_thresholds        -- JSONB: {"hot":80, "warm":60, "cold":40}
│
├── ── UI Configuration ──
├── dashboard_title         -- "Catering Prospect Dashboard" / "Franchise Buyer Pipeline"
├── search_panel_title      -- "AI Prospect Search"
├── results_empty_message   -- "Enter a location to find potential catering clients"
├── business_type_defaults  -- JSONB: checked/unchecked defaults for search filters
├── marketing_insights      -- JSONB: per-business-type marketing copy
│
├── ── Outreach Configuration ──
├── outreach_channels       -- JSONB: ["email","phone","in_person"]  vs  ["linkedin","email","phone"]
├── outreach_templates      -- JSONB: template text for each channel
│
├── is_active
├── created_at / updated_at
```

### Profile-Aware Scoring Rules

Scoring rules already support per-franchisee overrides. Extend this with a `profile_id` FK:

```
scoring_rules
├── ...existing fields...
├── profile_id (FK)         -- Which profile this rule belongs to
```

This way:
- **Catering profile** has rules like: "Has Conference Room (+5)", "Large Company (+8)", "Missing Address (-15)"
- **Franchise Buyer profile** has rules like: "MBA Degree (+10)", "Management Experience (+8)", "High Net Worth Area (+12)"
- **Hotel Conference profile** has rules like: "Fortune 500 Company (+15)", "Annual Conference History (+10)", "Travel Budget Indicator (+8)"

---

## Example Prospecting Profiles

### Profile 1: Franchise Catering Clients (Current Use Case)

```json
{
  "code": "franchise_catering",
  "name": "B2B Catering Prospect Discovery",
  "prospect_noun": "prospect",
  "target_entity_type": "business",
  "target_description": "businesses with corporate catering needs",

  "ai_system_prompt": "You are an expert business analyst specializing in corporate catering market analysis. Analyze businesses for their potential as catering clients. Consider factors like employee count, meeting facilities, company type, and location.",

  "osm_query_tags": [
    {"key": "office", "value": "*"},
    {"key": "tourism", "value": "hotel"},
    {"key": "amenity", "value": "conference_centre"},
    {"key": "amenity", "value": "hospital"},
    {"key": "amenity", "value": "university"}
  ],

  "google_place_types": [
    "corporate_office", "accounting", "bank", "insurance_agency",
    "hospital", "university", "lodging", "courthouse"
  ],

  "scoring_dimensions": ["fit", "engagement", "catering_potential", "proximity"],

  "data_sources_enabled": ["osm", "google_places", "bbb", "demographics"],

  "scoring_rules": [
    {"rule": "has_conference_room", "points": 5, "type": "bonus"},
    {"rule": "has_event_space", "points": 7, "type": "bonus"},
    {"rule": "large_company", "points": 8, "type": "bonus"},
    {"rule": "missing_address", "points": -15, "type": "penalty"}
  ]
}
```

### Profile 2: Franchise Buyer Prospecting (Planned Pivot)

```json
{
  "code": "franchise_buyers",
  "name": "Franchise Buyer Candidate Discovery",
  "prospect_noun": "candidate",
  "target_entity_type": "individual",
  "target_description": "individuals with potential to invest in a franchise",

  "ai_system_prompt": "You are an expert franchise development analyst. Evaluate individuals for their potential as franchise investors. Consider financial capacity ($100K-$500K liquid capital), entrepreneurial mindset, management experience, geographic fit with available territories, and career stage indicators like corporate transitions or early retirement.",

  "linkedin_search_config": {
    "titles": ["VP", "Director", "General Manager", "Regional Manager", "Owner"],
    "industries": ["Food Service", "Retail", "Hospitality", "Management"],
    "keywords": ["entrepreneur", "franchise", "business owner", "investor"],
    "company_sizes": ["51-200", "201-500", "501-1000", "1001-5000"]
  },

  "google_place_types": [],
  "osm_query_tags": [],

  "scoring_dimensions": ["investment_readiness", "entrepreneurial_fit", "territory_match", "timing"],

  "data_sources_enabled": ["linkedin", "demographics", "sba_data", "bizbuysell"],

  "scoring_rules": [
    {"rule": "mba_degree", "points": 10, "type": "bonus"},
    {"rule": "management_experience", "points": 8, "type": "bonus"},
    {"rule": "high_net_worth_area", "points": 12, "type": "bonus"},
    {"rule": "career_transition", "points": 15, "type": "bonus"},
    {"rule": "food_service_background", "points": 10, "type": "bonus"},
    {"rule": "no_linkedin_profile", "points": -20, "type": "penalty"},
    {"rule": "outside_territory", "points": -10, "type": "penalty"}
  ]
}
```

### Profile 3: Hotel Conference Venue Customers

```json
{
  "code": "hotel_conference",
  "name": "Conference & Event Venue Customer Discovery",
  "prospect_noun": "lead",
  "target_entity_type": "business",
  "target_description": "organizations that host conferences, retreats, and corporate events",

  "ai_system_prompt": "You are a hospitality industry analyst specializing in conference and event venue sales. Analyze organizations for their potential as conference venue customers. Consider annual event budgets, conference history, attendee counts, geographic preferences, and decision-maker accessibility. Focus on identifying organizations that host recurring annual events, off-site retreats, training programs, or large-scale conferences.",

  "osm_query_tags": [
    {"key": "office", "value": "company"},
    {"key": "office", "value": "corporation"},
    {"key": "amenity", "value": "university"},
    {"key": "office", "value": "association"},
    {"key": "office", "value": "ngo"}
  ],

  "google_place_types": [
    "corporate_office", "university", "local_government_office",
    "insurance_agency", "accounting", "bank"
  ],

  "scoring_dimensions": ["event_potential", "budget_fit", "geographic_match", "decision_maker_access"],

  "data_sources_enabled": ["osm", "google_places", "linkedin", "demographics"],

  "scoring_rules": [
    {"rule": "fortune_500", "points": 15, "type": "bonus"},
    {"rule": "annual_conference_history", "points": 10, "type": "bonus"},
    {"rule": "travel_budget_indicator", "points": 8, "type": "bonus"},
    {"rule": "large_org_500plus", "points": 12, "type": "bonus"},
    {"rule": "professional_association", "points": 10, "type": "bonus"},
    {"rule": "no_event_history", "points": -5, "type": "penalty"},
    {"rule": "small_org_under_20", "points": -8, "type": "penalty"}
  ]
}
```

### Profile 4: Commercial Real Estate Tenant Prospecting

```json
{
  "code": "commercial_re_tenants",
  "name": "Commercial Tenant Prospect Discovery",
  "prospect_noun": "tenant lead",
  "target_entity_type": "business",
  "target_description": "growing businesses seeking commercial office or retail space",

  "ai_system_prompt": "You are a commercial real estate market analyst. Evaluate businesses for their potential as commercial tenants. Consider growth trajectory, employee count trends, current lease expiration signals, geographic expansion plans, and space requirements. Focus on businesses showing signs of growth that may need larger or additional space.",

  "osm_query_tags": [
    {"key": "office", "value": "*"},
    {"key": "shop", "value": "*"},
    {"key": "amenity", "value": "coworking_space"}
  ],

  "scoring_dimensions": ["growth_trajectory", "space_need", "financial_stability", "timing"],

  "data_sources_enabled": ["osm", "google_places", "linkedin", "demographics"],

  "scoring_rules": [
    {"rule": "rapid_hiring", "points": 15, "type": "bonus"},
    {"rule": "coworking_current", "points": 12, "type": "bonus"},
    {"rule": "lease_expiring", "points": 10, "type": "bonus"},
    {"rule": "expansion_announced", "points": 15, "type": "bonus"},
    {"rule": "recently_funded", "points": 12, "type": "bonus"},
    {"rule": "declining_revenue", "points": -10, "type": "penalty"}
  ]
}
```

---

## Architecture Changes Required

### Layer 1: Data Model (Low Effort)

| Change | Description |
|--------|-------------|
| Add `prospecting_profiles` table | Core profile configuration as defined above |
| Add `profile_id` FK to `scoring_rules` | Scope rules to profiles |
| Add `profile_id` FK to `franchisees` or `company_types` | Associate tenants with profiles |
| Rename `catering_potential_score` → `relevance_score` | In `industries` table and `prospect_scores` table |
| Add `active_profile_id` to `app_config` | System-level default profile selection |
| Add profile-scoped `industries` | Different profiles need different industry lists with different relevance scores |

The `prospects` table itself is already generic — business_name, address, scores, status, contacts. No schema change needed there.

### Layer 2: AI Prompts (Medium Effort)

**Current state**: Hardcoded strings in `OpenAIEngine.cpp:253-370` and `GeminiEngine.cpp:249-366`.

**Target state**: Load prompts from `prospecting_profiles` table at startup, inject into AI calls.

```cpp
// BEFORE (hardcoded)
systemPrompt = "You are an expert business analyst specializing in corporate catering...";

// AFTER (profile-driven)
systemPrompt = activeProfile_.getAiSystemPrompt();
```

Changes needed:
- `AIEngine` base class gets a `setProfile(ProspectingProfile)` method
- `OpenAIEngine` and `GeminiEngine` read prompts from profile instead of hardcoded strings
- `ProspectingProfileDTO` loaded from database at app startup

This is a straightforward refactor — the prompt strings already exist as `std::string` variables; they just need to be initialized from the database instead of literal strings.

### Layer 3: Search Configuration (Medium Effort)

**Current state**: `OpenStreetMapAPI::buildCateringProspectQuery()` has hardcoded Overpass tags. `GooglePlacesAPI::getCateringProspectTypes()` has hardcoded place types.

**Target state**: Read search tags/types from `prospecting_profiles.osm_query_tags` and `google_place_types`.

```cpp
// BEFORE
query << "nw[\"office\"][\"name\"];";
query << "nw[\"tourism\"=\"hotel\"];";

// AFTER
for (const auto& tag : profile.getOsmQueryTags()) {
    if (tag.value == "*")
        query << "nw[\"" << tag.key << "\"][\"name\"];";
    else
        query << "nw[\"" << tag.key << "\"=\"" << tag.value << "\"];";
}
```

### Layer 4: Field Name Generalization (Medium-High Effort)

The field `cateringPotentialScore` needs to become `relevanceScore` (or keep both with an alias). This touches ~50 locations. It's mechanical but wide-reaching.

Alternatively, keep `relevanceScore` as the generic field in the model and let the Prospecting Profile define its label:
- Catering profile: displays as "Catering Potential"
- Franchise Buyer profile: displays as "Investment Readiness"
- Hotel Conference profile: displays as "Venue Fit Score"

### Layer 5: UI Text (Low-Medium Effort)

Replace hardcoded UI strings with profile-driven text:

```cpp
// BEFORE
searchDescription_->setText("Find potential catering clients in your area");

// AFTER
searchDescription_->setText(activeProfile_.getSearchDescription());
```

The Wt framework supports this naturally — all text is set via `setText()` or `setTitle()` calls that can read from configuration.

### Layer 6: Data Sources (Low Effort — Already Extensible)

The `data_sources_enabled` field in the profile controls which APIs to call. The `AISearchService` already orchestrates multiple sources in parallel — it just needs a check:

```cpp
if (profile.isDataSourceEnabled("osm")) {
    // launch OSM search
}
if (profile.isDataSourceEnabled("linkedin")) {
    // launch LinkedIn search (new integration)
}
```

---

## Tenant Configuration Model

The relationship between profiles and tenants:

```
                    ┌──────────────────────┐
                    │ prospecting_profiles │
                    │                      │
                    │ • franchise_catering  │
                    │ • franchise_buyers    │
                    │ • hotel_conference    │
                    │ • commercial_re       │
                    └──────────┬───────────┘
                               │
                    ┌──────────┴───────────┐
                    │    company_types      │
                    │                       │
                    │ • Food Service Fran.  │──→ profile: franchise_catering
                    │ • Franchise Dev Team  │──→ profile: franchise_buyers
                    │ • Hotel/Resort Chain  │──→ profile: hotel_conference
                    │ • Commercial RE Firm  │──→ profile: commercial_re
                    └──────────┬───────────┘
                               │
                    ┌──────────┴───────────┐
                    │    franchisees        │
                    │    (tenants)          │
                    │                       │
                    │ • Pittsburgh Catering │──→ company_type → profile
                    │ • Franchise Dev Corp │──→ company_type → profile
                    │ • Marriott Hotels    │──→ company_type → profile
                    │ • Metro Realty       │──→ company_type → profile
                    └──────────────────────┘
```

A single deployment can serve multiple tenants with different profiles simultaneously. Each franchisee (tenant) inherits a profile through their company type, or can have a direct profile override.

---

## Implementation Phases

### Phase 1: Profile Infrastructure (Foundation)

- Create `prospecting_profiles` table and DTO
- Create seed data for 2-3 profiles (catering + franchise buyers)
- Add `profile_id` FK to `company_types` and `scoring_rules`
- Load active profile at app startup
- Build `ProspectingProfileService` for CRUD operations

**Result**: Database stores profiles, but code still hardcoded. No behavior change yet.

### Phase 2: AI Prompt Injection (Biggest ROI)

- Refactor `AIEngine`, `OpenAIEngine`, `GeminiEngine` to accept profile
- Replace hardcoded prompt strings with `profile.getAiSystemPrompt()` etc.
- Test with both catering and franchise buyer profiles

**Result**: AI analysis adapts to use case. Same platform analyzes catering prospects OR franchise buyer candidates depending on profile.

### Phase 3: Search Configuration (Core Flexibility)

- Refactor `OpenStreetMapAPI` to build queries from profile OSM tags
- Refactor `GooglePlacesAPI` to use profile place types
- Add LinkedIn search support gated by profile `data_sources_enabled`
- Rename `buildCateringProspectQuery()` → `buildProspectQuery(profile)`

**Result**: Search results change based on profile. Hotel conference profile finds different businesses than catering profile.

### Phase 4: UI Generalization

- Replace hardcoded UI strings with profile-driven text
- Rename `cateringPotentialScore` displays to use `profile.relevance_field_name`
- Update marketing insights to read from profile configuration
- Update default search filter checkboxes from profile

**Result**: Platform looks and feels purpose-built for each use case, but it's the same codebase.

### Phase 5: Profile Management UI

- Admin settings page to create/edit profiles
- Profile selector for system administrators
- Per-franchisee profile override capability

**Result**: New prospecting use cases can be launched without code changes — just seed data and profile configuration.

---

## What Stays the Same (No Changes Needed)

These components are already generic and need no modification:

- **Database schema** for prospects, contacts, outreach, notes, tags
- **Authentication system** (users, sessions, roles, audit)
- **Geographic infrastructure** (regions, territories, demographics)
- **Map visualization** (Leaflet.js, OpenStreetMap tiles)
- **Scoring engine architecture** (just needs profile-scoped rules)
- **ApiLogicServerClient** REST patterns
- **ThreadPool** and parallel search orchestration
- **Geocoding service** with multi-provider fallback
- **Session management** and multi-tenancy
- **Audit trail** system

---

## Effort Estimate Summary

| Phase | Scope | Files Affected |
|-------|-------|---------------|
| Phase 1: Profile Infrastructure | New table, DTO, service, seed data | ~5 new files, ~3 modified |
| Phase 2: AI Prompt Injection | Replace hardcoded prompts | 3-4 files (AIEngine, OpenAI, Gemini) |
| Phase 3: Search Configuration | Parameterize search queries | 3-4 files (OSM API, Google API, AISearch) |
| Phase 4: UI Generalization | Replace hardcoded strings | 5-6 files (App, SearchPanel, Results, etc.) |
| Phase 5: Profile Management UI | Admin configuration page | 2-3 new widget files |

---

## Conclusion

The platform is well-positioned for this pivot. The multi-tenant architecture, configurable scoring engine, and service abstraction layers provide a solid foundation. The primary work is extracting hardcoded domain assumptions (AI prompts, search queries, UI text) into a database-driven Prospecting Profile.

The result would be a platform where deploying a new prospecting use case is a matter of:

1. Define a Prospecting Profile (AI prompts, search config, scoring rules, UI text)
2. Create seed data (industries, tags, default rules)
3. Assign the profile to a company type
4. Create tenant accounts under that company type

No code changes. No recompilation. No redeployment.

This transforms the platform from a "franchise catering prospecting tool" into a **"prospecting-as-a-platform"** that can serve any B2B or B2C prospecting use case — franchise buyers, hotel conference sales, commercial real estate, staffing agencies, equipment vendors, or any other vertical.
