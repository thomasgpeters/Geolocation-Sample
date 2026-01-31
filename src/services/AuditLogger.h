#ifndef AUDIT_LOGGER_H
#define AUDIT_LOGGER_H

#include <string>
#include <map>
#include <memory>
#include "ApiLogicServerClient.h"

namespace FranchiseAI {
namespace Services {

/**
 * @brief Event types for audit logging
 */
namespace AuditEventType {
    // Authentication events
    constexpr const char* LOGIN = "login";
    constexpr const char* LOGOUT = "logout";
    constexpr const char* FAILED_LOGIN = "failed_login";
    constexpr const char* PASSWORD_CHANGE = "password_change";
    constexpr const char* PASSWORD_RESET = "password_reset";

    // User management events
    constexpr const char* USER_CREATE = "user_create";
    constexpr const char* USER_UPDATE = "user_update";
    constexpr const char* USER_DELETE = "user_delete";

    // Franchisee events
    constexpr const char* FRANCHISEE_CREATE = "franchisee_create";
    constexpr const char* FRANCHISEE_UPDATE = "franchisee_update";
    constexpr const char* FRANCHISEE_DELETE = "franchisee_delete";

    // Store events
    constexpr const char* STORE_CREATE = "store_create";
    constexpr const char* STORE_UPDATE = "store_update";
    constexpr const char* STORE_DELETE = "store_delete";

    // Settings events
    constexpr const char* SETTINGS_CHANGE = "settings_change";
    constexpr const char* CONFIG_UPDATE = "config_update";

    // Prospect events
    constexpr const char* PROSPECT_CREATE = "prospect_create";
    constexpr const char* PROSPECT_UPDATE = "prospect_update";
    constexpr const char* PROSPECT_DELETE = "prospect_delete";
    constexpr const char* PROSPECT_STATUS_CHANGE = "prospect_status_change";

    // Search events
    constexpr const char* SEARCH_PERFORMED = "search_performed";
    constexpr const char* EXPORT_DATA = "export_data";

    // System events
    constexpr const char* SESSION_EXPIRED = "session_expired";
    constexpr const char* API_ERROR = "api_error";
}

/**
 * @brief Audit Logger service for recording application events
 *
 * Provides a simple interface to log user actions and system events
 * to the audit_log table for compliance and security monitoring.
 */
class AuditLogger {
public:
    /**
     * @brief Get singleton instance
     */
    static AuditLogger& instance();

    /**
     * @brief Log an audit event
     * @param userId The user who performed the action (empty for anonymous)
     * @param eventType The type of event (use AuditEventType constants)
     * @param details Additional details about the event
     * @param ipAddress The client's IP address
     */
    void log(const std::string& userId,
             const std::string& eventType,
             const std::map<std::string, std::string>& details = {},
             const std::string& ipAddress = "");

    /**
     * @brief Log an audit event with string details
     * @param userId The user who performed the action
     * @param eventType The type of event
     * @param detailsJson JSON string of event details
     * @param ipAddress The client's IP address
     */
    void log(const std::string& userId,
             const std::string& eventType,
             const std::string& detailsJson,
             const std::string& ipAddress = "");

    /**
     * @brief Quick log method for common events
     */
    void logLogin(const std::string& userId, const std::string& email, const std::string& ipAddress);
    void logLogout(const std::string& userId, const std::string& ipAddress);
    void logFailedLogin(const std::string& email, const std::string& reason, const std::string& ipAddress);
    void logSettingsChange(const std::string& userId, const std::string& setting, const std::string& oldValue, const std::string& newValue);
    void logFranchiseeUpdate(const std::string& userId, const std::string& franchiseeId, const std::string& changes);
    void logStoreUpdate(const std::string& userId, const std::string& storeId, const std::string& changes);

private:
    AuditLogger();
    ~AuditLogger() = default;

    // Prevent copying
    AuditLogger(const AuditLogger&) = delete;
    AuditLogger& operator=(const AuditLogger&) = delete;

    /**
     * @brief Convert map to JSON string
     */
    std::string mapToJson(const std::map<std::string, std::string>& data);

    std::unique_ptr<ApiLogicServerClient> alsClient_;
};

} // namespace Services
} // namespace FranchiseAI

#endif // AUDIT_LOGGER_H
