# FranchiseAI Release Notes - Version 2.0

This document details the recent upgrades to the FranchiseAI application, covering new features, user experience improvements, and performance optimizations.

---

## Table of Contents

1. [Hot Prospects Dashboard](#hot-prospects-dashboard)
2. [Configurable Scoring Engine](#configurable-scoring-engine)
3. [Progressive Loading & Score Optimization](#progressive-loading--score-optimization)
4. [User Interface Improvements](#user-interface-improvements)
5. [Performance Optimizations](#performance-optimizations)
6. [Summary of Changes](#summary-of-changes)

---

## Hot Prospects Dashboard

### Overview

The Dashboard now features a **Hot Prospects** section that provides instant visibility into your top-performing prospects without navigating away from the home screen.

### Features

| Feature | Description |
|---------|-------------|
| **Top 5 Display** | Shows the five highest-scoring prospects at a glance |
| **Score Badges** | Color-coded badges indicate prospect quality |
| **Quick Preview** | Click to open a detailed preview dialog |
| **One-Click Add** | Add prospects directly to your saved list |
| **Auto-Refresh** | Pulls from your most recent search or saved prospects |

### Score Badge Colors

| Score Range | Color | Label |
|-------------|-------|-------|
| 80-100 | Green | Excellent |
| 60-79 | Blue | Good |
| 40-59 | Amber | Fair |
| 0-39 | Gray | Low |

### Score Legend

A color-coded legend appears in the Hot Prospects header, helping users quickly understand what each badge color means without referring to documentation.

### User Experience Benefits

- **Faster Decision Making**: See your best prospects immediately upon login
- **Visual Clarity**: Color coding makes quality assessment instant
- **Reduced Clicks**: Preview and add prospects without leaving the Dashboard
- **Always Current**: Automatically displays the most relevant prospects

---

## Configurable Scoring Engine

### Overview

Franchisees can now customize how prospects are scored through an intuitive Settings interface. This allows each franchise to tailor scoring rules to their specific market and preferences.

### How It Works

The Scoring Engine applies **penalties** and **bonuses** to base prospect scores based on data quality and business attributes.

### Default Scoring Rules

#### Penalties (Subtract from Score)

| Rule | Default Points | Description |
|------|----------------|-------------|
| Missing Address | -10 | No street address available |
| Missing Employee Count | -3 | Unknown organization size |
| Missing Contact Info | -5 | No phone or email available |

#### Bonuses (Add to Score)

| Rule | Default Points | Description |
|------|----------------|-------------|
| Verified Business | +5 | Confirmed accurate information |
| BBB Accredited | +10 | Better Business Bureau accredited |
| High Rating | +5 | 4+ star customer rating |
| Has Conference Room | +5 | Indicates catering opportunity |
| Has Event Space | +7 | High catering potential |
| Large Company | +8 | 100+ employees |

### Settings UI Controls

Navigate to **Settings > Scoring Rules** to customize:

- **Sliders**: Adjust point values for each rule (-20 to +20)
- **Checkboxes**: Enable or disable individual rules
- **Preview**: See how changes affect sample prospects
- **Reset**: Restore default values

### Persistence

All scoring rule customizations are saved to your franchise profile and automatically applied to every search. Rules persist across sessions via the PostgreSQL database.

### User Experience Benefits

- **Personalization**: Adapt scoring to your local market conditions
- **Flexibility**: Emphasize what matters most to your business
- **Consistency**: Same rules apply to all searches automatically
- **Control**: Enable/disable rules without deleting configurations

---

## Progressive Loading & Score Optimization

### Overview

Search results now appear **instantly** while score optimization happens in the background. This provides a responsive experience without sacrificing scoring accuracy.

### How It Works

```
User clicks "Search"
        |
        v
  [1-3 seconds]
        |
        v
  Results appear immediately (base scores)
        |
        v
  "Optimizing scores..." indicator appears
        |
        v
  Scoring Engine runs in background
        |
        v
  Cards re-sort with final scores
        |
        v
  Indicator disappears
```

### Visual Indicators

| State | Indicator |
|-------|-----------|
| Loading | Spinner with "Searching..." text |
| Results Ready | Results display with base scores |
| Optimizing | Pulsing badge: "Optimizing scores..." |
| Complete | Final sorted results, no indicator |

### Technical Details

- **Initial Paint**: OpenStreetMap results display in 1-3 seconds
- **Background Scoring**: Scoring Engine processes results asynchronously
- **Auto-Sort**: Results re-sort by adjusted score when optimization completes
- **No Blocking**: Users can interact with results during optimization

### User Experience Benefits

- **Instant Feedback**: See results immediately, no waiting
- **Transparency**: Know when scoring is still in progress
- **Interactivity**: Browse and interact before final scores
- **Smooth Transitions**: Cards animate to new positions seamlessly

---

## User Interface Improvements

### Floating Search Panel

The AI Search page now features a **floating search panel** with improved visual design:

| Improvement | Description |
|-------------|-------------|
| **Transparent Background** | Left column uses transparent background for depth |
| **White Panel** | Search controls on white background for clarity |
| **Lighter Clear Button** | Softer styling on the Clear button |
| **Field Reordering** | Logical order: City, State, ZIP, then Radius |

### Sticky Results Header

The results toolbar now **sticks to the top** when scrolling:

- White background maintains readability
- Always visible sort and filter controls
- Results count always accessible
- Improves navigation on long result lists

### Compact Result Cards

Result cards have been optimized for space efficiency:

| Change | Benefit |
|--------|---------|
| **Action Buttons in Header** | View Details and Add buttons moved to card header |
| **Reduced Height** | More results visible without scrolling |
| **Transparent Background** | Modern, clean appearance |
| **Rounded Headers** | Polished, professional look |

### Hot Prospects Layout

The Hot Prospects section has been refined:

- **Limited to 4 Visible Rows**: Prevents Dashboard overcrowding
- **Scroll for More**: Smooth scrolling reveals additional prospects
- **No Excess Whitespace**: Fixed flex expansion issues
- **Score Legend**: Clear explanation of badge colors

### User Dropdown Menu

The user dropdown menu positioning has been fixed:

- **Viewport Awareness**: Menu stays within visible area
- **Proper Alignment**: Consistent positioning across screen sizes

---

## Performance Optimizations

### Search Speed Improvements

Typical search time has been reduced from **10-30 seconds to 1-3 seconds**.

### Overpass API Optimizations

| Optimization | Impact |
|--------------|--------|
| **Bounding Box Queries** | 5-10x faster than radius queries |
| **Combined Node/Way Queries** | 50% fewer query clauses |
| **LZ4 Mirror Endpoint** | Faster compression |
| **Quadtile Sorting** | Faster result streaming |
| **Result Limiting** | Max 50 per query type |

### Network Optimizations

| Setting | Value | Benefit |
|---------|-------|---------|
| HTTP Compression | gzip/deflate | Smaller payload sizes |
| TCP_NODELAY | Enabled | Immediate packet delivery |
| TCP_KEEPALIVE | Enabled | Connection health monitoring |
| Connect Timeout | 3 seconds | Fast failure detection |
| Request Timeout | 8 seconds | Prevents hanging requests |

### Timeout Configuration by Service

| Service | Connect | Request |
|---------|---------|---------|
| Overpass API | 3s | 8s |
| Nominatim | 3s | 8s |
| Google APIs | 3s | 5-8s |
| OpenAI | 5s | 30s |

### User Experience Benefits

- **Near-Instant Results**: See prospects in 1-3 seconds
- **No Timeouts**: Aggressive timeouts prevent UI freezes
- **Reliable Fallbacks**: Automatic retry and fallback logic
- **Responsive UI**: Non-blocking operations throughout

---

## Summary of Changes

### New Features

- Hot Prospects section on Dashboard
- Configurable Scoring Engine with Settings UI
- Progressive loading with score optimization
- Score legend in Hot Prospects header

### UI/UX Improvements

- Floating search panel with transparent backgrounds
- Sticky results header toolbar
- Action buttons moved to result card headers
- Compact result cards with reduced height
- Fixed user dropdown menu positioning
- Limited Hot Prospects rows with scrolling
- Reorganized search field order

### Performance

- Search time reduced from 10-30s to 1-3s
- Bounding box queries for faster geolocation
- Optimized network settings and timeouts
- Progressive loading for instant feedback

### Data & Persistence

- Scoring rules stored in PostgreSQL
- Client-side UUID generation
- ApiLogicServer integration for all CRUD operations
- JSON:API format for all requests

---

## Upgrade Path

These improvements are automatically available to all FranchiseAI users. No action is required to benefit from the new features and performance improvements.

### First-Time Setup for Scoring Rules

1. Navigate to **Settings** in the sidebar
2. Select the **Scoring Rules** tab
3. Review and customize the default scoring rules
4. Click **Save** to apply your preferences

Your custom scoring rules will be applied to all future searches immediately.

---

*FranchiseAI v2.0 - Faster, Smarter, More Personalized*
