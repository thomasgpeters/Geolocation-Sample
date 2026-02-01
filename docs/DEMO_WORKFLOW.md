# FranchiseAI Tuesday Demo Workflow

## Overview
This document outlines the demonstration workflow for showcasing FranchiseAI's prospect discovery and management capabilities.

## Demo Flow

### 1. Login
- Launch the application
- Login with demo credentials
- System authenticates and loads franchisee data

### 2. Dashboard Review
- **Quick overview of key metrics:**
  - Projected Revenue
  - Active Prospects count
  - Conversion Rate
  - Recent Activity
- **Hot Prospects section:**
  - View top-scoring prospects
  - Note the score legend (80+ Excellent, 60-79 Good, 40-59 Fair, <40 Low)
- Point out the Quick Actions panel

### 3. Navigate to AI Search
- Click "AI Search" in the sidebar or use Quick Actions
- This opens the AI-Powered Prospect Search page

### 4. AI Search Panel Review
- **Location Section:**
  - Search Location field (address or area)
  - City, State, ZIP fields
  - Search Radius slider (5-100 miles)
- **Search Filters:**
  - Keywords input
  - Minimum Potential Score filter
  - Advanced filters (Sort by options)
- Execute a search to demonstrate prospect discovery
- **Results display:**
  - Instant results from OpenStreetMap
  - "Optimizing scores..." indicator while scoring engine runs
  - Results update with calculated scores
  - Filter chips (All, High 60+, Conference)

### 5. Review Prospects
- Click on individual prospect cards to expand details
- Use "View Details" for full prospect information
- Add promising prospects with "+ Add to Prospects" button
- Navigate to "My Prospects" page to see saved prospects
- Demonstrate prospect management features

### 6. Return to Dashboard
- Navigate back to Dashboard
- Note that Hot Prospects section now shows recently added prospects
- Metrics may have updated based on new prospects

### 7. Open Street Map View
- Click "Open Street Map" in the sidebar
- **Key Feature:** Prospects from AI Search are automatically synchronized
  - No need to search again
  - Map is pre-positioned on the franchisee's location
  - All discovered prospects are visible as markers on the map
- Demonstrate map navigation and zoom controls

### 8. Analytics & Research Tools
- Review the map-based analytics features
- Demonstrate Street Map research tools:
  - Area analysis
  - Proximity searches
  - Demographic overlays (if available)
- Show how map and data views complement each other

### 9. Q&A Session
- Open floor for questions
- Demonstrate any specific features requested
- Discuss integration capabilities and roadmap

## Technical Notes

### Prospect Synchronization
The AI Search results are automatically synchronized with the Open Street Map view:
- `lastResults_` stores the most recent search results in `FranchiseApp`
- When navigating to Map view, prospects are loaded from this shared state
- Map automatically centers on the franchisee's configured location
- No duplicate API calls needed - data is cached from initial search

### Score Categories
| Score Range | Category  | Color  |
|-------------|-----------|--------|
| 80+         | Excellent | Green  |
| 60-79       | Good      | Blue   |
| 40-59       | Fair      | Amber  |
| <40         | Low       | Gray   |

### Data Sources
- OpenStreetMap: Business locations and basic info
- Google Places: Ratings, reviews, contact info
- BBB: Accreditation and ratings
- AI Engine: Intelligent scoring and recommendations

## Pre-Demo Checklist
- [ ] Verify login credentials work
- [ ] Confirm API connections (OpenStreetMap, Google, BBB)
- [ ] Test search in demo location area
- [ ] Verify map loads and displays correctly
- [ ] Check that prospect synchronization works
- [ ] Clear any test data from previous sessions (optional)

## Troubleshooting
- **Search returns no results:** Expand radius or try different location
- **Map not loading:** Check network connection, verify map API key
- **Slow performance:** Allow time for initial data load, check network latency
- **Scoring not updating:** Verify ScoringEngine is enabled in settings
