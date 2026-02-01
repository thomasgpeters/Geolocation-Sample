# Transition to Multiple Entry Points Architecture

## Wt Application Architecture Migration Guide

**Version:** 1.0
**Last Updated:** 2026-02-01
**Status:** Planning / Analysis
**Purpose:** Document the architectural changes required to migrate from single entry point with internal path routing to multiple entry points with token-based authentication

---

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [Current Architecture](#current-architecture)
3. [Target Architecture](#target-architecture)
4. [Architecture Comparison](#architecture-comparison)
5. [Migration Strategy](#migration-strategy)
6. [Implementation Details](#implementation-details)
7. [Session State Management](#session-state-management)
8. [Code Examples](#code-examples)
9. [File Structure Changes](#file-structure-changes)
10. [Effort Estimation](#effort-estimation)
11. [Benefits and Tradeoffs](#benefits-and-tradeoffs)
12. [Decision Criteria](#decision-criteria)

---

## Executive Summary

This document outlines the architectural changes required to transform the FranchiseAI application from a single Wt entry point with internal path routing to multiple entry points with token-based authentication passing.

### Key Change

```
CURRENT:  Single WApplication handles all pages via internal path routing
TARGET:   Separate WApplication per page, with token passed on redirect
```

### Pattern Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Multiple Entry Points Pattern                     │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   User → /  (LoginApp)                                              │
│              │                                                       │
│              │ Authentication successful                             │
│              │ Generate session token                                │
│              ▼                                                       │
│          redirect("/dashboard?token=abc123")                        │
│              │                                                       │
│              ▼                                                       │
│   User → /dashboard?token=abc123  (DashboardApp)                    │
│              │                                                       │
│              │ Validate token                                        │
│              │ Load user context                                     │
│              │ Render dashboard                                      │
│              │                                                       │
│              │ User clicks "AI Search"                               │
│              ▼                                                       │
│          redirect("/search?token=abc123")                           │
│              │                                                       │
│              ▼                                                       │
│   User → /search?token=abc123  (SearchApp)                          │
│              │                                                       │
│              │ Validate token (same pattern)                         │
│              │ Load user context + search state                      │
│              │ Render search page                                    │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Current Architecture

### Single Entry Point Configuration

**File:** `src/main.cpp`

```cpp
Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

server.addEntryPoint(
    Wt::EntryPointType::Application,
    FranchiseAI::createFranchiseApp,  // Single factory function
    "",                                // Root path
    ""                                 // Favicon
);

server.run();
```

### Current URL Routing

**File:** `src/FranchiseApp.cpp` (Lines 346-383)

```cpp
void FranchiseApp::setupRouting() {
    internalPathChanged().connect([this] {
        std::string path = internalPath();

        if (path == "/dashboard") {
            showDashboardPage();
        } else if (path == "/search") {
            showAISearchPage();
        } else if (path == "/prospects") {
            showProspectsPage();
        }
        // ... other routes
    });
}
```

### Current Navigation Pattern

```cpp
void FranchiseApp::onMenuItemSelected(const std::string& itemId) {
    if (itemId == "dashboard") {
        showDashboardPage();           // Swap widgets (instant)
        setInternalPath("/dashboard"); // Update URL
    }
    // All within same WApplication instance
}
```

### Current State Storage

All state is stored in the single `FranchiseApp` instance:

```cpp
class FranchiseApp : public Wt::WApplication {
private:
    // Authentication
    bool isAuthenticated_ = false;
    std::string sessionToken_;
    Services::UserDTO currentUser_;

    // Franchisee context
    Models::Franchisee franchisee_;
    std::string currentStoreLocationId_;
    std::string currentFranchiseeId_;

    // Search state (shared across pages)
    Models::SearchArea currentSearchArea_;
    std::string currentSearchLocation_;
    bool hasActiveSearch_ = false;

    // Cached results
    Models::SearchResults lastResults_;
    std::vector<Models::SearchResultItem> savedProspects_;

    // UI components
    Widgets::Sidebar* sidebar_;
    Widgets::Navigation* navigation_;
    Wt::WContainerWidget* workArea_;
};
```

### Current Page Methods

Each page is a method that clears and rebuilds `workArea_`:

| Method | URL Path | Line |
|--------|----------|------|
| `showDashboardPage()` | `/dashboard` | 891 |
| `showAISearchPage()` | `/search` | 1189 |
| `showProspectsPage()` | `/prospects` | 1273 |
| `showOpenStreetMapPage()` | `/openstreetmap` | 1479 |
| `showReportsPage()` | `/reports` | 2358 |
| `showSettingsPage()` | `/settings` | 2387 |
| `showAuditTrailPage()` | `/audit` | 3086 |

---

## Target Architecture

### Multiple Entry Points Configuration

**File:** `src/main.cpp` (proposed)

```cpp
Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

// Root: Authentication only (no token required)
server.addEntryPoint(
    Wt::EntryPointType::Application,
    [](const Wt::WEnvironment& env) {
        return std::make_unique<LoginApp>(env);
    },
    "",        // Root path: /
    ""
);

// Protected pages (token required)
server.addEntryPoint(
    Wt::EntryPointType::Application,
    [](const Wt::WEnvironment& env) {
        return std::make_unique<DashboardApp>(env);
    },
    "/dashboard",
    ""
);

server.addEntryPoint(
    Wt::EntryPointType::Application,
    [](const Wt::WEnvironment& env) {
        return std::make_unique<SearchApp>(env);
    },
    "/search",
    ""
);

server.addEntryPoint(
    Wt::EntryPointType::Application,
    [](const Wt::WEnvironment& env) {
        return std::make_unique<ProspectsApp>(env);
    },
    "/prospects",
    ""
);

server.addEntryPoint(
    Wt::EntryPointType::Application,
    [](const Wt::WEnvironment& env) {
        return std::make_unique<MapApp>(env);
    },
    "/map",
    ""
);

server.addEntryPoint(
    Wt::EntryPointType::Application,
    [](const Wt::WEnvironment& env) {
        return std::make_unique<ReportsApp>(env);
    },
    "/reports",
    ""
);

server.addEntryPoint(
    Wt::EntryPointType::Application,
    [](const Wt::WEnvironment& env) {
        return std::make_unique<SettingsApp>(env);
    },
    "/settings",
    ""
);

server.addEntryPoint(
    Wt::EntryPointType::Application,
    [](const Wt::WEnvironment& env) {
        return std::make_unique<AuditApp>(env);
    },
    "/audit",
    ""
);

server.run();
```

### Entry Points Summary

| Entry Point | Path | Application Class | Auth Required |
|-------------|------|-------------------|---------------|
| Login | `/` | `LoginApp` | No |
| Dashboard | `/dashboard` | `DashboardApp` | Yes (token) |
| AI Search | `/search` | `SearchApp` | Yes (token) |
| Prospects | `/prospects` | `ProspectsApp` | Yes (token) |
| Map | `/map` | `MapApp` | Yes (token) |
| Reports | `/reports` | `ReportsApp` | Yes (token) |
| Settings | `/settings` | `SettingsApp` | Yes (token) |
| Audit Trail | `/audit` | `AuditApp` | Yes (token) |

### Target Navigation Pattern

```cpp
void BaseProtectedApp::navigateTo(const std::string& path) {
    // Save current state to database
    saveSessionState();

    // Redirect with token (full page navigation)
    redirect(path + "?token=" + sessionToken_);
}
```

---

## Architecture Comparison

| Aspect | Current (Internal Paths) | Target (Multiple Entry Points) |
|--------|-------------------------|-------------------------------|
| **Entry Points** | 1 | 8 |
| **WApplication Instances** | 1 per session | 1 per page view |
| **URL Format** | `/search?_=...` or `/search` | `/search?token=xxx` |
| **Page Transitions** | Instant (widget swap) | Full page reload |
| **State Storage** | In-memory (FranchiseApp) | Database (session_state table) |
| **Memory Usage** | All widgets loaded | Only current page widgets |
| **Authentication** | Once at login | Token validated per page |
| **Browser Back/Forward** | Wt handles internally | Native browser behavior |
| **Bookmarkable URLs** | Requires token parameter | Requires token parameter |
| **Code Organization** | Single large class | Multiple smaller classes |

---

## Migration Strategy

### Phase 1: Infrastructure Setup

1. Create `BaseProtectedApp` base class
2. Create `SessionStateService` for state persistence
3. Add `session_state` table to database schema
4. Create `LoginApp` class

### Phase 2: Extract Common UI

1. Extract common UI setup from `FranchiseApp`
2. Refactor `Sidebar` to use `redirect()` instead of signals
3. Refactor `Navigation` for multi-entry point support

### Phase 3: Create Page Applications

1. Create `DashboardApp` from `showDashboardPage()`
2. Create `SearchApp` from `showAISearchPage()`
3. Create `ProspectsApp` from `showProspectsPage()`
4. Create `MapApp` from `showOpenStreetMapPage()`
5. Create `ReportsApp` from `showReportsPage()`
6. Create `SettingsApp` from `showSettingsPage()`
7. Create `AuditApp` from `showAuditTrailPage()`

### Phase 4: State Migration

1. Implement search state serialization
2. Implement results caching in database
3. Test state persistence across navigations

### Phase 5: Cleanup

1. Remove old `FranchiseApp` class
2. Update `main.cpp` with all entry points
3. Update documentation
4. Comprehensive testing

---

## Implementation Details

### Base Protected Application Class

**File:** `src/apps/BaseProtectedApp.h`

```cpp
#ifndef BASE_PROTECTED_APP_H
#define BASE_PROTECTED_APP_H

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <memory>
#include <string>

#include "services/AuthService.h"
#include "services/ApiLogicServerClient.h"
#include "services/SessionStateService.h"
#include "widgets/Sidebar.h"
#include "widgets/Navigation.h"
#include "models/Franchisee.h"

namespace FranchiseAI {

class BaseProtectedApp : public Wt::WApplication {
public:
    BaseProtectedApp(const Wt::WEnvironment& env, const std::string& pageName);
    virtual ~BaseProtectedApp() = default;

protected:
    // Must be implemented by each page
    virtual void setupPage() = 0;

    // Navigation helper
    void navigateTo(const std::string& path);

    // Logout handler
    void handleLogout();

    // State management
    void saveSessionState();
    void loadSessionState();

    // Accessors for derived classes
    const Models::Franchisee& getFranchisee() const { return franchisee_; }
    const Services::UserDTO& getCurrentUser() const { return currentUser_; }
    const std::string& getSessionToken() const { return sessionToken_; }
    Services::ApiLogicServerClient& getApiClient() { return *apiClient_; }

    // Search state (shared across search-related pages)
    Models::SearchArea currentSearchArea_;
    std::string currentSearchLocation_;
    bool hasActiveSearch_ = false;

    // Common UI components
    Widgets::Sidebar* sidebar_ = nullptr;
    Widgets::Navigation* navigation_ = nullptr;
    Wt::WContainerWidget* contentArea_ = nullptr;

private:
    // Token validation
    bool validateToken(const std::string& token);

    // Context loading
    void loadUserContext();
    void loadFranchiseeData();

    // Common UI setup
    void setupCommonUI();
    void setupSidebar();
    void setupNavigation();

    // Page identification
    std::string pageName_;

    // Authentication state
    std::string sessionToken_;
    Services::UserDTO currentUser_;

    // Franchisee context
    Models::Franchisee franchisee_;
    std::string currentStoreLocationId_;
    std::string currentFranchiseeId_;

    // Services
    std::unique_ptr<Services::AuthService> authService_;
    std::unique_ptr<Services::ApiLogicServerClient> apiClient_;
    std::unique_ptr<Services::SessionStateService> sessionStateService_;
};

} // namespace FranchiseAI

#endif // BASE_PROTECTED_APP_H
```

### Base Protected Application Implementation

**File:** `src/apps/BaseProtectedApp.cpp`

```cpp
#include "BaseProtectedApp.h"
#include <Wt/WBootstrap5Theme.h>

namespace FranchiseAI {

BaseProtectedApp::BaseProtectedApp(const Wt::WEnvironment& env,
                                   const std::string& pageName)
    : Wt::WApplication(env), pageName_(pageName)
{
    // Set theme
    auto theme = std::make_shared<Wt::WBootstrap5Theme>();
    setTheme(theme);

    // Load stylesheet
    useStyleSheet("css/styles.css");

    // Initialize services
    authService_ = std::make_unique<Services::AuthService>();
    apiClient_ = std::make_unique<Services::ApiLogicServerClient>();
    sessionStateService_ = std::make_unique<Services::SessionStateService>();

    // Validate token from URL
    const std::string* token = env.getParameter("token");
    if (!token || token->empty()) {
        // No token - redirect to login
        redirect("/");
        return;
    }

    if (!validateToken(*token)) {
        // Invalid token - redirect to login
        redirect("/");
        return;
    }

    sessionToken_ = *token;

    // Load user context and state
    loadUserContext();
    loadSessionState();

    // Setup common UI (sidebar, navigation)
    setupCommonUI();

    // Setup page-specific content (implemented by derived class)
    setupPage();
}

bool BaseProtectedApp::validateToken(const std::string& token) {
    auto session = authService_->validateSession(token);
    if (session.isValid) {
        currentUser_ = authService_->getUser(session.userId);
        return true;
    }
    return false;
}

void BaseProtectedApp::loadUserContext() {
    loadFranchiseeData();
}

void BaseProtectedApp::loadFranchiseeData() {
    // Load current store/franchisee from app config
    apiClient_->loadAppConfigs();
    currentStoreLocationId_ = apiClient_->getAppConfigValue("current_store_id");
    currentFranchiseeId_ = apiClient_->getAppConfigValue("current_franchisee_id");

    if (!currentStoreLocationId_.empty()) {
        auto response = apiClient_->getStoreLocation(currentStoreLocationId_);
        if (response.success) {
            auto locations = Services::ApiLogicServerClient::parseStoreLocations(response);
            if (!locations.empty()) {
                franchisee_ = locations[0].toFranchisee();
            }
        }
    }
}

void BaseProtectedApp::loadSessionState() {
    auto state = sessionStateService_->loadState(sessionToken_);

    if (!state.searchLocation.empty()) {
        currentSearchLocation_ = state.searchLocation;
        currentSearchArea_ = Models::SearchArea::fromMiles(
            Models::GeoLocation{state.searchLat, state.searchLon},
            state.searchRadiusMiles
        );
        hasActiveSearch_ = true;
    }
}

void BaseProtectedApp::saveSessionState() {
    Services::SessionState state;
    state.currentPage = pageName_;
    state.searchLocation = currentSearchLocation_;
    state.searchLat = currentSearchArea_.center.latitude;
    state.searchLon = currentSearchArea_.center.longitude;
    state.searchRadiusMiles = currentSearchArea_.radiusMiles;

    sessionStateService_->saveState(sessionToken_, state);
}

void BaseProtectedApp::setupCommonUI() {
    // Main layout container
    auto mainContainer = root()->addWidget(
        std::make_unique<Wt::WContainerWidget>());
    mainContainer->setStyleClass("app-container");

    // Sidebar
    setupSidebar();
    mainContainer->addWidget(std::unique_ptr<Wt::WWidget>(sidebar_));

    // Main content area
    auto mainContent = mainContainer->addWidget(
        std::make_unique<Wt::WContainerWidget>());
    mainContent->setStyleClass("main-content");

    // Navigation bar
    setupNavigation();
    mainContent->addWidget(std::unique_ptr<Wt::WWidget>(navigation_));

    // Content area for page-specific content
    contentArea_ = mainContent->addWidget(
        std::make_unique<Wt::WContainerWidget>());
    contentArea_->setStyleClass("content-area");
}

void BaseProtectedApp::setupSidebar() {
    sidebar_ = new Widgets::Sidebar();
    sidebar_->setActiveItem(pageName_);
    sidebar_->setUserRole(currentUser_.role);

    // Connect navigation - uses redirect instead of internal paths
    sidebar_->itemSelected().connect([this](const std::string& itemId) {
        if (itemId == "dashboard") {
            navigateTo("/dashboard");
        } else if (itemId == "ai-search") {
            navigateTo("/search");
        } else if (itemId == "prospects") {
            navigateTo("/prospects");
        } else if (itemId == "openstreetmap") {
            navigateTo("/map");
        } else if (itemId == "reports") {
            navigateTo("/reports");
        } else if (itemId == "settings") {
            navigateTo("/settings");
        } else if (itemId == "audit-trail") {
            navigateTo("/audit");
        }
    });
}

void BaseProtectedApp::setupNavigation() {
    navigation_ = new Widgets::Navigation();
    navigation_->setUserName(currentUser_.firstName + " " + currentUser_.lastName);
    navigation_->setUserEmail(currentUser_.email);

    // Logout handler
    navigation_->logoutRequested().connect(this, &BaseProtectedApp::handleLogout);
}

void BaseProtectedApp::navigateTo(const std::string& path) {
    // Save current state before navigating
    saveSessionState();

    // Redirect with token
    redirect(path + "?token=" + sessionToken_);
}

void BaseProtectedApp::handleLogout() {
    // Log the logout event
    Services::AuditLogger::instance().logLogout(
        currentUser_.id,
        environment().clientAddress()
    );

    // Invalidate session on server
    authService_->logout(sessionToken_);

    // Redirect to login page (no token)
    redirect("/");
}

} // namespace FranchiseAI
```

### Login Application

**File:** `src/apps/LoginApp.h`

```cpp
#ifndef LOGIN_APP_H
#define LOGIN_APP_H

#include <Wt/WApplication.h>
#include <memory>
#include "services/AuthService.h"
#include "widgets/LoginDialog.h"

namespace FranchiseAI {

class LoginApp : public Wt::WApplication {
public:
    explicit LoginApp(const Wt::WEnvironment& env);

private:
    void onLoginSuccessful(const Services::LoginResult& result);

    std::unique_ptr<Services::AuthService> authService_;
    Widgets::LoginDialog* loginDialog_ = nullptr;
};

} // namespace FranchiseAI

#endif // LOGIN_APP_H
```

**File:** `src/apps/LoginApp.cpp`

```cpp
#include "LoginApp.h"
#include <Wt/WBootstrap5Theme.h>

namespace FranchiseAI {

LoginApp::LoginApp(const Wt::WEnvironment& env)
    : Wt::WApplication(env)
{
    setTitle("FranchiseAI - Login");

    // Set theme
    auto theme = std::make_shared<Wt::WBootstrap5Theme>();
    setTheme(theme);

    // Load stylesheet
    useStyleSheet("css/styles.css");

    // Initialize auth service
    authService_ = std::make_unique<Services::AuthService>();

    // Check if user already has valid session (e.g., from cookie)
    // If so, redirect directly to dashboard

    // Create and show login dialog
    auto loginDialogPtr = std::make_unique<Widgets::LoginDialog>();
    loginDialog_ = loginDialogPtr.get();

    loginDialog_->loginSuccessful().connect(
        this, &LoginApp::onLoginSuccessful);

    root()->addWidget(std::move(loginDialogPtr));
    loginDialog_->show();
}

void LoginApp::onLoginSuccessful(const Services::LoginResult& result) {
    // Log successful login
    Services::AuditLogger::instance().logLogin(
        result.userId,
        environment().clientAddress(),
        true
    );

    // Redirect to dashboard with token
    redirect("/dashboard?token=" + result.sessionToken);
}

} // namespace FranchiseAI
```

### Example Page Application: Dashboard

**File:** `src/apps/DashboardApp.h`

```cpp
#ifndef DASHBOARD_APP_H
#define DASHBOARD_APP_H

#include "BaseProtectedApp.h"

namespace FranchiseAI {

class DashboardApp : public BaseProtectedApp {
public:
    explicit DashboardApp(const Wt::WEnvironment& env);

protected:
    void setupPage() override;

private:
    void addQuickStats(Wt::WContainerWidget* parent);
    void addRecentActivity(Wt::WContainerWidget* parent);
    void addHotProspects(Wt::WContainerWidget* parent);
    void addMarketInsights(Wt::WContainerWidget* parent);
};

} // namespace FranchiseAI

#endif // DASHBOARD_APP_H
```

**File:** `src/apps/DashboardApp.cpp`

```cpp
#include "DashboardApp.h"
#include <Wt/WText.h>

namespace FranchiseAI {

DashboardApp::DashboardApp(const Wt::WEnvironment& env)
    : BaseProtectedApp(env, "dashboard")
{
    // Base class handles token validation, UI setup, and calls setupPage()
}

void DashboardApp::setupPage() {
    // Set navigation state
    navigation_->setPageTitle("Dashboard");
    navigation_->setBreadcrumbs({"Home", "Dashboard"});

    // Build dashboard content
    auto container = contentArea_->addWidget(
        std::make_unique<Wt::WContainerWidget>());
    container->setStyleClass("page-container dashboard-page");

    // Add dashboard sections
    addQuickStats(container);
    addRecentActivity(container);
    addHotProspects(container);
    addMarketInsights(container);
}

void DashboardApp::addQuickStats(Wt::WContainerWidget* parent) {
    // Implementation from FranchiseApp::showDashboardPage()
    // Lines 920-1050 approximately
}

void DashboardApp::addRecentActivity(Wt::WContainerWidget* parent) {
    // Implementation from FranchiseApp::showDashboardPage()
}

void DashboardApp::addHotProspects(Wt::WContainerWidget* parent) {
    // Implementation from FranchiseApp::showDashboardPage()
}

void DashboardApp::addMarketInsights(Wt::WContainerWidget* parent) {
    // Implementation from FranchiseApp::showDashboardPage()
}

} // namespace FranchiseAI
```

### Example Page Application: Search

**File:** `src/apps/SearchApp.h`

```cpp
#ifndef SEARCH_APP_H
#define SEARCH_APP_H

#include "BaseProtectedApp.h"
#include "widgets/SearchPanel.h"
#include "widgets/ResultsDisplay.h"
#include "services/AISearchService.h"

namespace FranchiseAI {

class SearchApp : public BaseProtectedApp {
public:
    explicit SearchApp(const Wt::WEnvironment& env);

protected:
    void setupPage() override;

private:
    void onSearchRequested(const Models::SearchQuery& query);
    void onSearchProgress(const Services::SearchProgress& progress);
    void onSearchComplete(const Models::SearchResults& results);
    void onSearchCancelled();

    void restoreSearchState();
    void showBusinessDetails(const Models::BusinessInfo& business);
    void addToProspects(const Models::SearchResultItem& item);

    // Search-specific widgets
    Widgets::SearchPanel* searchPanel_ = nullptr;
    Widgets::ResultsDisplay* resultsDisplay_ = nullptr;

    // Search service
    std::unique_ptr<Services::AISearchService> searchService_;

    // Cached results for this session
    Models::SearchResults lastResults_;
};

} // namespace FranchiseAI

#endif // SEARCH_APP_H
```

---

## Session State Management

### Database Schema Addition

**File:** `database/schema.sql` (addition)

```sql
-- ============================================================================
-- SESSION STATE TABLE
-- ============================================================================

CREATE TABLE session_state (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    session_token VARCHAR(255) NOT NULL UNIQUE,
    user_id UUID REFERENCES users(id) ON DELETE CASCADE,

    -- Current navigation state
    current_page VARCHAR(50),

    -- Search state
    search_location VARCHAR(500),
    search_lat DECIMAL(10, 8),
    search_lon DECIMAL(11, 8),
    search_radius_miles DECIMAL(6, 2),

    -- Cached data (optional, for large result sets)
    last_results_json JSONB,

    -- Timestamps
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_session_state_token ON session_state(session_token);
CREATE INDEX idx_session_state_user ON session_state(user_id);

CREATE TRIGGER update_session_state_updated_at
    BEFORE UPDATE ON session_state
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

COMMENT ON TABLE session_state IS 'Stores transient session state for multi-entry-point navigation';
```

### Session State Service

**File:** `src/services/SessionStateService.h`

```cpp
#ifndef SESSION_STATE_SERVICE_H
#define SESSION_STATE_SERVICE_H

#include <string>

namespace FranchiseAI {
namespace Services {

struct SessionState {
    std::string currentPage;
    std::string searchLocation;
    double searchLat = 0.0;
    double searchLon = 0.0;
    double searchRadiusMiles = 5.0;
    std::string lastResultsJson;  // Optional: serialized SearchResults
};

class SessionStateService {
public:
    SessionStateService();
    ~SessionStateService();

    /**
     * @brief Save session state to database
     * @param token Session token
     * @param state State to save
     * @return true if successful
     */
    bool saveState(const std::string& token, const SessionState& state);

    /**
     * @brief Load session state from database
     * @param token Session token
     * @return Session state (empty if not found)
     */
    SessionState loadState(const std::string& token);

    /**
     * @brief Delete session state (on logout)
     * @param token Session token
     */
    void deleteState(const std::string& token);

    /**
     * @brief Clean up expired session states
     * @param maxAgeMinutes Maximum age in minutes
     */
    void cleanupExpired(int maxAgeMinutes = 1440);

private:
    // Uses ApiLogicServerClient internally
};

} // namespace Services
} // namespace FranchiseAI

#endif // SESSION_STATE_SERVICE_H
```

---

## File Structure Changes

### Current Structure

```
src/
├── main.cpp
├── FranchiseApp.h              # Single monolithic app (3400+ lines)
├── FranchiseApp.cpp
├── AppConfig.h
├── AppConfig.cpp
├── models/
│   ├── BusinessInfo.h
│   ├── DemographicData.h
│   ├── Franchisee.h
│   ├── GeoLocation.h
│   └── SearchResult.h
├── services/
│   ├── AISearchService.h
│   ├── ApiLogicServerClient.h
│   ├── AuthService.h
│   └── ...
└── widgets/
    ├── LoginDialog.h
    ├── Navigation.h
    ├── ResultsDisplay.h
    ├── SearchPanel.h
    └── Sidebar.h
```

### Target Structure

```
src/
├── main.cpp                    # Multiple entry points
├── AppConfig.h
├── AppConfig.cpp
├── apps/                       # NEW: Application classes
│   ├── BaseProtectedApp.h      # Base class for protected pages
│   ├── BaseProtectedApp.cpp
│   ├── LoginApp.h              # Authentication entry point
│   ├── LoginApp.cpp
│   ├── DashboardApp.h          # Dashboard page
│   ├── DashboardApp.cpp
│   ├── SearchApp.h             # AI Search page
│   ├── SearchApp.cpp
│   ├── ProspectsApp.h          # Prospects page
│   ├── ProspectsApp.cpp
│   ├── MapApp.h                # OpenStreetMap page
│   ├── MapApp.cpp
│   ├── ReportsApp.h            # Reports page
│   ├── ReportsApp.cpp
│   ├── SettingsApp.h           # Settings page
│   ├── SettingsApp.cpp
│   └── AuditApp.h              # Audit Trail page
│       AuditApp.cpp
├── models/
│   └── ... (unchanged)
├── services/
│   ├── SessionStateService.h   # NEW: Session state persistence
│   ├── SessionStateService.cpp
│   └── ... (unchanged)
└── widgets/
    └── ... (unchanged, but Sidebar navigation refactored)
```

### Files to Create

| File | Purpose | Estimated Lines |
|------|---------|-----------------|
| `src/apps/BaseProtectedApp.h` | Base class header | 80 |
| `src/apps/BaseProtectedApp.cpp` | Base class implementation | 200 |
| `src/apps/LoginApp.h` | Login app header | 30 |
| `src/apps/LoginApp.cpp` | Login app implementation | 60 |
| `src/apps/DashboardApp.h` | Dashboard header | 30 |
| `src/apps/DashboardApp.cpp` | Dashboard implementation | 300 |
| `src/apps/SearchApp.h` | Search header | 50 |
| `src/apps/SearchApp.cpp` | Search implementation | 400 |
| `src/apps/ProspectsApp.h` | Prospects header | 30 |
| `src/apps/ProspectsApp.cpp` | Prospects implementation | 250 |
| `src/apps/MapApp.h` | Map header | 30 |
| `src/apps/MapApp.cpp` | Map implementation | 200 |
| `src/apps/ReportsApp.h` | Reports header | 30 |
| `src/apps/ReportsApp.cpp` | Reports implementation | 150 |
| `src/apps/SettingsApp.h` | Settings header | 40 |
| `src/apps/SettingsApp.cpp` | Settings implementation | 350 |
| `src/apps/AuditApp.h` | Audit header | 30 |
| `src/apps/AuditApp.cpp` | Audit implementation | 150 |
| `src/services/SessionStateService.h` | Session state header | 50 |
| `src/services/SessionStateService.cpp` | Session state implementation | 150 |

### Files to Delete

| File | Reason |
|------|--------|
| `src/FranchiseApp.h` | Replaced by individual app classes |
| `src/FranchiseApp.cpp` | Replaced by individual app classes |

### Files to Modify

| File | Changes |
|------|---------|
| `src/main.cpp` | Add multiple entry points |
| `src/widgets/Sidebar.cpp` | Change navigation to use redirect |
| `database/schema.sql` | Add session_state table |
| `CMakeLists.txt` | Add new source files |

---

## Effort Estimation

### By Component

| Component | Effort | Hours | Notes |
|-----------|--------|-------|-------|
| `BaseProtectedApp` | Medium | 4-6 | Extract common code from FranchiseApp |
| `LoginApp` | Low | 1-2 | Simple extraction |
| `DashboardApp` | Medium | 3-4 | Extract from showDashboardPage() |
| `SearchApp` | High | 6-8 | Complex state, signals |
| `ProspectsApp` | Medium | 3-4 | Extract from showProspectsPage() |
| `MapApp` | Medium | 3-4 | Extract from showOpenStreetMapPage() |
| `ReportsApp` | Low | 2-3 | Simple page |
| `SettingsApp` | Medium | 4-5 | Complex forms, saving |
| `AuditApp` | Low | 2-3 | Extract from showAuditTrailPage() |
| `SessionStateService` | Medium | 4-5 | New service + database |
| `Sidebar refactor` | Low | 2-3 | Change navigation pattern |
| `main.cpp changes` | Low | 1 | Add entry points |
| `Testing` | High | 8-10 | All pages, state sync |
| **Total** | | **43-58 hours** | **~6-8 days** |

### By Phase

| Phase | Components | Hours | Days |
|-------|------------|-------|------|
| Phase 1: Infrastructure | Base class, Session service, Login | 10-14 | 1.5-2 |
| Phase 2: Common UI | Sidebar refactor, Navigation | 3-4 | 0.5 |
| Phase 3: Page Apps | All 7 page applications | 22-30 | 3-4 |
| Phase 4: State Migration | Search serialization, testing | 8-10 | 1-1.5 |
| **Total** | | **43-58** | **6-8** |

---

## Benefits and Tradeoffs

### Benefits

| Benefit | Description |
|---------|-------------|
| **Clean URLs** | No `?_=` fragments ever |
| **Smaller Memory Footprint** | Only current page widgets loaded |
| **Natural Browser Behavior** | Back/forward works as expected |
| **Independent Deployment** | Could separate pages into modules |
| **Easier Testing** | Each page is self-contained |
| **Clearer Code Organization** | One class per page |
| **Bookmarkable** | URLs with token can be shared |

### Tradeoffs

| Tradeoff | Description | Mitigation |
|----------|-------------|------------|
| **Page Reload Latency** | Full page load on navigation | Optimize page load time |
| **State Serialization** | Must persist state to database | SessionStateService handles this |
| **More Boilerplate** | Each page is a class | Base class reduces duplication |
| **Token in URL** | Less clean than cookie-based | Could use cookies instead |
| **Database Dependency** | State requires database | Cache locally if needed |
| **Complexity** | More moving parts | Good documentation helps |

### Performance Comparison

| Metric | Current | Multi-Entry | Notes |
|--------|---------|-------------|-------|
| Initial Load | ~500ms | ~300ms | Less widgets to load |
| Page Navigation | ~50ms | ~200-400ms | Full page reload |
| Memory (per user) | ~15MB | ~8MB | Only current page |
| Server Connections | 1 | 1 per page | WebSocket reconnection |

---

## Decision Criteria

### When to Migrate

Migrate to multiple entry points if:

- [ ] Clean URLs are a hard requirement
- [ ] Memory usage per user is a concern
- [ ] Pages are largely independent
- [ ] Browser back/forward behavior is important
- [ ] You plan to split the app into microservices
- [ ] SEO matters (different meta tags per page)

### When to Keep Current Architecture

Keep single entry point if:

- [ ] Instant page transitions are important
- [ ] Heavy state sharing between pages
- [ ] Real-time updates across pages needed
- [ ] Development velocity is priority
- [ ] Team is familiar with current patterns

### Hybrid Approach

Consider a hybrid approach:

- Keep single entry point for core workflow (Search → Results → Prospects)
- Use separate entry point for isolated pages (Settings, Audit, Reports)

```cpp
// Hybrid: Main app handles core workflow
server.addEntryPoint(..., createMainApp, "", "");      // Dashboard, Search, Prospects
server.addEntryPoint(..., createSettingsApp, "/settings", "");
server.addEntryPoint(..., createAuditApp, "/audit", "");
server.addEntryPoint(..., createReportsApp, "/reports", "");
```

---

## Appendix: Migration Checklist

### Pre-Migration

- [ ] Review and understand current FranchiseApp structure
- [ ] Identify all shared state that needs persistence
- [ ] Plan database schema changes
- [ ] Set up development branch

### Phase 1: Infrastructure

- [ ] Create `session_state` table
- [ ] Implement `SessionStateService`
- [ ] Create `BaseProtectedApp` class
- [ ] Create `LoginApp` class
- [ ] Test basic login → dashboard flow

### Phase 2: Common UI

- [ ] Refactor `Sidebar` for redirect navigation
- [ ] Test sidebar navigation with token passing
- [ ] Verify logout flow

### Phase 3: Page Applications

- [ ] Create `DashboardApp`
- [ ] Create `SearchApp` with state restoration
- [ ] Create `ProspectsApp`
- [ ] Create `MapApp`
- [ ] Create `ReportsApp`
- [ ] Create `SettingsApp`
- [ ] Create `AuditApp`

### Phase 4: State Migration

- [ ] Implement search state serialization
- [ ] Test state persistence across navigations
- [ ] Test browser back/forward
- [ ] Test session expiry handling

### Phase 5: Cleanup

- [ ] Remove `FranchiseApp.h/cpp`
- [ ] Update `CMakeLists.txt`
- [ ] Update documentation
- [ ] Comprehensive testing
- [ ] Performance benchmarking

---

## Document Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-02-01 | AI Analysis | Initial architecture analysis and migration guide |
