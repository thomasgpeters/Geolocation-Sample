#include "AuditLogger.h"
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>

namespace FranchiseAI {
namespace Services {

AuditLogger& AuditLogger::instance() {
    static AuditLogger instance;
    return instance;
}

AuditLogger::AuditLogger() {
    alsClient_ = std::make_unique<ApiLogicServerClient>();
}

void AuditLogger::log(const std::string& userId,
                      const std::string& eventType,
                      const std::map<std::string, std::string>& details,
                      const std::string& ipAddress) {
    log(userId, eventType, mapToJson(details), ipAddress);
}

void AuditLogger::log(const std::string& userId,
                      const std::string& eventType,
                      const std::string& detailsJson,
                      const std::string& ipAddress) {
    std::cout << "[AuditLogger] Logging event: " << eventType
              << " for user: " << (userId.empty() ? "anonymous" : userId) << std::endl;

    // Validate UUID format before including in request
    std::string trimmedUserId = userId;
    size_t start = trimmedUserId.find_first_not_of(" \t\n\r");
    size_t end = trimmedUserId.find_last_not_of(" \t\n\r");
    if (start != std::string::npos && end != std::string::npos) {
        trimmedUserId = trimmedUserId.substr(start, end - start + 1);
    } else {
        trimmedUserId = "";
    }
    bool isValidUuid = trimmedUserId.length() == 36 &&
                       trimmedUserId[8] == '-' && trimmedUserId[13] == '-' &&
                       trimmedUserId[18] == '-' && trimmedUserId[23] == '-';

    // Build JSON:API request body
    std::ostringstream json;
    json << "{\"data\":{\"type\":\"AuditLog\",\"attributes\":{";
    json << "\"event_type\":\"" << eventType << "\"";

    if (isValidUuid) {
        json << ",\"user_id\":\"" << trimmedUserId << "\"";
    }

    if (!detailsJson.empty() && detailsJson != "{}") {
        // Escape quotes in details JSON for embedding
        std::string escapedDetails = detailsJson;
        // For JSON:API, event_details should be a JSON object, not a string
        // We'll pass it directly
        json << ",\"event_details\":" << detailsJson;
    }

    if (!ipAddress.empty()) {
        json << ",\"ip_address\":\"" << ipAddress << "\"";
    }

    json << "}}}";

    // Send to API
    std::string response = alsClient_->createResource("AuditLog", json.str());

    if (response.empty()) {
        std::cerr << "[AuditLogger] Failed to create audit log entry" << std::endl;
    }
}

void AuditLogger::logLogin(const std::string& userId, const std::string& email, const std::string& ipAddress) {
    std::map<std::string, std::string> details;
    details["email"] = email;
    details["action"] = "User logged in successfully";
    log(userId, AuditEventType::LOGIN, details, ipAddress);
}

void AuditLogger::logLogout(const std::string& userId, const std::string& ipAddress) {
    std::map<std::string, std::string> details;
    details["action"] = "User logged out";
    log(userId, AuditEventType::LOGOUT, details, ipAddress);
}

void AuditLogger::logFailedLogin(const std::string& email, const std::string& reason, const std::string& ipAddress) {
    std::map<std::string, std::string> details;
    details["email"] = email;
    details["reason"] = reason;
    log("", AuditEventType::FAILED_LOGIN, details, ipAddress);
}

void AuditLogger::logSettingsChange(const std::string& userId,
                                    const std::string& setting,
                                    const std::string& oldValue,
                                    const std::string& newValue) {
    std::map<std::string, std::string> details;
    details["setting"] = setting;
    details["old_value"] = oldValue;
    details["new_value"] = newValue;
    log(userId, AuditEventType::SETTINGS_CHANGE, details, "");
}

void AuditLogger::logFranchiseeUpdate(const std::string& userId,
                                       const std::string& franchiseeId,
                                       const std::string& changes) {
    std::map<std::string, std::string> details;
    details["franchisee_id"] = franchiseeId;
    details["changes"] = changes;
    log(userId, AuditEventType::FRANCHISEE_UPDATE, details, "");
}

void AuditLogger::logStoreUpdate(const std::string& userId,
                                  const std::string& storeId,
                                  const std::string& changes) {
    std::map<std::string, std::string> details;
    details["store_id"] = storeId;
    details["changes"] = changes;
    log(userId, AuditEventType::STORE_UPDATE, details, "");
}

std::string AuditLogger::mapToJson(const std::map<std::string, std::string>& data) {
    if (data.empty()) {
        return "{}";
    }

    std::ostringstream json;
    json << "{";

    bool first = true;
    for (const auto& [key, value] : data) {
        if (!first) json << ",";
        first = false;

        // Escape special characters in value
        std::string escapedValue;
        for (char c : value) {
            switch (c) {
                case '"': escapedValue += "\\\""; break;
                case '\\': escapedValue += "\\\\"; break;
                case '\n': escapedValue += "\\n"; break;
                case '\r': escapedValue += "\\r"; break;
                case '\t': escapedValue += "\\t"; break;
                default: escapedValue += c;
            }
        }

        json << "\"" << key << "\":\"" << escapedValue << "\"";
    }

    json << "}";
    return json.str();
}

} // namespace Services
} // namespace FranchiseAI
