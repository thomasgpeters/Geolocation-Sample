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

#### Twilio SMS Outbound Communication

Integrate Twilio Programmable Messaging APIs to enable direct SMS-based outreach to franchise buyer candidates. SMS provides a high-open-rate, immediate-delivery channel for pipeline hooks, marketing campaigns, and long-term grooming sequences.

##### Twilio Integration Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                  Twilio SMS Integration Layer                        │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌──────────────┐    ┌──────────────┐    ┌───────────────────────┐  │
│  │ TwilioSMS    │───▶│ Campaign     │───▶│ Message Template      │  │
│  │ Service      │    │ Engine       │    │ Engine                │  │
│  └──────────────┘    └──────────────┘    └───────────────────────┘  │
│         │                  │                       │                │
│         ▼                  ▼                       ▼                │
│  ┌──────────────┐    ┌──────────────┐    ┌───────────────────────┐  │
│  │ Webhook      │    │ Scheduler    │    │ AI Personalization    │  │
│  │ Handler      │    │ (Cron/Queue) │    │ (GPT message tuning)  │  │
│  └──────────────┘    └──────────────┘    └───────────────────────┘  │
│         │                  │                       │                │
│         ▼                  ▼                       ▼                │
│  ┌──────────────┐    ┌──────────────┐    ┌───────────────────────┐  │
│  │ Delivery     │    │ Opt-In/Out   │    │ Analytics &           │  │
│  │ Tracking     │    │ Manager      │    │ Reporting             │  │
│  └──────────────┘    └──────────────┘    └───────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

##### Twilio API Products Required

| Product | Purpose | Pricing Model |
|---------|---------|---------------|
| **Programmable Messaging** | Send/receive SMS and MMS | Per-message ($0.0079/SMS outbound) |
| **Messaging Services** | Sender pools, opt-out handling, rate limiting | Included with Messaging |
| **Twilio Phone Numbers** | Dedicated sending numbers (local, toll-free, short code) | $1-$1,000/month depending on type |
| **Twilio Verify** | Phone number validation before sending | Per-verification ($0.05) |
| **Twilio Lookup** | Carrier/line-type detection (mobile vs. landline) | Per-lookup ($0.01) |

##### TwilioSMSService Class

```cpp
// src/services/TwilioSMSService.h
class TwilioSMSService {
public:
    // Configuration
    void configure(const std::string& accountSid,
                   const std::string& authToken,
                   const std::string& messagingServiceSid);

    // Single Message
    SendResult sendSMS(const std::string& toNumber,
                       const std::string& messageBody);
    SendResult sendTemplateSMS(const std::string& toNumber,
                               const std::string& templateId,
                               const TemplateContext& context);
    SendResult sendMMS(const std::string& toNumber,
                       const std::string& messageBody,
                       const std::string& mediaUrl);

    // Bulk / Campaign
    CampaignResult launchCampaign(const Campaign& campaign);
    void pauseCampaign(const std::string& campaignId);
    void resumeCampaign(const std::string& campaignId);
    CampaignStats getCampaignStats(const std::string& campaignId);

    // Pipeline Hooks
    void registerHook(PipelineStage stage, const MessageTemplate& tmpl);
    void triggerHook(PipelineStage stage, const CandidateProfile& candidate);

    // Grooming Sequences
    std::string enrollInSequence(const std::string& candidateId,
                                  const std::string& sequenceId);
    void advanceSequenceStep(const std::string& enrollmentId);
    void pauseSequence(const std::string& enrollmentId);
    void removeFromSequence(const std::string& enrollmentId);

    // Compliance
    bool verifyOptIn(const std::string& phoneNumber);
    void processOptOut(const std::string& phoneNumber);
    bool isNumberMobile(const std::string& phoneNumber);
    ConsentStatus getConsentStatus(const std::string& candidateId);

    // Webhooks (inbound from Twilio)
    void handleDeliveryCallback(const DeliveryEvent& event);
    void handleInboundSMS(const InboundMessage& message);
    void handleOptOutCallback(const OptOutEvent& event);

    // Analytics
    MessagingStats getStats(const DateRange& range);
    std::vector<MessageLog> getMessageHistory(const std::string& candidateId);

private:
    std::string accountSid_;
    std::string authToken_;
    std::string messagingServiceSid_;
    RateLimiter rateLimiter_;
    DeliveryTracker deliveryTracker_;
    ConsentManager consentManager_;
};
```

##### SMS Communication Types

**1. Pipeline Hook Messages (Event-Driven)**

Automated SMS messages triggered by candidate pipeline stage changes and system events.

| Hook Trigger | Message Purpose | Timing |
|-------------|-----------------|--------|
| **New Lead Added** | Welcome / introduction | Immediate |
| **Qualified Stage** | Discovery day invitation | Immediate |
| **FDD Sent** | Confirmation + next steps | Immediate |
| **Application Received** | Acknowledgment + timeline | Within 1 hour |
| **Approval Granted** | Congratulations + onboarding info | Immediate |
| **Stale Lead (7 days)** | Re-engagement nudge | Automated |
| **Discovery Day Reminder** | Event reminder | 24 hours before |
| **Document Deadline** | Reminder to complete paperwork | 48 hours before |
| **Birthday/Anniversary** | Personal touch | Day-of |

**Hook Registration Model:**

```cpp
struct PipelineHook {
    std::string hookId;
    PipelineStage triggerStage;      // Lead, Qualified, Applied, etc.
    HookEvent triggerEvent;          // STAGE_ENTER, STAGE_EXIT, STALE, REMINDER
    std::string messageTemplateId;
    int delayMinutes;                // 0 = immediate
    bool requiresOptIn;              // true (always)
    bool isActive;
    std::string createdBy;
};
```

**2. Marketing Campaign Messages (Bulk Outreach)**

Targeted SMS campaigns to engage potential franchise buyers at scale.

| Campaign Type | Target Audience | Content Focus |
|--------------|-----------------|---------------|
| **Territory Launch** | Candidates in new territory area | "New franchise opportunity in [City]" |
| **Franchise Expo Follow-Up** | Expo attendees | "Great meeting you at [Expo] - next steps" |
| **Webinar Invitation** | Qualified leads, warm prospects | "Join our franchise info session" |
| **Success Story** | All pipeline candidates | "Meet [Name], our newest franchise owner" |
| **Limited Availability** | High-score candidates in target territory | "Only [N] territories remaining in [Area]" |
| **Seasonal Promotion** | All opted-in candidates | Reduced franchise fee, financing offers |
| **Referral Request** | Funded franchisees | "Know someone who'd be a great owner?" |

**Campaign Configuration:**

```cpp
struct Campaign {
    std::string campaignId;
    std::string name;
    CampaignType type;               // MARKETING, ANNOUNCEMENT, REFERRAL
    std::string messageTemplateId;

    // Targeting
    CandidateFilter targetFilter;    // Pipeline stage, score range, territory, etc.
    std::vector<std::string> excludeCandidateIds;

    // Scheduling
    ScheduleType schedule;           // IMMEDIATE, SCHEDULED, RECURRING
    std::string scheduledTime;       // ISO 8601
    std::string recurringCron;       // e.g. "0 10 * * MON" (Monday 10 AM)
    std::string timezone;

    // Throttling
    int maxMessagesPerHour;          // Rate limiting
    int maxMessagesPerDay;
    bool respectQuietHours;          // No messages 9PM-9AM local time

    // Tracking
    CampaignStatus status;           // DRAFT, SCHEDULED, ACTIVE, PAUSED, COMPLETED
    int totalRecipients;
    int messagesSent;
    int messagesDelivered;
    int messagesFailed;
    int optOuts;
    int responses;
};
```

**3. Grooming Campaign Messages (Nurture Sequences)**

Multi-touch SMS sequences designed to build trust, educate, and move candidates through the franchise buying journey over time.

| Sequence | Target Stage | Duration | Touches | Goal |
|----------|-------------|----------|---------|------|
| **Awareness Drip** | New Leads | 30 days | 6 messages | Educate on franchise ownership benefits |
| **Qualification Nurture** | Qualified | 21 days | 5 messages | Build urgency, share success stories |
| **FDD Follow-Up** | Applied (FDD Sent) | 14 days | 4 messages | Answer concerns, encourage review |
| **Re-Engagement** | Stale Leads (30+ days) | 45 days | 4 messages | Rekindle interest |
| **Post-Discovery Day** | Post-Event | 10 days | 3 messages | Close the deal, address objections |
| **Long-Term Nurture** | Not Ready (6-12 mo) | 6 months | 12 messages | Stay top-of-mind |

**Grooming Sequence Model:**

```cpp
struct GroomingSequence {
    std::string sequenceId;
    std::string name;
    std::string description;
    PipelineStage targetStage;
    int totalSteps;
    bool autoEnroll;                 // Auto-enroll when candidate enters target stage
    bool exitOnStageChange;          // Remove from sequence if pipeline stage advances

    std::vector<SequenceStep> steps;
};

struct SequenceStep {
    int stepNumber;
    int delayDays;                   // Days after previous step (or enrollment)
    std::string messageTemplateId;
    std::string sendTime;            // Preferred time of day "10:00"
    bool skipWeekends;
    std::string exitCondition;       // e.g. "candidate.stage != 'Lead'"

    // A/B Testing
    bool abTestEnabled;
    std::string variantATemplateId;
    std::string variantBTemplateId;
    int variantASplitPercent;        // e.g. 50
};

struct SequenceEnrollment {
    std::string enrollmentId;
    std::string candidateId;
    std::string sequenceId;
    int currentStep;
    EnrollmentStatus status;         // ACTIVE, PAUSED, COMPLETED, EXITED
    std::string enrolledAt;
    std::string nextMessageAt;
    std::string completedAt;
    std::string exitReason;          // "completed", "opt_out", "stage_changed", "manual"
};
```

**Example: Awareness Drip Sequence (30 Days)**

| Day | Message Theme | Example Content |
|-----|--------------|-----------------|
| 1 | Welcome | "Hi [Name], thanks for your interest in [Franchise]. We help entrepreneurs build wealth through franchise ownership. Reply STOP to opt out." |
| 5 | Social Proof | "[Name], did you know our average franchise owner reaches profitability within 18 months? Here's how: [link]" |
| 10 | Education | "Thinking about franchise ownership? Here are the top 5 questions every investor asks: [link]" |
| 17 | Success Story | "Meet [Owner Name] - went from corporate VP to franchise owner in [City]. Read their story: [link]" |
| 24 | Territory Alert | "[Name], we have [N] territories available near [City]. Interested in learning more? Reply YES." |
| 30 | Call to Action | "[Name], ready to take the next step? Schedule a discovery call with our franchise team: [link]" |

##### AI-Powered Message Personalization

Leverage the existing AI engine (OpenAI/Gemini) to personalize SMS content based on candidate profiles.

```
AI Personalization Prompt:

You are a franchise development copywriter. Personalize this SMS template
for the candidate based on their profile.

TEMPLATE: "{template_text}"
CANDIDATE PROFILE: {profile_json}
MESSAGE TYPE: {hook|marketing|grooming}
TONE: Professional, warm, entrepreneurial
CHARACTER LIMIT: 160 (SMS segment)

Rules:
- Keep under 160 characters for single SMS segment when possible
- Reference candidate's industry or background naturally
- Match urgency level to pipeline stage
- Never include financial guarantees or earnings claims
- Always include opt-out language on first message

Return personalized message text only.
```

| Personalization Factor | Example Adaptation |
|-----------------------|-------------------|
| **Industry Background** | "Your hospitality experience is exactly what makes great franchise owners..." |
| **Career Stage** | "Many executives like you have found franchise ownership the perfect next chapter..." |
| **Location** | "The [City] market is growing fast - perfect timing for a new franchise..." |
| **Financial Tier** | Adjust investment language: "accessible investment" vs. "portfolio diversification" |
| **Engagement Level** | First touch = softer; 5th touch = more direct call-to-action |

##### SMS Compliance & Regulations

**TCPA (Telephone Consumer Protection Act) Requirements:**

| Requirement | Implementation |
|-------------|----------------|
| **Prior Express Written Consent** | Digital opt-in form with clear disclosure before any SMS |
| **Opt-Out Handling** | Honor STOP/UNSUBSCRIBE/CANCEL/QUIT/END keywords immediately |
| **Quiet Hours** | No messages before 8 AM or after 9 PM in recipient's local timezone |
| **Message Frequency Disclosure** | "Msg frequency varies. Msg & data rates may apply." on opt-in |
| **Business Identification** | Every message must identify the sender business name |
| **Record Retention** | Store consent records, opt-out timestamps, message logs for 5 years |

**10DLC Registration (Required for A2P SMS):**

| Step | Description | Status |
|------|-------------|--------|
| 1 | Register brand with The Campaign Registry (TCR) | Required |
| 2 | Create campaign use case (Franchise Lead Nurturing) | Required |
| 3 | Obtain 10DLC phone numbers | Required |
| 4 | Submit campaign for carrier approval | Required |
| 5 | Maintain throughput within approved tier | Ongoing |

**Consent Management Database:**

```sql
-- New table: sms_consent
CREATE TABLE sms_consent (
    id UUID PRIMARY KEY,
    candidate_id UUID REFERENCES candidate_profiles(id),
    phone_number VARCHAR(20) NOT NULL,
    consent_status VARCHAR(20) NOT NULL,        -- OPTED_IN, OPTED_OUT, PENDING
    consent_source VARCHAR(100),                -- web_form, discovery_day, expo, manual
    consent_timestamp TIMESTAMP NOT NULL,
    opt_out_timestamp TIMESTAMP,
    opt_out_keyword VARCHAR(20),                -- STOP, UNSUBSCRIBE, etc.
    ip_address VARCHAR(45),                     -- For web form consent
    consent_text TEXT,                          -- Exact disclosure shown
    created_at TIMESTAMP DEFAULT NOW()
);

-- New table: sms_messages
CREATE TABLE sms_messages (
    id UUID PRIMARY KEY,
    candidate_id UUID REFERENCES candidate_profiles(id),
    campaign_id UUID,
    sequence_enrollment_id UUID,
    hook_id UUID,
    message_type VARCHAR(20) NOT NULL,          -- HOOK, MARKETING, GROOMING
    twilio_message_sid VARCHAR(50),
    from_number VARCHAR(20),
    to_number VARCHAR(20),
    message_body TEXT NOT NULL,
    media_url VARCHAR(500),
    direction VARCHAR(10) NOT NULL,             -- OUTBOUND, INBOUND
    status VARCHAR(20),                         -- QUEUED, SENT, DELIVERED, FAILED, UNDELIVERED
    error_code VARCHAR(10),
    error_message TEXT,
    sent_at TIMESTAMP,
    delivered_at TIMESTAMP,
    created_at TIMESTAMP DEFAULT NOW()
);

-- New table: sms_campaigns
CREATE TABLE sms_campaigns (
    id UUID PRIMARY KEY,
    name VARCHAR(200) NOT NULL,
    campaign_type VARCHAR(50),
    message_template_id UUID,
    target_filter JSONB,
    schedule_type VARCHAR(20),
    scheduled_time TIMESTAMP,
    recurring_cron VARCHAR(50),
    timezone VARCHAR(50),
    max_messages_per_hour INTEGER DEFAULT 60,
    respect_quiet_hours BOOLEAN DEFAULT TRUE,
    status VARCHAR(20) DEFAULT 'DRAFT',
    total_recipients INTEGER DEFAULT 0,
    messages_sent INTEGER DEFAULT 0,
    messages_delivered INTEGER DEFAULT 0,
    messages_failed INTEGER DEFAULT 0,
    opt_outs INTEGER DEFAULT 0,
    responses INTEGER DEFAULT 0,
    created_by VARCHAR(100),
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- New table: grooming_sequences
CREATE TABLE grooming_sequences (
    id UUID PRIMARY KEY,
    name VARCHAR(200) NOT NULL,
    description TEXT,
    target_stage VARCHAR(50),
    total_steps INTEGER,
    auto_enroll BOOLEAN DEFAULT FALSE,
    exit_on_stage_change BOOLEAN DEFAULT TRUE,
    is_active BOOLEAN DEFAULT TRUE,
    steps JSONB NOT NULL,
    created_at TIMESTAMP DEFAULT NOW()
);

-- New table: sequence_enrollments
CREATE TABLE sequence_enrollments (
    id UUID PRIMARY KEY,
    candidate_id UUID REFERENCES candidate_profiles(id),
    sequence_id UUID REFERENCES grooming_sequences(id),
    current_step INTEGER DEFAULT 0,
    status VARCHAR(20) DEFAULT 'ACTIVE',
    enrolled_at TIMESTAMP DEFAULT NOW(),
    next_message_at TIMESTAMP,
    completed_at TIMESTAMP,
    exit_reason VARCHAR(100)
);
```

##### SMS Settings UI

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Settings > SMS Configuration                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Twilio Credentials                                                 │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Account SID:          [AC_______________________________]    │  │
│  │ Auth Token:           [••••••••••••••••••••••••••••••••]     │  │
│  │ Messaging Service ID: [MG_______________________________]    │  │
│  │ Status: ✅ Connected                         [Test Connection]│  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  Sending Preferences                                                │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Default Sender Number: [(412) 555-0100          ▼]           │  │
│  │ Quiet Hours:           [9:00 PM] to [8:00 AM]               │  │
│  │ Max Messages/Day:      [200        ]                        │  │
│  │ Timezone:              [America/New_York       ▼]           │  │
│  │ □ Respect weekends (no marketing on Sat/Sun)                │  │
│  │ ☑ Require opt-in before first message                       │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  Pipeline Hooks                                                     │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ ☑ New Lead Welcome          Template: [Welcome v2       ▼]  │  │
│  │ ☑ Qualified Notification    Template: [Discovery Invite ▼]  │  │
│  │ ☑ FDD Sent Confirmation     Template: [FDD Confirm      ▼]  │  │
│  │ ☑ Application Received      Template: [App Received     ▼]  │  │
│  │ ☑ Approval Granted          Template: [Congrats         ▼]  │  │
│  │ ☑ Stale Lead (7 days)       Template: [Re-Engage        ▼]  │  │
│  │ □ Discovery Day Reminder    Template: [DD Reminder      ▼]  │  │
│  │                                                              │  │
│  │ [+ Add Custom Hook]                                          │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  Grooming Sequences                                                 │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Name                  │ Target    │ Steps │ Enrolled │ Status │  │
│  │ Awareness Drip        │ Lead      │ 6     │ 48       │ Active │  │
│  │ Qualification Nurture │ Qualified │ 5     │ 22       │ Active │  │
│  │ FDD Follow-Up         │ Applied   │ 4     │ 8        │ Active │  │
│  │ Re-Engagement         │ Stale     │ 4     │ 15       │ Active │  │
│  │ Long-Term Nurture     │ Not Ready │ 12    │ 34       │ Active │  │
│  │                                                              │  │
│  │ [+ Create New Sequence]  [Edit]  [Pause All]                 │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  Message Templates                                                  │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ [+ New Template]                                             │  │
│  │                                                              │  │
│  │ Template Name     │ Type      │ Characters │ AI Personal. │  │
│  │ Welcome v2        │ Hook      │ 142        │ ☑           │  │
│  │ Discovery Invite  │ Hook      │ 156        │ ☑           │  │
│  │ Territory Alert   │ Marketing │ 148        │ ☑           │  │
│  │ Success Story #1  │ Grooming  │ 155        │ □           │  │
│  │ ROI Teaser        │ Marketing │ 138        │ ☑           │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  [Save Settings]                                                    │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

##### SMS Analytics & Reporting

| Metric | Description | Target |
|--------|-------------|--------|
| **Delivery Rate** | Messages delivered / messages sent | > 95% |
| **Opt-Out Rate** | Opt-outs / total recipients per campaign | < 3% |
| **Response Rate** | Inbound replies / messages delivered | > 10% |
| **Click-Through Rate** | Link clicks / messages delivered | > 5% |
| **Conversion Rate** | Stage advances attributed to SMS / messages sent | > 2% |
| **Cost per Conversion** | Total SMS spend / conversions | < $15 |
| **Sequence Completion Rate** | Enrollments completed / enrollments started | > 60% |

```
SMS Dashboard:
├── Messages Sent (MTD): 1,247
├── Delivery Rate: 97.2%
├── Response Rate: 12.4%
├── Opt-Out Rate: 1.8%
├── Active Campaigns: 3
├── Active Sequence Enrollments: 127
├── Cost (MTD): $9.85
├── Pipeline Advances Attributed to SMS: 18
└── Top Performing Template: "Discovery Invite" (22% response rate)
```

##### Twilio Cost Estimation

| Component | Unit Cost | Monthly Estimate (200 candidates) |
|-----------|-----------|----------------------------------|
| **Outbound SMS** | $0.0079/message | ~$12 (avg 8 msgs/candidate) |
| **Inbound SMS** | $0.0075/message | ~$3 (est. 20% reply rate) |
| **Phone Number** | $1.00/month (local) | $1-$5 |
| **Lookup (carrier)** | $0.01/lookup | $2 |
| **Verify (opt-in)** | $0.05/verification | $10 |
| **Messaging Service** | Included | $0 |
| **Estimated Monthly Total** | | **$28-$32** |

---

## Implementation Roadmap

### Timeline Overview

```
Month 1-2          Month 3-4           Month 5-6          Month 7-8          Month 9-10
──────────────    ──────────────     ──────────────    ──────────────    ──────────────
Phase 1:          Phase 2:           Phase 3:          Phase 4-5:        Phase 6:
Foundation        LinkedIn           AI Engine         UI & Advanced     Twilio SMS
                  Integration        Updates           Features          Outreach
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

### Phase 6: Twilio SMS Outreach (Weeks 41-48)

| Week | Tasks | Deliverables |
|------|-------|--------------|
| 41-42 | Twilio account setup, 10DLC registration, TwilioSMSService class | API integration, send/receive SMS, delivery tracking |
| 43-44 | Pipeline hooks implementation | Automated SMS triggers on stage changes, webhook handlers |
| 45-46 | Marketing campaigns & grooming sequences | Campaign engine, sequence scheduler, enrollment management |
| 47-48 | AI message personalization, analytics, SMS settings UI | GPT-powered templates, SMS dashboard, compliance tools |

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
| TCPA SMS compliance violation | Low | Critical | Legal review, strict opt-in enforcement, automated opt-out |
| SMS opt-out rate too high | Medium | Medium | A/B test messaging, respect frequency limits, personalize content |
| 10DLC registration delays | Medium | Low | Begin registration early in Phase 5, use toll-free as interim |
| Carrier filtering/blocking | Low | Medium | Follow 10DLC best practices, monitor delivery rates, rotate numbers |

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

### C. Twilio API Resources

- [Twilio Programmable Messaging](https://www.twilio.com/docs/messaging)
- [Twilio Messaging Services](https://www.twilio.com/docs/messaging/services)
- [Twilio 10DLC Registration](https://www.twilio.com/docs/messaging/guides/10dlc)
- [Twilio Webhooks](https://www.twilio.com/docs/messaging/guides/webhook-request)
- [TCPA Compliance Guide](https://www.twilio.com/docs/messaging/compliance)
- [Twilio Lookup API](https://www.twilio.com/docs/lookup)
- [Twilio Verify API](https://www.twilio.com/docs/verify)

### D. Competitor Analysis

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
| 1.1 | 2026-02-04 | Claude AI | Added Twilio SMS outbound communication section (hooks, marketing campaigns, grooming sequences) |

---

*This document outlines the strategic transition plan for FranchiseAI. Implementation details may be adjusted based on technical discoveries and business requirements during development.*
