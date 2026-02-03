# Franchise Buyer Targeting Transition Plan

## Executive Summary

This document outlines the strategic pivot of FranchiseAI from a **B2B catering lead generation platform** to a **franchise buyer prospecting platform**. The application will be repurposed to help franchise development teams identify, qualify, and engage potential franchise investors.

---

## Current State vs. New Direction

| Aspect | Current (Catering Leads) | New (Franchise Buyers) |
|--------|-------------------------|------------------------|
| **Target** | Businesses needing catering | Individuals wanting to own a franchise |
| **Lead Type** | B2B (corporate accounts) | B2C/Investor (entrepreneurs, executives) |
| **Value Proposition** | Find catering customers | Find franchise investors |
| **Revenue Model** | Franchise sells pizza | Franchise sells franchise opportunities |
| **Primary Data Source** | Google My Business, OSM | LinkedIn, Demographics |
| **Success Metric** | Catering orders | Franchise agreements signed |

---

## Part 1: AI Search Capabilities

### Target Profile: Ideal Franchise Buyer

The ideal franchise buyer candidate has the following characteristics:

- **Financial Capacity**: $100K-$500K liquid capital (typical pizza franchise investment)
- **Entrepreneurial Mindset**: Desire to own and operate a business
- **Geographic Fit**: Located near available franchise territories
- **Career Stage**: Corporate professionals seeking change, military transitioning, early retirees, small business owners expanding
- **Work Ethic**: Willing to be hands-on in day-to-day operations
- **Wealth Building Goals**: Looking for equity/asset building, not just income
- **Background**: Management experience, preferably in food service, retail, or hospitality

### Data Sources & API Integrations

#### 1. LinkedIn API (PRIMARY - New Integration)

LinkedIn will be the primary data source for identifying potential franchise buyers.

| Capability | Use Case |
|------------|----------|
| **People Search** | Find executives, managers, business owners by title/industry |
| **Career Transitions** | Identify recently departed corporate employees |
| **Education Filters** | MBA holders, business degrees |
| **Industry Experience** | Food service, retail, hospitality, management backgrounds |
| **Geographic Targeting** | Filter by metro area, zip code radius |
| **Company Size** | Current/past employers as income proxy |
| **Interests/Groups** | Franchise groups, entrepreneurship communities |

**LinkedIn API Products Required:**

| Product | Purpose | Access Level |
|---------|---------|--------------|
| **LinkedIn Marketing API** | Matched Audiences, Lead Gen Forms | Partner |
| **LinkedIn Sales Navigator API** | Advanced search, lead recommendations | Enterprise |
| **LinkedIn Talent Solutions** | Professional data, career history | Partner |

**Key Integration Features:**
- OAuth 2.0 authentication flow
- Profile search with advanced filters
- Career transition detection
- Profile enrichment and data sync
- InMail integration for outreach
- CRM synchronization

#### 2. Demographic/Economic Data (Repurpose Existing)

Leverage existing demographic integrations with new focus areas:

| Data Point | Use Case |
|------------|----------|
| **Household Income** | Target high-income zip codes ($150K+ HHI) |
| **Population Growth** | Areas with growing demand for franchises |
| **Business Formation Rates** | Entrepreneurial activity indicators |
| **Competitor Density** | Territory availability assessment |
| **Commercial Real Estate** | Available restaurant spaces |
| **Employment Statistics** | Corporate layoff areas, job market shifts |

#### 3. Google My Business (Repurpose)

Shift focus from catering targets to franchise ecosystem:

| Use Case | Description |
|----------|-------------|
| **Existing Small Business Owners** | Potential multi-unit operators |
| **Franchise Consultants** | Partnership and referral opportunities |
| **Business Brokers** | Lead generation sources |
| **Competitor Locations** | Territory mapping and gap analysis |
| **Commercial Realtors** | Site selection partners |

#### 4. New Data Source Integrations

| Source | Data Provided | Priority |
|--------|--------------|----------|
| **SBA Loan Data** | Franchise financing activity by geographic area | Medium |
| **Franchise Broker Networks** | Active franchise seekers with verified interest | High |
| **BizBuySell API** | Business opportunity seekers actively looking | High |
| **BusinessBroker.net** | Qualified buyer inquiries | High |
| **Franchise Expo Databases** | High-intent prospects who attended expos | High |
| **Chamber of Commerce** | Local business leaders and networkers | Medium |
| **SCORE/SBDC Programs** | Entrepreneurship program participants | Medium |
| **VetFran Program** | Military veterans interested in franchising | Medium |
| **FranNet/FranChoice** | Franchise consultant referral networks | High |

#### 5. Social & Content Signal Tracking

| Signal | Platform | Intent Indicator |
|--------|----------|------------------|
| **Franchise content engagement** | LinkedIn, YouTube | Active research phase |
| **Business opportunity group membership** | Facebook, LinkedIn | Interest signals |
| **Entrepreneurship newsletter subscriptions** | Various | Passive interest |
| **Franchise review site activity** | FranchiseGrade, FDD Exchange | Due diligence stage |
| **Franchise webinar attendance** | Zoom, GoToWebinar | High intent |
| **Discovery day RSVPs** | Internal tracking | Very high intent |

### AI Analysis Capabilities

The AI engine will be retrained/reprompted for franchise buyer analysis:

| Analysis Type | Purpose | Output |
|---------------|---------|--------|
| **Investor Profile Scoring** | Overall fit assessment | 0-100 score |
| **Financial Readiness Assessment** | Investment capacity estimation | Tier classification |
| **Entrepreneurial Fit Analysis** | Personality/background alignment | Fit report |
| **Territory Match** | Geographic fit with available territories | Territory recommendations |
| **Timing Signal Detection** | Career transitions, life events | Urgency rating |
| **Outreach Recommendations** | Personalized approach strategy | Talking points |
| **Objection Prediction** | Likely concerns based on profile | Objection handling guide |

---

## Part 2: Application Repurposing Plan

### Phase 1: Core Model Transformation (Foundation)

#### Data Model Changes

**Replace `BusinessInfo` with `CandidateProfile`:**

```
CandidateProfile
├── Personal Information
│   ├── name
│   ├── email
│   ├── phone
│   ├── location (city, state, zip)
│   └── linkedInUrl
│
├── Professional Background
│   ├── currentTitle
│   ├── currentEmployer
│   ├── yearsExperience
│   ├── industryBackground[]
│   ├── managementExperience (boolean)
│   ├── businessOwnershipHistory[]
│   └── education[]
│
├── Financial Indicators
│   ├── estimatedNetWorthTier (enum)
│   ├── investmentCapacityRange
│   ├── financingPreference (cash/SBA/partner)
│   ├── creditIndicators
│   └── liquiditySignals
│
├── Franchise Fit Signals
│   ├── entrepreneurialScore
│   ├── foodServiceExperience (boolean)
│   ├── retailExperience (boolean)
│   ├── multiUnitPotential (boolean)
│   ├── handsOnWillingness
│   └── familyInvolvement
│
├── Engagement Tracking
│   ├── source (LinkedIn/Referral/Expo/Organic)
│   ├── contentEngagement[]
│   ├── websiteVisits[]
│   ├── emailOpens[]
│   └── callHistory[]
│
├── Pipeline Status
│   ├── stage (Lead/Qualified/Applied/Approved/Funded)
│   ├── assignedTo
│   ├── nextAction
│   ├── nextActionDate
│   └── notes[]
│
└── AI Analysis
    ├── investorScore (0-100)
    ├── fitAssessment
    ├── financialReadiness
    ├── recommendedApproach
    ├── timingIndicators
    ├── predictedObjections[]
    └── analysisDate
```

#### Scoring Rules Transformation

| Current Rule (Catering) | New Rule (Investor) | Points |
|------------------------|---------------------|--------|
| Has conference room | Has management experience | +15 |
| High employee count | 10+ years professional experience | +10 |
| Good Google rating | Complete LinkedIn profile | +8 |
| BBB accreditation | Previous business ownership | +20 |
| Has event space | Food/retail industry background | +12 |
| Regular meetings | Recent career transition | +15 |
| Verified business | Verified contact information | +10 |
| Missing address | Missing LinkedIn profile | -15 |
| No phone number | No direct contact method | -10 |
| Closed business | Currently unemployed (risk) | -5 |

**New Scoring Categories:**

| Category | Weight | Factors |
|----------|--------|---------|
| **Financial Readiness** | 30% | Net worth tier, liquidity signals, financing history |
| **Professional Fit** | 25% | Industry experience, management level, business ownership |
| **Entrepreneurial Indicators** | 20% | Career moves, risk tolerance, self-starter signals |
| **Geographic Fit** | 15% | Territory availability, relocation willingness |
| **Timing Signals** | 10% | Career transition, life stage, urgency indicators |

#### Database Schema Updates

```sql
-- New table: candidate_profiles
CREATE TABLE candidate_profiles (
    id UUID PRIMARY KEY,
    store_location_id UUID REFERENCES store_locations(id),

    -- Personal Info
    first_name VARCHAR(100),
    last_name VARCHAR(100),
    email VARCHAR(255),
    phone VARCHAR(20),
    linkedin_url VARCHAR(500),
    city VARCHAR(100),
    state VARCHAR(50),
    zip_code VARCHAR(20),

    -- Professional Background
    current_title VARCHAR(200),
    current_employer VARCHAR(200),
    years_experience INTEGER,
    industry_background JSONB,
    has_management_experience BOOLEAN,
    has_business_ownership BOOLEAN,
    education JSONB,

    -- Financial Indicators
    net_worth_tier VARCHAR(50),
    investment_capacity_min INTEGER,
    investment_capacity_max INTEGER,
    financing_preference VARCHAR(50),

    -- Franchise Fit
    entrepreneurial_score INTEGER,
    has_food_service_exp BOOLEAN,
    has_retail_exp BOOLEAN,
    multi_unit_potential BOOLEAN,

    -- Pipeline
    pipeline_stage VARCHAR(50),
    assigned_to VARCHAR(100),
    next_action TEXT,
    next_action_date DATE,

    -- AI Analysis
    investor_score INTEGER,
    fit_assessment TEXT,
    recommended_approach TEXT,
    timing_indicators JSONB,
    predicted_objections JSONB,

    -- Metadata
    source VARCHAR(100),
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW(),
    is_active BOOLEAN DEFAULT TRUE
);

-- New table: candidate_interactions
CREATE TABLE candidate_interactions (
    id UUID PRIMARY KEY,
    candidate_id UUID REFERENCES candidate_profiles(id),
    interaction_type VARCHAR(50),
    interaction_date TIMESTAMP,
    notes TEXT,
    outcome VARCHAR(100),
    created_by VARCHAR(100),
    created_at TIMESTAMP DEFAULT NOW()
);

-- New table: franchise_territories
CREATE TABLE franchise_territories (
    id UUID PRIMARY KEY,
    territory_name VARCHAR(200),
    territory_code VARCHAR(50),
    city VARCHAR(100),
    state VARCHAR(50),
    zip_codes JSONB,
    status VARCHAR(50),
    population INTEGER,
    household_income_median INTEGER,
    competitor_count INTEGER,
    market_potential_score INTEGER,
    created_at TIMESTAMP DEFAULT NOW()
);
```

### Phase 2: LinkedIn Integration (Critical Path)

#### Implementation Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    LinkedIn Integration                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────┐    ┌──────────────┐    ┌───────────────┐  │
│  │ OAuth 2.0   │───▶│ LinkedInAPI  │───▶│ Profile       │  │
│  │ Flow        │    │ Service      │    │ Transformer   │  │
│  └─────────────┘    └──────────────┘    └───────────────┘  │
│         │                  │                    │           │
│         ▼                  ▼                    ▼           │
│  ┌─────────────┐    ┌──────────────┐    ┌───────────────┐  │
│  │ Token       │    │ Rate Limiter │    │ Candidate     │  │
│  │ Manager     │    │ & Cache      │    │ Profile       │  │
│  └─────────────┘    └──────────────┘    └───────────────┘  │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

#### LinkedInAPI Service Class

```cpp
// src/services/LinkedInAPI.h
class LinkedInAPI {
public:
    // Authentication
    std::string getAuthorizationUrl();
    bool exchangeCodeForToken(const std::string& code);
    bool refreshAccessToken();

    // Profile Search
    SearchResults searchProfiles(const ProfileSearchCriteria& criteria);
    CandidateProfile getProfileById(const std::string& linkedInId);
    std::vector<CandidateProfile> getSimilarProfiles(const std::string& profileId);

    // Career Transition Detection
    std::vector<CandidateProfile> findRecentCareerTransitions(
        const std::string& location,
        int radiusMiles,
        int daysBack = 90
    );

    // Saved Searches
    std::string createSavedSearch(const ProfileSearchCriteria& criteria);
    std::vector<CandidateProfile> executeSavedSearch(const std::string& searchId);

    // Enrichment
    CandidateProfile enrichProfile(const CandidateProfile& basic);

    // Messaging
    bool sendInMail(const std::string& profileId, const std::string& message);

private:
    std::string accessToken_;
    std::string refreshToken_;
    TokenManager tokenManager_;
    RateLimiter rateLimiter_;
    ResponseCache cache_;
};
```

#### Profile Search Criteria

```cpp
struct ProfileSearchCriteria {
    // Location
    std::string city;
    std::string state;
    std::string zipCode;
    int radiusMiles;

    // Professional
    std::vector<std::string> titles;
    std::vector<std::string> industries;
    int minYearsExperience;
    int maxYearsExperience;
    std::vector<std::string> companies;
    std::vector<std::string> schools;

    // Filters
    bool hasManagementExperience;
    bool isBusinessOwner;
    bool recentCareerChange;
    int careerChangeDaysBack;

    // LinkedIn Specific
    std::vector<std::string> groups;
    std::vector<std::string> skills;
    std::string connectionDegree; // 1st, 2nd, 3rd

    // Pagination
    int offset;
    int limit;
};
```

### Phase 3: AI Engine Updates

#### New AI Prompts

**Investor Analysis Prompt:**

```
You are a franchise development analyst. Analyze this candidate profile
and assess their potential as a franchise investor.

CANDIDATE PROFILE:
{profile_json}

FRANCHISE REQUIREMENTS:
- Initial investment: $150,000 - $400,000
- Liquid capital required: $100,000 minimum
- Net worth required: $300,000 minimum
- Time commitment: Full-time owner-operator preferred

Provide analysis in the following JSON format:
{
    "investorScore": 0-100,
    "financialReadiness": "High/Medium/Low",
    "entrepreneurialFit": "Excellent/Good/Fair/Poor",
    "keyStrengths": ["strength1", "strength2"],
    "potentialConcerns": ["concern1", "concern2"],
    "predictedObjections": ["objection1", "objection2"],
    "recommendedApproach": "Personalized outreach strategy...",
    "timingAssessment": "Ready now/3-6 months/6-12 months/Long-term nurture",
    "talkingPoints": ["point1", "point2", "point3"]
}
```

**Territory Match Prompt:**

```
Analyze the match between this candidate and available franchise territories.

CANDIDATE LOCATION: {candidate_location}
RELOCATION WILLINGNESS: {relocation_preference}

AVAILABLE TERRITORIES:
{territories_json}

Provide territory recommendations with reasoning.
```

#### AI Analysis Output Structure

```cpp
struct InvestorAnalysis {
    int investorScore;                      // 0-100
    std::string financialReadiness;         // High/Medium/Low
    std::string entrepreneurialFit;         // Excellent/Good/Fair/Poor
    std::vector<std::string> keyStrengths;
    std::vector<std::string> potentialConcerns;
    std::vector<std::string> predictedObjections;
    std::string recommendedApproach;
    std::string timingAssessment;
    std::vector<std::string> talkingPoints;
    std::vector<TerritoryMatch> territoryMatches;
    double confidenceScore;
    std::string analysisTimestamp;
};
```

### Phase 4: UI Transformation

#### Page Mapping

| Current Page | New Page | Key Changes |
|--------------|----------|-------------|
| **Dashboard** | **Candidate Dashboard** | Pipeline overview, hot candidates, conversion metrics |
| **AI Search** | **Candidate Search** | LinkedIn filters, investor criteria, territory targeting |
| **My Prospects** | **Candidate Pipeline** | Pipeline stages, candidate cards, AI insights |
| **Demographics** | **Territory Analysis** | Available territories, market opportunity maps |
| **Settings** | **Settings** | LinkedIn credentials, outreach templates, scoring rules |
| **Reports** | **Pipeline Reports** | Funnel metrics, conversion rates, source attribution |

#### New Dashboard Widgets

```
┌─────────────────────────────────────────────────────────────┐
│                    Candidate Dashboard                       │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐         │
│  │ New Leads   │  │ Qualified   │  │ In Process  │         │
│  │     24      │  │     12      │  │      5      │         │
│  └─────────────┘  └─────────────┘  └─────────────┘         │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              Hot Candidates (Score 80+)              │   │
│  │  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐   │   │
│  │  │ John D. │ │ Sarah M.│ │ Mike R. │ │ Lisa T. │   │   │
│  │  │ Score:92│ │ Score:88│ │ Score:85│ │ Score:82│   │   │
│  │  │ Ready   │ │ 3-6 mo  │ │ Ready   │ │ Ready   │   │   │
│  │  └─────────┘ └─────────┘ └─────────┘ └─────────┘   │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
│  ┌─────────────────────┐  ┌─────────────────────────────┐  │
│  │ Pipeline Funnel     │  │ Recent Activity             │  │
│  │                     │  │                             │  │
│  │ Leads:      124     │  │ • John D. moved to Qualified│  │
│  │ Qualified:   45     │  │ • New lead: Maria S.        │  │
│  │ Applied:     12     │  │ • FDD sent to Mike R.       │  │
│  │ Approved:     4     │  │ • Discovery day: Lisa T.    │  │
│  │ Funded:       1     │  │                             │  │
│  └─────────────────────┘  └─────────────────────────────┘  │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

#### Candidate Search Page

```
┌─────────────────────────────────────────────────────────────┐
│                    Candidate Search                          │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Location: [Pittsburgh, PA    ] Radius: [25 miles ▼]       │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ Professional Criteria                                │   │
│  │                                                      │   │
│  │ Titles: [Executive, Director, VP, Owner      ]      │   │
│  │ Industries: [□ Food Service □ Retail □ Hospitality] │   │
│  │ Experience: [10+ years ▼]                           │   │
│  │ □ Has Management Experience                         │   │
│  │ □ Previous Business Owner                           │   │
│  │ □ Recent Career Transition (90 days)                │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ Investment Criteria                                  │   │
│  │                                                      │   │
│  │ Estimated Net Worth: [$500K+ ▼]                     │   │
│  │ Investment Timeline: [Ready within 6 months ▼]      │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
│  [🔍 Search Candidates]                                    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

#### Candidate Card Design

```
┌─────────────────────────────────────────────────────────────┐
│ [Photo] John Davidson                          Score: [92] │
│         VP of Operations, Retired                          │
│         Pittsburgh, PA                                      │
├─────────────────────────────────────────────────────────────┤
│ 💼 25 years experience | 🏢 Former: Marriott Hotels        │
│ 🎓 MBA, Penn State | 💰 Est. Net Worth: $750K+             │
├─────────────────────────────────────────────────────────────┤
│ AI Insights:                                                │
│ "Strong candidate with extensive hospitality management     │
│  experience. Recent retirement suggests ready to invest     │
│  time and capital. Food service background is excellent fit.│
│  Recommend discovery day invitation."                       │
├─────────────────────────────────────────────────────────────┤
│ Key Strengths:                                              │
│ • 15+ years food service management                         │
│ • Multi-unit operations experience                          │
│ • Strong financial position                                 │
├─────────────────────────────────────────────────────────────┤
│ Recommended Approach:                                       │
│ • Lead with ROI and wealth-building potential               │
│ • Emphasize semi-absentee ownership options                 │
│ • Connect with existing franchisee in hospitality           │
├─────────────────────────────────────────────────────────────┤
│ [📧 Send InMail] [📞 Log Call] [➕ Add to Pipeline]        │
└─────────────────────────────────────────────────────────────┘
```

### Phase 5: New Feature Additions

#### Territory Management

```
┌─────────────────────────────────────────────────────────────┐
│                   Territory Manager                          │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │                    [Interactive Map]                 │   │
│  │                                                      │   │
│  │     🟢 Available    🟡 Reserved    🔴 Sold          │   │
│  │                                                      │   │
│  │              [Map with territory overlays]           │   │
│  │                                                      │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
│  Available Territories:                                     │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ North Hills, PA    | Pop: 125K | HHI: $85K | Score: 87│  │
│  │ South Side, PA     | Pop: 98K  | HHI: $72K | Score: 79│  │
│  │ Monroeville, PA    | Pop: 145K | HHI: $91K | Score: 92│  │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

#### Pipeline Workflow

```
Lead ──▶ Qualified ──▶ Applied ──▶ Approved ──▶ Funded
  │          │            │           │           │
  │          │            │           │           │
  ▼          ▼            ▼           ▼           ▼
Initial    Discovery    FDD Sent    Background   Franchise
Contact    Day Invite   & Review    Check Pass   Agreement
```

#### Investment Calculator

Provide prospects with ROI projections:
- Initial investment breakdown
- Projected revenue scenarios
- Break-even analysis
- 5-year wealth building projection

#### CRM Integration

| CRM | Integration Type | Data Sync |
|-----|------------------|-----------|
| **Salesforce** | Native API | Bi-directional |
| **HubSpot** | Native API | Bi-directional |
| **Zoho CRM** | REST API | Bi-directional |
| **FranConnect** | Partner API | Bi-directional |

---

## Implementation Roadmap

### Timeline Overview

```
Month 1-2          Month 3-4           Month 5-6          Month 7-8
──────────────    ──────────────     ──────────────    ──────────────
Phase 1:          Phase 2:           Phase 3:          Phase 4-5:
Foundation        LinkedIn           AI Engine         UI & Advanced
                  Integration        Updates           Features
```

### Phase 1: Foundation (Weeks 1-8)

| Week | Tasks | Deliverables |
|------|-------|--------------|
| 1-2 | Data model design | CandidateProfile schema, ER diagrams |
| 3-4 | Database migration | New tables, migration scripts |
| 5-6 | Scoring engine update | New scoring rules, configuration UI |
| 7-8 | Search criteria refactor | New search filters, API updates |

### Phase 2: LinkedIn Integration (Weeks 9-16)

| Week | Tasks | Deliverables |
|------|-------|--------------|
| 9-10 | LinkedIn API setup | OAuth flow, token management |
| 11-12 | Profile search implementation | Search API, result parsing |
| 13-14 | Career transition detection | Algorithm, alerting |
| 15-16 | Profile enrichment | Data mapping, caching |

### Phase 3: AI Engine Updates (Weeks 17-24)

| Week | Tasks | Deliverables |
|------|-------|--------------|
| 17-18 | Prompt engineering | New analysis prompts |
| 19-20 | Investor scoring AI | Scoring integration |
| 21-22 | Outreach recommendations | Personalization engine |
| 23-24 | Territory matching | Match algorithm |

### Phase 4: UI Transformation (Weeks 25-32)

| Week | Tasks | Deliverables |
|------|-------|--------------|
| 25-26 | Dashboard redesign | New widgets, metrics |
| 27-28 | Search page rebuild | New filters, results display |
| 29-30 | Pipeline management | Stage workflow, candidate cards |
| 31-32 | Territory analysis | Map updates, territory view |

### Phase 5: Advanced Features (Weeks 33-40)

| Week | Tasks | Deliverables |
|------|-------|--------------|
| 33-34 | Territory manager | Full territory CRUD |
| 35-36 | Investment calculator | ROI projections |
| 37-38 | CRM integration | Salesforce/HubSpot sync |
| 39-40 | Reporting & polish | Pipeline reports, bug fixes |

---

## Technical Decisions

### LinkedIn API Strategy

**Recommended Approach: Sales Navigator API**

| Option | Pros | Cons | Recommendation |
|--------|------|------|----------------|
| **Marketing API** | Lead Gen Forms, Matched Audiences | Requires ad spend, limited profile data | Supplement |
| **Sales Navigator API** | Deep profile search, CRM sync, InMail | Expensive ($100+/user/month) | **Primary** |
| **Recruiter Lite API** | Profile access, InMail | Designed for recruiting, ToS concerns | Avoid |

**Implementation Notes:**
- Apply for LinkedIn Partner Program
- Budget $1,200+/year per user for Sales Navigator
- Implement robust rate limiting (100 requests/day typical)
- Cache aggressively to minimize API calls

### Data Privacy & Compliance

| Requirement | Implementation |
|-------------|----------------|
| **GDPR Compliance** | Consent tracking, data deletion, export |
| **CCPA Compliance** | Opt-out mechanism, disclosure |
| **LinkedIn ToS** | No scraping, respect rate limits, proper attribution |
| **Data Retention** | 2-year retention, automated purge |
| **Consent Management** | Explicit opt-in for outreach |

### Infrastructure Considerations

| Component | Current | Recommended |
|-----------|---------|-------------|
| **Database** | PostgreSQL | PostgreSQL (no change) |
| **Caching** | In-memory | Redis for distributed caching |
| **Queue** | None | RabbitMQ for async processing |
| **Search** | SQL queries | Elasticsearch for profile search |

---

## Success Metrics

### Key Performance Indicators (KPIs)

| Metric | Target | Measurement |
|--------|--------|-------------|
| **Candidate Discovery Rate** | 50+ qualified/month | New qualified leads |
| **AI Accuracy** | 80%+ | Scored 80+ that convert |
| **Pipeline Velocity** | 45 days avg | Lead to Application time |
| **Conversion Rate** | 5% | Lead to Funded |
| **Cost per Lead** | <$50 | Total cost / qualified leads |
| **User Adoption** | 80% | Active users / total users |

### Reporting Dashboard

```
Monthly Metrics:
├── Candidates Discovered: XXX
├── Candidates Qualified: XX
├── Applications Received: X
├── Approvals: X
├── Franchises Funded: X
├── Pipeline Value: $X.XM
├── Average Score of Converts: XX
└── Top Performing Territories: [List]
```

---

## Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| LinkedIn API access denied | Medium | High | Alternative data sources, broker partnerships |
| High API costs | Medium | Medium | Aggressive caching, tiered usage |
| Low AI accuracy | Low | High | Continuous prompt tuning, feedback loops |
| User adoption resistance | Medium | Medium | Training, gradual rollout |
| Data privacy issues | Low | High | Legal review, compliance framework |
| Integration complexity | Medium | Medium | Phased approach, MVP first |

---

## Appendix

### A. LinkedIn API Resources

- [LinkedIn Marketing API Documentation](https://docs.microsoft.com/en-us/linkedin/marketing/)
- [Sales Navigator API](https://docs.microsoft.com/en-us/linkedin/sales/)
- [LinkedIn Partner Program](https://business.linkedin.com/marketing-solutions/partners)

### B. Franchise Industry Data Sources

- International Franchise Association (IFA)
- Franchise Times Top 400
- Entrepreneur Franchise 500
- FRANdata

### C. Competitor Analysis

| Competitor | Focus | Differentiator |
|------------|-------|----------------|
| FranConnect | Franchise CRM | Full franchise management |
| ClientTether | Lead follow-up | Automated nurturing |
| Scorpion | Digital marketing | Lead generation |
| **FranchiseAI** | AI-powered prospecting | Intelligent candidate discovery |

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-02-03 | Claude AI | Initial draft |

---

*This document outlines the strategic transition plan for FranchiseAI. Implementation details may be adjusted based on technical discoveries and business requirements during development.*
