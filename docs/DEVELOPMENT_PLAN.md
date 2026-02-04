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
9. [Authentication Sprint](#9-authentication-sprint)
10. [Product Key Licensing](#10-product-key-licensing)

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
- [ ] Duplicate prospect detection - grey out "Add to Prospects" button if business name + address already exists in My Prospects for the current franchisee
  - **Dependencies**: Requires unique key strategy for Prospect table (name + address composite) or client-side lookup against existing prospects
  - **Implementation options**:
    1. Client-side: Query `getProspectsForFranchisee()` and compare before enabling button
    2. Database: Add unique constraint on `(franchisee_id, business_name, address)` to Prospect table
- [ ] Non-blocking "Add to Prospects" confirmation - replace modal dialog with floating toast notification
  - **Current**: Modal dialog blocks interaction until dismissed
  - **Proposed**: Floating bubble/toast showing prospect name + AI analysis summary
  - **Behavior**:
    - Appears in corner (bottom-right or top-right) when prospect is added
    - Shows business name, score badge, and brief AI analysis excerpt
    - Auto-fades after 6 seconds
    - Multiple toasts stack if adding prospects quickly
    - Optional "View" link to jump to My Prospects
  - **Benefits**: Users can continue browsing/adding prospects without mouse interruption

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

## 9. Authentication Sprint

### Sprint Overview
**Goal:** Implement secure user authentication and session management for multi-user support with role-based access control.

**Prerequisites Completed:**
- ✅ Franchisee/store location loading from ALS
- ✅ AppConfig caching system
- ✅ Clean URL routing

### Phase 1: Database Schema & API Setup (Week 1)

#### Users Table
```sql
CREATE TABLE users (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    email VARCHAR(255) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    first_name VARCHAR(100),
    last_name VARCHAR(100),
    role VARCHAR(50) DEFAULT 'franchisee',  -- 'admin', 'franchisee', 'staff'
    franchisee_id UUID REFERENCES franchisees(id),
    is_active BOOLEAN DEFAULT true,
    last_login TIMESTAMP WITH TIME ZONE,
    failed_login_attempts INTEGER DEFAULT 0,
    locked_until TIMESTAMP WITH TIME ZONE,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);
```

#### Sessions Table
```sql
CREATE TABLE user_sessions (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    session_token VARCHAR(255) NOT NULL UNIQUE,
    expires_at TIMESTAMP WITH TIME ZONE NOT NULL,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);
```

#### Tasks
- [ ] Add users table to schema.sql
- [ ] Add user_sessions table to schema.sql
- [ ] Create default admin user in seed data
- [ ] Update ApiLogicServer model
- [ ] Test CRUD operations via ALS API

### Phase 2: Authentication Service (Week 1)

#### AuthService Class Design
```cpp
class AuthService {
public:
    struct LoginResult {
        bool success;
        std::string userId;
        std::string sessionToken;
        std::string role;
        std::string franchiseeId;
        std::string errorMessage;
    };

    LoginResult login(const std::string& email, const std::string& password);
    bool logout(const std::string& sessionToken);
    bool validateSession(const std::string& sessionToken);
    bool changePassword(const std::string& userId,
                       const std::string& oldPassword,
                       const std::string& newPassword);
};
```

#### Tasks
- [ ] Create AuthService.h header file
- [ ] Implement AuthService.cpp
- [ ] Add password hashing (bcrypt)
- [ ] Add secure session token generation
- [ ] Add unit tests for AuthService

### Phase 3: Login UI (Week 1)

#### Components
- Email input field
- Password input field
- "Remember me" checkbox
- Login button
- Error message display

#### Tasks
- [ ] Create showLoginPage() in FranchiseApp
- [ ] Design login page CSS
- [ ] Implement form validation
- [ ] Handle login success/failure
- [ ] Store session token in app state

### Phase 4: Session Management (Week 2)

#### Features
- Session token storage in memory
- Validate session on route changes
- Auto-refresh before expiration
- Handle session timeout gracefully

#### Tasks
- [ ] Add session token member to FranchiseApp
- [ ] Add currentUser_ member (UserDTO)
- [ ] Implement session validation on navigation
- [ ] Add logout button to sidebar
- [ ] Handle session expiration notification

### Phase 5: Route Protection (Week 2)

#### Protected Routes
| Route | Roles Allowed |
|-------|---------------|
| /dashboard | all authenticated |
| /search | all authenticated |
| /prospects | all authenticated |
| /settings | all authenticated |
| /admin/* | admin only |

#### Tasks
- [ ] Add requireAuth() check to route handler
- [ ] Add requireRole() check for admin routes
- [ ] Redirect unauthenticated users to login
- [ ] Show "Access Denied" for unauthorized access
- [ ] Add admin menu items (conditional)

### Phase 6: User Profile (Week 3)

#### Features
- View/edit name
- Change password
- View associated franchisee
- Last login timestamp

#### Tasks
- [ ] Add Profile tab to Settings page
- [ ] Implement profile update form
- [ ] Implement change password form
- [ ] Show user info in sidebar header

### Phase 7: Admin User Management (Week 3)

#### Features
- List all users with filters
- Create/edit/deactivate users
- Assign users to franchisees
- Change user roles

#### Tasks
- [ ] Create showAdminUsersPage()
- [ ] Implement user list table
- [ ] Implement user create/edit modal
- [ ] Add role/franchisee dropdowns

### Phase 8: Security Hardening (Week 4)

#### Measures
- [ ] Password requirements (min 8 chars, mixed case, numbers)
- [ ] Rate limiting on login attempts
- [ ] Account lockout after 5 failures
- [ ] Secure session tokens (256-bit random)
- [ ] SQL injection prevention
- [ ] XSS prevention

#### Audit Logging
```sql
CREATE TABLE audit_log (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id),
    action VARCHAR(100) NOT NULL,
    entity_type VARCHAR(50),
    entity_id UUID,
    ip_address VARCHAR(45),
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);
```

### Phase 9: Testing (Week 4)

#### Test Cases
- [ ] Login with valid/invalid credentials
- [ ] Login with locked account
- [ ] Session validation and expiration
- [ ] Password change flow
- [ ] Role-based access control
- [ ] Admin user management CRUD

### Definition of Done

- [ ] Users can login and logout
- [ ] Sessions persist across page refreshes
- [ ] Sessions expire after timeout
- [ ] Protected routes redirect to login
- [ ] Role-based access control works
- [ ] Users can change passwords
- [ ] Admins can manage users
- [ ] All auth test cases pass
- [ ] Security review completed

### Dependencies

- **bcrypt library** - password hashing (add to CMakeLists.txt)
- **ApiLogicServer** - users/sessions endpoints
- **Secure random generator** - session tokens

### Future Considerations

- OAuth2/OIDC integration (Google, Microsoft login)
- Two-factor authentication (2FA)
- Email verification
- Password reset via email

---

## 10. Product Key Licensing

### Overview

**Priority: CRITICAL — Must be implemented before any deployment beyond DEV.**

The Product Key Licensing system is a hash-based activation gate that prevents the FranchiseAI application from running in any non-DEV environment (TEST, UAT, PROD) without a valid, purchased license. This is the primary mechanism for protecting the software from unauthorized use and ensuring revenue collection before production deployment.

**Core Principle:** The application binary is inert outside of DEV without a valid license. No pages load, no API calls are made, no data is accessible. The app displays a "License Required" lockout screen and refuses to start its main services.

### Threat Model

| Threat | Mitigation |
|--------|------------|
| Running the binary without purchasing a license | Startup gate blocks all functionality; app exits or shows lockout page |
| Copying a license file to another machine | Machine fingerprint binding — license is tied to specific hardware/host |
| Tampering with environment variable to fake DEV mode | Environment detection uses multiple signals, not just a single env var |
| Modifying the binary to skip license check | Integrity checks at multiple code paths; obfuscated validation logic |
| Sharing a license key between deployments | Each license is bound to a unique machine fingerprint hash |
| Replaying an expired license | Expiration date embedded in license payload and validated at startup |

### License Key Format

The license key is a structured, human-readable string with embedded metadata:

```
FRAI-XXXX-XXXX-XXXX-XXXX
```

**Structure:**

```
┌──────────────────────────────────────────────────────────┐
│  FRAI-A1B2-C3D4-E5F6-G7H8                               │
│  ├── FRAI      Product prefix (FranchiseAI)              │
│  ├── A1B2      Licensee identifier (encoded)             │
│  ├── C3D4      Edition + feature flags                   │
│  ├── E5F6      Expiration date (encoded)                 │
│  └── G7H8      Checksum segment                          │
└──────────────────────────────────────────────────────────┘
```

**Editions:**

| Edition | Code | Features |
|---------|------|----------|
| **Standard** | `STD` | Single franchisee, basic search, local scoring |
| **Professional** | `PRO` | Multi-franchisee, AI analysis, demographics |
| **Enterprise** | `ENT` | Unlimited franchisees, API access, white-label, priority support |

### Machine Fingerprint

The license is bound to the deployment target using a composite hardware/host fingerprint. This prevents license file copying between machines.

**Fingerprint Components:**

```
fingerprint = SHA-256(
    hostname +
    primary_mac_address +
    cpu_id +
    os_product_id +
    disk_serial_number
)
```

**Fallback Strategy (if a component is unavailable):**

| Component | Primary Source | Fallback |
|-----------|---------------|----------|
| Hostname | `gethostname()` | `/etc/hostname` |
| MAC Address | Primary NIC | First non-loopback interface |
| CPU ID | `/proc/cpuinfo` (Linux), `sysctl` (macOS) | Omit from hash |
| OS Product ID | `/etc/machine-id` (Linux) | `/var/lib/dbus/machine-id` |
| Disk Serial | `/sys/block/sda/device/serial` | `lsblk --nodeps -o serial` |

At least 3 of 5 components must be available for the fingerprint to be considered valid. This prevents false negatives on minimal VMs/containers while maintaining binding strength.

### Activation Flow

```
┌─────────────────┐     ┌──────────────────┐     ┌──────────────────┐
│   Customer       │     │   License Server  │     │   Target Host    │
│   (purchases)    │     │   (or offline)    │     │   (deploys)      │
└────────┬────────┘     └────────┬─────────┘     └────────┬─────────┘
         │                       │                         │
         │  1. Purchase license  │                         │
         │──────────────────────▶│                         │
         │                       │                         │
         │  2. Receive product   │                         │
         │     key + activation  │                         │
         │     instructions      │                         │
         │◀──────────────────────│                         │
         │                       │                         │
         │                       │  3. Run activation      │
         │                       │     command on host     │
         │                       │◀────────────────────────│
         │                       │                         │
         │                       │  4. Host sends:         │
         │                       │     - Product key       │
         │                       │     - Machine           │
         │                       │       fingerprint       │
         │                       │────────────────────────▶│
         │                       │                         │
         │                       │  5. Server returns      │
         │                       │     signed license      │
         │                       │     file (.lic)         │
         │                       │◀────────────────────────│
         │                       │                         │
         │                       │  6. License file saved  │
         │                       │     to config/          │
         │                       │     franchise.lic       │
         │                       │────────────────────────▶│
         │                       │                         │
         │                       │  7. App starts          │
         │                       │     successfully        │
         │                       │         ✓               │
         └───────────────────────┴─────────────────────────┘
```

**Offline Activation (air-gapped environments):**

```bash
# 1. Generate fingerprint file on target host
./franchise_ai_search --generate-fingerprint > fingerprint.txt

# 2. Transfer fingerprint.txt to a machine with internet access
# 3. Submit fingerprint + product key to activation portal
# 4. Receive franchise.lic file
# 5. Copy franchise.lic to target host config/ directory
```

### License File Format (`franchise.lic`)

The license file is a signed, tamper-evident JSON payload:

```json
{
  "license": {
    "version": 1,
    "product_key": "FRAI-A1B2-C3D4-E5F6-G7H8",
    "licensee": "Acme Franchise Corp",
    "edition": "PRO",
    "issued_at": "2026-02-04T00:00:00Z",
    "expires_at": "2027-02-04T00:00:00Z",
    "max_franchisees": 10,
    "features": ["ai_analysis", "demographics", "multi_store"],
    "machine_fingerprint": "a3f8c2...b7d1e9"
  },
  "signature": "SHA256_HMAC(license_payload, signing_secret)"
}
```

**Validation Chain:**

1. File exists at `config/franchise.lic`
2. JSON parses successfully
3. Signature is valid (HMAC-SHA256 with embedded public component)
4. `product_key` matches expected format
5. `machine_fingerprint` matches current host
6. `expires_at` is in the future
7. `edition` supports the features being used

### LicenseService Class Design

```cpp
namespace FranchiseAI {
namespace Services {

enum class LicenseEdition {
    None,        // No license / invalid
    Standard,
    Professional,
    Enterprise
};

enum class LicenseStatus {
    Valid,
    Missing,           // No license file found
    Corrupt,           // File exists but cannot be parsed
    InvalidSignature,  // Tampered or forged
    Expired,           // Past expiration date
    WrongMachine,      // Fingerprint mismatch
    InvalidKey,        // Product key format invalid
    Revoked            // Key has been revoked (online check)
};

struct LicenseInfo {
    LicenseStatus status;
    LicenseEdition edition;
    std::string licensee;
    std::string productKey;
    std::string issuedAt;
    std::string expiresAt;
    int maxFranchisees;
    std::vector<std::string> features;
    std::string errorMessage;

    bool isValid() const { return status == LicenseStatus::Valid; }
    bool hasFeature(const std::string& feature) const;
};

class LicenseService {
public:
    /**
     * @brief Validate the license at application startup
     * @return LicenseInfo with validation results
     */
    LicenseInfo validateLicense();

    /**
     * @brief Check if the current environment requires a license
     * @return true if not DEV (i.e., license is required)
     */
    bool isLicenseRequired();

    /**
     * @brief Generate the machine fingerprint for this host
     * @return Hex-encoded SHA-256 fingerprint
     */
    std::string generateMachineFingerprint();

    /**
     * @brief Activate a license using a product key (online)
     * @param productKey The FRAI-XXXX-XXXX-XXXX-XXXX key
     * @return LicenseInfo with activation result
     */
    LicenseInfo activateLicense(const std::string& productKey);

    /**
     * @brief Export machine fingerprint for offline activation
     * @param outputPath Path to write fingerprint file
     * @return true if exported successfully
     */
    bool exportFingerprint(const std::string& outputPath);

    /**
     * @brief Get cached license info (no re-validation)
     */
    const LicenseInfo& getCachedLicense() const;

    /**
     * @brief Check if a specific feature is licensed
     */
    bool isFeatureLicensed(const std::string& feature) const;

private:
    LicenseInfo cachedLicense_;

    // Fingerprint generation helpers
    std::string getHostname();
    std::string getMacAddress();
    std::string getCpuId();
    std::string getMachineId();
    std::string getDiskSerial();

    // Crypto helpers
    std::string sha256(const std::string& input);
    std::string hmacSha256(const std::string& data,
                           const std::string& key);
    bool verifySignature(const std::string& payload,
                         const std::string& signature);

    // License file I/O
    std::string readLicenseFile();
    bool writeLicenseFile(const std::string& content);

    // Environment detection
    std::string detectEnvironment();
};

} // namespace Services
} // namespace FranchiseAI
```

### Environment Detection

The system uses multiple signals to determine the current environment, preventing trivial bypass via a single environment variable:

```cpp
/**
 * Environment detection strategy (multi-signal):
 *
 * 1. FRANCHISEAI_ENV environment variable
 * 2. config/env file contents
 * 3. Hostname pattern matching (e.g., *-prod-*, *-uat-*)
 * 4. Presence of config/franchise.lic file
 * 5. Compile-time flag (NDEBUG / Release build)
 *
 * Rules:
 * - If FRANCHISEAI_ENV is explicitly "DEV" AND binary is a Debug build
 *   → DEV mode (no license required)
 * - All other combinations → license required
 * - Release builds ALWAYS require a license regardless of env var
 */
```

| Signal | DEV (no license) | Non-DEV (license required) |
|--------|-------------------|---------------------------|
| `FRANCHISEAI_ENV` | `"DEV"` | `"TEST"`, `"UAT"`, `"PROD"`, or unset |
| Build type | Debug (`-DCMAKE_BUILD_TYPE=Debug`) | Release (`-DCMAKE_BUILD_TYPE=Release`) |
| Combined rule | Both must be true for DEV | Either signal triggers license requirement |

**Key design decision:** Release builds always require a license. This means the `make package` release packages (which are Release builds) can never run without activation, even if someone sets `FRANCHISEAI_ENV=DEV`.

### Startup Gate Integration

The license check happens in `main.cpp` **before** the Wt server starts, ensuring zero functionality is available without activation:

```cpp
// main.cpp — License validation gate (pseudocode)

int main(int argc, char** argv) {
    printBanner();

    // --- LICENSE GATE (runs before anything else) ---
    auto& licenseService = LicenseService::instance();

    if (licenseService.isLicenseRequired()) {
        auto license = licenseService.validateLicense();

        if (!license.isValid()) {
            std::cerr << "========================================" << std::endl;
            std::cerr << "  LICENSE REQUIRED" << std::endl;
            std::cerr << "========================================" << std::endl;
            std::cerr << "  Status: " << toString(license.status) << std::endl;
            std::cerr << "  " << license.errorMessage << std::endl;
            std::cerr << std::endl;
            std::cerr << "  To activate, run:" << std::endl;
            std::cerr << "    ./franchise_ai_search --activate FRAI-XXXX-XXXX-XXXX-XXXX" << std::endl;
            std::cerr << std::endl;
            std::cerr << "  For offline activation:" << std::endl;
            std::cerr << "    ./franchise_ai_search --generate-fingerprint" << std::endl;
            std::cerr << "========================================" << std::endl;
            return EXIT_FAILURE;  // Hard exit — no server, no UI, nothing
        }

        std::cout << "License: Valid (" << license.edition << ")" << std::endl;
        std::cout << "Licensee: " << license.licensee << std::endl;
        std::cout << "Expires: " << license.expiresAt << std::endl;
    } else {
        std::cout << "Environment: DEV (license not required)" << std::endl;
    }
    // --- END LICENSE GATE ---

    // ... rest of startup (config, Wt server, etc.)
}
```

### Secondary Runtime Checks

Beyond the startup gate, the license is re-validated at runtime to prevent long-running processes from outliving an expired license:

| Check Point | Frequency | Action on Failure |
|-------------|-----------|-------------------|
| Application startup (`main.cpp`) | Once | Hard exit (`EXIT_FAILURE`) |
| New session created (`FranchiseApp` constructor) | Per session | Redirect to lockout page |
| Periodic background timer | Every 60 minutes | Log warning, block new sessions |
| Feature access (e.g., AI analysis) | Per use | Return "Feature not licensed" error |
| Settings page | On load | Display license status panel |

### CLI Commands

The binary supports license-related command-line arguments:

```bash
# Activate with a product key (online)
./franchise_ai_search --activate FRAI-A1B2-C3D4-E5F6-G7H8

# Generate machine fingerprint (for offline activation)
./franchise_ai_search --generate-fingerprint

# Show current license status
./franchise_ai_search --license-status

# Deactivate (remove license from this machine)
./franchise_ai_search --deactivate
```

### Configuration Integration

**`config/app_config.json` additions:**

```json
{
  "license": {
    "license_file": "config/franchise.lic",
    "activation_server": "https://license.franchiseai.com/api/v1",
    "check_interval_minutes": 60,
    "grace_period_days": 7
  }
}
```

**`config/env` additions:**

```bash
# License configuration
FRANCHISEAI_ENV=PROD
FRANCHISEAI_LICENSE_FILE=/opt/franchiseai/config/franchise.lic
```

**`start.sh` integration:**

The startup script validates the license before launching the Wt server:

```bash
# Pre-flight license check
echo "Checking license..."
LICENSE_CHECK=$("${SCRIPT_DIR}/franchise_ai_search" --license-status 2>&1)
if echo "$LICENSE_CHECK" | grep -q "INVALID\|MISSING\|EXPIRED"; then
    echo "ERROR: License validation failed."
    echo "$LICENSE_CHECK"
    exit 1
fi
```

### Release Package Integration

Update the release package script (`package-release.sh`) to include:

```
franchiseai-YYYYMMDD/
├── bin/
│   ├── franchise_ai_search
│   └── start.sh
├── config/
│   ├── app_config.sample.json    # Includes license config section
│   ├── env.sample                # Includes FRANCHISEAI_ENV=PROD
│   └── franchise.lic             # NOT included — must be activated per-host
├── ...
```

The `franchise.lic` file is **never** included in the release package. Each deployment target must independently activate.

### Database Schema

```sql
-- License audit trail (tracks activation/deactivation events)
CREATE TABLE license_events (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    event_type VARCHAR(50) NOT NULL,     -- 'activated', 'deactivated', 'expired', 'validation_failed'
    product_key VARCHAR(50),
    machine_fingerprint VARCHAR(64),
    licensee VARCHAR(255),
    edition VARCHAR(20),
    error_message TEXT,
    ip_address VARCHAR(45),
    hostname VARCHAR(255),
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);
```

### Implementation Phases

#### Phase 1: Core License Validation (Priority: Critical)

- [ ] Create `LicenseService.h` header with `LicenseStatus`, `LicenseEdition`, `LicenseInfo` types
- [ ] Implement `LicenseService.cpp` with SHA-256 hashing (using OpenSSL or embedded implementation)
- [ ] Implement machine fingerprint generation (`getHostname`, `getMacAddress`, `getMachineId`)
- [ ] Implement license file parsing (JSON read + field extraction)
- [ ] Implement HMAC-SHA256 signature verification
- [ ] Implement environment detection (multi-signal: env var + build type)
- [ ] Add `--license-status` CLI argument to `main.cpp`
- [ ] Add startup gate to `main.cpp` (hard exit on invalid license)
- [ ] Add license config fields to `AppConfig`
- [ ] Unit tests for all validation paths (`Valid`, `Missing`, `Expired`, `WrongMachine`, `InvalidSignature`)

#### Phase 2: Activation Workflow (Priority: Critical)

- [ ] Implement `--activate <product-key>` CLI command
- [ ] Implement `--generate-fingerprint` CLI command for offline activation
- [ ] Implement `--deactivate` CLI command
- [ ] Build activation server endpoint (or define offline activation process)
- [ ] Write license file to `config/franchise.lic` on successful activation
- [ ] Add `license_events` table to `schema.sql`
- [ ] Log activation/deactivation events to audit trail

#### Phase 3: Runtime Enforcement (Priority: High)

- [ ] Add license check in `FranchiseApp` constructor (session-level gate)
- [ ] Create lockout page UI ("License Required" with activation instructions)
- [ ] Add periodic re-validation timer (every 60 minutes)
- [ ] Implement grace period logic (7-day grace after expiration with warnings)
- [ ] Add feature-level license checks (edition gating)
- [ ] Add license status panel to Settings page

#### Phase 4: Release Pipeline Integration (Priority: High)

- [ ] Update `package-release.sh` to include license config templates
- [ ] Update `start.sh` with pre-flight license check
- [ ] Update `CMakeLists.txt` — set compile-time flag for Release builds
- [ ] Update `Dockerfile` — ensure Release build in container
- [ ] Update `docker-compose.yml` — add license file volume mount
- [ ] Update `DEPLOYMENT_GUIDE.md` with license activation steps
- [ ] Update `DEPLOYMENT_CHECKLIST.md` with license verification step

#### Phase 5: Key Generation Tool (Priority: High)

- [ ] Build `franchise_keygen` utility (separate binary, not distributed)
- [ ] Implement product key generation algorithm
- [ ] Implement license file signing (HMAC-SHA256 with master secret)
- [ ] Support batch key generation for resellers
- [ ] Key revocation list management
- [ ] Secure storage of signing secrets (HSM or vault integration)

#### Phase 6: Hardening (Priority: Medium)

- [ ] Obfuscate validation logic in compiled binary (string encryption, control flow)
- [ ] Add integrity self-check (binary hash validation)
- [ ] Add anti-debugging detection (optional, non-blocking — log only)
- [ ] Implement license check at multiple code paths (not just startup)
- [ ] Rate-limit activation attempts
- [ ] Add telemetry for license validation failures (opt-in)

### Security Considerations

| Consideration | Approach |
|---------------|----------|
| **Signing secret protection** | Never embedded in distributed binary; use asymmetric verification (public key in binary, private key on license server only) |
| **Binary reverse engineering** | Accept that determined attackers can bypass; focus on making casual piracy inconvenient |
| **License file portability** | Machine fingerprint binding prevents file copying; grace period handles hardware changes |
| **Clock manipulation** | Use both local clock and (optional) NTP check; license server timestamp as anchor |
| **Network dependency** | Offline activation fully supported; no phone-home required after initial activation |
| **Container/VM deployment** | Machine fingerprint uses `/etc/machine-id` which persists across reboots but changes on new VM instances; document re-activation procedure |

### Dependencies

- **OpenSSL** (or embedded SHA-256) — hashing and HMAC (`libssl-dev` / `libcrypto`)
- **nlohmann/json** (or existing JSON parser) — license file parsing
- **CMake build type detection** — `CMAKE_BUILD_TYPE` propagated as compile-time define

### Definition of Done

- [ ] Release builds (via `make package`) refuse to start without a valid `franchise.lic`
- [ ] Debug builds on DEV run without a license
- [ ] License is bound to the specific machine (copying `franchise.lic` to another host fails validation)
- [ ] Expired licenses display a clear error with renewal instructions
- [ ] CLI activation workflow (`--activate`) works end-to-end
- [ ] Offline activation workflow (`--generate-fingerprint` + manual `.lic` transfer) works
- [ ] License status is visible on the Settings page
- [ ] All license validation unit tests pass
- [ ] `DEPLOYMENT_GUIDE.md` and `DEPLOYMENT_CHECKLIST.md` updated with activation steps
- [ ] Startup script (`start.sh`) includes pre-flight license check

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
- Demographics map visualization (renamed to Open Street Map)

### v1.1 - Persistence ✅
- ✅ Data persistence via ApiLogicServer
- ✅ AppConfig caching system
- ✅ Franchisee and StoreLocation CRUD
- ✅ Clean URL routing
- Search history
- Export functionality

### v1.2 - Product Key Licensing (GATE — Required Before Non-DEV Deployment)
- Hash-based product key validation (SHA-256)
- Machine fingerprint binding (hostname, MAC, machine-id)
- Startup gate — Release builds refuse to run without valid license
- CLI activation workflow (`--activate`, `--generate-fingerprint`, `--license-status`)
- Signed license file (`franchise.lic`) with HMAC-SHA256
- Offline activation support for air-gapped environments
- Edition-based feature gating (Standard / Professional / Enterprise)
- License status panel in Settings page
- Grace period for expired licenses (7-day warning window)
- Key generation tool (`franchise_keygen`, internal use only)

### v1.3 - Enhanced Search
- Advanced filters
- Map integration in search
- Bulk operations

### v1.4 - CRM Features
- Prospect status tracking
- Contact management
- Activity logging

### v1.5 - Authentication
- User login/logout system
- Session management
- Role-based access control (admin, franchisee, staff)
- User profile management
- Admin user management

### v2.0 - Enterprise
- Multi-user collaboration
- Team features
- Advanced reporting
- Territory management
- OAuth2/OIDC integration

---

*Last Updated: February 2026*
