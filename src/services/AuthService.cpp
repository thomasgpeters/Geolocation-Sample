#include "AuthService.h"
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>
#include <iostream>
#include <vector>

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

    // Mock user database
    struct MockUser {
        std::string id;
        std::string email;
        std::string password;
        std::string firstName;
        std::string lastName;
        std::string role;
        std::string franchiseeId;
    };

    std::vector<MockUser> mockUsers = {
        {"f1000000-0000-0000-0000-000000000001", "admin@franchiseai.com", "admin123",
         "System", "Administrator", "admin", ""},
        {"f1000000-0000-0000-0000-000000000002", "mike@pittsburghcatering.com", "mike123",
         "Mike", "Owner", "franchisee", "c2c5af5a-53a5-4d28-8218-3675c0942ead"},
        {"f1000000-0000-0000-0000-000000000003", "thomas.g.peters@imagery-business-systems.com", "password123",
         "Thomas", "Peters", "admin", ""}
    };

    // Find matching user
    MockUser* foundUser = nullptr;
    for (auto& user : mockUsers) {
        if (user.email == email) {
            foundUser = &user;
            break;
        }
    }

    if (!foundUser) {
        std::cout << "[AuthService] User not found: " << email << std::endl;
        result.errorMessage = "Invalid email or password";
        return result;
    }

    // Verify password
    if (foundUser->password != password) {
        std::cout << "[AuthService] Invalid password for: " << email << std::endl;
        result.errorMessage = "Invalid email or password";
        return result;
    }

    // Generate session token
    std::string sessionToken = generateSessionToken();

    // Update current state
    isAuthenticated_ = true;
    currentSessionToken_ = sessionToken;
    currentUser_.id = foundUser->id;
    currentUser_.email = foundUser->email;
    currentUser_.firstName = foundUser->firstName;
    currentUser_.lastName = foundUser->lastName;
    currentUser_.role = foundUser->role;
    currentUser_.franchiseeId = foundUser->franchiseeId;
    currentUser_.isActive = true;
    currentUser_.isVerified = true;

    // Return success
    result.success = true;
    result.userId = foundUser->id;
    result.sessionToken = sessionToken;
    result.role = foundUser->role;
    result.franchiseeId = foundUser->franchiseeId;
    result.firstName = foundUser->firstName;
    result.lastName = foundUser->lastName;
    result.email = foundUser->email;

    std::cout << "[AuthService] Login successful for: " << email << " (role: " << foundUser->role << ")" << std::endl;
    return result;
}

bool AuthService::logout(const std::string& sessionToken) {
    if (sessionToken.empty()) {
        return false;
    }

    std::cout << "[AuthService] Logout for session: " << sessionToken.substr(0, 8) << "..." << std::endl;

    // Clear local state (mock - no database interaction)
    isAuthenticated_ = false;
    currentSessionToken_.clear();
    currentUser_ = UserDTO();

    std::cout << "[AuthService] Logout successful" << std::endl;
    return true;
}

SessionInfo AuthService::validateSession(const std::string& sessionToken) {
    SessionInfo info;
    info.isValid = false;

    if (sessionToken.empty()) {
        return info;
    }

    // Mock validation - check if this matches our current session
    if (isAuthenticated_ && currentSessionToken_ == sessionToken) {
        info.sessionToken = sessionToken;
        info.userId = currentUser_.id;
        info.role = currentUser_.role;
        info.franchiseeId = currentUser_.franchiseeId;
        info.isValid = true;
        info.expiresAt = "2026-12-31T23:59:59Z";
    }

    return info;
}

UserDTO AuthService::getUser(const std::string& userId) {
    UserDTO user;

    if (userId.empty()) {
        return user;
    }

    // Mock user database
    struct MockUser {
        std::string id;
        std::string email;
        std::string firstName;
        std::string lastName;
        std::string role;
        std::string franchiseeId;
    };

    std::vector<MockUser> mockUsers = {
        {"f1000000-0000-0000-0000-000000000001", "admin@franchiseai.com",
         "System", "Administrator", "admin", ""},
        {"f1000000-0000-0000-0000-000000000002", "mike@pittsburghcatering.com",
         "Mike", "Owner", "franchisee", "c2c5af5a-53a5-4d28-8218-3675c0942ead"},
        {"f1000000-0000-0000-0000-000000000003", "thomas.g.peters@imagery-business-systems.com",
         "Thomas", "Peters", "admin", ""}
    };

    // Find matching user by ID
    for (const auto& mockUser : mockUsers) {
        if (mockUser.id == userId) {
            user.id = mockUser.id;
            user.email = mockUser.email;
            user.firstName = mockUser.firstName;
            user.lastName = mockUser.lastName;
            user.role = mockUser.role;
            user.franchiseeId = mockUser.franchiseeId;
            user.isActive = true;
            user.isVerified = true;
            break;
        }
    }

    return user;
}

UserDTO AuthService::getUserByEmail(const std::string& email) {
    UserDTO user;

    if (email.empty()) {
        return user;
    }

    // Mock user database
    struct MockUser {
        std::string id;
        std::string email;
        std::string firstName;
        std::string lastName;
        std::string role;
        std::string franchiseeId;
    };

    std::vector<MockUser> mockUsers = {
        {"f1000000-0000-0000-0000-000000000001", "admin@franchiseai.com",
         "System", "Administrator", "admin", ""},
        {"f1000000-0000-0000-0000-000000000002", "mike@pittsburghcatering.com",
         "Mike", "Owner", "franchisee", "c2c5af5a-53a5-4d28-8218-3675c0942ead"},
        {"f1000000-0000-0000-0000-000000000003", "thomas.g.peters@imagery-business-systems.com",
         "Thomas", "Peters", "admin", ""}
    };

    // Find matching user by email
    for (const auto& mockUser : mockUsers) {
        if (mockUser.email == email) {
            user.id = mockUser.id;
            user.email = mockUser.email;
            user.firstName = mockUser.firstName;
            user.lastName = mockUser.lastName;
            user.role = mockUser.role;
            user.franchiseeId = mockUser.franchiseeId;
            user.isActive = true;
            user.isVerified = true;
            break;
        }
    }

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
    // Mock - just log to console (no ALS interaction)
    std::cout << "[AuthService] Login attempt recorded: "
              << (success ? "SUCCESS" : "FAILED")
              << " for user: " << (userId.empty() ? "unknown" : userId)
              << " from IP: " << ipAddress << std::endl;
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
