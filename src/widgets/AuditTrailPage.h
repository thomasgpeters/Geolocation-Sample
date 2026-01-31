#ifndef AUDIT_TRAIL_PAGE_H
#define AUDIT_TRAIL_PAGE_H

#include <Wt/WContainerWidget.h>
#include <Wt/WTable.h>
#include <Wt/WComboBox.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WDateEdit.h>
#include <vector>
#include <string>
#include "../services/ApiLogicServerClient.h"

namespace FranchiseAI {
namespace Widgets {

/**
 * @brief Audit log entry for display
 */
struct AuditLogEntry {
    std::string id;
    std::string userId;
    std::string userEmail;
    std::string userName;
    std::string eventType;
    std::string eventDetails;
    std::string ipAddress;
    std::string userAgent;
    std::string createdAt;
};

/**
 * @brief Audit Trail page for administrators
 *
 * Displays a filterable, paginated list of audit log entries
 * showing who did what and when in the application.
 */
class AuditTrailPage : public Wt::WContainerWidget {
public:
    AuditTrailPage();
    ~AuditTrailPage() override = default;

    /**
     * @brief Refresh the audit log display
     */
    void refresh() override;

private:
    void setupUI();
    void setupFilters();
    void setupTable();
    void loadAuditLogs();
    void applyFilters();
    void clearFilters();
    void onPageChange(int page);

    // Parse audit log entries from API response
    std::vector<AuditLogEntry> parseAuditLogs(const std::string& jsonResponse);

    // Format timestamp for display
    std::string formatTimestamp(const std::string& isoTimestamp);

    // Get badge class for event type
    std::string getEventTypeBadgeClass(const std::string& eventType);

    // UI Components - Filters
    Wt::WComboBox* eventTypeFilter_ = nullptr;
    Wt::WLineEdit* userFilter_ = nullptr;
    Wt::WLineEdit* dateFromFilter_ = nullptr;
    Wt::WLineEdit* dateToFilter_ = nullptr;
    Wt::WPushButton* applyFilterBtn_ = nullptr;
    Wt::WPushButton* clearFilterBtn_ = nullptr;

    // UI Components - Table
    Wt::WTable* auditTable_ = nullptr;
    Wt::WContainerWidget* tableContainer_ = nullptr;
    Wt::WText* statusText_ = nullptr;

    // UI Components - Pagination
    Wt::WContainerWidget* paginationContainer_ = nullptr;
    int currentPage_ = 1;
    int totalPages_ = 1;
    int pageSize_ = 25;

    // Data
    std::vector<AuditLogEntry> auditLogs_;
    std::unique_ptr<Services::ApiLogicServerClient> alsClient_;
};

} // namespace Widgets
} // namespace FranchiseAI

#endif // AUDIT_TRAIL_PAGE_H
