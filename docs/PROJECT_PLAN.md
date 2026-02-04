# FranchiseAI — End-to-End Project Plan

## From Inception to Delivery

---

| Field | Details |
|-------|---------|
| **Document Title** | Project Plan — FranchiseAI Platform Development |
| **Version** | 1.0 |
| **Date** | February 4, 2026 |
| **Related SOW** | SOW-FRAI-2026-001 |
| **Methodology** | Agile (Scrum) with 2-week sprints |

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Project Lifecycle](#2-project-lifecycle)
3. [Phase 0 — Platform Abstraction](#3-phase-0--platform-abstraction)
4. [Phase 1 — Franchise Buyer Vertical](#4-phase-1--franchise-buyer-vertical)
5. [Phase 2 — New Data Sources](#5-phase-2--new-data-sources)
6. [Phase 3 — Outreach & Engagement](#6-phase-3--outreach--engagement)
7. [Phase 4 — Additional Verticals & Polish](#7-phase-4--additional-verticals--polish)
8. [Cross-Cutting Workstreams](#8-cross-cutting-workstreams)
9. [Sprint Calendar](#9-sprint-calendar)
10. [Resource Plan](#10-resource-plan)
11. [Quality Assurance Strategy](#11-quality-assurance-strategy)
12. [Deployment Strategy](#12-deployment-strategy)
13. [Risk Mitigation Plan](#13-risk-mitigation-plan)
14. [Metrics & Reporting](#14-metrics--reporting)
15. [Definition of Done](#15-definition-of-done)
16. [Project Closeout](#16-project-closeout)

---

## 1. Project Overview

### 1.1 Objective

Evolve FranchiseAI from a single-vertical catering prospect discovery tool (v2.0) into a **generic, configuration-driven AI prospecting platform** that supports multiple business verticals through Prospect Profile configurations, then deliver three production-ready verticals (franchise buyer targeting, catering client prospecting, hotel conference venue sales) with automated outreach capabilities.

### 1.2 Success Criteria

| Metric | Target |
|--------|--------|
| Time to deploy a new vertical | < 1 day (config only, zero code changes) |
| Cross-vertical feature parity | 100% — all platform features available to all profiles |
| Search result display time | < 3 seconds |
| Background scoring completion | < 10 seconds |
| AI scoring accuracy (80+ scored prospects that convert) | > 80% |
| Prospect discovery rate | 50+ qualified per month per profile |
| SMS delivery rate | > 95% |
| Platform uptime | 99.5% |
| Test coverage | > 80% line coverage |
| Zero hardcoded domain strings in source | 0 occurrences of vertical-specific text in code |

### 1.3 Current State Assessment

The platform has completed v2.0 with a solid foundation. The codebase coupling analysis reveals:

```
Domain Coupling Heatmap:

  ████████████████░░░░  ~60% Already Generic
  ░░░░░░░░████████████  ~40% Needs Extraction

  Already Generic:                    Needs Extraction:
  ✓ Search orchestration              ✗ calculateCateringPotential()
  ✓ Multi-source data aggregation     ✗ Hardcoded AI prompt strings
  ✓ Geocoding service                 ✗ "minCateringScore" field name
  ✓ All API client classes            ✗ ~14 "catering"/"Vocelli" UI strings
  ✓ Thread pool & parallel processing
  ✓ HTTP/CURL infrastructure
  ✓ ApiLogicServer REST client
  ✓ Authentication service
  ✓ Audit logging
  ✓ Map visualization (Leaflet.js)
  ✓ Wt framework widgets
  ✓ Scoring rule engine (structure)
  ✓ Progressive loading
```

### 1.4 What We're Building

```
┌─────────────────────────────────────────────────────────────────────┐
│               FranchiseAI Generic Prospecting Platform              │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Prospect Profiles (JSON Configuration)                             │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐              │
│  │  Franchise    │  │  Catering    │  │   Hotel      │    + Future  │
│  │  Buyers       │  │  Clients     │  │   Venues     │    Verticals │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘              │
│         └─────────────────┼─────────────────┘                       │
│                           ▼                                         │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                SHARED PLATFORM ENGINE                        │    │
│  │                                                             │    │
│  │  ProspectProfileManager ─── DataSourceRegistry              │    │
│  │  ScoringEngine ──────────── AIPromptManager                 │    │
│  │  TerminologyManager ─────── PipelineManager                 │    │
│  │  AISearchService ────────── GeocodingService                │    │
│  │  AuthService ────────────── AuditLogger                     │    │
│  │  TwilioSMSService ──────── CampaignEngine                  │    │
│  │  CRM Connectors ────────── ReportingEngine                  │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                           │                                         │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │              DATA SOURCE PLUGINS                             │    │
│  │  OSM │ Google │ BBB │ Census │ LinkedIn │ Eventbrite │ ...  │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                           │                                         │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │     PostgreSQL + ApiLogicServer (JSON:API REST Layer)        │    │
│  └─────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 2. Project Lifecycle

### 2.1 Lifecycle Model

The project follows an **Agile (Scrum)** methodology with fixed-length 2-week sprints, organized into 5 sequential phases. Each phase has a clear entry gate, exit criteria, and a formal acceptance checkpoint.

```
  INCEPTION     PHASE 0        PHASE 1        PHASE 2        PHASE 3        PHASE 4      CLOSEOUT
  ─────────   ──────────    ──────────     ──────────    ──────────     ──────────    ──────────
  Week 0      Weeks 1-8     Weeks 9-16     Weeks 17-24   Weeks 25-32   Weeks 33-40   Weeks 41-42

  ┌─────┐    ┌──────────┐  ┌──────────┐   ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────┐
  │Setup│───▶│ Platform │──▶│Franchise │──▶│ Data     │──▶│Outreach &│──▶│Verticals │──▶│Handoff │
  │     │    │Abstraction│  │ Buyer    │   │ Sources  │  │Engagement│  │ & Polish │  │        │
  └─────┘    └──────────┘  └──────────┘   └──────────┘  └──────────┘  └──────────┘  └────────┘
     │            │              │              │              │              │            │
     M1           M2             M3             M4             M5           M6,M7          M8
  Kickoff     Platform       1st Vertical   LinkedIn      SMS Live     All Verticals  Closeout
              Complete        Live           Live                       Production
```

### 2.2 Entry & Exit Gates

| Phase | Entry Gate | Exit Gate |
|-------|-----------|-----------|
| Inception | SOW signed; team assembled | Dev environment operational; backlog groomed |
| Phase 0 | Dev environment ready | Profile switching works; all UI labels dynamic; DB migrated |
| Phase 1 | Phase 0 accepted | End-to-end franchise buyer flow operational |
| Phase 2 | Phase 1 accepted | LinkedIn integration working; enrichment pipeline active |
| Phase 3 | Phase 2 accepted | SMS campaigns operational; CRM sync verified |
| Phase 4 | Phase 3 accepted | All 3 verticals deployed; production readiness verified |
| Closeout | Phase 4 accepted | Knowledge transfer complete; warranty begins |

---

## 3. Phase 0 — Platform Abstraction

**Duration**: Weeks 1–8 (4 sprints)
**Objective**: Extract all domain-specific logic from the codebase into a configuration-driven architecture.

### 3.1 Sprint Breakdown

#### Sprint 1 (Weeks 1–2): Foundation

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S1.1** Set up development environment | Verify C++17 build, PostgreSQL, ApiLogicServer for all developers | DevOps | 2d |
| **S1.2** Design ProspectProfile JSON schema | Define complete schema with target, data sources, search criteria, scoring rules, AI prompts, pipeline, terminology | Architect | 3d |
| **S1.3** Build ProspectProfile config validator | JSON schema validation library; validates against schema on load | Backend Dev | 3d |
| **S1.4** Create ProspectRecord data model | Unified `ProspectRecord.h` supporting BUSINESS and INDIVIDUAL targets | Backend Dev | 2d |
| **S1.5** Write 3 seed profile configs | `franchise-buyer-prospecting.json`, `franchise-catering-clients.json`, `hotel-conference-venues.json` | Architect | 3d |
| **S1.6** Sprint 1 testing | Unit tests for schema validation and data model | QA | 2d |

**Sprint 1 Definition of Done**:
- [ ] ProspectProfile schema documented and validated
- [ ] ProspectRecord model compiles; unit tests pass
- [ ] All 3 seed configs pass schema validation
- [ ] Dev environments operational for all team members

#### Sprint 2 (Weeks 3–4): Core Services

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S2.1** Build ProspectProfileManager | Profile loading from `config/profiles/`, runtime switching, profile caching | Backend Dev | 4d |
| **S2.2** Build DataSourceRegistry | Plugin-style registry; `IDataSourceAPI` interface; source activation per profile | Backend Dev | 3d |
| **S2.3** Build TerminologyManager | Dynamic label lookups from profile terminology map; `Wt::WString` integration | Frontend Dev | 2d |
| **S2.4** Replace hardcoded UI strings | Remove all "catering", "Vocelli", domain-specific strings from `FranchiseApp.cpp`, `Sidebar.h`; replace with TerminologyManager lookups | Frontend Dev | 3d |
| **S2.5** Adapt existing data sources to IDataSourceAPI | Wrap `OpenStreetMapAPI`, `GooglePlacesAPI`, `BBBAPI`, `DemographicsAPI` to implement the common interface | Backend Dev | 3d |
| **S2.6** Sprint 2 testing | Integration tests for profile loading, source registration, terminology | QA | 2d |

**Sprint 2 Definition of Done**:
- [ ] Profiles load from disk; switching profiles updates TerminologyManager
- [ ] DataSourceRegistry activates correct sources per profile config
- [ ] Zero hardcoded domain strings remain in UI code
- [ ] All existing API clients implement IDataSourceAPI

#### Sprint 3 (Weeks 5–6): Scoring & AI

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S3.1** Refactor ScoringEngine | Replace `calculateCateringPotential()` with `calculateProspectScore()` that reads rules from active profile | Backend Dev | 4d |
| **S3.2** Build AIPromptManager | Template engine for AI prompts with `{variable}` substitution; loads prompts from profile config | Backend Dev | 3d |
| **S3.3** Update OpenAIEngine | Remove hardcoded prompts; source prompts from AIPromptManager | Backend Dev | 2d |
| **S3.4** Update GeminiEngine | Remove hardcoded prompts; source prompts from AIPromptManager | Backend Dev | 1d |
| **S3.5** Rename domain-specific fields | `minCateringScore` → `minProspectScore`, `cateringPotentialScore` → `prospectScore` across all models and services | Backend Dev | 2d |
| **S3.6** Profile selector UI | Add profile selector dropdown to Settings or sidebar header | Frontend Dev | 3d |
| **S3.7** Sprint 3 testing | Scoring accuracy tests across all 3 profiles; AI prompt rendering tests | QA | 3d |

**Sprint 3 Definition of Done**:
- [ ] ScoringEngine produces correct scores for all 3 profile configurations
- [ ] AI prompts render correctly with variable substitution
- [ ] No hardcoded prompts remain in AI engine classes
- [ ] Profile selector visible and functional in UI
- [ ] All field renames complete; build succeeds without warnings

#### Sprint 4 (Weeks 7–8): Database & Integration

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S4.1** Database schema migration | Create `prospect_records` and `prospect_profiles` tables; write migration script from `saved_prospects` | Backend Dev | 3d |
| **S4.2** Update ApiLogicServer models | Add new tables to ALS configuration; generate REST endpoints | Backend Dev | 2d |
| **S4.3** Update ApiLogicServerClient | Modify C++ client to use new `prospect_records` endpoints; CRUD operations | Backend Dev | 3d |
| **S4.4** End-to-end integration testing | Test complete flow: select profile → search → score → save → retrieve for all 3 profiles | QA | 3d |
| **S4.5** Data migration execution | Migrate existing saved prospects to new schema; validate data integrity | Backend Dev | 2d |
| **S4.6** Performance benchmarking | Measure profile switching time, scoring time, search time; compare to v2.0 baselines | QA | 1d |
| **S4.7** Phase 0 acceptance preparation | Documentation, demo preparation, known issues list | Architect | 1d |

**Sprint 4 Definition of Done**:
- [ ] Database migration complete; no data loss
- [ ] CRUD operations work for all 3 profiles via ALS
- [ ] End-to-end flow passes for all 3 profiles
- [ ] Performance meets baselines (< 3s search, < 10s scoring)
- [ ] Phase 0 ready for acceptance review

### 3.2 Phase 0 Deliverables Summary

| Deliverable | Acceptance Criteria |
|-------------|-------------------|
| ProspectProfile JSON Schema | Validates all 3 seed configs; documented in config guide |
| ProspectRecord Unified Data Model | Handles BUSINESS + INDIVIDUAL; unit tests pass |
| ProspectProfileManager + DataSourceRegistry | Runtime profile switching; source activation per config |
| TerminologyManager + Dynamic UI Labels | All labels update on profile switch; zero hardcoded strings |
| Refactored ScoringEngine + AIPromptManager | Config-driven scoring; templated prompts |
| Database Migration | Data preserved; new schema operational |

---

## 4. Phase 1 — Franchise Buyer Vertical

**Duration**: Weeks 9–16 (4 sprints)
**Objective**: Deploy the first new prospecting vertical for franchise buyer/investor targeting.

### 4.1 Sprint Breakdown

#### Sprint 5 (Weeks 9–10): Franchise Buyer Search

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S5.1** Finalize franchise buyer profile config | Complete search criteria, scoring rules, and AI prompts based on stakeholder feedback | Architect | 2d |
| **S5.2** Candidate search criteria UI | Dynamic search form rendering from profile's `searchCriteria` definition (job titles, industries, experience, net worth tier) | Frontend Dev | 4d |
| **S5.3** Candidate result card design | Individual-target card layout (name, title, employer, experience, LinkedIn, investor score badge) | Frontend Dev | 3d |
| **S5.4** Search flow integration | Connect candidate search UI to AISearchService with franchise buyer profile active | Backend Dev | 3d |
| **S5.5** Sprint 5 testing | Search criteria rendering; result card display; data mapping | QA | 2d |

#### Sprint 6 (Weeks 11–12): Pipeline & Scoring

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S6.1** Pipeline stage UI | Implement 7-stage pipeline view (Lead → Qualified → Discovery Day → FDD Sent → Applied → Approved → Funded) | Frontend Dev | 4d |
| **S6.2** Pipeline drag-and-drop | Drag prospects between pipeline stages; persist stage changes | Frontend Dev | 3d |
| **S6.3** Investor scoring validation | Test scoring rules against sample franchise buyer data; calibrate weights | Backend Dev | 3d |
| **S6.4** AI investor analysis prompts | Fine-tune prompts for franchise investment fit; validate JSON response format | Backend Dev | 2d |
| **S6.5** Pipeline persistence | Save/load pipeline stage data via ApiLogicServer | Backend Dev | 2d |
| **S6.6** Sprint 6 testing | Pipeline transitions; scoring accuracy; AI analysis quality | QA | 2d |

#### Sprint 7 (Weeks 13–14): Territory & Tools

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S7.1** Territory management - backend | Territory CRUD model and API endpoints; GeoJSON boundary storage | Backend Dev | 4d |
| **S7.2** Territory management - UI | Map-based territory drawing (Leaflet draw plugin); territory list view | Frontend Dev | 4d |
| **S7.3** Territory assignment | Assign territories to users/staff; validate non-overlapping boundaries | Backend Dev | 2d |
| **S7.4** Investment calculator | ROI projection widget (initial investment, liquid capital, projected revenue) | Frontend Dev | 2d |
| **S7.5** Sprint 7 testing | Territory CRUD; boundary validation; calculator math | QA | 2d |

#### Sprint 8 (Weeks 15–16): Integration & Acceptance

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S8.1** End-to-end franchise buyer workflow | Full demo flow: search candidates → score → save → pipeline → territory | All | 3d |
| **S8.2** Hot Prospects for franchise buyer profile | Dashboard adapts to show top franchise buyer candidates with investor score badges | Frontend Dev | 2d |
| **S8.3** Performance optimization | Profile query optimization, caching, pagination for large result sets | Backend Dev | 2d |
| **S8.4** Bug fixes and polish | Address all Sprint 5–7 defects; UX refinements | All | 3d |
| **S8.5** Phase 1 acceptance preparation | Demo, documentation updates, known issues | Architect | 1d |
| **S8.6** Regression testing | Verify catering profile still works correctly alongside franchise buyer profile | QA | 2d |

### 4.2 Phase 1 Deliverables Summary

| Deliverable | Acceptance Criteria |
|-------------|-------------------|
| Franchise Buyer Profile (complete) | End-to-end search → score → pipeline flow |
| Candidate Result Cards | Display individual-target data with investor score badges |
| 7-Stage Pipeline | Drag-and-drop stage transitions with persistence |
| Territory Management | Map-based territory CRUD with assignment |
| Investment Calculator | ROI projections with editable parameters |

---

## 5. Phase 2 — New Data Sources

**Duration**: Weeks 17–24 (4 sprints)
**Objective**: Integrate LinkedIn and additional data sources to expand prospect discovery.

### 5.1 Sprint Breakdown

#### Sprint 9 (Weeks 17–18): LinkedIn Foundation

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S9.1** LinkedIn API partner application | Submit application; begin integration planning with available documentation | Architect | 1d |
| **S9.2** LinkedInAPI class implementation | `IDataSourceAPI` implementation for LinkedIn; OAuth 2.0 flow | Backend Dev | 5d |
| **S9.3** OAuth token management | Token storage, automatic refresh, secure credential handling | Backend Dev | 2d |
| **S9.4** LinkedIn search result mapping | Map LinkedIn profile data to ProspectRecord fields | Backend Dev | 3d |
| **S9.5** Sprint 9 testing | OAuth flow; search API; result mapping accuracy | QA | 2d |

#### Sprint 10 (Weeks 19–20): LinkedIn Enrichment

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S10.1** Profile enrichment pipeline | Background worker that enriches ProspectRecords from secondary sources | Backend Dev | 4d |
| **S10.2** Career transition detection | Algorithm to identify career transitions (role changes, gaps, "open to opportunities") | Backend Dev | 4d |
| **S10.3** LinkedIn data display | Candidate cards updated with LinkedIn-sourced data (photo, connections, skills) | Frontend Dev | 3d |
| **S10.4** Rate limit management | Implement LinkedIn API rate limiting, request queuing, backoff | Backend Dev | 2d |
| **S10.5** Sprint 10 testing | Enrichment pipeline; transition detection accuracy; rate limiting | QA | 2d |

#### Sprint 11 (Weeks 21–22): Additional Sources

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S11.1** Franchise broker API integration | `FranchiseBrokerAPI` class implementing IDataSourceAPI | Backend Dev | 4d |
| **S11.2** Eventbrite API integration | `EventbriteAPI` class for event/venue data | Backend Dev | 3d |
| **S11.3** Data source conflict resolution | Merge logic when multiple sources provide conflicting data for same prospect | Backend Dev | 3d |
| **S11.4** Data source status dashboard | Settings page showing each source's status, usage, last sync | Frontend Dev | 2d |
| **S11.5** Sprint 11 testing | Source integration; conflict resolution; dashboard display | QA | 2d |

#### Sprint 12 (Weeks 23–24): Consolidation

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S12.1** Multi-source search optimization | Parallel queries across all active sources; unified result ranking | Backend Dev | 3d |
| **S12.2** Source attribution tracking | Track which data source contributed which fields; display in prospect detail | Backend Dev | 2d |
| **S12.3** Caching layer for external APIs | 24-hour intelligent cache for LinkedIn, franchise broker, Eventbrite responses | Backend Dev | 3d |
| **S12.4** Bug fixes and polish | Address Sprint 9–11 defects | All | 2d |
| **S12.5** Phase 2 acceptance preparation | Demo, documentation, performance benchmarks | Architect | 1d |
| **S12.6** Regression testing | Full regression across all profiles and data sources | QA | 3d |

### 5.2 Phase 2 Deliverables Summary

| Deliverable | Acceptance Criteria |
|-------------|-------------------|
| LinkedIn API Integration | OAuth flow, search, result mapping; rate limits handled |
| Career Transition Detection | ≥70% accuracy on test dataset |
| Franchise Broker API | Search and result mapping operational |
| Eventbrite API | Event data retrieval and mapping operational |
| Data Source Status Dashboard | All sources shown with status, usage, last sync |

### 5.3 LinkedIn Contingency

If LinkedIn Partner API access is denied or delayed:

1. **Alternative**: Implement web-accessible professional profile enrichment from public sources
2. **Alternative**: Partner with data enrichment providers (Clearbit, ZoomInfo) as substitute source
3. **Reallocation**: Redirect LinkedIn sprint capacity to franchise broker and Eventbrite integrations plus additional enrichment sources

---

## 6. Phase 3 — Outreach & Engagement

**Duration**: Weeks 25–32 (4 sprints)
**Objective**: Build automated multi-channel outreach with SMS, email, and CRM integration.

### 6.1 Sprint Breakdown

#### Sprint 13 (Weeks 25–26): SMS Foundation

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S13.1** TwilioSMSService implementation | Send SMS, receive status callbacks, delivery tracking | Backend Dev | 4d |
| **S13.2** SMS template system | Template CRUD, variable placeholders, preview | Backend Dev | 3d |
| **S13.3** Opt-in/opt-out management | STOP/START keyword processing, consent tracking, opt-out list | Backend Dev | 3d |
| **S13.4** SMS settings UI | Twilio credentials, default templates, opt-out list view | Frontend Dev | 2d |
| **S13.5** Sprint 13 testing | Send/receive; delivery status; opt-out flow | QA | 2d |

#### Sprint 14 (Weeks 27–28): Campaigns & Automation

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S14.1** Campaign engine | Multi-step nurture sequence execution with scheduling | Backend Dev | 5d |
| **S14.2** Pipeline event hooks | Event-driven messaging on pipeline stage transitions | Backend Dev | 3d |
| **S14.3** AI message personalization | GPT-powered template personalization per prospect | Backend Dev | 2d |
| **S14.4** Campaign management UI | Create/edit campaigns, sequence builder, activation toggle | Frontend Dev | 4d |
| **S14.5** Sprint 14 testing | Campaign scheduling; event triggers; personalization quality | QA | 2d |

#### Sprint 15 (Weeks 29–30): CRM Integration

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S15.1** Salesforce connector | Bidirectional sync: prospects, pipeline stages, activities | Backend Dev | 5d |
| **S15.2** HubSpot connector | Bidirectional sync: contacts, deals, activities | Backend Dev | 4d |
| **S15.3** Sync configuration UI | CRM credentials, field mapping, sync frequency, conflict resolution | Frontend Dev | 3d |
| **S15.4** Sync monitoring | Dashboard showing sync status, last sync, error log | Frontend Dev | 2d |
| **S15.5** Sprint 15 testing | Bidirectional sync; conflict handling; error recovery | QA | 2d |

#### Sprint 16 (Weeks 31–32): Compliance & Analytics

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S16.1** TCPA compliance controls | Quiet hours enforcement (no SMS before 8am or after 9pm local time); frequency caps | Backend Dev | 3d |
| **S16.2** 10DLC registration support | Documentation and configuration for 10DLC business registration | Backend Dev | 2d |
| **S16.3** Outreach analytics dashboard | SMS delivery rates, open rates, response rates, campaign performance | Frontend Dev | 4d |
| **S16.4** Email outreach (basic) | Email sending via SMTP; template system shared with SMS | Backend Dev | 3d |
| **S16.5** Bug fixes and polish | Sprint 13–15 defects | All | 2d |
| **S16.6** Phase 3 acceptance preparation | Demo, compliance documentation, integration guides | Architect | 1d |

### 6.2 Phase 3 Deliverables Summary

| Deliverable | Acceptance Criteria |
|-------------|-------------------|
| Twilio SMS Integration | Send/receive; delivery tracking; opt-out compliance |
| Campaign Engine | Multi-step sequences execute on schedule with personalization |
| CRM Integration (Salesforce + HubSpot) | Bidirectional sync within 5 minutes |
| TCPA Compliance Controls | Quiet hours, frequency caps, opt-out automation |
| Outreach Analytics Dashboard | Delivery, open, response rates per campaign |

---

## 7. Phase 4 — Additional Verticals & Polish

**Duration**: Weeks 33–40 (4 sprints)
**Objective**: Deploy remaining verticals via config, build reporting, and prepare for production.

### 7.1 Sprint Breakdown

#### Sprint 17 (Weeks 33–34): Config-Only Verticals

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S17.1** Catering client profile deployment | Finalize `franchise-catering-clients.json`; verify original catering functionality through generic platform | Architect | 2d |
| **S17.2** Hotel venue profile deployment | Finalize `hotel-conference-venues.json`; verify venue prospecting flows | Architect | 2d |
| **S17.3** Cross-profile regression testing | Full test suite across all 3 profiles; verify data isolation | QA | 4d |
| **S17.4** Profile-specific onboarding | First-use wizard per profile explaining terminology and workflow | Frontend Dev | 3d |
| **S17.5** Profile template creator | `_template.json` with documentation for creating new profiles | Architect | 1d |

**Key Validation**: Sprints 17's work items S17.1 and S17.2 must be completed with **zero code changes** — config files only. If code changes are required, it indicates an abstraction gap in Phase 0 that must be addressed.

#### Sprint 18 (Weeks 35–36): Reporting

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S18.1** Reporting engine | Profile-aware data aggregation; date range filtering; grouping | Backend Dev | 4d |
| **S18.2** Prospect Summary Report | Total prospects by status, score distribution, source attribution per profile | Frontend Dev | 3d |
| **S18.3** Pipeline Funnel Report | Conversion funnel visualization per profile; stage-to-stage velocity | Frontend Dev | 3d |
| **S18.4** Export functionality | CSV and PDF export for all reports | Backend Dev | 2d |
| **S18.5** Sprint 18 testing | Report accuracy; export formatting; date filtering | QA | 2d |

#### Sprint 19 (Weeks 37–38): Production Readiness

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S19.1** CI/CD pipeline finalization | GitHub Actions: build → test → deploy to staging; manual promotion to prod | DevOps | 4d |
| **S19.2** Production environment setup | Server provisioning, PostgreSQL setup, SSL, DNS, firewall | DevOps | 3d |
| **S19.3** Monitoring and alerting | Application health checks, error tracking, API usage dashboards | DevOps | 2d |
| **S19.4** Load testing | Simulate concurrent users; identify bottlenecks; optimize | QA | 3d |
| **S19.5** Security audit | OWASP Top 10 review; penetration testing; credential audit | QA + Architect | 2d |

#### Sprint 20 (Weeks 39–40): Final Polish & Documentation

| Task | Description | Owner | Est. |
|------|-------------|-------|------|
| **S20.1** User Guide update | Complete end-user documentation covering all profiles and features | Architect | 3d |
| **S20.2** Administrator Guide | Deployment, configuration, monitoring, backup procedures | DevOps | 2d |
| **S20.3** Profile Configuration Guide | How to create and deploy new verticals | Architect | 2d |
| **S20.4** API documentation | All REST endpoints, authentication, request/response examples | Backend Dev | 2d |
| **S20.5** Final bug fixes | Address all remaining defects from bug triage | All | 3d |
| **S20.6** Phase 4 / Production acceptance | Final demo, acceptance sign-off, go-live checklist | All | 1d |

### 7.2 Phase 4 Deliverables Summary

| Deliverable | Acceptance Criteria |
|-------------|-------------------|
| Catering Client Profile | Config-only deployment; all catering features work |
| Hotel Venue Profile | Config-only deployment; venue prospecting works |
| Reporting Dashboards | Per-profile reports with export (CSV/PDF) |
| CI/CD Pipeline | Automated build/test/deploy; staging and production |
| Production Environment | Server operational with monitoring and alerting |
| Complete Documentation Suite | User guide, admin guide, config guide, API docs |

---

## 8. Cross-Cutting Workstreams

These workstreams run concurrently across multiple phases and are not assigned to specific sprints. They are integrated into sprint planning based on priority and capacity.

### 8.1 Enterprise Authentication

| Work Item | Target Phase | Description |
|-----------|-------------|-------------|
| Replace MD5 with bcrypt/Argon2 | Phase 0 | Production-grade password hashing |
| HTTP-only session cookies | Phase 0 | Replace URL-based session tokens |
| CSRF protection | Phase 0 | Token-based form protection |
| HTTPS enforcement | Phase 4 | Redirect all HTTP to HTTPS |
| Session expiration refinement | Phase 1 | Configurable timeout; refresh-before-expiry |
| Password policy enforcement | Phase 1 | Min 8 chars, mixed case, numbers |
| Account lockout | Phase 1 | Lock after 5 failures; 15-minute cooldown |

### 8.2 Testing Infrastructure

| Work Item | Target Phase | Description |
|-----------|-------------|-------------|
| Unit test framework setup | Phase 0 | CMake test targets; test runner |
| Service unit tests | All phases | Tests for each service class |
| API integration tests | Phase 0+ | Tests for each external API client |
| UI component tests | Phase 1+ | Widget rendering and interaction tests |
| End-to-end tests | Phase 4 | Full workflow automation |
| Performance tests | Phase 4 | Load testing, response time benchmarks |

### 8.3 CI/CD Pipeline

| Stage | Implementation | Target Phase |
|-------|---------------|-------------|
| Build | CMake compile + link on Linux | Phase 0 |
| Unit Tests | Run all unit tests; fail pipeline on failure | Phase 0 |
| Integration Tests | Run API client tests against mock/sandbox | Phase 1 |
| Static Analysis | Code quality checks, compiler warnings | Phase 1 |
| Deploy to Staging | Automated deployment to staging server | Phase 2 |
| Deploy to Production | Manual promotion from staging | Phase 4 |

---

## 9. Sprint Calendar

### 9.1 Full Sprint Schedule

| Sprint | Weeks | Dates* | Phase | Focus |
|--------|-------|--------|-------|-------|
| Sprint 1 | 1–2 | Weeks 1-2 | Phase 0 | Schema, data model, seed configs |
| Sprint 2 | 3–4 | Weeks 3-4 | Phase 0 | Profile manager, source registry, terminology |
| Sprint 3 | 5–6 | Weeks 5-6 | Phase 0 | Scoring refactor, AI prompts, field renames |
| Sprint 4 | 7–8 | Weeks 7-8 | Phase 0 | Database migration, integration testing |
| — | — | — | — | **Phase 0 Acceptance Review** |
| Sprint 5 | 9–10 | Weeks 9-10 | Phase 1 | Franchise buyer search UI and flow |
| Sprint 6 | 11–12 | Weeks 11-12 | Phase 1 | Pipeline stages, scoring validation |
| Sprint 7 | 13–14 | Weeks 13-14 | Phase 1 | Territory management, investment calculator |
| Sprint 8 | 15–16 | Weeks 15-16 | Phase 1 | Integration, polish, regression |
| — | — | — | — | **Phase 1 Acceptance Review** |
| Sprint 9 | 17–18 | Weeks 17-18 | Phase 2 | LinkedIn API foundation |
| Sprint 10 | 19–20 | Weeks 19-20 | Phase 2 | LinkedIn enrichment, career detection |
| Sprint 11 | 21–22 | Weeks 21-22 | Phase 2 | Franchise broker, Eventbrite APIs |
| Sprint 12 | 23–24 | Weeks 23-24 | Phase 2 | Multi-source optimization, caching |
| — | — | — | — | **Phase 2 Acceptance Review** |
| Sprint 13 | 25–26 | Weeks 25-26 | Phase 3 | Twilio SMS, templates, opt-out |
| Sprint 14 | 27–28 | Weeks 27-28 | Phase 3 | Campaigns, automation, personalization |
| Sprint 15 | 29–30 | Weeks 29-30 | Phase 3 | CRM integration (Salesforce, HubSpot) |
| Sprint 16 | 31–32 | Weeks 31-32 | Phase 3 | Compliance, analytics, email |
| — | — | — | — | **Phase 3 Acceptance Review** |
| Sprint 17 | 33–34 | Weeks 33-34 | Phase 4 | Config-only verticals, cross-profile testing |
| Sprint 18 | 35–36 | Weeks 35-36 | Phase 4 | Reporting engine, export |
| Sprint 19 | 37–38 | Weeks 37-38 | Phase 4 | Production readiness, security audit |
| Sprint 20 | 39–40 | Weeks 39-40 | Phase 4 | Documentation, final polish, go-live |
| — | — | — | — | **Phase 4 / Production Acceptance Review** |
| Closeout | 41–42 | Weeks 41-42 | — | Knowledge transfer, warranty begins |

*Actual dates determined at project kickoff based on start date.

### 9.2 Ceremony Schedule (Per Sprint)

| Day | Time | Ceremony | Duration |
|-----|------|----------|----------|
| Sprint Day 1 (Monday) | 9:00 AM | Sprint Planning | 2 hours |
| Daily (Mon–Fri) | 9:00 AM | Daily Standup | 15 minutes |
| Sprint Day 9 (Friday) | 2:00 PM | Sprint Review / Demo | 1 hour |
| Sprint Day 9 (Friday) | 3:30 PM | Sprint Retrospective | 45 minutes |

---

## 10. Resource Plan

### 10.1 Team Allocation by Phase

| Role | Phase 0 | Phase 1 | Phase 2 | Phase 3 | Phase 4 |
|------|---------|---------|---------|---------|---------|
| Technical Lead / Architect | 100% | 100% | 100% | 100% | 100% |
| Senior C++ Developer | 100% | 100% | 100% | 100% | 50% |
| Frontend Developer | 80% | 100% | 80% | 100% | 80% |
| Backend Developer | 100% | 80% | 100% | 100% | 50% |
| QA Engineer | 50% | 80% | 80% | 80% | 100% |
| DevOps Engineer | 30% | 20% | 20% | 20% | 60% |

### 10.2 Skill Requirements

| Role | Required Skills |
|------|----------------|
| Technical Lead | C++17, system architecture, API design, Wt framework, PostgreSQL, code review |
| Senior C++ Developer | C++17, CMake, CURL, JSON parsing, multithreading, template patterns |
| Frontend Developer | Wt widgets, CSS3, JavaScript, Leaflet.js, responsive design |
| Backend Developer | PostgreSQL, REST APIs, ApiLogicServer, Python, data migration, OAuth 2.0 |
| QA Engineer | Test automation, C++ unit testing, API testing, performance testing, security testing |
| DevOps Engineer | Linux administration, Docker, GitHub Actions, PostgreSQL DBA, SSL/TLS, monitoring |

### 10.3 Key Dependencies

- **Architect** is the critical path for profile schema design and cross-vertical consistency
- **Backend Developer** is the critical path for API integrations (LinkedIn, Twilio, CRM)
- **QA Engineer** scales from 50% (Phase 0) to 100% (Phase 4) as test surface grows

---

## 11. Quality Assurance Strategy

### 11.1 Testing Pyramid

```
                    ┌───────────┐
                    │   E2E     │   5-10 critical workflow tests
                    │  Tests    │   Full browser automation
                    ├───────────┤
                    │Integration│   30-50 tests
                    │  Tests    │   API clients, DB operations
                    ├───────────┤
                    │  Unit     │   200+ tests
                    │  Tests    │   Services, models, utilities
                    └───────────┘
```

### 11.2 Test Categories

| Category | Scope | Tools | Coverage Target |
|----------|-------|-------|----------------|
| **Unit Tests** | Individual functions, classes, methods | C++ test framework (Catch2 or Google Test) | 80% line coverage |
| **Integration Tests** | API clients, database operations, service interactions | Test framework + mock servers | All API clients, all DB operations |
| **UI Tests** | Widget rendering, user interactions, navigation | Wt test utilities | Critical user flows |
| **Performance Tests** | Response times, throughput, resource usage | Custom benchmarks, load tools | Baselines met for all operations |
| **Security Tests** | Authentication, authorization, input validation | OWASP ZAP, manual review | No OWASP Top 10 vulnerabilities |
| **Cross-Profile Tests** | Same operations across all 3 profiles | Parameterized tests | 100% feature parity verified |
| **Regression Tests** | All previously passing tests | CI pipeline | Zero regressions per sprint |

### 11.3 Quality Gates

Every sprint delivery must pass:

1. All unit tests pass (zero failures)
2. All integration tests pass
3. No new compiler warnings (C++17 strict mode)
4. Code review approved by Technical Lead
5. No critical or high severity defects open
6. Performance baselines met
7. Cross-profile regression suite passes

### 11.4 Defect Management

| Severity | Definition | Resolution SLA |
|----------|-----------|----------------|
| **Critical** | System crash, data loss, security breach | Same sprint (within 2 business days) |
| **High** | Feature non-functional, significant UX issue | Same sprint (within 5 business days) |
| **Medium** | Feature partially working, cosmetic issue with workaround | Next sprint |
| **Low** | Minor cosmetic, documentation error | Backlog (prioritized by Product Owner) |

---

## 12. Deployment Strategy

### 12.1 Environments

| Environment | Purpose | Update Frequency | Access |
|-------------|---------|-----------------|--------|
| **Local Dev** | Individual development and debugging | Continuous | Developers |
| **CI Build** | Automated build and test on every commit | Per commit | Automated |
| **Staging** | Integration testing, UAT, demos | Per sprint (automated deploy) | Dev team, Product Owner |
| **Production** | Live user environment | Per phase acceptance (manual promotion) | All users |

### 12.2 Deployment Pipeline

```
  ┌─────────┐    ┌─────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐
  │  Code   │───▶│  Build  │───▶│  Unit    │───▶│  Deploy  │───▶│  Deploy  │
  │  Commit │    │  (CMake)│    │  Tests   │    │  Staging │    │  Prod    │
  └─────────┘    └─────────┘    └──────────┘    └──────────┘    └──────────┘
       │              │              │               │               │
   Automatic      Automatic      Automatic       Automatic        Manual
   (git push)     (2-5 min)      (5-10 min)      (per sprint)   (per phase)
                                                     │
                                               ┌──────────┐
                                               │  UAT &   │
                                               │  Approval │
                                               └──────────┘
```

### 12.3 Production Go-Live Checklist

- [ ] All Phase 4 deliverables accepted
- [ ] Security audit passed (no critical/high findings)
- [ ] Load testing passed (concurrent user targets met)
- [ ] Database backups configured and tested
- [ ] SSL certificate installed and verified
- [ ] DNS configured and propagated
- [ ] Monitoring and alerting operational
- [ ] Runbook documented (startup, shutdown, rollback, troubleshooting)
- [ ] API keys rotated for production (separate from staging)
- [ ] Admin user created with production credentials
- [ ] Franchise data imported (store locations, franchisee info)
- [ ] Prospect profiles loaded and validated
- [ ] Twilio 10DLC registration complete
- [ ] CRM sync configured and tested
- [ ] Product Owner sign-off received

---

## 13. Risk Mitigation Plan

### 13.1 Technical Risks

| Risk | Trigger | Mitigation | Contingency |
|------|---------|-----------|-------------|
| **Phase 0 abstraction gap** | Code changes needed to deploy a new vertical in Phase 4 | Validate against 3 concrete profiles before closing Phase 0 | Time-box fix; add abstraction in Sprint 17 |
| **Performance regression** | Search or scoring slower than v2.0 baselines | Benchmark every sprint; profile-cache in memory | Optimize hot paths; add database indexes |
| **API cost overruns** | Monthly API spend exceeds budget | Implement caching (24-hour), on-demand analysis, per-profile budgets | Reduce analysis frequency; switch to cheaper models |
| **Wt framework limitations** | Complex UI pattern not achievable in Wt | Use JavaScript interop (`doJavaScript`); Wt JSignal callbacks | Hybrid approach with custom JS widgets |
| **Database migration failure** | Data loss during schema migration | Full backup before migration; staged migration; rollback script | Restore from backup; revert schema |

### 13.2 External Risks

| Risk | Trigger | Mitigation | Contingency |
|------|---------|-----------|-------------|
| **LinkedIn API denied** | Partner application rejected or delayed | Apply early (Sprint 9); prepare alternatives | Substitute with Clearbit/ZoomInfo; reallocate capacity |
| **Twilio 10DLC delay** | Registration takes longer than expected | Start registration in Phase 2 (early) | Use toll-free number initially; migrate to 10DLC later |
| **CRM API changes** | Salesforce/HubSpot deprecates endpoints | Use versioned APIs; monitor deprecation notices | Update connector; test in staging |
| **Third-party API outage** | Google, OSM, or other API unavailable | Graceful degradation; fallback sources; cached responses | Display cached results; notify user of reduced coverage |

### 13.3 Project Risks

| Risk | Trigger | Mitigation | Contingency |
|------|---------|-----------|-------------|
| **Scope creep** | New work items added without removing others | Formal Change Order process; Product Owner prioritization | Defer to next phase; extend timeline with approved CO |
| **Client availability** | Product Owner unresponsive for decisions | 3-day SLA; auto-acceptance after 10 days | Escalate to steering committee; document decisions made |
| **Team attrition** | Key developer leaves project | Cross-training; documentation; pair programming | Onboard replacement; knowledge transfer period |
| **Requirement ambiguity** | Acceptance criteria unclear for a deliverable | Spike/prototype early; weekly clarification sessions | Time-box investigation; document assumptions |

---

## 14. Metrics & Reporting

### 14.1 Sprint Metrics

| Metric | Measurement | Target |
|--------|-------------|--------|
| **Velocity** | Story points completed per sprint | Establish baseline by Sprint 3; maintain ±10% |
| **Sprint Burndown** | Remaining work over sprint duration | Consistent daily progress; no last-day spikes |
| **Defect Rate** | New defects found per sprint | Decreasing trend over time |
| **Defect Resolution** | Avg days to close a defect | Critical: < 2 days; High: < 5 days |
| **Code Coverage** | Unit test line coverage | ≥ 80% |
| **Build Success Rate** | % of CI builds that pass | ≥ 95% |
| **Sprint Goal Achievement** | % of sprint goals met | ≥ 90% |

### 14.2 Phase Metrics

| Metric | Measurement | Target |
|--------|-------------|--------|
| **Schedule Variance** | Actual phase end vs. planned phase end | ≤ 1 week deviation |
| **Scope Completion** | % of phase work items delivered and accepted | 100% |
| **Defect Escapement** | Defects found in acceptance that should have been caught in testing | ≤ 5 per phase |
| **Acceptance Cycle Time** | Days from phase delivery to acceptance | ≤ 10 business days |

### 14.3 Weekly Status Report Template

```
Week of: [Date]
Sprint: [Number] of 20
Phase: [Phase Name]

OVERALL STATUS: [Green / Yellow / Red]

Completed This Week:
- [Item 1]
- [Item 2]

Planned Next Week:
- [Item 1]
- [Item 2]

Blockers / Risks:
- [Blocker 1] — [Mitigation / Escalation]

Metrics:
- Velocity: [X] points
- Defects Open: [X] (Critical: [X], High: [X])
- Test Coverage: [X]%
- Build Pass Rate: [X]%
```

---

## 15. Definition of Done

### 15.1 Story Level

A user story is **done** when:

- [ ] Code is written and compiles without warnings
- [ ] Unit tests written and passing
- [ ] Integration test written (if applicable)
- [ ] Code reviewed and approved by Technical Lead
- [ ] No known defects (Critical or High severity)
- [ ] UI matches design specifications (if applicable)
- [ ] Works correctly across all active Prospect Profiles
- [ ] Deployed to staging and verified
- [ ] Documented (code comments for complex logic; user-facing docs updated if needed)

### 15.2 Sprint Level

A sprint is **done** when:

- [ ] All sprint goal stories are done (per story DoD above)
- [ ] All unit and integration tests pass (zero failures)
- [ ] No critical or high defects remain open
- [ ] Sprint demo conducted and feedback captured
- [ ] Sprint retrospective completed
- [ ] Sprint metrics recorded

### 15.3 Phase Level

A phase is **done** when:

- [ ] All sprint-level DoDs met
- [ ] All phase deliverables complete and demonstrable
- [ ] Cross-profile regression suite passes
- [ ] Performance baselines met
- [ ] Phase acceptance demo conducted
- [ ] Client acceptance received (or auto-accepted after 10 days)
- [ ] Documentation updated for new features/changes

### 15.4 Project Level

The project is **done** when:

- [ ] All phase-level DoDs met
- [ ] All 18 software deliverables (D-01 through D-18) accepted
- [ ] All 7 documentation deliverables (DD-01 through DD-07) complete
- [ ] Production environment operational with monitoring
- [ ] Security audit passed
- [ ] Load testing passed
- [ ] Knowledge transfer completed
- [ ] Final project report delivered
- [ ] Warranty period initiated

---

## 16. Project Closeout

### 16.1 Closeout Activities (Weeks 41–42)

| Activity | Duration | Owner |
|----------|----------|-------|
| **Knowledge Transfer Sessions** | 3 days | All → Client team |
| **Architecture Walkthrough** | 1 day | Architect |
| **Runbook Review** | 0.5 day | DevOps |
| **Repository Transfer** | 0.5 day | DevOps |
| **Final Project Report** | 1 day | Architect |
| **Lessons Learned Review** | 0.5 day | All |
| **Warranty Period Kickoff** | 0.5 day | Architect + Client |

### 16.2 Knowledge Transfer Agenda

| Session | Duration | Topics |
|---------|----------|--------|
| **Session 1: Architecture** | 4 hours | System architecture, component diagram, data flow, design decisions, technology rationale |
| **Session 2: Codebase** | 4 hours | Source code walkthrough, service layer, model layer, widget layer, coding conventions |
| **Session 3: Configuration** | 2 hours | Prospect Profile creation, profile switching, scoring rule customization, AI prompt tuning |
| **Session 4: Data Layer** | 2 hours | PostgreSQL schema, ApiLogicServer, data migration, backup/restore procedures |
| **Session 5: Operations** | 4 hours | Build process, CI/CD pipeline, deployment procedures, monitoring, alerting, troubleshooting |
| **Session 6: Integrations** | 2 hours | API client configuration, LinkedIn OAuth, Twilio SMS, CRM connectors, rate limits |

### 16.3 Final Project Report Contents

1. Executive summary of delivered capabilities
2. Comparison of planned vs. actual: scope, schedule, budget
3. All accepted deliverables with acceptance dates
4. Final defect summary (open, closed, deferred)
5. Performance benchmarks achieved
6. Change Orders executed (if any)
7. Risk register final status
8. Lessons learned and recommendations
9. Warranty period terms and contacts

### 16.4 Post-Project Support Options

| Option | Coverage | Suggested For |
|--------|----------|--------------|
| **Warranty Only** | 90-day defect remediation | Clients with internal dev capability |
| **Maintenance & Support (MSA)** | Bug fixes, security patches, minor enhancements | Ongoing platform operation |
| **Managed Service** | Full operational management including monitoring, updates, vertical deployment | Clients without dedicated IT staff |
| **Enhancement Retainer** | Monthly hours for new features and vertical development | Growing platform needs |

---

## Appendix A: Work Breakdown Structure (WBS)

```
1.0 FranchiseAI Platform
├── 1.1 Inception
│   ├── 1.1.1 Project kickoff
│   ├── 1.1.2 Environment setup
│   └── 1.1.3 Backlog grooming
│
├── 1.2 Phase 0 — Platform Abstraction
│   ├── 1.2.1 ProspectProfile schema design
│   ├── 1.2.2 ProspectRecord data model
│   ├── 1.2.3 Profile config validator
│   ├── 1.2.4 ProspectProfileManager
│   ├── 1.2.5 DataSourceRegistry
│   ├── 1.2.6 TerminologyManager
│   ├── 1.2.7 ScoringEngine refactor
│   ├── 1.2.8 AIPromptManager
│   ├── 1.2.9 Field renames
│   ├── 1.2.10 Profile selector UI
│   ├── 1.2.11 Database migration
│   ├── 1.2.12 ALS model updates
│   └── 1.2.13 Integration testing
│
├── 1.3 Phase 1 — Franchise Buyer Vertical
│   ├── 1.3.1 Profile config finalization
│   ├── 1.3.2 Candidate search UI
│   ├── 1.3.3 Candidate result cards
│   ├── 1.3.4 Pipeline UI (7 stages)
│   ├── 1.3.5 Investor scoring validation
│   ├── 1.3.6 Territory management
│   ├── 1.3.7 Investment calculator
│   └── 1.3.8 Integration & regression
│
├── 1.4 Phase 2 — New Data Sources
│   ├── 1.4.1 LinkedIn API integration
│   ├── 1.4.2 OAuth token management
│   ├── 1.4.3 Career transition detection
│   ├── 1.4.4 Profile enrichment pipeline
│   ├── 1.4.5 Franchise broker API
│   ├── 1.4.6 Eventbrite API
│   ├── 1.4.7 Conflict resolution
│   ├── 1.4.8 Caching layer
│   └── 1.4.9 Source status dashboard
│
├── 1.5 Phase 3 — Outreach & Engagement
│   ├── 1.5.1 Twilio SMS service
│   ├── 1.5.2 SMS template system
│   ├── 1.5.3 Opt-in/opt-out management
│   ├── 1.5.4 Campaign engine
│   ├── 1.5.5 Pipeline event hooks
│   ├── 1.5.6 AI message personalization
│   ├── 1.5.7 Salesforce connector
│   ├── 1.5.8 HubSpot connector
│   ├── 1.5.9 TCPA compliance
│   ├── 1.5.10 Outreach analytics
│   └── 1.5.11 Email outreach
│
├── 1.6 Phase 4 — Additional Verticals & Polish
│   ├── 1.6.1 Catering client profile (config only)
│   ├── 1.6.2 Hotel venue profile (config only)
│   ├── 1.6.3 Cross-profile testing
│   ├── 1.6.4 Reporting engine
│   ├── 1.6.5 Export (CSV/PDF)
│   ├── 1.6.6 CI/CD finalization
│   ├── 1.6.7 Production environment
│   ├── 1.6.8 Load testing
│   ├── 1.6.9 Security audit
│   └── 1.6.10 Documentation suite
│
├── 1.7 Cross-Cutting
│   ├── 1.7.1 Enterprise authentication
│   ├── 1.7.2 Security hardening
│   ├── 1.7.3 Test infrastructure
│   ├── 1.7.4 CI/CD pipeline
│   ├── 1.7.5 Monitoring & logging
│   └── 1.7.6 Performance optimization
│
└── 1.8 Closeout
    ├── 1.8.1 Knowledge transfer
    ├── 1.8.2 Final project report
    ├── 1.8.3 Repository transfer
    └── 1.8.4 Warranty kickoff
```

## Appendix B: Dependency Map

```
                 ┌────────────────────────────────┐
                 │          INCEPTION              │
                 │  Environment, Backlog, Kickoff  │
                 └───────────────┬────────────────┘
                                 │
                 ┌───────────────▼────────────────┐
                 │      PHASE 0: ABSTRACTION      │
                 │  Schema, Models, Profile Mgr,  │
                 │  Scoring, AI Prompts, DB Migr  │
                 └───────────────┬────────────────┘
                                 │
                 ┌───────────────▼────────────────┐
                 │    PHASE 1: FRANCHISE BUYER     │
                 │  Search, Pipeline, Territory,   │
                 │  Investment Calculator          │
                 └───────────────┬────────────────┘
                                 │
              ┌──────────────────┴──────────────────┐
              │                                      │
  ┌───────────▼──────────────┐        ┌─────────────▼──────────────┐
  │   PHASE 2: DATA SOURCES  │        │  Cross-Cutting: Auth,      │
  │  LinkedIn, Broker,       │        │  Security, Testing, CI/CD  │
  │  Eventbrite, Enrichment  │        │  (runs in parallel)        │
  └───────────┬──────────────┘        └────────────────────────────┘
              │
  ┌───────────▼──────────────┐
  │ PHASE 3: OUTREACH        │
  │ Twilio, Campaigns, CRM,  │
  │ Compliance, Analytics    │
  └───────────┬──────────────┘
              │
  ┌───────────▼──────────────┐
  │ PHASE 4: VERTICALS       │
  │ Config deployment,       │
  │ Reporting, Prod setup,   │
  │ Docs, Go-live            │
  └───────────┬──────────────┘
              │
  ┌───────────▼──────────────┐
  │       CLOSEOUT            │
  │  KT, Report, Warranty    │
  └──────────────────────────┘
```

### Critical Path

The critical path runs through:

1. **Phase 0** (all tasks are sequential prerequisites for Phase 1)
2. **Phase 1, Sprint 5** (search UI depends on Phase 0's profile system)
3. **Phase 2, Sprint 9** (LinkedIn depends on Phase 1's candidate cards)
4. **Phase 3, Sprint 13** (SMS depends on Phase 2's prospect data)
5. **Phase 4, Sprint 17** (config deployment validates Phase 0's abstraction)

Any delay on the critical path directly delays the project end date.

## Appendix C: RACI Matrix

| Activity | Tech Lead | Sr. C++ Dev | Frontend Dev | Backend Dev | QA | DevOps | Product Owner |
|----------|-----------|-------------|--------------|-------------|-----|--------|--------------|
| Architecture decisions | **A** | C | C | C | I | C | I |
| Sprint planning | **R** | R | R | R | R | C | **A** |
| Code development | C | **R** | **R** | **R** | I | I | I |
| Code review | **A** | R | R | R | I | I | I |
| Unit testing | C | **R** | **R** | **R** | C | I | I |
| Integration testing | C | R | R | R | **A/R** | C | I |
| Sprint demo | **R** | R | R | R | R | I | **A** |
| Acceptance testing | I | I | I | I | R | I | **A** |
| Deployment | C | I | I | I | I | **R** | **A** |
| Risk management | **R** | C | I | C | C | C | **A** |
| Change orders | **R** | I | I | I | I | I | **A** |
| Documentation | **R** | R | R | R | R | R | C |

**R** = Responsible, **A** = Accountable, **C** = Consulted, **I** = Informed

---

*End of Project Plan — FranchiseAI Platform Development*
