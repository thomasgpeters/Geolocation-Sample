#ifndef FRANCHISE_APP_H
#define FRANCHISE_APP_H

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <memory>
#include "widgets/Sidebar.h"
#include "widgets/Navigation.h"
#include "widgets/SearchPanel.h"
#include "widgets/ResultsDisplay.h"
#include "services/AISearchService.h"
#include "services/ApiLogicServerClient.h"
#include "models/Franchisee.h"
#include "models/GeoLocation.h"

namespace FranchiseAI {

/**
 * @brief Main application class for Franchise AI Search
 *
 * This is the main Wt application that provides the UI
 * for franchise owners to find potential catering clients.
 */
class FranchiseApp : public Wt::WApplication {
public:
    explicit FranchiseApp(const Wt::WEnvironment& env);
    ~FranchiseApp() override = default;

    /**
     * @brief Get the current franchisee
     */
    const Models::Franchisee& getFranchisee() const { return franchisee_; }

    /**
     * @brief Check if franchisee is configured
     */
    bool isFranchiseeConfigured() const { return franchisee_.isConfigured; }

    /**
     * @brief Get the current search area (shared between pages)
     */
    const Models::SearchArea& getCurrentSearchArea() const { return currentSearchArea_; }

    /**
     * @brief Check if a search has been performed
     */
    bool hasActiveSearch() const { return hasActiveSearch_; }

private:
    void setupUI();
    void setupRouting();
    void loadStyleSheet();

    // Navigation handlers
    void onMenuItemSelected(const std::string& itemId);
    void onQuickSearch(const std::string& query);

    // Search handlers
    void onSearchRequested(const Models::SearchQuery& query);
    void onSearchCancelled();
    void onSearchProgress(const Services::SearchProgress& progress);
    void onSearchComplete(const Models::SearchResults& results);

    // Result handlers
    void onViewDetails(const std::string& id);
    void onAddToProspects(const std::string& id);
    void onExportResults();

    // Franchisee setup
    void onFranchiseeSetupComplete(const Models::Franchisee& franchisee);
    void updateHeaderWithFranchisee();

    // Page rendering
    void showSetupPage();
    void showDashboardPage();
    void showAISearchPage();
    void showProspectsPage();
    void showDemographicsPage();
    void showReportsPage();
    void showSettingsPage();

    // Franchisee data
    Models::Franchisee franchisee_;
    std::string currentStoreLocationId_;  // UUID for ALS updates

    // ApiLogicServer integration
    void loadStoreLocationFromALS();
    bool saveStoreLocationToALS();

    // Services
    std::unique_ptr<Services::AISearchService> searchService_;
    std::unique_ptr<Services::ApiLogicServerClient> alsClient_;

    // Main layout containers
    Wt::WContainerWidget* mainContainer_ = nullptr;
    Wt::WContainerWidget* contentArea_ = nullptr;
    Wt::WContainerWidget* workArea_ = nullptr;

    // Widgets
    Widgets::Sidebar* sidebar_ = nullptr;
    Widgets::Navigation* navigation_ = nullptr;
    Widgets::SearchPanel* searchPanel_ = nullptr;
    Widgets::ResultsDisplay* resultsDisplay_ = nullptr;

    // Current state
    std::string currentPage_ = "ai-search";
    Models::SearchResults lastResults_;

    // Shared search context (synced between AI Search and Demographics)
    Models::SearchArea currentSearchArea_;
    std::string currentSearchLocation_;
    bool hasActiveSearch_ = false;

    // Saved prospects list (AI analysis performed when added)
    std::vector<Models::SearchResultItem> savedProspects_;

    // Helper to perform AI analysis on a single prospect
    void analyzeProspect(Models::SearchResultItem& item);
};

/**
 * @brief Application creator function for Wt
 */
std::unique_ptr<Wt::WApplication> createFranchiseApp(const Wt::WEnvironment& env);

} // namespace FranchiseAI

#endif // FRANCHISE_APP_H
