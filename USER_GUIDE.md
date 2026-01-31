# FranchiseAI User Guide

Welcome to FranchiseAI! This guide will help you get started with finding catering prospects for your franchise business.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Setting Up Your Store](#setting-up-your-store)
3. [Configuring AI Analysis](#configuring-ai-analysis)
4. [Finding Prospects](#finding-prospects)
5. [Managing Your Prospects](#managing-your-prospects)
6. [Exploring Demographics](#exploring-demographics)
7. [Tips & Best Practices](#tips--best-practices)

---

## Getting Started

### First Launch

When you first open FranchiseAI, you'll be taken to the **Store Setup** page. This is where you configure your franchise location, which becomes the center point for all prospect searches.

### Navigation

The application has a sidebar with the following sections:

| Icon | Page | Description |
|------|------|-------------|
| 🏠 | Dashboard | Overview and quick actions |
| 🔍 | AI Search | Find new catering prospects |
| 👥 | My Prospects | View saved prospects with AI analysis |
| 📊 | Demographics | Explore area data on an interactive map |
| 📈 | Reports | Analytics and reports |
| ⚙️ | Settings | Configure API keys and preferences |

---

## Setting Up Your Store

### Required Information

1. **Store Name**: Your franchise location name (e.g., "Vocelli Pizza - Downtown")
2. **Store Address**: Full address for geocoding your location
3. **Owner/Manager Name**: Contact person for the store
4. **Store Phone**: Contact number

### Search Preferences

Configure your default search settings:

- **Default Search Radius**: How far from your store to search (in miles)
- **Target Business Types**: Check the types of businesses you want to target:
  - Corporate Offices
  - Conference Centers
  - Hotels
  - Medical Facilities
  - Tech Companies
  - Coworking Spaces
  - And more...
- **Target Organization Size**: Filter by employee count

Click **"Save and Continue to Search"** to complete setup.

---

## Configuring AI Analysis

FranchiseAI can use OpenAI's GPT models to provide deep analysis of catering prospects. This is optional but highly recommended.

### Getting an OpenAI API Key

1. Go to [platform.openai.com](https://platform.openai.com)
2. Sign up or log in to your account
3. Navigate to API Keys section
4. Create a new API key
5. Copy the key (starts with `sk-`)

### Adding Your API Key

#### Option 1: Settings Page (Easiest)

1. Click **Settings** in the sidebar
2. Find the **AI Configuration** section
3. Paste your OpenAI API key
4. Select your preferred model:
   - **gpt-4o** (Recommended) - Best quality analysis
   - **gpt-4o-mini** - Faster and lower cost
5. Click **Save Settings**

#### Option 2: Environment Variable

```bash
export OPENAI_API_KEY="sk-your-api-key-here"
```

#### Option 3: Configuration File

Create `config/app_config.json`:
```json
{
  "openai_api_key": "sk-your-api-key-here",
  "openai_model": "gpt-4o"
}
```

### Verifying Configuration

The Settings page shows your AI configuration status:
- **Green badge**: AI Engine configured and ready
- **Yellow badge**: Not configured (local analysis will be used)

---

## Finding Prospects

### Using AI Search

1. Click **AI Search** in the sidebar
2. Enter a location (city, address, or ZIP code)
3. Adjust the search radius using the slider
4. Select which data sources to include:
   - **OpenStreetMap** (always available, free)
   - **Google My Business** (requires API key)
   - **Better Business Bureau** (requires API key)
5. Click **Search**

### Understanding Search Results

Each result shows:
- **Business Name**: The prospect's name
- **Type & Size**: Business category and approximate employee count
- **Catering Score**: A percentage indicating catering potential
- **Local Insights**: Quick analysis of why this is a good prospect

Results are sorted by score, with the best prospects first.

### Adding Prospects

When you find a promising prospect:

1. Click **"Add to Prospects"** on the result card
2. If AI is configured, the system will analyze the business using OpenAI
3. A confirmation dialog shows the AI analysis summary
4. The prospect is saved to your My Prospects list

**Why is AI analysis done here?** This approach keeps searches fast (no waiting for AI) while providing deep analysis only for prospects you're interested in.

---

## Managing Your Prospects

### Viewing Saved Prospects

Click **My Prospects** in the sidebar to see all your saved prospects.

Each prospect card shows:
- **Business Name** with score badge
- **Address** and business type
- **AI Analysis**: Summary of catering potential (if AI was configured)
- **Key Highlights**: Important points about the business
- **Actions**: Remove button

### Score Badges

| Color | Score | Meaning |
|-------|-------|---------|
| 🟢 Green | 70%+ | High catering potential |
| 🟡 Yellow | 40-69% | Medium potential |
| 🔴 Red | <40% | Lower potential |

### Removing Prospects

Click the **Remove** button on any prospect card to delete it from your list.

---

## Exploring Demographics

The Demographics page helps you understand the business landscape in any area.

### Using the Map

1. Enter a location in the search bar
2. Select a radius (5-50 km)
3. Click **Analyze Area**

### Category Pills

Add business categories to visualize on the map:

1. Select a category from the dropdown (e.g., "Offices", "Hotels")
2. A colored pill appears in the tray
3. Use the slider to control how many POIs to display
4. Markers appear on the map in matching colors

Each pill shows:
- Category name and total count
- Slider to limit displayed POIs
- Remove button (×)

### Market Score

The navigation bar shows a **Market Potential Score**:
- **80-100** (Green): High potential area
- **50-79** (Yellow): Medium potential
- **0-49** (Red): Lower potential

---

## Tips & Best Practices

### Search Tips

1. **Start broad, then narrow**: Begin with a larger radius to understand the area
2. **Use OpenStreetMap**: It's free and provides good coverage
3. **Check Demographics first**: Use the map to identify promising areas before searching

### Saving Prospects

1. **Be selective**: Only save prospects you're genuinely interested in
2. **AI analysis is per-prospect**: Each save triggers one API call
3. **Review the AI summary**: It provides actionable insights

### API Usage

1. **OpenAI costs money**: Each AI analysis uses tokens
2. **gpt-4o-mini is cheaper**: Use it for budget-conscious analysis
3. **Local analysis works too**: The app functions without API keys

### Getting Better Results

1. **Configure your store location accurately**: This affects distance calculations
2. **Select relevant business types**: Narrow targeting improves results
3. **Use the Demographics map**: Visual exploration helps find hotspots

---

## Troubleshooting

### Search Not Returning Results

- Check your internet connection
- Verify the location is valid (try a major city first)
- Expand the search radius

### AI Analysis Not Working

- Verify your API key in Settings
- Check the status indicator shows "Configured"
- Ensure your OpenAI account has credits

### Map Not Loading

- Refresh the page
- Check if Leaflet CDN is accessible
- Try a different browser

---

## Getting Help

- **GitHub Issues**: Report bugs or request features
- **README.md**: Technical documentation
- **Settings Page**: API configuration help text

---

*FranchiseAI - Discover Your Next Catering Client*
