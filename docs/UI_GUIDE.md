# FranchiseAI UI Guide

This document describes the user interface layout, sidebar navigation, and page functionality.

---

## Application Layout

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                     │
│  ┌──────────┐  ┌─────────────────────────────────────────────────┐  │
│  │          │  │              NAVIGATION HEADER                  │  │
│  │          │  ├─────────────────────────────────────────────────┤  │
│  │  SIDE    │  │                                                 │  │
│  │  BAR     │  │                                                 │  │
│  │          │  │              WORK AREA                          │  │
│  │          │  │              (Page Content)                     │  │
│  │          │  │                                                 │  │
│  │          │  │                                                 │  │
│  └──────────┘  └─────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Sidebar

The sidebar provides primary navigation and user context. Located on the left side of the application.

### Sidebar Header

| Component | Description |
|-----------|-------------|
| **Brand Logo** | FranchiseAI logo with catering icon |
| **User Avatar** | Visual identifier for the logged-in user |
| **User Name** | Display name of the franchise owner |
| **Franchise Name** | Name of the franchise location |

### Sidebar Menu Items

| Icon | Label | ID | Route | Description |
|------|-------|-----|-------|-------------|
| 📊 | Dashboard | `dashboard` | `/dashboard` | Overview of prospect discovery metrics |
| 🔍 | AI Search | `ai-search` | `/search` | Main AI-powered prospect search (default page) |
| 👥 | My Prospects | `prospects` | `/prospects` | Saved prospects management |
| 📍 | Demographics | `demographics` | `/demographics` | Area demographic analysis |
| 📈 | Reports | `reports` | `/reports` | Analytics and reporting |
| ⚙️ | Settings | `settings` | `/settings` | Application configuration |

### Sidebar Footer

| Component | Description |
|-----------|-------------|
| **Collapse Button** | Toggle sidebar between expanded and collapsed states |
| **Version** | Current application version (v1.0.0) |

---

## Navigation Header

The top navigation bar provides context, quick search, and utility actions.

### Left Section

| Component | Description |
|-----------|-------------|
| **Page Title** | Large text showing the current page name |
| **Breadcrumbs** | Navigation path (e.g., "Home > AI Search") |

### Center Section

| Component | Description |
|-----------|-------------|
| **Quick Search** | Text input for fast location-based searches |
| **Search Button** | Triggers quick search and navigates to AI Search page |

### Right Section

| Component | Description |
|-----------|-------------|
| **Help Button** | Access help documentation and tutorials |
| **Notifications** | View system notifications with badge count |
| **User Menu** | Profile and account options |

---

## Page Descriptions

### 1. Dashboard (`/dashboard`)

**Purpose:** Provides a high-level overview of prospect discovery activity.

**Components:**
- **Welcome Header** - Greeting with page description
- **Statistics Grid** - Four metric cards:
  - Total Prospects - Count of all discovered prospects
  - Hot Leads - High-potential prospects requiring attention
  - Contact Rate - Percentage of prospects contacted
  - Projected Revenue - Estimated revenue from current pipeline
- **Quick Actions** - Shortcut buttons:
  - Start AI Search - Navigate to search page
  - View Prospects - Navigate to prospects list
  - View Reports - Navigate to reports

---

### 2. AI Search (`/search`) - DEFAULT PAGE

**Purpose:** Primary feature for discovering potential catering clients.

**Layout:** Two-column design
- **Left Column:** Search Panel (form inputs)
- **Right Column:** Results Display (search results)

#### Search Panel Components:

**Location Section:**
| Field | Type | Description |
|-------|------|-------------|
| Search Location | Text input | City, state, or address |
| Search Radius | Slider (5-100 mi) | Distance from location |
| ZIP Code | Text input | Specific ZIP code |
| City | Text input | City name |
| State | Dropdown | State selection |

**Search Filters:**
| Field | Type | Description |
|-------|------|-------------|
| Keywords | Text input | Search terms (e.g., "technology") |
| Min. Potential Score | Slider (0-80) | Minimum catering potential |
| Business Types | Checkboxes | Filter by type (see below) |
| Sort By | Dropdown | Result ordering |

**Business Type Filters:**
- Corporate Offices
- Warehouses
- Conference Centers
- Tech Companies
- Hotels
- Coworking Spaces

**Data Sources:**
- Google My Business - Business listings and ratings
- Better Business Bureau - Accreditation and reviews
- Demographics Data - Population and business statistics

**Action Buttons:**
- **Clear** - Reset all form fields
- **Cancel** - Stop in-progress search
- **Search Prospects** - Execute the search

#### Results Display Components:

**States:**
- **Empty** - Initial state with instructions
- **Loading** - Animated spinner during search
- **Error** - Error message display
- **Results** - List of prospect cards

**Summary Section:**
- Total prospects found
- Search duration
- AI Analysis summary
- Export and batch action buttons

**Result Cards:**
Each prospect displays:
- Catering Potential Score (0-100)
- Business name and address
- Business type badge
- Key metrics (employees, rating, features)
- AI-generated insights
- Recommended actions
- View Details / Add to Prospects buttons

---

### 3. My Prospects (`/prospects`)

**Purpose:** Manage saved prospects from searches.

**Current State:** Placeholder (future implementation)

**Planned Features:**
- List of saved prospects
- Contact status tracking
- Notes and follow-up scheduling
- Export to CRM
- Bulk actions

---

### 4. Demographics (`/demographics`)

**Purpose:** Explore market potential by geographic area.

**Current State:** Placeholder (future implementation)

**Planned Features:**
- Interactive map visualization
- ZIP code analysis
- Population and income data
- Business density metrics
- Market potential scoring
- Comparison tools

---

### 5. Reports (`/reports`)

**Purpose:** Analytics and performance tracking.

**Current State:** Placeholder (future implementation)

**Planned Features:**
- Search activity history
- Conversion metrics
- Territory coverage analysis
- Export capabilities
- Scheduled reports

---

### 6. Settings (`/settings`)

**Purpose:** Configure application behavior.

**Current Sections:**

| Section | Description |
|---------|-------------|
| **API Configuration** | Set Google My Business and BBB API keys |
| **Search Preferences** | Default search radius and business types |
| **Franchise Profile** | Location and contact information |

---

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Enter` (in Quick Search) | Execute quick search |
| `Esc` | Cancel current operation |

---

## Responsive Behavior

| Breakpoint | Behavior |
|------------|----------|
| Desktop (>1200px) | Full layout with sidebar and two-column search |
| Tablet (768-1200px) | Stacked search columns, hidden quick search |
| Mobile (<768px) | Collapsible sidebar, single-column layout |

---

## CSS Classes Reference

### Layout Classes
- `.app-container` - Main application wrapper
- `.sidebar` - Sidebar container
- `.content-area` - Main content wrapper
- `.work-area` - Page content area

### Sidebar Classes
- `.sidebar-header` - Logo and user section
- `.sidebar-menu` - Navigation menu
- `.menu-item` - Individual menu item
- `.menu-item.active` - Currently selected item

### Navigation Classes
- `.top-navigation` - Header bar
- `.nav-left` - Title and breadcrumbs
- `.nav-center` - Quick search
- `.nav-right` - Utility buttons

### Page Classes
- `.page-container` - Page wrapper
- `.page-header` - Page title area
- `.page-title` - Main heading
- `.page-subtitle` - Secondary text
