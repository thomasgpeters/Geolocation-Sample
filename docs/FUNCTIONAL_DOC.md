# FranchiseAI Functional Documentation

## Authentication System

### Overview

The FranchiseAI application implements a session-based authentication system that requires users to log in before accessing the application. The authentication flow uses URL-based session tokens to maintain state across page navigations.

### Authentication Flow

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│  User visits /  │────▶│  Login Dialog   │────▶│  Validate       │
│  (root URL)     │     │  Displayed      │     │  Credentials    │
└─────────────────┘     └─────────────────┘     └────────┬────────┘
                                                         │
                        ┌─────────────────┐              │
                        │  /dashboard     │◀─────────────┤ Success
                        │  ?token=xxx     │              │
                        └─────────────────┘              │
                                                         │
                        ┌─────────────────┐              │
                        │  Show Error     │◀─────────────┘ Failure
                        │  Message        │
                        └─────────────────┘
```

### URL Structure

| Route | Description | Authentication Required |
|-------|-------------|------------------------|
| `/` | Login page (shows modal dialog) | No |
| `/dashboard?token=xxx` | Main dashboard | Yes |
| `/search?token=xxx` | AI Search page | Yes |
| `/prospects?token=xxx` | Prospects list | Yes |
| `/openstreetmap?token=xxx` | Map view | Yes |
| `/reports?token=xxx` | Reports page | Yes |
| `/settings?token=xxx` | Settings page | Yes |

### Session Token

After successful authentication, a 64-character hexadecimal session token is generated and:
1. Stored in the `user_sessions` database table
2. Added to the URL as a query parameter (`?token=xxx`)
3. Validated on each page load from URL parameters

**Token Format:** `[a-f0-9]{64}` (64 hex characters)

**Example URL:** `/dashboard?token=a1b2c3d4e5f6...`

---

## Database Schema

### Users Table

```sql
CREATE TABLE users (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    email VARCHAR(255) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,

    -- Profile
    first_name VARCHAR(100),
    last_name VARCHAR(100),
    phone VARCHAR(30),

    -- Role & Permissions
    role VARCHAR(50) DEFAULT 'franchisee',  -- 'admin', 'franchisee', 'staff'

    -- Franchise Association
    franchisee_id UUID REFERENCES franchisees(id),

    -- Account Status
    is_active BOOLEAN DEFAULT true,
    is_verified BOOLEAN DEFAULT false,

    -- Security
    failed_login_attempts INTEGER DEFAULT 0,
    locked_until TIMESTAMP WITH TIME ZONE,
    last_login TIMESTAMP WITH TIME ZONE,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);
```

### User Sessions Table

```sql
CREATE TABLE user_sessions (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,

    session_token VARCHAR(255) NOT NULL UNIQUE,
    refresh_token VARCHAR(255),

    -- Session Info
    ip_address VARCHAR(45),
    user_agent TEXT,

    -- Expiration
    expires_at TIMESTAMP WITH TIME ZONE NOT NULL,
    is_active BOOLEAN DEFAULT true,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);
```

### Audit Log Table

```sql
CREATE TABLE audit_log (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id),

    event_type VARCHAR(50) NOT NULL,  -- 'login', 'logout', 'failed_login'
    event_details JSONB,

    ip_address VARCHAR(45),
    user_agent TEXT,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);
```

---

## User Roles

| Role | Description | Permissions |
|------|-------------|-------------|
| `admin` | System administrator | Full access to all features and users |
| `franchisee` | Franchise owner | Access to own franchise data, prospects, settings |
| `staff` | Franchise staff member | Limited access based on assignment |

### Role-Based Access

- **Admin users** (`role='admin'`): Can access all franchisees and system settings
- **Franchisee users** (`role='franchisee'`): Linked to a specific franchisee via `franchisee_id`, can only access their own data
- **Staff users** (`role='staff'`): Limited access, linked to franchisee

---

## Default Credentials

For development and testing purposes, the following default users are seeded:

| Email | Password | Role | Franchisee |
|-------|----------|------|------------|
| `admin@franchiseai.com` | `admin123` | admin | None |
| `mike@pittsburghcatering.com` | `mike123` | franchisee | Pittsburgh Catering Co |

> **Note:** These credentials use MD5 hashing for development only. Production deployments should use bcrypt or Argon2.

---

## Components

### AuthService (`src/services/AuthService.h`)

The authentication service handles all authentication operations:

```cpp
class AuthService {
public:
    // Authentication
    LoginResult login(const std::string& email,
                     const std::string& password,
                     const std::string& ipAddress = "");
    bool logout(const std::string& sessionToken);
    SessionInfo validateSession(const std::string& sessionToken);

    // User Management
    UserDTO getUser(const std::string& userId);
    UserDTO getUserByEmail(const std::string& email);

    // Password Management
    bool changePassword(const std::string& userId,
                       const std::string& oldPassword,
                       const std::string& newPassword);

    // Account Security
    bool isAccountLocked(const std::string& email);
};
```

### LoginResult Structure

```cpp
struct LoginResult {
    bool success = false;
    std::string userId;
    std::string sessionToken;
    std::string errorMessage;
    std::string role;
    std::string franchiseeId;
    std::string firstName;
    std::string lastName;
    std::string email;
};
```

### LoginDialog (`src/widgets/LoginDialog.h`)

Modal dialog widget for user authentication:

- Email input field
- Password input field (masked)
- "Remember me" checkbox
- Sign In button
- Error message display
- Loading state during authentication

**Styling:** The dialog uses custom CSS with a gradient background and centered card layout.

---

## Application Integration

### FranchiseApp Constructor Flow

```cpp
FranchiseApp::FranchiseApp(const Wt::WEnvironment& env) {
    // 1. Initialize services
    authService_ = std::make_unique<Services::AuthService>();

    // 2. Check for token in URL parameters
    const std::string* tokenParam = env.getParameter("token");
    if (tokenParam && !tokenParam->empty()) {
        SessionInfo session = authService_->validateSession(*tokenParam);
        if (session.isValid) {
            isAuthenticated_ = true;
            sessionToken_ = *tokenParam;
            // Load user data...
        }
    }

    // 3. Show login if at root URL and not authenticated
    if ((initialPath.empty() || initialPath == "/") && !isAuthenticated_) {
        showLoginDialog();
        return;
    }

    // 4. Redirect to login if not authenticated
    if (!isAuthenticated_ && initialPath != "/") {
        redirectToLogin();
        return;
    }

    // 5. Load app and show dashboard
    setupUI();
    setupRouting();
    showDashboardPage();
}
```

### Post-Login Redirect

After successful login:

```cpp
void FranchiseApp::onLoginSuccessful(const Services::LoginResult& result) {
    // Store authentication state
    isAuthenticated_ = true;
    sessionToken_ = result.sessionToken;
    currentUser_ = /* user data from result */;

    // Update browser URL with token
    std::ostringstream js;
    js << "window.history.replaceState({}, '', '/dashboard?token="
       << sessionToken_ << "');";
    doJavaScript(js.str());

    // Show dashboard
    showDashboardPage();
}
```

---

## Security Considerations

### Current Implementation (Development)

- MD5 password hashing (for simplicity)
- Session tokens stored in URL
- Basic input validation

### Production Recommendations

1. **Password Hashing:** Use bcrypt or Argon2 with proper salting
2. **Session Storage:** Consider HTTP-only cookies instead of URL tokens
3. **HTTPS:** Enforce HTTPS for all connections
4. **Rate Limiting:** Implement login attempt rate limiting
5. **Token Expiration:** Enforce session expiration (currently 8 hours)
6. **CSRF Protection:** Add CSRF tokens for form submissions
7. **Account Lockout:** Lock accounts after failed attempts (5 attempts, 15 min lockout)

### Account Lockout Policy

```cpp
int maxFailedAttempts_ = 5;      // Lock after 5 failed attempts
int lockoutMinutes_ = 15;        // Lock for 15 minutes
int sessionDurationMinutes_ = 480; // 8 hour session duration
```

---

## API Integration

The authentication system integrates with ApiLogicServer through generic resource methods:

```cpp
// Get user by email filter
std::string userJson = alsClient.getResource("User", "", "email=" + email);

// Create session
std::string sessionResult = alsClient.createResource("UserSession", sessionJson);

// Update session (logout)
alsClient.updateResource("UserSession", sessionId, updateJson);
```

### JSON:API Format

All API requests follow the JSON:API specification:

```json
{
  "data": {
    "type": "User",
    "id": "uuid-here",
    "attributes": {
      "email": "user@example.com",
      "first_name": "John",
      "last_name": "Doe",
      "role": "franchisee"
    }
  }
}
```

---

## Future Enhancements

- [ ] OAuth2/OpenID Connect integration
- [ ] Two-factor authentication (2FA)
- [ ] Password reset via email
- [ ] Email verification for new accounts
- [ ] Session management UI (view/revoke sessions)
- [ ] Role-based UI customization
- [ ] API key authentication for integrations
