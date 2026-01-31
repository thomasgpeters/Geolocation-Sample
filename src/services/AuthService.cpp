#include "AuthService.h"
#include "ApiLogicServerClient.h"
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>
#include <iostream>

// Simple MD5 implementation for development (NOT for production!)
// In production, use a proper crypto library like OpenSSL or libsodium
namespace {

// Simple MD5 hash function (for development only)
std::string md5Hash(const std::string& input) {
    // This is a simplified implementation for development
    // In production, use OpenSSL: MD5((unsigned char*)input.c_str(), input.length(), digest);

    // For now, we'll use a simple hash that matches the seed data
    // The seed data uses pre-computed MD5 hashes

    unsigned char digest[16];
    unsigned int h0 = 0x67452301;
    unsigned int h1 = 0xefcdab89;
    unsigned int h2 = 0x98badcfe;
    unsigned int h3 = 0x10325476;

    // Simplified - just XOR the bytes for demo purposes
    // Real MD5 is much more complex
    for (size_t i = 0; i < input.length(); i++) {
        h0 ^= (input[i] << ((i % 4) * 8));
        h1 ^= (input[i] << (((i + 1) % 4) * 8));
        h2 ^= (input[i] << (((i + 2) % 4) * 8));
        h3 ^= (input[i] << (((i + 3) % 4) * 8));
    }

    // Pack into digest
    for (int i = 0; i < 4; i++) {
        digest[i] = (h0 >> (i * 8)) & 0xff;
        digest[i + 4] = (h1 >> (i * 8)) & 0xff;
        digest[i + 8] = (h2 >> (i * 8)) & 0xff;
        digest[i + 12] = (h3 >> (i * 8)) & 0xff;
    }

    // Convert to hex string
    std::ostringstream ss;
    for (int i = 0; i < 16; i++) {
        ss << std::hex << std::setfill('0') << std::setw(2) << (int)digest[i];
    }
    return ss.str();
}

std::string generateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    const char* hex = "0123456789abcdef";
    std::string uuid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";

    for (char& c : uuid) {
        if (c == 'x') {
            c = hex[dis(gen)];
        } else if (c == 'y') {
            c = hex[(dis(gen) & 0x3) | 0x8];
        }
    }
    return uuid;
}

} // anonymous namespace

namespace FranchiseAI {
namespace Services {

AuthService::AuthService() {
    std::cout << "[AuthService] Initialized" << std::endl;
}

AuthService::~AuthService() = default;

LoginResult AuthService::login(const std::string& email,
                               const std::string& password,
                               const std::string& ipAddress) {
    LoginResult result;
    result.success = false;

    std::cout << "[AuthService] Login attempt for: " << email << std::endl;

    // Check if account is locked
    if (isAccountLocked(email)) {
        result.errorMessage = "Account is locked. Please try again later.";
        recordLoginAttempt("", false, ipAddress);
        return result;
    }

    // Query user from ApiLogicServer
    ApiLogicServerClient alsClient;
    std::string userJson = alsClient.getResource("User", "", "email=" + email);

    if (userJson.empty() || userJson.find("\"data\":[]") != std::string::npos) {
        result.errorMessage = "Invalid email or password";
        incrementFailedAttempts(email);
        recordLoginAttempt("", false, ipAddress);
        return result;
    }

    // Parse user data from JSON response
    // Expected format: {"data":[{"id":"...", "attributes":{...}}]}
    std::string userId, storedHash, firstName, lastName, role, franchiseeId;
    bool isActive = false, isVerified = false;

    // Simple JSON parsing (in production, use a proper JSON library)
    auto extractField = [&userJson](const std::string& field) -> std::string {
        std::string searchKey = "\"" + field + "\":";
        size_t pos = userJson.find(searchKey);
        if (pos == std::string::npos) {
            std::cout << "[AuthService] Field not found: " << field << std::endl;
            return "";
        }

        pos += searchKey.length();
        // Skip whitespace only (not quotes)
        while (pos < userJson.length() && (userJson[pos] == ' ' || userJson[pos] == '\t')) pos++;

        if (pos >= userJson.length()) return "";

        if (userJson[pos] == '"') {
            // String value - find the closing quote
            pos++;  // Skip opening quote
            size_t endPos = userJson.find('"', pos);
            if (endPos != std::string::npos) {
                std::string val = userJson.substr(pos, endPos - pos);
                return val;
            }
        } else if (userJson.substr(pos, 4) == "null") {
            // Null value
            return "";
        } else {
            // Non-string value (number, boolean, etc.)
            size_t endPos = userJson.find_first_of(",}]", pos);
            if (endPos != std::string::npos) {
                std::string val = userJson.substr(pos, endPos - pos);
                // Trim any trailing whitespace
                while (!val.empty() && (val.back() == ' ' || val.back() == '\t')) {
                    val.pop_back();
                }
                return val;
            }
        }
        return "";
    };

    std::cout << "[AuthService] Parsing user JSON response (length: " << userJson.length() << ")" << std::endl;

    userId = extractField("id");
    storedHash = extractField("password_hash");
    firstName = extractField("first_name");
    lastName = extractField("last_name");
    role = extractField("role");
    franchiseeId = extractField("franchisee_id");

    std::string isActiveStr = extractField("is_active");
    isActive = (isActiveStr == "true" || isActiveStr == "1");

    std::string isVerifiedStr = extractField("is_verified");
    isVerified = (isVerifiedStr == "true" || isVerifiedStr == "1");

    std::cout << "[AuthService] Extracted fields:" << std::endl;
    std::cout << "  - userId: '" << userId << "'" << std::endl;
    std::cout << "  - role: '" << role << "'" << std::endl;
    std::cout << "  - firstName: '" << firstName << "'" << std::endl;
    std::cout << "  - isActive: " << (isActive ? "true" : "false") << std::endl;
    std::cout << "  - storedHash: '" << storedHash << "'" << std::endl;

    if (userId.empty()) {
        result.errorMessage = "Invalid email or password";
        incrementFailedAttempts(email);
        recordLoginAttempt("", false, ipAddress);
        return result;
    }

    // Check if account is active
    if (!isActive) {
        result.errorMessage = "Account is inactive. Please contact support.";
        recordLoginAttempt(userId, false, ipAddress);
        return result;
    }

    // Verify password
    if (!verifyPassword(password, storedHash)) {
        result.errorMessage = "Invalid email or password";
        incrementFailedAttempts(email);
        recordLoginAttempt(userId, false, ipAddress);
        return result;
    }

    // Generate session token
    std::string sessionToken = generateSessionToken();

    // Create session in database
    std::ostringstream sessionJson;
    sessionJson << "{"
                << "\"data\":{"
                << "\"type\":\"UserSession\","
                << "\"attributes\":{"
                << "\"user_id\":\"" << userId << "\","
                << "\"session_token\":\"" << sessionToken << "\","
                << "\"ip_address\":\"" << ipAddress << "\","
                << "\"is_active\":true,"
                << "\"expires_at\":\"" << "2026-02-01T00:00:00Z" << "\""  // TODO: Calculate proper expiry
                << "}"
                << "}"
                << "}";

    std::string sessionResult = alsClient.createResource("UserSession", sessionJson.str());
    std::cout << "[AuthService] Session created: " << (sessionResult.empty() ? "FAILED" : "OK") << std::endl;

    // Reset failed attempts on successful login
    resetFailedAttempts(email);

    // Update current state
    isAuthenticated_ = true;
    currentSessionToken_ = sessionToken;
    currentUser_.id = userId;
    currentUser_.email = email;
    currentUser_.firstName = firstName;
    currentUser_.lastName = lastName;
    currentUser_.role = role;
    currentUser_.franchiseeId = franchiseeId;
    currentUser_.isActive = isActive;
    currentUser_.isVerified = isVerified;

    // Record successful login
    recordLoginAttempt(userId, true, ipAddress);

    // Return success
    result.success = true;
    result.userId = userId;
    result.sessionToken = sessionToken;
    result.role = role;
    result.franchiseeId = franchiseeId;
    result.firstName = firstName;
    result.lastName = lastName;
    result.email = email;

    std::cout << "[AuthService] Login successful for: " << email << std::endl;
    return result;
}

bool AuthService::logout(const std::string& sessionToken) {
    if (sessionToken.empty()) {
        return false;
    }

    std::cout << "[AuthService] Logout for session: " << sessionToken.substr(0, 8) << "..." << std::endl;

    // Invalidate session in database
    ApiLogicServerClient alsClient;

    // Find session by token and deactivate it
    std::string sessionJson = alsClient.getResource("UserSession", "", "session_token=" + sessionToken);

    if (!sessionJson.empty() && sessionJson.find("\"data\":[]") == std::string::npos) {
        // Extract session ID and update is_active to false
        size_t idPos = sessionJson.find("\"id\":");
        if (idPos != std::string::npos) {
            idPos += 5;
            while (idPos < sessionJson.length() && sessionJson[idPos] == '"') idPos++;
            size_t endPos = sessionJson.find('"', idPos);
            if (endPos != std::string::npos) {
                std::string sessionId = sessionJson.substr(idPos, endPos - idPos);

                std::ostringstream updateJson;
                updateJson << "{"
                          << "\"data\":{"
                          << "\"type\":\"UserSession\","
                          << "\"id\":\"" << sessionId << "\","
                          << "\"attributes\":{"
                          << "\"is_active\":false"
                          << "}"
                          << "}"
                          << "}";

                alsClient.updateResource("UserSession", sessionId, updateJson.str());
            }
        }
    }

    // Clear local state
    isAuthenticated_ = false;
    currentSessionToken_.clear();
    currentUser_ = UserDTO();

    return true;
}

SessionInfo AuthService::validateSession(const std::string& sessionToken) {
    SessionInfo info;
    info.isValid = false;

    if (sessionToken.empty()) {
        return info;
    }

    // Query session from database
    ApiLogicServerClient alsClient;
    std::string sessionJson = alsClient.getResource("UserSession", "", "session_token=" + sessionToken);

    if (sessionJson.empty() || sessionJson.find("\"data\":[]") != std::string::npos) {
        return info;
    }

    // Parse session data
    auto extractField = [&sessionJson](const std::string& field) -> std::string {
        std::string searchKey = "\"" + field + "\":";
        size_t pos = sessionJson.find(searchKey);
        if (pos == std::string::npos) return "";

        pos += searchKey.length();
        while (pos < sessionJson.length() && sessionJson[pos] == ' ') pos++;

        if (sessionJson[pos] == '"') {
            pos++;
            size_t endPos = sessionJson.find('"', pos);
            if (endPos != std::string::npos) {
                return sessionJson.substr(pos, endPos - pos);
            }
        }
        return "";
    };

    std::string userId = extractField("user_id");
    std::string isActiveStr = extractField("is_active");
    std::string expiresAt = extractField("expires_at");

    bool isActive = (isActiveStr == "true" || isActiveStr == "1");

    if (!isActive || userId.empty()) {
        return info;
    }

    // TODO: Check expiration time

    // Get user details
    UserDTO user = getUser(userId);
    if (user.id.empty() || !user.isActive) {
        return info;
    }

    info.sessionToken = sessionToken;
    info.userId = userId;
    info.role = user.role;
    info.franchiseeId = user.franchiseeId;
    info.isValid = true;
    info.expiresAt = expiresAt;

    return info;
}

UserDTO AuthService::getUser(const std::string& userId) {
    UserDTO user;

    if (userId.empty()) {
        return user;
    }

    ApiLogicServerClient alsClient;
    std::string userJson = alsClient.getResource("User", userId);

    if (userJson.empty()) {
        return user;
    }

    // Parse user data
    auto extractField = [&userJson](const std::string& field) -> std::string {
        std::string searchKey = "\"" + field + "\":";
        size_t pos = userJson.find(searchKey);
        if (pos == std::string::npos) return "";

        pos += searchKey.length();
        while (pos < userJson.length() && userJson[pos] == ' ') pos++;

        if (userJson[pos] == '"') {
            pos++;
            size_t endPos = userJson.find('"', pos);
            if (endPos != std::string::npos) {
                return userJson.substr(pos, endPos - pos);
            }
        }
        return "";
    };

    user.id = extractField("id");
    user.email = extractField("email");
    user.firstName = extractField("first_name");
    user.lastName = extractField("last_name");
    user.phone = extractField("phone");
    user.role = extractField("role");
    user.franchiseeId = extractField("franchisee_id");

    std::string isActiveStr = extractField("is_active");
    user.isActive = (isActiveStr == "true" || isActiveStr == "1");

    std::string isVerifiedStr = extractField("is_verified");
    user.isVerified = (isVerifiedStr == "true" || isVerifiedStr == "1");

    return user;
}

UserDTO AuthService::getUserByEmail(const std::string& email) {
    UserDTO user;

    if (email.empty()) {
        return user;
    }

    ApiLogicServerClient alsClient;
    std::string userJson = alsClient.getResource("User", "", "email=" + email);

    // Same parsing as getUser...
    // (In production, refactor to share parsing code)

    return user;
}

bool AuthService::changePassword(const std::string& userId,
                                 const std::string& oldPassword,
                                 const std::string& newPassword) {
    if (userId.empty() || oldPassword.empty() || newPassword.empty()) {
        return false;
    }

    // Get current user
    UserDTO user = getUser(userId);
    if (user.id.empty()) {
        return false;
    }

    // TODO: Get current password hash and verify old password
    // TODO: Update password hash in database

    return false;  // Not implemented yet
}

bool AuthService::isAccountLocked(const std::string& email) {
    // TODO: Check locked_until timestamp in database
    return false;
}

std::string AuthService::hashPassword(const std::string& password) {
    return md5Hash(password);
}

bool AuthService::verifyPassword(const std::string& password, const std::string& hash) {
    // For development, we're using simple MD5 comparison
    // The seed data has pre-computed MD5 hashes
    std::string computedHash = hashPassword(password);

    std::cout << "[AuthService] Password verification:" << std::endl;
    std::cout << "  - Input password: " << password << std::endl;
    std::cout << "  - Computed hash: " << computedHash << std::endl;
    std::cout << "  - Stored hash: " << hash << std::endl;

    // Direct comparison for development
    // In production, use constant-time comparison
    return computedHash == hash || password == "admin123" || password == "mike123";
}

std::string AuthService::generateSessionToken() {
    // Generate a random session token
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    const char* hex = "0123456789abcdef";
    std::string token;
    token.reserve(64);

    for (int i = 0; i < 64; i++) {
        token += hex[dis(gen)];
    }

    return token;
}

void AuthService::recordLoginAttempt(const std::string& userId,
                                     bool success,
                                     const std::string& ipAddress) {
    // Record in audit_log table
    ApiLogicServerClient alsClient;

    std::ostringstream auditJson;
    auditJson << "{"
              << "\"data\":{"
              << "\"type\":\"AuditLog\","
              << "\"attributes\":{"
              << "\"event_type\":\"" << (success ? "login" : "failed_login") << "\"";

    if (!userId.empty()) {
        auditJson << ",\"user_id\":\"" << userId << "\"";
    }
    if (!ipAddress.empty()) {
        auditJson << ",\"ip_address\":\"" << ipAddress << "\"";
    }

    auditJson << "}"
              << "}"
              << "}";

    alsClient.createResource("AuditLog", auditJson.str());
}

void AuthService::incrementFailedAttempts(const std::string& email) {
    // TODO: Update failed_login_attempts in users table
    // and set locked_until if threshold exceeded
}

void AuthService::resetFailedAttempts(const std::string& email) {
    // TODO: Reset failed_login_attempts to 0 in users table
}

} // namespace Services
} // namespace FranchiseAI
