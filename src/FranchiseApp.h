#ifndef FRANCHISE_APP_H
#define FRANCHISE_APP_H

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <memory>
#include <map>
#include <vector>
#include "widgets/Sidebar.h"
#include "widgets/Navigation.h"
#include "widgets/SearchPanel.h"
#include "widgets/ResultsDisplay.h"
#include "widgets/LoginDialog.h"
#include "widgets/AuditTrailPage.h"
#include "services/AISearchService.h"
#include "services/ScoringEngine.h"
#include "services/AuditLogger.h"
#include "services/ApiLogicServerClient.h"
#include "services/AuthService.h"
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

    /**
     * @brief Check if user is authenticated
     */
    bool isAuthenticated() const { return isAuthenticated_; }

    /**
     * @brief Get current session token
     */
    const std::string& getSessionToken() const { return sessionToken_; }

    /**
     * @brief Get current user info
     */
    const Services::UserDTO& getCurrentUser() const { return currentUser_; }

private:
    // Authentication
    void showLoginDialog();
    void onLoginSuccessful(const Services::LoginResult& result);
    void onLogout();
    bool checkAuthentication();
    void redirectToLogin();
    void setupUI();
    void setupRouting();
    void loadStyleSheet();

    // Navigation handlers
    void onMenuItemSelected(const std::string& itemId);
    void onQuickSearch(const std::string& query);

    // Search handlers
    void onSearchRequested(const Models::SearchQuery& query);
    void executeSearch(const Models::SearchQuery& query);
    void onSearchCancelled();
    void onSearchProgress(const Services::SearchProgress& progress);
    void onSearchComplete(const Models::SearchResults& results);

    // Result handlers
    void onViewDetails(const std::string& id);
    void onAddToProspects(const std::string& id);
    void onAddSelectedToProspects(const std::vector<std::string>& ids);
    void onExportResults();

    // Franchisee setup
    void onFranchiseeSetupComplete(const Models::Franchisee& franchisee);
    void updateHeaderWithFranchisee();

    // Page rendering
    void showSetupPage();
    void showDashboardPage();
    void showAISearchPage();
    void showProspectsPage();
    void showOpenStreetMapPage();
    void showReportsPage();
    void showSettingsPage();
    void showAuditTrailPage();

    // Franchisee data
    Models::Franchisee franchisee_;
    std::string currentStoreLocationId_;  // UUID for ALS updates
    std::string currentFranchiseeId_;     // UUID for ALS updates

    // ApiLogicServer integration - Store Location
    void loadStoreLocationFromALS();
    bool saveStoreLocationToALS();
    void selectStoreById(const std::string& storeId);
    std::vector<Services::StoreLocationDTO> loadAvailableStores();

    // ApiLogicServer integration - Franchisee
    void loadFranchiseeFromALS();
    bool saveFranchiseeToALS();
    void selectFranchiseeById(const std::string& franchiseeId);
    std::vector<Services::FranchiseeDTO> loadAvailableFranchisees();

    // ApiLogicServer integration - Scoring Rules
    void loadScoringRulesFromALS();
    bool saveScoringRulesToALS();

    // Mapping of ruleId -> database UUID for scoring rules
    std::map<std::string, std::string> scoringRuleDbIds_;

    // ApiLogicServer integration - Prospects
    void loadProspectsFromALS();
    bool saveProspectToALS(const Models::SearchResultItem& item);
    bool deleteProspectFromALS(const std::string& prospectId);
    Services::ProspectDTO prospectItemToDTO(const Models::SearchResultItem& item);
    Models::SearchResultItem dtoToProspectItem(const Services::ProspectDTO& dto);

    // Cached list of available stores (for selector)
    std::vector<Services::StoreLocationDTO> availableStores_;

    // Cached list of available franchisees (for selector)
    std::vector<Services::FranchiseeDTO> availableFranchisees_;

    // Services
    std::unique_ptr<Services::AISearchService> searchService_;
    std::unique_ptr<Services::ScoringEngine> scoringEngine_;
    std::unique_ptr<Services::ApiLogicServerClient> alsClient_;
    std::unique_ptr<Services::AuthService> authService_;

    // Authentication state
    bool isAuthenticated_ = false;
    std::string sessionToken_;
    Services::UserDTO currentUser_;
    Widgets::LoginDialog* loginDialog_ = nullptr;

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

    // Helper to perform AI analysis on a single prospect (synchronous)
    void analyzeProspect(Models::SearchResultItem& item);

    // Background AI analysis queue
    std::vector<std::string> analysisQueue_;  // Queue of prospect IDs pending analysis
    bool isAnalysisRunning_ = false;

    // Queue a prospect for background AI analysis
    void queueForAnalysis(const std::string& prospectId);

    // Process the next item in the analysis queue
    void processAnalysisQueue();

    // Find a saved prospect by ID (returns pointer or nullptr)
    Models::SearchResultItem* findSavedProspect(const std::string& id);

    // Toast notification system
    void showToast(const std::string& title, const std::string& message,
                   int score = -1, int durationMs = 6000);
    Wt::WContainerWidget* toastContainer_ = nullptr;

    // Search progress toast
    void showSearchToast();
    void hideSearchToast();
    Wt::WContainerWidget* searchToast_ = nullptr;
};

/**
 * @brief Application creator function for Wt
 */
std::unique_ptr<Wt::WApplication> createFranchiseApp(const Wt::WEnvironment& env);

} // namespace FranchiseAI

#endif // FRANCHISE_APP_H
