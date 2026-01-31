# Authentication Sprint Plan

## Sprint Overview
**Sprint Goal:** Implement secure user authentication and session management for the FranchiseAI application, enabling multi-user support with role-based access control.

**Prerequisites:**
- Current franchisee/store location loading from ALS (completed)
- AppConfig caching system (completed)
- Clean URL routing (completed)

---

## Phase 1: Database Schema & API Setup

### 1.1 User Table Schema
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
    verification_token VARCHAR(255),

    -- Password Reset
    reset_token VARCHAR(255),
    reset_token_expires TIMESTAMP WITH TIME ZONE,

    -- Session Management
    last_login TIMESTAMP WITH TIME ZONE,
    failed_login_attempts INTEGER DEFAULT 0,
    locked_until TIMESTAMP WITH TIME ZONE,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_users_email ON users(email);
CREATE INDEX idx_users_franchisee ON users(franchisee_id);
```

### 1.2 Session Table Schema
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
    refresh_expires_at TIMESTAMP WITH TIME ZONE,

    -- Status
    is_active BOOLEAN DEFAULT true,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_sessions_token ON user_sessions(session_token);
CREATE INDEX idx_sessions_user ON user_sessions(user_id);
```

### 1.3 Tasks
- [ ] Add users table to schema.sql
- [ ] Add user_sessions table to schema.sql
- [ ] Create default admin user in seed data
- [ ] Update ApiLogicServer model (rebuild from schema)
- [ ] Test CRUD operations via ALS API

---

## Phase 2: Authentication Service (C++ Client)

### 2.1 AuthService Class
```cpp
// src/services/AuthService.h
class AuthService {
public:
    struct LoginResult {
        bool success;
        std::string userId;
        std::string sessionToken;
        std::string errorMessage;
        std::string role;
        std::string franchiseeId;
    };

    struct UserDTO {
        std::string id;
        std::string email;
        std::string firstName;
        std::string lastName;
        std::string role;
        std::string franchiseeId;
        bool isActive;
    };

    // Authentication
    LoginResult login(const std::string& email, const std::string& password);
    bool logout(const std::string& sessionToken);
    bool validateSession(const std::string& sessionToken);

    // Password Management
    bool changePassword(const std::string& userId, const std::string& oldPassword, const std::string& newPassword);
    bool requestPasswordReset(const std::string& email);
    bool resetPassword(const std::string& token, const std::string& newPassword);

    // User Management (admin only)
    UserDTO getUser(const std::string& userId);
    std::vector<UserDTO> getUsers();
    bool createUser(const UserDTO& user, const std::string& password);
    bool updateUser(const UserDTO& user);
    bool deleteUser(const std::string& userId);

private:
    std::string hashPassword(const std::string& password);
    bool verifyPassword(const std::string& password, const std::string& hash);
    std::string generateSessionToken();
};
```

### 2.2 Tasks
- [ ] Create AuthService.h header file
- [ ] Implement AuthService.cpp
- [ ] Add password hashing (bcrypt or similar)
- [ ] Add session token generation (secure random)
- [ ] Integrate with ApiLogicServerClient for API calls
- [ ] Add unit tests for AuthService

---

## Phase 3: Login UI

### 3.1 Login Page Components
- Email input field
- Password input field
- "Remember me" checkbox
- Login button
- "Forgot password" link
- Error message display
- Loading indicator

### 3.2 Login Flow
1. User enters email/password
2. Client validates input format
3. Send credentials to AuthService
4. On success: Store session, redirect to dashboard
5. On failure: Display error, increment attempt counter
6. After 5 failures: Lock account for 15 minutes

### 3.3 Tasks
- [ ] Create showLoginPage() in FranchiseApp
- [ ] Design login page CSS (login-container, login-form)
- [ ] Implement form validation
- [ ] Handle login success/failure
- [ ] Store session token in app state
- [ ] Redirect to appropriate page after login

---

## Phase 4: Session Management

### 4.1 Session Handling
- Store session token in memory (FranchiseApp member)
- Validate session on each page navigation
- Auto-refresh session before expiration
- Handle session expiration gracefully
- Support "Remember me" with longer session duration

### 4.2 Logout Flow
1. Clear local session state
2. Call AuthService::logout() to invalidate server session
3. Redirect to login page
4. Clear any cached user data

### 4.3 Tasks
- [ ] Add session token member to FranchiseApp
- [ ] Add currentUser_ member (UserDTO)
- [ ] Implement session validation on route changes
- [ ] Add logout button to sidebar
- [ ] Implement session refresh mechanism
- [ ] Handle session timeout with user notification

---

## Phase 5: Route Protection

### 5.1 Protected Routes
All routes except login should require authentication:
- /dashboard - requires login
- /search - requires login
- /prospects - requires login
- /openstreetmap - requires login
- /reports - requires login
- /settings - requires login
- /admin/* - requires admin role

### 5.2 Role-Based Access
| Route | Roles Allowed |
|-------|---------------|
| /dashboard | all authenticated |
| /search | all authenticated |
| /prospects | all authenticated |
| /settings | all authenticated |
| /admin/users | admin only |
| /admin/franchisees | admin only |

### 5.3 Tasks
- [ ] Add requireAuth() check to route handler
- [ ] Add requireRole() check for admin routes
- [ ] Redirect unauthenticated users to login
- [ ] Show "Access Denied" for unauthorized role access
- [ ] Add admin menu items (conditional display)

---

## Phase 6: User Profile & Settings

### 6.1 Profile Page Features
- View/edit first name, last name
- View email (read-only)
- Change password
- View associated franchisee
- View role (read-only)
- Last login timestamp

### 6.2 Tasks
- [ ] Add Profile tab to Settings page
- [ ] Implement profile update form
- [ ] Implement change password form
- [ ] Show user info in sidebar header

---

## Phase 7: Admin User Management

### 7.1 Admin Features
- List all users with filters
- Create new user
- Edit user details
- Assign user to franchisee
- Change user role
- Deactivate/reactivate user
- Reset user password

### 7.2 Tasks
- [ ] Create showAdminUsersPage()
- [ ] Implement user list table
- [ ] Implement user create/edit modal
- [ ] Add franchisee assignment dropdown
- [ ] Add role selection dropdown
- [ ] Implement user deactivation

---

## Phase 8: Security Hardening

### 8.1 Security Measures
- [ ] Password requirements (min 8 chars, mixed case, numbers)
- [ ] Rate limiting on login attempts
- [ ] Account lockout after failed attempts
- [ ] Secure session token generation (256-bit random)
- [ ] Session token rotation on sensitive actions
- [ ] HTTPS enforcement (production)
- [ ] CSRF protection
- [ ] SQL injection prevention (parameterized queries)
- [ ] XSS prevention (output encoding)

### 8.2 Audit Logging
```sql
CREATE TABLE audit_log (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID REFERENCES users(id),
    action VARCHAR(100) NOT NULL,
    entity_type VARCHAR(50),
    entity_id UUID,
    old_values JSONB,
    new_values JSONB,
    ip_address VARCHAR(45),
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);
```

### 8.3 Tasks
- [ ] Implement password strength validation
- [ ] Add login attempt tracking
- [ ] Implement account lockout
- [ ] Add audit logging for sensitive actions
- [ ] Review all API calls for security

---

## Phase 9: Testing

### 9.1 Test Cases
- [ ] Login with valid credentials
- [ ] Login with invalid credentials
- [ ] Login with locked account
- [ ] Session validation
- [ ] Session expiration
- [ ] Password change
- [ ] Password reset flow
- [ ] Role-based access control
- [ ] Admin user management

### 9.2 Tasks
- [ ] Add auth test cases to test suite
- [ ] Test all login scenarios
- [ ] Test session management
- [ ] Test role-based access
- [ ] Security penetration testing

---

## Implementation Order

### Week 1: Foundation
1. Database schema (users, sessions)
2. AuthService class (login, logout, validate)
3. Basic login page UI

### Week 2: Core Authentication
4. Session management
5. Route protection
6. Logout functionality

### Week 3: User Management
7. User profile page
8. Change password
9. Admin user list

### Week 4: Polish & Security
10. Admin CRUD operations
11. Security hardening
12. Testing & bug fixes

---

## Definition of Done

- [ ] Users can register (admin creates) and login
- [ ] Sessions persist across page refreshes
- [ ] Sessions expire after configured timeout
- [ ] Protected routes redirect to login
- [ ] Role-based access control works
- [ ] Users can change their password
- [ ] Admins can manage users
- [ ] All auth test cases pass
- [ ] Security review completed

---

## Dependencies

- **bcrypt library** (password hashing) - may need to add to CMakeLists.txt
- **ApiLogicServer** - users/sessions endpoints
- **Secure random generator** - for session tokens

---

## Notes

- Consider OAuth2/OIDC integration in future sprint
- Consider 2FA (two-factor authentication) in future sprint
- Email verification can be added when email service is configured
