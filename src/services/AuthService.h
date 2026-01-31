#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace FranchiseAI {
namespace Services {

/**
 * @brief Result of a login attempt
 */
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

/**
 * @brief User data transfer object
 */
struct UserDTO {
    std::string id;
    std::string email;
    std::string firstName;
    std::string lastName;
    std::string phone;
    std::string role;
    std::string franchiseeId;
    bool isActive = true;
    bool isVerified = false;
};

/**
 * @brief Session information
 */
struct SessionInfo {
    std::string sessionToken;
    std::string userId;
    std::string role;
    std::string franchiseeId;
    bool isValid = false;
    std::string expiresAt;
};

/**
 * @brief Authentication service for user login/logout and session management
 *
 * This service handles:
 * - User authentication (login/logout)
 * - Session token generation and validation
 * - Password hashing and verification
 * - Integration with ApiLogicServer for user data
 */
class AuthService {
public:
    AuthService();
    ~AuthService();

    /**
     * @brief Authenticate user with email and password
     * @param email User's email address
     * @param password User's password (plaintext)
     * @param ipAddress Client IP address for audit logging
     * @return LoginResult with session token on success
     */
    LoginResult login(const std::string& email,
                     const std::string& password,
                     const std::string& ipAddress = "");

    /**
     * @brief End user session
     * @param sessionToken The session token to invalidate
     * @return true if logout successful
     */
    bool logout(const std::string& sessionToken);

    /**
     * @brief Validate a session token
     * @param sessionToken The token to validate
     * @return SessionInfo with validation status
     */
    SessionInfo validateSession(const std::string& sessionToken);

    /**
     * @brief Get user by ID
     * @param userId The user's UUID
     * @return UserDTO if found
     */
    UserDTO getUser(const std::string& userId);

    /**
     * @brief Get user by email
     * @param email The user's email address
     * @return UserDTO if found
     */
    UserDTO getUserByEmail(const std::string& email);

    /**
     * @brief Change user password
     * @param userId User's ID
     * @param oldPassword Current password
     * @param newPassword New password
     * @return true if password changed successfully
     */
    bool changePassword(const std::string& userId,
                       const std::string& oldPassword,
                       const std::string& newPassword);

    /**
     * @brief Check if a user account is locked
     * @param email User's email
     * @return true if account is locked
     */
    bool isAccountLocked(const std::string& email);

    /**
     * @brief Get the current session token (if logged in)
     */
    const std::string& getCurrentSessionToken() const { return currentSessionToken_; }

    /**
     * @brief Get the current user (if logged in)
     */
    const UserDTO& getCurrentUser() const { return currentUser_; }

    /**
     * @brief Check if user is currently authenticated
     */
    bool isAuthenticated() const { return isAuthenticated_; }

private:
    /**
     * @brief Hash password using MD5 (for development)
     * NOTE: In production, use bcrypt or Argon2
     */
    std::string hashPassword(const std::string& password);

    /**
     * @brief Verify password against stored hash
     */
    bool verifyPassword(const std::string& password, const std::string& hash);

    /**
     * @brief Generate a secure session token
     */
    std::string generateSessionToken();

    /**
     * @brief Record login attempt in audit log
     */
    void recordLoginAttempt(const std::string& userId,
                           bool success,
                           const std::string& ipAddress);

    /**
     * @brief Increment failed login counter
     */
    void incrementFailedAttempts(const std::string& email);

    /**
     * @brief Reset failed login counter
     */
    void resetFailedAttempts(const std::string& email);

    // Current session state
    bool isAuthenticated_ = false;
    std::string currentSessionToken_;
    UserDTO currentUser_;

    // Configuration
    int maxFailedAttempts_ = 5;
    int lockoutMinutes_ = 15;
    int sessionDurationMinutes_ = 480;  // 8 hours
};

} // namespace Services
} // namespace FranchiseAI

#endif // AUTH_SERVICE_H
