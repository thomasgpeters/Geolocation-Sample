# Statement of Work (SOW)

## FranchiseAI — AI-Powered Prospect Discovery Platform

---

| Field | Details |
|-------|---------|
| **Document Title** | Statement of Work — FranchiseAI Platform Development |
| **Version** | 1.0 |
| **Date** | February 4, 2026 |
| **Prepared For** | [Client Name / Organization] |
| **Prepared By** | [Provider Name / Organization] |
| **SOW Reference** | SOW-FRAI-2026-001 |

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Project Background](#2-project-background)
3. [Scope of Work](#3-scope-of-work)
4. [Deliverables](#4-deliverables)
5. [Technical Architecture](#5-technical-architecture)
6. [Assumptions & Dependencies](#6-assumptions--dependencies)
7. [Out of Scope](#7-out-of-scope)
8. [Acceptance Criteria](#8-acceptance-criteria)
9. [Project Timeline](#9-project-timeline)
10. [Team & Roles](#10-team--roles)
11. [Communication & Governance](#11-communication--governance)
12. [Change Management](#12-change-management)
13. [Risk Management](#13-risk-management)
14. [Investment Summary](#14-investment-summary)
15. [Intellectual Property & Ownership](#15-intellectual-property--ownership)
16. [Warranty & Support](#16-warranty--support)
17. [Termination](#17-termination)
18. [Signoff](#18-signoff)

---

## 1. Executive Summary

This Statement of Work defines the scope, deliverables, timeline, and terms for the design, development, and delivery of the **FranchiseAI Platform** — an AI-powered prospect discovery application that enables franchise operators to identify, qualify, score, and manage potential clients through intelligent multi-source data aggregation, configurable scoring, and AI-driven analysis.

The platform is being developed as a **generic, configuration-driven prospecting engine** capable of supporting multiple business verticals (catering client prospecting, franchise buyer targeting, hotel conference venue sales, and future verticals) through a single shared codebase driven by Prospect Profile configurations. This approach maximizes reuse, minimizes per-vertical development cost, and positions the platform as a scalable SaaS product.

This SOW serves as the authoritative agreement governing all project work. No work outside the scope defined herein shall be performed without a written Change Order approved by both parties.

---

## 2. Project Background

### 2.1 Business Problem

Franchise sales teams and operators waste an estimated 40% of their prospecting time on manual research across fragmented data sources (Google, Yelp, LinkedIn, business directories). This results in:

- Low lead quality (~20% qualified from manual search)
- Incomplete market coverage (<30% of potential prospects discovered)
- High cost per qualified lead ($45–$75)
- Duplicate data entry across CRM systems and spreadsheets
- Inconsistent scoring and qualification standards

### 2.2 Proposed Solution

FranchiseAI addresses these problems by providing:

- **Automated multi-source search** across OpenStreetMap, Google Places, BBB, demographics APIs, and future sources (LinkedIn, Eventbrite, franchise broker databases)
- **AI-powered scoring and analysis** using OpenAI GPT-4o or Google Gemini to evaluate and rank prospects
- **Configurable scoring engine** with penalty/bonus rules customizable per franchise location
- **Interactive demographics mapping** with Leaflet.js for territory visualization
- **Prospect management pipeline** for tracking leads from discovery through conversion
- **Generic platform architecture** enabling new business verticals via JSON configuration without code changes

### 2.3 Current State

The platform has completed Version 2.0 development with the following capabilities operational:

| Capability | Status |
|------------|--------|
| AI Search with multi-source aggregation | Complete |
| Hot Prospects Dashboard with score badges | Complete |
| Configurable Scoring Engine (14 rules) | Complete |
| My Prospects management with persistence | Complete |
| Demographics explorer with Leaflet.js map | Complete |
| Settings (5 tabs: Franchisee, Marketing, AI, Data Sources, Scoring) | Complete |
| Session-based authentication with role-based access | Complete |
| Audit trail for admin users | Complete |
| Progressive loading with background score optimization | Complete |
| ApiLogicServer integration for PostgreSQL persistence | Complete |
| Toast notifications for non-blocking UX | Complete |

### 2.4 Strategic Direction

The project is transitioning from a single-vertical catering prospecting tool into a **generic configurable prospecting platform** where business domain logic is defined by seed data and Prospect Profile configurations rather than hardcoded logic. The existing codebase is approximately 60% domain-agnostic; domain-specific coupling is concentrated in four areas (scoring method, AI prompts, field names, UI labels) that will be extracted into configuration.

---

## 3. Scope of Work

### 3.1 Phase 0 — Platform Abstraction

Extract domain-specific logic from the codebase into a configuration-driven architecture.

| Work Item | Description |
|-----------|-------------|
| **WI-0.1** Prospect Profile Schema | Design and implement the JSON schema for Prospect Profile configurations, defining target type, data sources, search criteria, scoring rules, AI prompts, pipeline stages, and UI terminology |
| **WI-0.2** Unified Data Model | Create `ProspectRecord` model that supports both BUSINESS and INDIVIDUAL target types with universal scoring, pipeline, and metadata fields plus extensible attributes |
| **WI-0.3** ProspectProfileManager | Build profile loader, validator, and runtime manager with profile switching capability |
| **WI-0.4** DataSourceRegistry | Implement plugin-style registry for data source activation based on active profile configuration |
| **WI-0.5** TerminologyManager | Replace ~14 hardcoded UI strings with dynamic lookups from profile terminology maps |
| **WI-0.6** ScoringEngine Refactor | Replace `calculateCateringPotential()` with generic `calculateProspectScore()` reading rules from active profile |
| **WI-0.7** AIPromptManager | Extract hardcoded AI prompts from `OpenAIEngine.cpp`, `GeminiEngine.cpp`, and `AIEngine.cpp` into profile-driven templates with variable substitution |
| **WI-0.8** Database Migration | Migrate from `saved_prospects` to unified `prospect_records` table; add `prospect_profiles` table for multi-tenant config storage |
| **WI-0.9** Profile Selector UI | Add profile selector dropdown to Settings or navigation for switching between active prospecting verticals |

### 3.2 Phase 1 — Franchise Buyer Vertical

Deploy the first new vertical using the generic platform architecture.

| Work Item | Description |
|-----------|-------------|
| **WI-1.1** Franchise Buyer Profile Config | Create `franchise-buyer-prospecting.json` with investor-focused scoring rules, search criteria, AI prompts, 7-stage pipeline, and terminology |
| **WI-1.2** Candidate Card Design | Design and implement result cards tailored for individual targets (job title, experience, net worth tier, LinkedIn) |
| **WI-1.3** Investor Analysis AI Prompts | Fine-tune AI prompts for franchise investment potential evaluation, fit assessment, and outreach recommendations |
| **WI-1.4** Territory Management | Implement territory CRUD operations with map-based territory definition and assignment |
| **WI-1.5** Investment Calculator | Build ROI projection tool for franchise investment scenarios |

### 3.3 Phase 2 — New Data Sources

Integrate additional data sources to expand prospect discovery capabilities.

| Work Item | Description |
|-----------|-------------|
| **WI-2.1** LinkedIn API Integration | Implement `LinkedInAPI` class with OAuth flow, profile search, and result mapping to `ProspectRecord` |
| **WI-2.2** Career Transition Detection | Build algorithm for identifying career transition signals from LinkedIn data |
| **WI-2.3** Profile Enrichment | Implement background enrichment pipeline for supplementing prospect data from secondary sources |
| **WI-2.4** Franchise Broker API | Integrate franchise broker database as additional data source plugin |
| **WI-2.5** Eventbrite API | Integrate event data source for conference venue vertical |

### 3.4 Phase 3 — Outreach & Engagement

Build automated outreach capabilities across communication channels.

| Work Item | Description |
|-----------|-------------|
| **WI-3.1** Twilio SMS Integration | Implement `TwilioSMSService` with send, receive, and delivery status tracking |
| **WI-3.2** Pipeline Event Hooks | Build event-driven messaging triggered by pipeline stage transitions |
| **WI-3.3** Marketing Campaigns | Implement campaign engine for multi-step nurture sequences (grooming drips) |
| **WI-3.4** AI Message Personalization | Integrate GPT-powered template personalization for outreach messages |
| **WI-3.5** CRM Integration | Build sync connectors for Salesforce and HubSpot |
| **WI-3.6** SMS Compliance | Implement TCPA/10DLC compliance controls, opt-in/opt-out management, and quiet hours enforcement |
| **WI-3.7** Outreach Analytics | Build SMS delivery, open, and response rate tracking dashboard |

### 3.5 Phase 4 — Additional Verticals & Polish

Deploy additional business verticals and finalize the platform.

| Work Item | Description |
|-----------|-------------|
| **WI-4.1** Catering Client Profile | Create `franchise-catering-clients.json` config (restoring original use case as a profile — config only, no code changes) |
| **WI-4.2** Hotel Conference Venue Profile | Create `hotel-conference-venues.json` config (config only, no code changes) |
| **WI-4.3** Per-Profile Reporting | Build profile-aware analytics dashboards with charts and export |
| **WI-4.4** Multi-Profile Testing | Comprehensive cross-profile validation and regression testing |
| **WI-4.5** Documentation & Training | Complete user guide, API documentation, admin guide, and video tutorials |

### 3.6 Cross-Cutting Work (All Phases)

| Work Item | Description |
|-----------|-------------|
| **WI-X.1** Enterprise Authentication | Replace mock auth with production-grade implementation (bcrypt/Argon2 password hashing, HTTP-only session cookies, CSRF protection, HTTPS enforcement) |
| **WI-X.2** Security Hardening | Input validation, SQL injection prevention, XSS prevention, rate limiting, account lockout enforcement |
| **WI-X.3** Performance Optimization | Response caching layer, lazy loading, bundle optimization, API rate limiting configuration |
| **WI-X.4** Testing Infrastructure | Unit tests for all services, integration tests for API clients, UI component tests, end-to-end tests |
| **WI-X.5** CI/CD Pipeline | Automated build, test, and deployment pipeline with staging and production environments |
| **WI-X.6** Monitoring & Logging | Application health monitoring, error tracking, API usage dashboards, alerting |

---

## 4. Deliverables

### 4.1 Software Deliverables

| ID | Deliverable | Phase | Acceptance Gate |
|----|-------------|-------|-----------------|
| **D-01** | Prospect Profile JSON Schema & Validator | Phase 0 | Schema validates all three vertical configs without error |
| **D-02** | ProspectRecord Unified Data Model | Phase 0 | Handles BUSINESS and INDIVIDUAL targets; passes unit tests |
| **D-03** | ProspectProfileManager & DataSourceRegistry | Phase 0 | Profile switching works at runtime; all registered sources activate per config |
| **D-04** | TerminologyManager & Dynamic UI Labels | Phase 0 | All UI labels update when profile is switched; no hardcoded domain strings remain |
| **D-05** | Refactored ScoringEngine & AIPromptManager | Phase 0 | Scoring produces correct results for all three profiles; AI prompts render from templates |
| **D-06** | Database Migration (prospect_records, prospect_profiles) | Phase 0 | Data migrated from legacy schema; CRUD operations verified |
| **D-07** | Franchise Buyer Vertical (complete) | Phase 1 | End-to-end search, score, save, pipeline flow for franchise buyer candidates |
| **D-08** | LinkedIn API Integration | Phase 2 | OAuth flow, search, result mapping operational; rate limits handled |
| **D-09** | Career Transition Detection Algorithm | Phase 2 | Correctly flags candidates with recent career changes from LinkedIn data |
| **D-10** | Twilio SMS Integration | Phase 3 | Send/receive SMS; delivery status tracking; TCPA compliance |
| **D-11** | Marketing Campaign Engine | Phase 3 | Multi-step nurture sequences execute on schedule with personalization |
| **D-12** | CRM Integration (Salesforce/HubSpot) | Phase 3 | Bidirectional sync of prospect records and pipeline stages |
| **D-13** | Catering Client Profile (config) | Phase 4 | Config-only deployment; original catering functionality restored without code changes |
| **D-14** | Hotel Conference Venue Profile (config) | Phase 4 | Config-only deployment; venue prospecting functional without code changes |
| **D-15** | Per-Profile Reporting Dashboards | Phase 4 | Charts, export (CSV/PDF), date range filtering per active profile |
| **D-16** | Enterprise Authentication System | Cross-cutting | bcrypt hashing, session management, RBAC, all security tests pass |
| **D-17** | CI/CD Pipeline | Cross-cutting | Automated build, test, deploy to staging; manual promotion to production |
| **D-18** | Complete Test Suite | Cross-cutting | Unit, integration, and E2E tests; minimum 80% code coverage |

### 4.2 Documentation Deliverables

| ID | Deliverable | Description |
|----|-------------|-------------|
| **DD-01** | Platform Architecture Document | System architecture, component diagrams, data flow, integration points |
| **DD-02** | Prospect Profile Configuration Guide | How to create, validate, and deploy new vertical profiles |
| **DD-03** | API Documentation | All REST endpoints, request/response formats, authentication |
| **DD-04** | Database Schema Documentation | ERD, table definitions, indexes, migration procedures |
| **DD-05** | User Guide (updated) | End-user documentation covering all features across all profiles |
| **DD-06** | Administrator Guide | Deployment, configuration, monitoring, troubleshooting |
| **DD-07** | Training Materials | Video walkthroughs, quick-start guides, FAQ |

---

## 5. Technical Architecture

### 5.1 Technology Stack

| Layer | Technology | Version |
|-------|-----------|---------|
| **Frontend Framework** | Wt (Witty) C++ Web Framework | 4.x |
| **Mapping** | Leaflet.js | Latest |
| **Backend Language** | C++17 | GCC 8+ / Clang 7+ |
| **Build System** | CMake | 3.16+ |
| **REST API Backend** | ApiLogicServer (Python) | Latest |
| **Database** | PostgreSQL | 14+ |
| **AI Providers** | OpenAI GPT-4o, Google Gemini | Current |
| **SMS** | Twilio | API v2 |
| **HTTP Client** | CURL | Latest |

### 5.2 External API Integrations

| Service | Purpose | Auth Type | Rate Limits |
|---------|---------|-----------|-------------|
| OpenStreetMap Overpass | POI search, geocoding | None (free) | Fair use policy |
| Google Places API | Business listings, ratings | API Key | 1,000 requests/day (basic) |
| Google Geocoding API | Address resolution | API Key | 40,000 requests/month |
| BBB API | Business accreditation | API Key | Per agreement |
| Census/Demographics | Population, income data | API Key | 500 requests/day |
| OpenAI GPT-4o | AI prospect analysis | API Key | Per tier |
| Google Gemini | Fallback AI analysis | API Key | Per tier |
| LinkedIn API | Professional profile search | OAuth 2.0 | Per partner agreement |
| Twilio | SMS outreach | Account SID + Auth Token | Per plan |
| Eventbrite | Event data | OAuth 2.0 | 2,000 requests/hour |

### 5.3 Architecture Principles

1. **Configuration over Code** — Business domain logic defined in Prospect Profile JSON, not source code
2. **Plugin Data Sources** — Data source registry pattern for adding new sources without modifying orchestration
3. **Progressive Enhancement** — Results appear instantly with base scores; AI refinement runs in background
4. **Graceful Degradation** — System remains functional when individual APIs fail or are unconfigured
5. **Layered Architecture** — UI → Service → Model → Persistence with clean separation of concerns
6. **Multi-Tenant Ready** — Profile and franchisee isolation at the data layer

### 5.4 System Diagram

```
                    ┌──────────────────────────┐
                    │       Client Browser      │
                    │  (Wt Framework + Leaflet) │
                    └────────────┬─────────────┘
                                 │
                    ┌────────────▼─────────────┐
                    │    FranchiseAI Server     │
                    │    (C++17 / Wt HTTP)      │
                    ├──────────────────────────┤
                    │  ┌─────────────────────┐  │
                    │  │  ProspectProfile     │  │
                    │  │  Manager             │  │
                    │  └────────┬────────────┘  │
                    │           │               │
                    │  ┌────────▼────────────┐  │
                    │  │  Service Layer       │  │
                    │  │  • AISearchService   │  │
                    │  │  • ScoringEngine     │  │
                    │  │  • AIPromptManager   │  │
                    │  │  • AuthService       │  │
                    │  │  • AuditLogger       │  │
                    │  └────────┬────────────┘  │
                    │           │               │
                    │  ┌────────▼────────────┐  │
                    │  │  DataSourceRegistry  │  │
                    │  │  ┌───┐┌───┐┌───┐    │  │
                    │  │  │OSM││GGL││BBB│... │  │
                    │  │  └───┘└───┘└───┘    │  │
                    │  └────────┬────────────┘  │
                    └───────────┼───────────────┘
                                │
                    ┌───────────▼───────────────┐
                    │   ApiLogicServer (REST)    │
                    │   JSON:API Endpoints       │
                    └───────────┬───────────────┘
                                │
                    ┌───────────▼───────────────┐
                    │      PostgreSQL 14+        │
                    │  prospect_records          │
                    │  prospect_profiles         │
                    │  users / sessions          │
                    │  audit_log                 │
                    └───────────────────────────┘
```

---

## 6. Assumptions & Dependencies

### 6.1 Assumptions

| ID | Assumption |
|----|------------|
| A-01 | Client will provide timely access to all required API keys and credentials (Google, BBB, Census, OpenAI, Twilio, LinkedIn partner access) |
| A-02 | Client will designate a single Product Owner with authority to make scope and priority decisions |
| A-03 | Client will participate in sprint reviews and provide feedback within 3 business days of each demo |
| A-04 | Target deployment environment is a Linux server (Ubuntu 22.04 LTS or equivalent) with C++17 compiler support |
| A-05 | PostgreSQL 14+ will be available as the production database |
| A-06 | Client's existing franchise data (store locations, franchisee info) can be imported via CSV or API |
| A-07 | OpenAI/Gemini API usage costs are borne by the Client as operational expenses separate from development costs |
| A-08 | LinkedIn API access requires a LinkedIn Partner Program application; approval timeline is outside Provider's control |
| A-09 | Twilio SMS requires separate account setup and 10DLC registration by the Client |
| A-10 | The Client is responsible for legal review of SMS outreach for TCPA compliance in their jurisdictions |

### 6.2 Dependencies

| ID | Dependency | Owner | Impact if Delayed |
|----|-----------|-------|-------------------|
| DEP-01 | LinkedIn Partner API approval | Client | Phase 2 LinkedIn work items blocked; alternative data sources may be substituted |
| DEP-02 | Twilio account + 10DLC registration | Client | Phase 3 SMS work items blocked |
| DEP-03 | Production hosting environment provisioned | Client | Phase 4 production deployment blocked |
| DEP-04 | API keys for Google, OpenAI, BBB, Census | Client | Reduced data source coverage; OSM remains available |
| DEP-05 | CRM sandbox access (Salesforce/HubSpot) | Client | Phase 3 CRM integration blocked |
| DEP-06 | SSL certificate for production domain | Client | HTTPS enforcement blocked |

---

## 7. Out of Scope

The following items are explicitly excluded from this SOW. They may be addressed in future engagements via separate Change Orders.

| Item | Reason |
|------|--------|
| Mobile native applications (iOS/Android) | Web application accessible on mobile browsers; native apps are a separate initiative |
| OAuth2/OIDC integration (Google/Microsoft SSO) | Enterprise SSO is Phase 2+ consideration; session-based auth is in scope |
| Multi-factor authentication (MFA/2FA) | Security enhancement planned for future phase |
| Custom CRM development | Integration with existing CRMs (Salesforce/HubSpot) is in scope; building a standalone CRM is not |
| White-label / multi-tenant SaaS hosting | Platform architecture supports multi-tenancy at the data layer; SaaS hosting operations are separate |
| Data migration from legacy CRM systems | Import via CSV/API is in scope; custom ETL pipelines for legacy systems are not |
| Real-time chat or messaging within the application | Prospect outreach via SMS/email is in scope; in-app messaging between users is not |
| Video production for training | Written and screenshot-based training materials are in scope; professional video production is not |
| Ongoing API cost management | Caching and optimization to reduce API calls are in scope; ongoing monitoring and cost management of third-party APIs are operational responsibilities |
| Regulatory compliance beyond TCPA for SMS | GDPR, CCPA, and other data privacy regulations require legal counsel and are not technical deliverables |
| Internationalization / Localization | English-only for this engagement; i18n architecture may be noted but not implemented |

---

## 8. Acceptance Criteria

### 8.1 General Acceptance Criteria

All deliverables must meet the following baseline criteria:

1. **Functional Completeness** — All work items described in Section 3 are implemented and demonstrable
2. **Code Quality** — Code compiles without warnings (C++17 standard), passes static analysis, and follows project coding standards
3. **Test Coverage** — Minimum 80% line coverage across services; all critical paths have integration tests
4. **Performance** — Search results display within 3 seconds; background scoring completes within 10 seconds
5. **Security** — No OWASP Top 10 vulnerabilities in delivered code; authentication and authorization enforced on all protected routes
6. **Documentation** — All documentation deliverables (DD-01 through DD-07) are complete, accurate, and current
7. **Data Integrity** — Prospect records persist correctly across sessions; no data loss during profile switching or pipeline stage transitions
8. **Browser Support** — Chrome (latest 2 versions), Firefox (latest 2 versions), Edge (latest 2 versions)

### 8.2 Phase-Specific Acceptance Criteria

#### Phase 0 — Platform Abstraction
- Profile switching at runtime updates all UI labels, scoring rules, AI prompts, and search criteria without application restart
- Three Prospect Profile configs (franchise buyer, catering client, hotel venue) validate against the schema without errors
- No hardcoded "catering," "Vocelli," or domain-specific strings remain in application source code
- Database migration preserves all existing prospect data

#### Phase 1 — Franchise Buyer Vertical
- End-to-end flow: Search for franchise buyer candidates → Score results → Save to pipeline → Advance through 7 pipeline stages
- Investor Score accurately reflects configured scoring rules
- AI analysis produces actionable fit assessments for franchise investment potential

#### Phase 2 — New Data Sources
- LinkedIn search returns results mapped to ProspectRecord; OAuth token refresh works automatically
- Career transition detection flags at least 70% of known transitions in test data
- Data source fallback: system remains functional when any single API is unavailable

#### Phase 3 — Outreach & Engagement
- SMS sent via Twilio and delivery status tracked; opt-out immediately stops messaging
- Campaign sequences execute on schedule; personalized messages pass human review for quality
- CRM sync reflects prospect additions and pipeline changes within 5 minutes

#### Phase 4 — Additional Verticals & Polish
- Catering client profile deployed with zero code changes; original catering functionality verified
- Hotel venue profile deployed with zero code changes; venue prospecting flows verified
- Cross-profile reporting shows correct data segmentation

### 8.3 Acceptance Process

1. Provider delivers each phase with a written Completion Notice listing all deliverables and test results
2. Client has **10 business days** to review, test, and accept or reject with documented defects
3. Provider remediates documented defects within **5 business days**
4. If no response is received within the 10-day review period, the phase is deemed accepted
5. Partial acceptance is permitted — Client may accept individual deliverables within a phase

---

## 9. Project Timeline

### 9.1 Timeline Overview

| Phase | Duration | Start | End | Key Milestone |
|-------|----------|-------|-----|---------------|
| **Phase 0**: Platform Abstraction | 8 weeks | Week 1 | Week 8 | Generic platform operational; profile switching works |
| **Phase 1**: Franchise Buyer Vertical | 8 weeks | Week 9 | Week 16 | First new vertical live with end-to-end pipeline |
| **Phase 2**: New Data Sources | 8 weeks | Week 17 | Week 24 | LinkedIn integration operational; enrichment pipeline active |
| **Phase 3**: Outreach & Engagement | 8 weeks | Week 25 | Week 32 | SMS campaigns operational; CRM sync active |
| **Phase 4**: Additional Verticals & Polish | 8 weeks | Week 33 | Week 40 | All three verticals deployed; production-ready |

**Total Duration: 40 weeks (10 months)**

### 9.2 Key Milestones

| Milestone | Target Date | Deliverables |
|-----------|-------------|--------------|
| **M1** — Project Kickoff | Week 1 | Kickoff meeting, dev environment setup, sprint 0 |
| **M2** — Platform Abstraction Complete | Week 8 | D-01 through D-06 accepted |
| **M3** — First Vertical Live | Week 16 | D-07 accepted; franchise buyer flow operational |
| **M4** — LinkedIn Integration Live | Week 20 | D-08 accepted |
| **M5** — SMS Outreach Live | Week 28 | D-10 accepted; TCPA compliance verified |
| **M6** — All Verticals Deployed | Week 36 | D-13, D-14 accepted |
| **M7** — Production Release | Week 40 | All deliverables accepted; production deployment |
| **M8** — Project Closeout | Week 42 | Knowledge transfer, warranty period begins |

### 9.3 Sprint Cadence

- **Sprint Duration**: 2 weeks
- **Sprint Planning**: First Monday of each sprint
- **Daily Standup**: 15 minutes, daily
- **Sprint Review/Demo**: Last Friday of each sprint
- **Sprint Retrospective**: Following Sprint Review
- **Total Sprints**: 20 sprints across 40 weeks

---

## 10. Team & Roles

### 10.1 Provider Team

| Role | Responsibilities | Allocation |
|------|-----------------|------------|
| **Technical Lead / Architect** | Architecture decisions, code reviews, technical direction, stakeholder communication | Full-time |
| **Senior C++ Developer** | Core platform development (services, models, APIs, scoring engine) | Full-time |
| **Frontend Developer** | Wt widget development, CSS/JS, UI/UX implementation, Leaflet.js maps | Full-time |
| **Backend Developer** | ApiLogicServer configuration, database schema, REST API endpoints, data migrations | Full-time |
| **QA Engineer** | Test strategy, test automation, regression testing, performance testing | Full-time (Phase 1+) |
| **DevOps Engineer** | CI/CD pipeline, deployment automation, monitoring, infrastructure | Part-time |

### 10.2 Client Team

| Role | Responsibilities | Allocation |
|------|-----------------|------------|
| **Product Owner** | Requirements clarification, priority decisions, acceptance testing, stakeholder management | ~10 hrs/week |
| **Business Analyst** | Domain expertise (franchise operations), user story refinement, UAT coordination | ~5 hrs/week |
| **IT/Infrastructure Contact** | Production environment provisioning, network access, SSL, DNS | As needed |
| **Franchise Operations SME** | Validate workflows, provide test data, pilot user feedback | Sprint reviews |

---

## 11. Communication & Governance

### 11.1 Communication Plan

| Activity | Frequency | Participants | Medium |
|----------|-----------|-------------|--------|
| Daily Standup | Daily (Mon–Fri) | Dev team, Tech Lead | Video call (15 min) |
| Sprint Planning | Biweekly | Full team, Product Owner | Video call (2 hrs) |
| Sprint Review / Demo | Biweekly | Full team, Product Owner, Stakeholders | Video call (1 hr) |
| Sprint Retrospective | Biweekly | Dev team, Tech Lead | Video call (45 min) |
| Steering Committee | Monthly | Tech Lead, Product Owner, Project Sponsors | Video call (1 hr) |
| Status Report | Weekly | Tech Lead → Product Owner | Written (email/document) |
| Ad-hoc Technical Discussion | As needed | Relevant parties | Video call or Slack |

### 11.2 Escalation Path

```
Level 1: Technical Lead ↔ Product Owner     (resolve within 2 business days)
Level 2: Project Sponsors                    (resolve within 5 business days)
Level 3: Executive Sponsors                  (resolve within 10 business days)
```

### 11.3 Tools

| Purpose | Tool |
|---------|------|
| Source Control | Git / GitHub |
| Project Tracking | GitHub Issues + Projects |
| Documentation | Markdown in repository (`/docs`) |
| Communication | Slack / Microsoft Teams |
| Video Calls | Zoom / Google Meet |
| CI/CD | GitHub Actions |

---

## 12. Change Management

### 12.1 Change Order Process

Any request to modify the scope, timeline, or budget defined in this SOW must follow this process:

1. **Request**: Either party submits a written Change Request describing the proposed change, rationale, and perceived impact
2. **Impact Assessment**: Provider evaluates schedule, cost, and technical impact within **5 business days**
3. **Approval**: Both parties must approve the Change Order in writing before work begins
4. **Execution**: Approved changes are incorporated into the sprint backlog and tracked as distinct work items
5. **Documentation**: All Change Orders are appended to this SOW as amendments

### 12.2 Change Order Template

| Field | Description |
|-------|-------------|
| Change Order # | Sequential identifier |
| Requested By | Name, role, date |
| Description | What is changing and why |
| Impact — Schedule | Days/weeks added or removed |
| Impact — Cost | Additional cost or credit |
| Impact — Risk | New risks introduced |
| Approval — Client | Signature, date |
| Approval — Provider | Signature, date |

### 12.3 No-Cost Changes

The following types of changes do not require a formal Change Order:

- Bug fixes for defects in delivered work (covered under warranty)
- Minor UI text/label adjustments (< 30 minutes effort)
- Re-prioritization of work items within the same phase (no new work added)

---

## 13. Risk Management

### 13.1 Risk Register

| ID | Risk | Probability | Impact | Mitigation Strategy |
|----|------|-------------|--------|---------------------|
| R-01 | LinkedIn API partner access denied or delayed | Medium | High | Design LinkedIn integration as optional; prepare alternative data sources (franchise broker databases, web scraping with consent) |
| R-02 | TCPA SMS compliance violation | Low | Critical | Legal review before SMS launch; strict opt-in enforcement; automated opt-out processing; quiet hours configuration |
| R-03 | OpenAI/Gemini API cost overruns | Medium | Medium | Aggressive response caching (24-hour), on-demand analysis only (not every search result), per-profile API budgets, usage dashboards |
| R-04 | Over-engineering the platform abstraction | Medium | Medium | Validate schema design against 3 concrete profiles before generalizing; time-box Phase 0 strictly |
| R-05 | API rate limit exhaustion (Google, OSM) | Medium | Low | Implement caching, request batching, exponential backoff; OSM fair-use compliance |
| R-06 | Performance degradation with multiple profiles | Low | Medium | Cache parsed profiles in memory; benchmark scoring engine with all profiles early |
| R-07 | Client team availability/responsiveness | Medium | Medium | Establish clear SLAs for feedback (3 business days); auto-acceptance clause after 10 days |
| R-08 | Wt framework limitations for complex UI | Low | Medium | Identified early in v2.0 development; workarounds established; JavaScript interop available |
| R-09 | Database migration data loss | Low | High | Full backup before migration; staged migration with rollback plan; data validation scripts |
| R-10 | Scope creep | Medium | High | Formal Change Order process; Product Owner prioritization authority; time-boxed sprints |

### 13.2 Risk Review

Risks will be reviewed at each monthly Steering Committee meeting. New risks are added to the register as identified. Probability and impact are reassessed each review cycle.

---

## 14. Investment Summary

### 14.1 Development Costs

| Phase | Duration | Estimated Cost |
|-------|----------|---------------|
| Phase 0 — Platform Abstraction | 8 weeks | $[Amount] |
| Phase 1 — Franchise Buyer Vertical | 8 weeks | $[Amount] |
| Phase 2 — New Data Sources | 8 weeks | $[Amount] |
| Phase 3 — Outreach & Engagement | 8 weeks | $[Amount] |
| Phase 4 — Additional Verticals & Polish | 8 weeks | $[Amount] |
| **Total Development** | **40 weeks** | **$[Total Amount]** |

### 14.2 Payment Schedule

| Milestone | Payment | Trigger |
|-----------|---------|---------|
| Project Kickoff (M1) | [X]% of total | SOW execution |
| Platform Abstraction Complete (M2) | [X]% of total | Phase 0 acceptance |
| First Vertical Live (M3) | [X]% of total | Phase 1 acceptance |
| LinkedIn Integration Live (M4) | [X]% of total | Phase 2 acceptance |
| SMS Outreach Live (M5) | [X]% of total | Phase 3 acceptance |
| Production Release (M7) | [X]% of total | Phase 4 acceptance |
| Project Closeout (M8) | [X]% of total | Final acceptance and knowledge transfer |

### 14.3 Ongoing Operational Costs (Client Responsibility)

| Category | Estimated Annual Cost |
|----------|--------------------|
| Hosting / Infrastructure | $12,000 – $24,000 |
| OpenAI API usage | $3,000 – $12,000 (usage-dependent) |
| Google APIs (Places, Geocoding) | $2,000 – $6,000 |
| Twilio SMS | $1,200 – $6,000 (volume-dependent) |
| LinkedIn API (if applicable) | Per partner agreement |
| PostgreSQL hosting | Included in infrastructure |
| SSL certificate | $0 – $200 (Let's Encrypt or commercial) |
| **Estimated Annual Total** | **$18,200 – $48,200** |

---

## 15. Intellectual Property & Ownership

### 15.1 Work Product Ownership

All custom software, configurations, documentation, and derivative works created under this SOW shall be owned by **[Client]** upon full payment of all associated fees.

### 15.2 Pre-Existing IP

Provider retains ownership of any pre-existing intellectual property, tools, frameworks, or libraries brought into the project. Client receives a perpetual, irrevocable, non-exclusive license to use such pre-existing IP as incorporated into the deliverables.

### 15.3 Open Source Components

The project uses the following open-source components under their respective licenses:

| Component | License |
|-----------|---------|
| Wt (Witty) Web Framework | GPL / Commercial |
| Leaflet.js | BSD-2-Clause |
| OpenStreetMap Data | ODbL |
| CURL | MIT/X-inspired |
| ApiLogicServer | MIT |
| PostgreSQL | PostgreSQL License |

Client acknowledges the obligations of these licenses. Wt framework licensing (GPL vs. commercial) should be reviewed with legal counsel for production deployment.

### 15.4 Source Code Repository

All source code will be maintained in a GitHub repository accessible to both parties throughout the engagement. Upon project completion, full repository ownership (including history) transfers to Client.

---

## 16. Warranty & Support

### 16.1 Warranty Period

Provider warrants all deliverables for **90 days** following final project acceptance (M8). During the warranty period:

- Defects in delivered functionality will be remediated at no additional cost
- "Defect" means behavior that does not conform to the accepted requirements documented in this SOW
- Warranty does not cover issues caused by Client modifications, third-party API changes, or infrastructure changes outside Provider's control

### 16.2 Post-Warranty Support

After the warranty period, ongoing support and maintenance may be contracted under a separate Maintenance & Support Agreement (MSA). Terms to be negotiated prior to warranty expiration.

---

## 17. Termination

### 17.1 Termination for Convenience

Either party may terminate this SOW with **30 days written notice**. Upon termination:

- Client pays for all work completed and accepted through the termination date
- Client pays for work in progress at the time of notice, prorated
- Provider delivers all completed and in-progress work product, source code, and documentation to Client
- Ongoing support obligations cease

### 17.2 Termination for Cause

Either party may terminate immediately upon written notice if:

- The other party materially breaches the SOW and fails to cure within 15 business days of written notice
- The other party becomes insolvent or files for bankruptcy

### 17.3 Survival

Sections 15 (Intellectual Property), 16 (Warranty), and confidentiality obligations survive termination.

---

## 18. Signoff

By signing below, both parties acknowledge they have read, understand, and agree to the terms, scope, deliverables, and obligations defined in this Statement of Work.

### Client Approval

| | |
|---|---|
| **Name** | ________________________________________ |
| **Title** | ________________________________________ |
| **Organization** | ________________________________________ |
| **Signature** | ________________________________________ |
| **Date** | ________________________________________ |

### Provider Approval

| | |
|---|---|
| **Name** | ________________________________________ |
| **Title** | ________________________________________ |
| **Organization** | ________________________________________ |
| **Signature** | ________________________________________ |
| **Date** | ________________________________________ |

---

## Appendix A: Referenced Documents

| Document | Location |
|----------|----------|
| Development Plan | `docs/DEVELOPMENT_PLAN.md` |
| Functional Documentation | `docs/FUNCTIONAL_DOC.md` |
| Generic Transition Plan | `docs/GENERIC_TRANSITION_PLAN2.md` |
| Release Notes v2.0 | `docs/RELEASE_NOTES_v2.md` |
| Data Dictionary | `docs/DATA_DICTIONARY.md` |
| Performance Optimization Guide | `docs/PERFORMANCE_OPTIMIZATION.md` |
| User Guide | `docs/USER_GUIDE.md` |
| Proposal Deck | `docs/PROPOSAL_DECK.md` |
| Database Schema | `database/schema.sql` |
| Project Plan | `docs/PROJECT_PLAN.md` |

## Appendix B: Glossary

| Term | Definition |
|------|-----------|
| **ALS** | ApiLogicServer — Python framework that auto-generates REST APIs from a PostgreSQL schema |
| **BBB** | Better Business Bureau — source of business accreditation and complaint data |
| **Prospect Profile** | JSON configuration that defines all domain-specific behavior for a prospecting vertical |
| **ProspectRecord** | Unified data model for both business and individual prospect targets |
| **Scoring Engine** | Configurable component that calculates prospect quality scores using penalty/bonus rules |
| **Vertical** | A specific business domain or use case (e.g., catering clients, franchise buyers, hotel venues) |
| **Wt (Witty)** | C++ web framework used for the application frontend and server |
| **TCPA** | Telephone Consumer Protection Act — US regulation governing automated SMS/calls |
| **10DLC** | 10-Digit Long Code — standard for business SMS messaging in the US |
| **Pipeline** | Ordered sequence of stages that a prospect moves through from discovery to conversion |
| **RBAC** | Role-Based Access Control — authorization model with admin, franchisee, and staff roles |

---

*End of Statement of Work — SOW-FRAI-2026-001*
