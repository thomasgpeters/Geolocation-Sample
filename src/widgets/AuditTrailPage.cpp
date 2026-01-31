#include "AuditTrailPage.h"
#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WLabel.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>

namespace FranchiseAI {
namespace Widgets {

AuditTrailPage::AuditTrailPage() {
    alsClient_ = std::make_unique<Services::ApiLogicServerClient>();
    setupUI();
    loadAuditLogs();
}

void AuditTrailPage::setupUI() {
    addStyleClass("audit-trail-page");

    // Add page styles
    auto* app = Wt::WApplication::instance();
    app->styleSheet().addRule(".audit-trail-page", "padding: 20px;");
    app->styleSheet().addRule(".audit-header", "margin-bottom: 24px;");
    app->styleSheet().addRule(".audit-title", "font-size: 24px; font-weight: 600; color: #1f2937; margin-bottom: 8px;");
    app->styleSheet().addRule(".audit-subtitle", "font-size: 14px; color: #6b7280;");
    app->styleSheet().addRule(".audit-filters", "background: white; border-radius: 8px; padding: 16px; margin-bottom: 20px; box-shadow: 0 1px 3px rgba(0,0,0,0.1);");
    app->styleSheet().addRule(".filter-row", "display: flex; gap: 16px; flex-wrap: wrap; align-items: flex-end;");
    app->styleSheet().addRule(".filter-group", "display: flex; flex-direction: column; gap: 4px;");
    app->styleSheet().addRule(".filter-group label", "font-size: 12px; font-weight: 500; color: #374151;");
    app->styleSheet().addRule(".filter-group input, .filter-group select", "padding: 8px 12px; border: 1px solid #d1d5db; border-radius: 6px; font-size: 14px;");
    app->styleSheet().addRule(".filter-buttons", "display: flex; gap: 8px;");
    app->styleSheet().addRule(".btn-filter", "padding: 8px 16px; border-radius: 6px; font-size: 14px; cursor: pointer;");
    app->styleSheet().addRule(".btn-apply", "background: #2563eb; color: white; border: none;");
    app->styleSheet().addRule(".btn-apply:hover", "background: #1d4ed8;");
    app->styleSheet().addRule(".btn-clear", "background: white; color: #374151; border: 1px solid #d1d5db;");
    app->styleSheet().addRule(".btn-clear:hover", "background: #f3f4f6;");
    app->styleSheet().addRule(".audit-table-container", "background: white; border-radius: 8px; box-shadow: 0 1px 3px rgba(0,0,0,0.1); overflow: hidden;");
    app->styleSheet().addRule(".audit-table", "width: 100%; border-collapse: collapse;");
    app->styleSheet().addRule(".audit-table th", "background: #f9fafb; padding: 12px 16px; text-align: left; font-size: 12px; font-weight: 600; color: #6b7280; text-transform: uppercase; border-bottom: 1px solid #e5e7eb;");
    app->styleSheet().addRule(".audit-table td", "padding: 12px 16px; border-bottom: 1px solid #e5e7eb; font-size: 14px; color: #374151;");
    app->styleSheet().addRule(".audit-table tr:hover", "background: #f9fafb;");
    app->styleSheet().addRule(".event-badge", "display: inline-block; padding: 4px 8px; border-radius: 4px; font-size: 12px; font-weight: 500;");
    app->styleSheet().addRule(".badge-login", "background: #d1fae5; color: #065f46;");
    app->styleSheet().addRule(".badge-logout", "background: #e0e7ff; color: #3730a3;");
    app->styleSheet().addRule(".badge-failed", "background: #fee2e2; color: #991b1b;");
    app->styleSheet().addRule(".badge-settings", "background: #fef3c7; color: #92400e;");
    app->styleSheet().addRule(".badge-create", "background: #cffafe; color: #0e7490;");
    app->styleSheet().addRule(".badge-update", "background: #f3e8ff; color: #7c3aed;");
    app->styleSheet().addRule(".badge-delete", "background: #fecaca; color: #dc2626;");
    app->styleSheet().addRule(".badge-default", "background: #e5e7eb; color: #374151;");
    app->styleSheet().addRule(".audit-pagination", "display: flex; justify-content: center; gap: 8px; padding: 16px;");
    app->styleSheet().addRule(".page-btn", "padding: 8px 12px; border: 1px solid #d1d5db; border-radius: 6px; background: white; cursor: pointer;");
    app->styleSheet().addRule(".page-btn:hover", "background: #f3f4f6;");
    app->styleSheet().addRule(".page-btn.active", "background: #2563eb; color: white; border-color: #2563eb;");
    app->styleSheet().addRule(".audit-status", "padding: 16px; text-align: center; color: #6b7280;");
    app->styleSheet().addRule(".user-info", "display: flex; flex-direction: column;");
    app->styleSheet().addRule(".user-name", "font-weight: 500;");
    app->styleSheet().addRule(".user-email", "font-size: 12px; color: #6b7280;");
    app->styleSheet().addRule(".ip-address", "font-family: monospace; font-size: 12px; color: #6b7280;");
    app->styleSheet().addRule(".timestamp", "font-size: 13px; color: #6b7280;");
    app->styleSheet().addRule(".details-cell", "max-width: 300px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap;");

    // Header - tagline only (title shown in navigation)
    auto* header = addWidget(std::make_unique<Wt::WContainerWidget>());
    header->addStyleClass("audit-header");

    auto* tagline = header->addWidget(std::make_unique<Wt::WText>("View all user activity and security events"));
    tagline->addStyleClass("page-tagline");

    // Filters
    setupFilters();

    // Table
    setupTable();
}

void AuditTrailPage::setupFilters() {
    auto* filtersContainer = addWidget(std::make_unique<Wt::WContainerWidget>());
    filtersContainer->addStyleClass("audit-filters");

    auto* filterRow = filtersContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    filterRow->addStyleClass("filter-row");

    // Event Type filter
    auto* eventTypeGroup = filterRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    eventTypeGroup->addStyleClass("filter-group");
    eventTypeGroup->addWidget(std::make_unique<Wt::WLabel>("Event Type"));
    eventTypeFilter_ = eventTypeGroup->addWidget(std::make_unique<Wt::WComboBox>());
    eventTypeFilter_->addItem("All Events");
    eventTypeFilter_->addItem("login");
    eventTypeFilter_->addItem("logout");
    eventTypeFilter_->addItem("failed_login");
    eventTypeFilter_->addItem("settings_change");
    eventTypeFilter_->addItem("franchisee_update");
    eventTypeFilter_->addItem("store_update");
    eventTypeFilter_->addItem("prospect_create");
    eventTypeFilter_->addItem("prospect_update");
    eventTypeFilter_->addItem("password_change");

    // User filter
    auto* userGroup = filterRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    userGroup->addStyleClass("filter-group");
    userGroup->addWidget(std::make_unique<Wt::WLabel>("User Email"));
    userFilter_ = userGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    userFilter_->setPlaceholderText("Filter by email...");

    // Date From filter
    auto* dateFromGroup = filterRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    dateFromGroup->addStyleClass("filter-group");
    dateFromGroup->addWidget(std::make_unique<Wt::WLabel>("From Date"));
    dateFromFilter_ = dateFromGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    dateFromFilter_->setPlaceholderText("YYYY-MM-DD");

    // Date To filter
    auto* dateToGroup = filterRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    dateToGroup->addStyleClass("filter-group");
    dateToGroup->addWidget(std::make_unique<Wt::WLabel>("To Date"));
    dateToFilter_ = dateToGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    dateToFilter_->setPlaceholderText("YYYY-MM-DD");

    // Buttons
    auto* buttonGroup = filterRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    buttonGroup->addStyleClass("filter-buttons");

    applyFilterBtn_ = buttonGroup->addWidget(std::make_unique<Wt::WPushButton>("Apply Filters"));
    applyFilterBtn_->addStyleClass("btn-filter btn-apply");
    applyFilterBtn_->clicked().connect(this, &AuditTrailPage::applyFilters);

    clearFilterBtn_ = buttonGroup->addWidget(std::make_unique<Wt::WPushButton>("Clear"));
    clearFilterBtn_->addStyleClass("btn-filter btn-clear");
    clearFilterBtn_->clicked().connect(this, &AuditTrailPage::clearFilters);
}

void AuditTrailPage::setupTable() {
    tableContainer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    tableContainer_->addStyleClass("audit-table-container");

    auditTable_ = tableContainer_->addWidget(std::make_unique<Wt::WTable>());
    auditTable_->addStyleClass("audit-table");

    // Header row
    auditTable_->setHeaderCount(1);
    auditTable_->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("Timestamp"));
    auditTable_->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("User"));
    auditTable_->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("Event"));
    auditTable_->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("Details"));
    auditTable_->elementAt(0, 4)->addWidget(std::make_unique<Wt::WText>("IP Address"));

    // Status text
    statusText_ = tableContainer_->addWidget(std::make_unique<Wt::WText>("Loading audit logs..."));
    statusText_->addStyleClass("audit-status");

    // Pagination
    paginationContainer_ = tableContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    paginationContainer_->addStyleClass("audit-pagination");
}

void AuditTrailPage::loadAuditLogs() {
    std::cout << "[AuditTrail] Loading audit logs..." << std::endl;

    // Build filter query
    std::string filter;
    if (eventTypeFilter_ && eventTypeFilter_->currentIndex() > 0) {
        filter = "event_type=" + eventTypeFilter_->currentText().toUTF8();
    }

    // Get audit logs from API
    std::string response = alsClient_->getResource("AuditLog", "", filter);

    if (response.empty()) {
        statusText_->setText("No audit logs found or unable to connect to server.");
        return;
    }

    auditLogs_ = parseAuditLogs(response);

    std::cout << "[AuditTrail] Loaded " << auditLogs_.size() << " entries" << std::endl;

    // Update table
    refresh();
}

void AuditTrailPage::refresh() {
    // Clear existing rows (except header)
    while (auditTable_->rowCount() > 1) {
        auditTable_->removeRow(1);
    }

    if (auditLogs_.empty()) {
        statusText_->setText("No audit log entries found.");
        statusText_->show();
        paginationContainer_->hide();
        return;
    }

    statusText_->hide();

    // Calculate pagination
    totalPages_ = (auditLogs_.size() + pageSize_ - 1) / pageSize_;
    int startIdx = (currentPage_ - 1) * pageSize_;
    int endIdx = std::min(startIdx + pageSize_, (int)auditLogs_.size());

    // Add rows for current page
    for (int i = startIdx; i < endIdx; i++) {
        const auto& entry = auditLogs_[i];
        int row = auditTable_->rowCount();

        // Timestamp
        auto* timestampCell = auditTable_->elementAt(row, 0);
        auto* timestampText = timestampCell->addWidget(std::make_unique<Wt::WText>(formatTimestamp(entry.createdAt)));
        timestampText->addStyleClass("timestamp");

        // User
        auto* userCell = auditTable_->elementAt(row, 1);
        auto* userContainer = userCell->addWidget(std::make_unique<Wt::WContainerWidget>());
        userContainer->addStyleClass("user-info");

        std::string displayName = entry.userName.empty() ? "Unknown" : entry.userName;
        auto* nameText = userContainer->addWidget(std::make_unique<Wt::WText>(displayName));
        nameText->addStyleClass("user-name");

        if (!entry.userEmail.empty()) {
            auto* emailText = userContainer->addWidget(std::make_unique<Wt::WText>(entry.userEmail));
            emailText->addStyleClass("user-email");
        }

        // Event Type
        auto* eventCell = auditTable_->elementAt(row, 2);
        auto* eventBadge = eventCell->addWidget(std::make_unique<Wt::WText>(entry.eventType));
        eventBadge->addStyleClass("event-badge " + getEventTypeBadgeClass(entry.eventType));

        // Details
        auto* detailsCell = auditTable_->elementAt(row, 3);
        detailsCell->addStyleClass("details-cell");
        std::string details = entry.eventDetails.empty() ? "-" : entry.eventDetails;
        // Truncate long details
        if (details.length() > 50) {
            details = details.substr(0, 47) + "...";
        }
        detailsCell->addWidget(std::make_unique<Wt::WText>(details));

        // IP Address
        auto* ipCell = auditTable_->elementAt(row, 4);
        std::string ip = entry.ipAddress.empty() ? "-" : entry.ipAddress;
        auto* ipText = ipCell->addWidget(std::make_unique<Wt::WText>(ip));
        ipText->addStyleClass("ip-address");
    }

    // Update pagination
    paginationContainer_->clear();
    paginationContainer_->show();

    if (totalPages_ > 1) {
        // Previous button
        if (currentPage_ > 1) {
            auto* prevBtn = paginationContainer_->addWidget(std::make_unique<Wt::WPushButton>("Previous"));
            prevBtn->addStyleClass("page-btn");
            prevBtn->clicked().connect([this] { onPageChange(currentPage_ - 1); });
        }

        // Page numbers
        int startPage = std::max(1, currentPage_ - 2);
        int endPage = std::min(totalPages_, currentPage_ + 2);

        for (int p = startPage; p <= endPage; p++) {
            auto* pageBtn = paginationContainer_->addWidget(std::make_unique<Wt::WPushButton>(std::to_string(p)));
            pageBtn->addStyleClass(p == currentPage_ ? "page-btn active" : "page-btn");
            int pageNum = p;
            pageBtn->clicked().connect([this, pageNum] { onPageChange(pageNum); });
        }

        // Next button
        if (currentPage_ < totalPages_) {
            auto* nextBtn = paginationContainer_->addWidget(std::make_unique<Wt::WPushButton>("Next"));
            nextBtn->addStyleClass("page-btn");
            nextBtn->clicked().connect([this] { onPageChange(currentPage_ + 1); });
        }
    }

    // Show count
    std::ostringstream countText;
    countText << "Showing " << (startIdx + 1) << "-" << endIdx << " of " << auditLogs_.size() << " entries";
    auto* countLabel = paginationContainer_->addWidget(std::make_unique<Wt::WText>(countText.str()));
    countLabel->setStyleClass("audit-status");
}

void AuditTrailPage::applyFilters() {
    currentPage_ = 1;
    loadAuditLogs();
}

void AuditTrailPage::clearFilters() {
    eventTypeFilter_->setCurrentIndex(0);
    userFilter_->setText("");
    dateFromFilter_->setText("");
    dateToFilter_->setText("");
    currentPage_ = 1;
    loadAuditLogs();
}

void AuditTrailPage::onPageChange(int page) {
    currentPage_ = page;
    refresh();
}

std::vector<AuditLogEntry> AuditTrailPage::parseAuditLogs(const std::string& jsonResponse) {
    std::vector<AuditLogEntry> entries;

    // Simple JSON parsing - look for audit log entries in the response
    // Format: {"data":[{"id":"...", "attributes":{...}}, ...]}

    size_t pos = 0;
    while ((pos = jsonResponse.find("\"id\":", pos)) != std::string::npos) {
        AuditLogEntry entry;

        // Extract ID
        size_t idStart = jsonResponse.find("\"", pos + 5) + 1;
        size_t idEnd = jsonResponse.find("\"", idStart);
        if (idStart != std::string::npos && idEnd != std::string::npos) {
            entry.id = jsonResponse.substr(idStart, idEnd - idStart);
        }

        // Helper lambda to extract field
        auto extractField = [&jsonResponse, pos](const std::string& fieldName) -> std::string {
            std::string searchKey = "\"" + fieldName + "\":";
            size_t fieldPos = jsonResponse.find(searchKey, pos);
            if (fieldPos == std::string::npos || fieldPos > pos + 2000) return "";

            size_t valueStart = fieldPos + searchKey.length();
            while (valueStart < jsonResponse.length() &&
                   (jsonResponse[valueStart] == ' ' || jsonResponse[valueStart] == '"')) {
                valueStart++;
            }

            // Check if it's a quoted string
            if (jsonResponse[valueStart - 1] == '"') {
                size_t valueEnd = jsonResponse.find("\"", valueStart);
                if (valueEnd != std::string::npos) {
                    return jsonResponse.substr(valueStart, valueEnd - valueStart);
                }
            } else {
                // Non-quoted value (null, number, boolean)
                size_t valueEnd = jsonResponse.find_first_of(",}]", valueStart);
                if (valueEnd != std::string::npos) {
                    std::string val = jsonResponse.substr(valueStart, valueEnd - valueStart);
                    if (val == "null") return "";
                    return val;
                }
            }
            return "";
        };

        entry.userId = extractField("user_id");
        entry.eventType = extractField("event_type");
        entry.ipAddress = extractField("ip_address");
        entry.userAgent = extractField("user_agent");
        entry.createdAt = extractField("created_at");

        // Parse event_details if present (it's a JSON object)
        size_t detailsPos = jsonResponse.find("\"event_details\":", pos);
        if (detailsPos != std::string::npos && detailsPos < pos + 2000) {
            size_t detailsStart = jsonResponse.find("{", detailsPos);
            if (detailsStart != std::string::npos) {
                // Find matching closing brace
                int braceCount = 1;
                size_t detailsEnd = detailsStart + 1;
                while (braceCount > 0 && detailsEnd < jsonResponse.length()) {
                    if (jsonResponse[detailsEnd] == '{') braceCount++;
                    else if (jsonResponse[detailsEnd] == '}') braceCount--;
                    detailsEnd++;
                }
                entry.eventDetails = jsonResponse.substr(detailsStart, detailsEnd - detailsStart);
            }
        }

        // Look up user info (in a real app, this would be joined in the query)
        // For now, we'll just show the user ID
        if (!entry.userId.empty()) {
            entry.userName = "User";
            entry.userEmail = entry.userId.substr(0, 8) + "...";
        }

        if (!entry.id.empty()) {
            entries.push_back(entry);
        }

        pos = idEnd + 1;
    }

    // Sort by created_at descending (most recent first)
    std::sort(entries.begin(), entries.end(), [](const AuditLogEntry& a, const AuditLogEntry& b) {
        return a.createdAt > b.createdAt;
    });

    return entries;
}

std::string AuditTrailPage::formatTimestamp(const std::string& isoTimestamp) {
    if (isoTimestamp.empty()) return "-";

    // Parse ISO timestamp: 2026-01-31T14:30:00Z
    // Return formatted: Jan 31, 2026 2:30 PM

    try {
        // Simple extraction for display
        std::string date = isoTimestamp.substr(0, 10);  // YYYY-MM-DD
        std::string time = isoTimestamp.length() > 11 ? isoTimestamp.substr(11, 5) : "";  // HH:MM

        // Parse month
        int month = std::stoi(date.substr(5, 2));
        const char* months[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

        std::ostringstream formatted;
        formatted << months[month] << " " << date.substr(8, 2) << ", " << date.substr(0, 4);
        if (!time.empty()) {
            formatted << " " << time;
        }
        return formatted.str();
    } catch (...) {
        return isoTimestamp;
    }
}

std::string AuditTrailPage::getEventTypeBadgeClass(const std::string& eventType) {
    if (eventType == "login") return "badge-login";
    if (eventType == "logout") return "badge-logout";
    if (eventType == "failed_login") return "badge-failed";
    if (eventType == "settings_change" || eventType == "password_change") return "badge-settings";
    if (eventType.find("create") != std::string::npos) return "badge-create";
    if (eventType.find("update") != std::string::npos) return "badge-update";
    if (eventType.find("delete") != std::string::npos) return "badge-delete";
    return "badge-default";
}

} // namespace Widgets
} // namespace FranchiseAI
