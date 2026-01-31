# FranchiseAI Development Plan

This document outlines the development roadmap for each interface in the FranchiseAI application. Each section details the current state, planned enhancements, and implementation priorities.

---

## Table of Contents

1. [Store Setup Page](#1-store-setup-page)
2. [Dashboard Page](#2-dashboard-page)
3. [AI Search Page](#3-ai-search-page)
4. [My Prospects Page](#4-my-prospects-page)
5. [Demographics Page](#5-demographics-page)
6. [Reports Page](#6-reports-page)
7. [Settings Page](#7-settings-page)
8. [Cross-Cutting Concerns](#8-cross-cutting-concerns)

---

## 1. Store Setup Page

### Current State
- Basic form for entering store name, address, and search preferences
- Geocoding integration to convert address to coordinates
- Stores franchisee configuration in memory

### Planned Enhancements

#### Phase 1: Data Persistence (Priority: High)
- [ ] Persist franchisee configuration to local storage or config file
- [ ] Auto-load configuration on app startup
- [ ] Add "Reset to Defaults" button

#### Phase 2: Enhanced Location Input (Priority: Medium)
- [ ] Add address autocomplete using Nominatim suggestions
- [ ] Display mini-map preview of selected location
- [ ] Support for multiple store locations (multi-franchise)

#### Phase 3: Business Profile (Priority: Medium)
- [ ] Add fields for business hours
- [ ] Catering menu categories/specialties
- [ ] Delivery radius configuration
- [ ] Contact information for outreach

#### Phase 4: Onboarding Wizard (Priority: Low)
- [ ] Convert to multi-step wizard with progress indicator
- [ ] Add guided tour for first-time users
- [ ] Import from existing CRM systems

---

## 2. Dashboard Page

### Current State
- Static stat cards with placeholder data (Total Prospects, Hot Leads, Contact Rate, Projected Revenue)
- Quick action buttons to navigate to other pages
- Welcome message

### Planned Enhancements

#### Phase 1: Live Statistics (Priority: High)
- [ ] Connect stats to actual saved prospects data
- [ ] Calculate real contact rate from prospect status tracking
- [ ] Derive projected revenue from prospect scores and conversion rates
- [ ] Add "Last 7 Days" / "Last 30 Days" / "All Time" filter

#### Phase 2: Activity Feed (Priority: Medium)
- [ ] Show recent prospect additions
- [ ] Display recent searches performed
- [ ] Log contact attempts and outcomes
- [ ] Timeline view of all activity

#### Phase 3: Charts & Visualizations (Priority: Medium)
- [ ] Prospect score distribution chart (pie/donut)
- [ ] Weekly prospect discovery trend (line chart)
- [ ] Geographic distribution map thumbnail
- [ ] Business type breakdown (bar chart)

#### Phase 4: Goal Tracking (Priority: Low)
- [ ] Set weekly/monthly prospect discovery goals
- [ ] Progress bars toward goals
- [ ] Achievement badges for milestones
- [ ] Email/notification when goals are met

#### Phase 5: Widgets System (Priority: Low)
- [ ] Customizable dashboard layout
- [ ] Drag-and-drop widget placement
- [ ] Widget library (clock, weather, calendar integration)
- [ ] Save dashboard configurations per user

---

## 3. AI Search Page

### Current State
- Two-column layout (search panel + results display)
- Location input with radius slider
- Business type filters and data source selection
- Results display with "Add to Prospects" action
- Local scoring for fast results

### Planned Enhancements

#### Phase 1: Search History (Priority: High)
- [ ] Save recent searches with one-click re-run
- [ ] Favorite/bookmark searches
- [ ] Search templates for common queries
- [ ] Clear search history option

#### Phase 2: Advanced Filters (Priority: High)
- [ ] Employee count range slider (min/max)
- [ ] Google rating minimum filter
- [ ] BBB accreditation toggle
- [ ] Conference room / event space requirement
- [ ] Year established filter
- [ ] Save filter presets

#### Phase 3: Results Enhancements (Priority: Medium)
- [ ] Infinite scroll / pagination for large result sets
- [ ] Sort options (score, distance, employee count, name)
- [ ] List view vs. card view toggle
- [ ] Bulk select for adding multiple prospects
- [ ] Export to CSV/Excel directly from results

#### Phase 4: Map Integration (Priority: Medium)
- [ ] Split view with map showing result locations
- [ ] Clickable markers that highlight corresponding cards
- [ ] Cluster markers for dense areas
- [ ] Draw-to-search polygon selection

#### Phase 5: Smart Suggestions (Priority: Low)
- [ ] "Similar businesses" recommendations
- [ ] AI-suggested search refinements
- [ ] Trending business types in area
- [ ] Competitor proximity warnings

---

## 4. My Prospects Page

### Current State
- Card-based display of saved prospects
- Color-coded stat badges (employees, rating, features)
- Score badges with high/medium/low indicators
- AI analysis summary display
- Remove prospect functionality

### Planned Enhancements

#### Phase 1: Data Persistence (Priority: Critical)
- [ ] Persist saved prospects to database/file
- [ ] Load prospects on app startup
- [ ] Auto-save on prospect add/remove
- [ ] Export/import prospects (JSON, CSV)

#### Phase 2: Prospect Status Tracking (Priority: High)
- [ ] Status workflow: New -> Contacted -> Meeting Scheduled -> Proposal Sent -> Won/Lost
- [ ] Status badges with color coding
- [ ] Filter by status
- [ ] Kanban board view option

#### Phase 3: Contact Management (Priority: High)
- [ ] Add contact name, email, phone fields
- [ ] Log contact attempts with notes
- [ ] Schedule follow-up reminders
- [ ] Email template integration
- [ ] Click-to-call / click-to-email links

#### Phase 4: Notes & Attachments (Priority: Medium)
- [ ] Rich text notes per prospect
- [ ] File attachments (proposals, menus, contracts)
- [ ] Activity timeline per prospect
- [ ] Tags/labels for organization

#### Phase 5: Analytics (Priority: Medium)
- [ ] Conversion funnel visualization
- [ ] Time-to-close metrics
- [ ] Revenue by source analysis
- [ ] Prospect quality scoring over time

#### Phase 6: Collaboration (Priority: Low)
- [ ] Assign prospects to team members
- [ ] Shared notes and updates
- [ ] Team activity feed
- [ ] Permission levels (view/edit/admin)

---

## 5. Demographics Page

### Current State
- Interactive Leaflet map with location input
- Category pill tray for POI selection
- POI limit sliders per category
- Color-coded markers on map
- Area statistics footer
- Market potential score in header

### Planned Enhancements

#### Phase 1: Heatmap Visualization (Priority: High)
- [ ] Business density heatmap layer toggle
- [ ] Income level heatmap overlay
- [ ] Population density visualization
- [ ] Competitor concentration overlay

#### Phase 2: Enhanced POI Details (Priority: Medium)
- [ ] Click marker to view POI details popup
- [ ] "Add to Prospects" directly from map marker
- [ ] Marker clustering for zoom levels
- [ ] Custom marker icons per category

#### Phase 3: Comparison Mode (Priority: Medium)
- [ ] Compare two areas side-by-side
- [ ] Score differential highlighting
- [ ] Best area recommendations
- [ ] Overlay comparison metrics

#### Phase 4: Territory Planning (Priority: Medium)
- [ ] Draw custom territories on map
- [ ] Save named territories
- [ ] Territory assignment to salespeople
- [ ] Non-overlapping territory validation

#### Phase 5: Real Demographics Data (Priority: High)
- [ ] Integrate Census API for real population data
- [ ] Income level data by ZIP code
- [ ] Age distribution statistics
- [ ] Business establishment trends

#### Phase 6: Route Planning (Priority: Low)
- [ ] Multi-stop route optimization
- [ ] Drive time calculations
- [ ] Daily visit scheduling
- [ ] Route export to Google Maps/Waze

---

## 6. Reports Page

### Current State
- Placeholder page with icon and description
- No functional reports implemented

### Planned Enhancements

#### Phase 1: Basic Reports (Priority: High)
- [ ] Prospect Summary Report
  - Total prospects by status
  - Score distribution breakdown
  - Source attribution (Google, OSM, BBB)
- [ ] Search Activity Report
  - Searches per day/week/month
  - Most searched locations
  - Filter usage statistics

#### Phase 2: Export Functionality (Priority: High)
- [ ] Export to PDF with formatting
- [ ] Export to Excel/CSV
- [ ] Print-optimized layouts
- [ ] Schedule automated reports

#### Phase 3: Visual Reports (Priority: Medium)
- [ ] Interactive charts (Chart.js integration)
- [ ] Geographic distribution map report
- [ ] Trend analysis over time
- [ ] Comparison period selection

#### Phase 4: Custom Reports (Priority: Medium)
- [ ] Report builder interface
- [ ] Drag-and-drop field selection
- [ ] Custom filters and grouping
- [ ] Save report templates

#### Phase 5: Advanced Analytics (Priority: Low)
- [ ] Predictive scoring models
- [ ] Churn risk identification
- [ ] Market penetration analysis
- [ ] ROI calculations per prospect source

---

## 7. Settings Page

### Current State
- AI Configuration section (OpenAI/Gemini API keys, model selection)
- Data Sources API section (Google, BBB, Census keys)
- Franchise Profile quick link
- Status indicators for configured APIs
- Save functionality

### Planned Enhancements

#### Phase 1: Validation & Testing (Priority: High)
- [ ] "Test Connection" button for each API key
- [ ] Visual feedback on validation (checkmark/error)
- [ ] Show API quota/usage if available
- [ ] Auto-validate on save

#### Phase 2: User Preferences (Priority: Medium)
- [ ] Theme selection (light/dark/system)
- [ ] Default search radius preference
- [ ] Default business types selection
- [ ] Language/locale settings
- [ ] Date/time format preferences

#### Phase 3: Notification Settings (Priority: Medium)
- [ ] Email notification preferences
- [ ] Browser notification permissions
- [ ] Daily/weekly digest options
- [ ] Alert thresholds configuration

#### Phase 4: Data Management (Priority: Medium)
- [ ] Clear all saved data option
- [ ] Backup/restore configuration
- [ ] Import/export all settings
- [ ] Data retention policies

#### Phase 5: Advanced Settings (Priority: Low)
- [ ] AI model temperature/parameters tuning
- [ ] API rate limiting configuration
- [ ] Cache duration settings
- [ ] Debug mode toggle
- [ ] Performance profiling options

---

## 8. Cross-Cutting Concerns

### Authentication & Authorization
- [ ] User login/registration system
- [ ] OAuth integration (Google, Microsoft)
- [ ] Role-based access control
- [ ] Session management
- [ ] Password reset flow

### Database Integration
- [ ] SQLite for local storage
- [ ] PostgreSQL for multi-user deployment
- [ ] Data migration tools
- [ ] Backup automation

### Performance Optimization
- [ ] Implement response caching
- [ ] Lazy loading for large lists
- [ ] Image optimization
- [ ] Bundle size reduction

### Mobile Responsiveness
- [ ] Responsive sidebar (hamburger menu on mobile)
- [ ] Touch-friendly controls
- [ ] Mobile-optimized map interactions
- [ ] Progressive Web App (PWA) support

### Accessibility
- [ ] ARIA labels throughout
- [ ] Keyboard navigation support
- [ ] Screen reader compatibility
- [ ] High contrast mode

### Internationalization
- [ ] String externalization
- [ ] Multi-language support
- [ ] RTL language support
- [ ] Currency/number formatting

### Testing
- [ ] Unit tests for services
- [ ] Integration tests for APIs
- [ ] UI component tests
- [ ] End-to-end testing with Selenium

### Documentation
- [ ] API documentation (Doxygen)
- [ ] User manual with screenshots
- [ ] Video tutorials
- [ ] FAQ section

---

## Priority Legend

| Priority | Description |
|----------|-------------|
| **Critical** | Must have for basic functionality |
| **High** | Important for user experience |
| **Medium** | Enhances productivity |
| **Low** | Nice to have, future consideration |

---

## Version Roadmap

### v1.0 - Foundation (Current)
- Basic search and prospect discovery
- OpenAI integration for AI analysis
- Stat badges and scoring
- Demographics map visualization

### v1.1 - Persistence
- Data persistence for prospects and settings
- Search history
- Export functionality

### v1.2 - Enhanced Search
- Advanced filters
- Map integration in search
- Bulk operations

### v1.3 - CRM Features
- Prospect status tracking
- Contact management
- Activity logging

### v2.0 - Enterprise
- Multi-user support
- Team collaboration
- Advanced reporting
- Territory management

---

*Last Updated: January 2026*
