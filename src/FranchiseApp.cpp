#include "FranchiseApp.h"
#include "AppConfig.h"
#include "models/GeoLocation.h"
#include "widgets/LoginDialog.h"
#include "widgets/AuditTrailPage.h"
#include "services/AuditLogger.h"
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WCssStyleSheet.h>
#include <Wt/WText.h>
#include <Wt/WMessageBox.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WBreak.h>
#include <Wt/WComboBox.h>
#include <Wt/WCheckBox.h>
#include <Wt/WSlider.h>
#include <Wt/WTimer.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <ctime>

namespace FranchiseAI {

FranchiseApp::FranchiseApp(const Wt::WEnvironment& env)
    : Wt::WApplication(env)
{
    setTitle("FranchiseAI - Prospect Search");

    // Initialize authentication service
    authService_ = std::make_unique<Services::AuthService>();

    // Initialize search service with configuration from AppConfig
    Services::AISearchConfig config;

    // Load API keys from global configuration
    auto& appConfig = AppConfig::instance();

    // Configure OpenAI if API key is available
    if (appConfig.hasOpenAIKey()) {
        config.aiEngineConfig.provider = Services::AIProvider::OPENAI;
        config.aiEngineConfig.apiKey = appConfig.getOpenAIApiKey();
        config.aiEngineConfig.model = appConfig.getOpenAIModel();
    }
    // Fall back to Gemini if available
    else if (appConfig.hasGeminiKey()) {
        config.aiEngineConfig.provider = Services::AIProvider::GEMINI;
        config.aiEngineConfig.apiKey = appConfig.getGeminiApiKey();
        config.aiEngineConfig.model = "gemini-pro";
    }

    // Configure other API keys
    if (appConfig.hasGoogleKey()) {
        config.googleConfig.apiKey = appConfig.getGoogleApiKey();
    }
    if (appConfig.hasBBBKey()) {
        config.bbbConfig.apiKey = appConfig.getBBBApiKey();
    }
    if (appConfig.hasCensusKey()) {
        config.demographicsConfig.apiKey = appConfig.getCensusApiKey();
    }

    searchService_ = std::make_unique<Services::AISearchService>(config);

    // Initialize Scoring Engine with default rules
    scoringEngine_ = std::make_unique<Services::ScoringEngine>();

    // Initialize ApiLogicServer client
    alsClient_ = std::make_unique<Services::ApiLogicServerClient>();

    // Load styles first (needed for login dialog)
    loadStyleSheet();

    // Enable HTML5 history-based URLs (cleaner than ?_= format)
    setInternalPathDefaultValid(true);

    // Get initial path and check for session token in URL
    std::string initialPath = internalPath();

    // Check for token in URL parameters (e.g., /dashboard?token=xxx)
    const std::string* tokenParam = env.getParameter("token");
    if (tokenParam && !tokenParam->empty()) {
        std::cout << "[FranchiseApp] Found token in URL, validating session..." << std::endl;
        Services::SessionInfo session = authService_->validateSession(*tokenParam);
        if (session.isValid) {
            isAuthenticated_ = true;
            sessionToken_ = *tokenParam;
            currentUser_ = authService_->getUser(session.userId);
            std::cout << "[FranchiseApp] Session valid for user: " << currentUser_.email << std::endl;
        }
    }

    // If at root URL (/) and not authenticated, show login dialog
    if ((initialPath.empty() || initialPath == "/") && !isAuthenticated_) {
        std::cout << "[FranchiseApp] User at root URL, showing login dialog" << std::endl;
        showLoginDialog();
        return;
    }

    // If not authenticated and trying to access protected pages, redirect to login
    if (!isAuthenticated_ && initialPath != "/") {
        std::cout << "[FranchiseApp] User not authenticated, redirecting to login" << std::endl;
        redirectToLogin();
        return;
    }

    // User is authenticated, load app data and UI
    alsClient_->loadAppConfigs();  // Load all app config into memory cache
    loadFranchiseeFromALS();       // Load current franchisee first (for linking)
    loadStoreLocationFromALS();    // Then load store location
    loadScoringRulesFromALS();     // Load scoring rules for score optimization

    // Setup UI
    setupUI();

    // Setup routing with clean URL paths
    setupRouting();

    // Handle initial path from URL or default to Dashboard
    if (!franchisee_.isConfigured) {
        sidebar_->setActiveItem("settings");
        setInternalPath("/settings", false);
        showSettingsPage();
    } else if (initialPath == "/settings") {
        sidebar_->setActiveItem("settings");
        showSettingsPage();
    } else if (initialPath == "/dashboard" || initialPath == "/" || initialPath.empty()) {
        // Default to Dashboard for authenticated users
        sidebar_->setActiveItem("dashboard");
        setInternalPath("/dashboard", false);
        showDashboardPage();
    } else if (initialPath == "/search") {
        sidebar_->setActiveItem("ai-search");
        showAISearchPage();
    } else if (initialPath == "/prospects") {
        sidebar_->setActiveItem("prospects");
        showProspectsPage();
    } else if (initialPath == "/openstreetmap") {
        sidebar_->setActiveItem("openstreetmap");
        showOpenStreetMapPage();
    } else if (initialPath == "/reports") {
        sidebar_->setActiveItem("reports");
        showReportsPage();
    } else {
        // Default to Dashboard
        sidebar_->setActiveItem("dashboard");
        setInternalPath("/dashboard", false);
        showDashboardPage();
    }
}

void FranchiseApp::loadStyleSheet() {
    // Use custom CSS
    useStyleSheet("css/style.css");

    // Add inline critical styles as fallback
    std::string inlineStyles = R"CSS(
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
            background-color: #f5f7fa;
            color: #333;
        }
    )CSS";

    styleSheet().addRule("*", "margin: 0; padding: 0; box-sizing: border-box;");
}

void FranchiseApp::showLoginDialog() {
    std::cout << "[FranchiseApp] Showing login dialog" << std::endl;

    // Create login page background with gradient
    auto* loginBackground = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    loginBackground->addStyleClass("login-page-bg");

    // Add background styles
    styleSheet().addRule(".login-page-bg", "position: fixed; top: 0; left: 0; right: 0; bottom: 0; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); z-index: 0;");

    // Create the login dialog as a separate widget (WDialog manages its own display)
    // Don't add it to the container - WDialog creates its own overlay
    auto loginDialogPtr = std::make_unique<Widgets::LoginDialog>();
    loginDialog_ = loginDialogPtr.get();

    // Connect login success signal
    loginDialog_->loginSuccessful().connect(this, &FranchiseApp::onLoginSuccessful);

    // Add to application (WDialog needs to be added to app, not container)
    addChild(std::move(loginDialogPtr));

    // Show the dialog
    loginDialog_->show();
    loginDialog_->focusEmail();
}

void FranchiseApp::onLoginSuccessful(const Services::LoginResult& result) {
    std::cout << "[FranchiseApp] Login successful for: " << result.email << std::endl;

    // Store authentication state
    isAuthenticated_ = true;
    sessionToken_ = result.sessionToken;
    currentUser_.id = result.userId;
    currentUser_.email = result.email;
    currentUser_.firstName = result.firstName;
    currentUser_.lastName = result.lastName;
    currentUser_.role = result.role;
    currentUser_.franchiseeId = result.franchiseeId;

    // Log the login event
    std::string ipAddress;
    if (environment().clientAddress().length() > 0) {
        ipAddress = environment().clientAddress();
    }
    Services::AuditLogger::instance().logLogin(result.userId, result.email, ipAddress);

    // Clear the login page
    root()->clear();

    // Load app configuration and data
    alsClient_->loadAppConfigs();

    // If user has an associated franchisee, load their data
    if (!result.franchiseeId.empty()) {
        // Set the current franchisee ID in AppConfig cache for loading
        alsClient_->setAppConfig("current_franchisee_id", result.franchiseeId);
    }

    loadFranchiseeFromALS();
    loadStoreLocationFromALS();

    // Setup the main UI
    setupUI();
    setupRouting();

    // Set user role on sidebar (shows/hides admin items like Audit Trail)
    sidebar_->setUserRole(result.role);

    // Redirect to Dashboard with token in URL
    std::string dashboardUrl = "/dashboard?token=" + sessionToken_;
    setInternalPath("/dashboard", false);

    // Update browser URL to include token
    std::ostringstream js;
    js << "window.history.replaceState({}, '', '/dashboard?token=" << sessionToken_ << "');";
    doJavaScript(js.str());

    // Show dashboard
    sidebar_->setActiveItem("dashboard");
    showDashboardPage();

    std::cout << "[FranchiseApp] User redirected to dashboard" << std::endl;
}

void FranchiseApp::onLogout() {
    std::cout << "[FranchiseApp] User logging out" << std::endl;

    // Log the logout event before clearing state
    std::string ipAddress;
    if (environment().clientAddress().length() > 0) {
        ipAddress = environment().clientAddress();
    }
    Services::AuditLogger::instance().logLogout(currentUser_.id, ipAddress);

    // Invalidate session
    if (authService_ && !sessionToken_.empty()) {
        authService_->logout(sessionToken_);
    }

    // Clear authentication state
    isAuthenticated_ = false;
    sessionToken_.clear();
    currentUser_ = Services::UserDTO();

    // Redirect to login
    redirectToLogin();
}

bool FranchiseApp::checkAuthentication() {
    if (!isAuthenticated_ || sessionToken_.empty()) {
        return false;
    }

    // Validate session is still valid
    if (authService_) {
        Services::SessionInfo session = authService_->validateSession(sessionToken_);
        if (!session.isValid) {
            isAuthenticated_ = false;
            sessionToken_.clear();
            return false;
        }
    }

    return true;
}

void FranchiseApp::redirectToLogin() {
    std::cout << "[FranchiseApp] Redirecting to login" << std::endl;

    // Clear the current UI
    root()->clear();

    // Update URL to root and clean up any query parameters
    setInternalPath("/", true);
    doJavaScript("if(window.history && window.history.replaceState) {"
                 "  window.history.replaceState({}, '', '/');"
                 "}");

    // Show login dialog
    showLoginDialog();
}

void FranchiseApp::setupUI() {
    // Main container - full viewport layout
    mainContainer_ = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    mainContainer_->setStyleClass("app-container");

    // Sidebar
    sidebar_ = mainContainer_->addWidget(std::make_unique<Widgets::Sidebar>());

    // Set user info and franchise details from loaded franchisee
    if (franchisee_.isConfigured) {
        updateHeaderWithFranchisee();
    } else {
        sidebar_->setUserInfo("Franchise Owner", "No Store Selected");
    }

    // Connect sidebar signals
    sidebar_->itemSelected().connect(this, &FranchiseApp::onMenuItemSelected);

    // Connect franchise popover actions
    sidebar_->editFranchiseRequested().connect([this] {
        // Navigate to Settings page when "Edit Profile" is clicked
        onMenuItemSelected("settings");
        setInternalPath("/settings", true);
    });

    sidebar_->viewProfileRequested().connect([this] {
        // Navigate to Settings page for viewing profile
        onMenuItemSelected("settings");
        setInternalPath("/settings", true);
    });

    sidebar_->logoutRequested().connect([this] {
        // Handle logout - reset to login page
        redirectToLogin();
        setInternalPath("/login", true);
    });

    // Content area (navigation + work area)
    contentArea_ = mainContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    contentArea_->setStyleClass("content-area");

    // Top navigation
    navigation_ = contentArea_->addWidget(std::make_unique<Widgets::Navigation>());
    navigation_->quickSearchSubmitted().connect(this, &FranchiseApp::onQuickSearch);

    // Work area
    workArea_ = contentArea_->addWidget(std::make_unique<Wt::WContainerWidget>());
    workArea_->setStyleClass("work-area");

    // Toast notification container (fixed position overlay, added to root for independent positioning)
    toastContainer_ = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    toastContainer_->setStyleClass("toast-container");
}

void FranchiseApp::setupRouting() {
    // Internal path handling - only triggered by browser navigation (back/forward)
    // Not triggered when we call setInternalPath with emitPathChanged=false
    internalPathChanged().connect([this] {
        std::string path = internalPath();

        // Skip if we're already on this page (avoids duplicate renders)
        if (path == "/" + currentPage_ || (path == "/search" && currentPage_ == "ai-search")) {
            return;
        }

        if (path == "/dashboard") {
            currentPage_ = "dashboard";
            sidebar_->setActiveItem("dashboard");
            showDashboardPage();
        } else if (path == "/search" || path == "/ai-search") {
            currentPage_ = "ai-search";
            sidebar_->setActiveItem("ai-search");
            showAISearchPage();
        } else if (path == "/prospects") {
            currentPage_ = "prospects";
            sidebar_->setActiveItem("prospects");
            showProspectsPage();
        } else if (path == "/openstreetmap") {
            currentPage_ = "openstreetmap";
            sidebar_->setActiveItem("openstreetmap");
            showOpenStreetMapPage();
        } else if (path == "/reports") {
            currentPage_ = "reports";
            sidebar_->setActiveItem("reports");
            showReportsPage();
        } else if (path == "/settings" || path == "/setup") {
            currentPage_ = "settings";
            sidebar_->setActiveItem("settings");
            showSettingsPage();
        }
    });
}

void FranchiseApp::onMenuItemSelected(const std::string& itemId) {
    currentPage_ = itemId;

    // Map menu item IDs to clean URL paths
    std::string path;
    if (itemId == "dashboard") {
        path = "/dashboard";
        showDashboardPage();
    } else if (itemId == "ai-search") {
        path = "/search";
        showAISearchPage();
    } else if (itemId == "prospects") {
        path = "/prospects";
        showProspectsPage();
    } else if (itemId == "openstreetmap") {
        path = "/openstreetmap";
        showOpenStreetMapPage();
    } else if (itemId == "reports") {
        path = "/reports";
        showReportsPage();
    } else if (itemId == "settings") {
        path = "/settings";
        showSettingsPage();
    } else if (itemId == "audit-trail") {
        // Admin only - Audit Trail
        if (currentUser_.role == "admin") {
            path = "/audit";
            showAuditTrailPage();
        }
    }

    // Set internal path and clean up URL (remove ?_= query parameter)
    if (!path.empty()) {
        setInternalPath(path, true);
        // Use HTML5 History API to ensure clean URL without ?_= parameter
        doJavaScript("if(window.history && window.history.replaceState) {"
                     "  var url = window.location.pathname;"
                     "  if(url.indexOf('?') > -1) url = url.split('?')[0];"
                     "  window.history.replaceState({}, '', '" + path + "');"
                     "}");
    }
}

void FranchiseApp::onQuickSearch(const std::string& query) {
    // Switch to search page and execute search
    showAISearchPage();
    sidebar_->setActiveItem("ai-search");

    Models::SearchQuery searchQuery;
    searchQuery.location = query;
    searchQuery.radiusMiles = 25;
    searchQuery.includeGoogleMyBusiness = true;
    searchQuery.includeBBB = true;
    searchQuery.includeDemographics = true;

    if (searchPanel_) {
        searchPanel_->setSearchQuery(searchQuery);
    }

    onSearchRequested(searchQuery);
}

void FranchiseApp::onSearchRequested(const Models::SearchQuery& query) {
    if (!searchService_) return;

    // Create a modified query that includes settings from Settings > Marketing tab
    Models::SearchQuery searchQuery = query;

    // Apply business types and data sources from franchisee's saved settings
    if (franchisee_.isConfigured) {
        searchQuery.businessTypes = franchisee_.searchCriteria.businessTypes;
        searchQuery.minEmployees = franchisee_.searchCriteria.minEmployees;
        searchQuery.maxEmployees = franchisee_.searchCriteria.maxEmployees;
        searchQuery.includeOpenStreetMap = franchisee_.searchCriteria.includeOpenStreetMap;
        searchQuery.includeBBB = franchisee_.searchCriteria.includeBBB;
        searchQuery.includeGoogleMyBusiness = true;  // Always include if configured
        searchQuery.includeDemographics = true;      // Always include demographics
    }

    // Store the search context for syncing with Open Street Map page
    currentSearchLocation_ = searchQuery.location;
    if (searchQuery.latitude != 0 && searchQuery.longitude != 0) {
        Models::GeoLocation location(searchQuery.latitude, searchQuery.longitude);
        location.formattedAddress = searchQuery.location;
        currentSearchArea_ = Models::SearchArea::fromMiles(location, searchQuery.radiusMiles);
    } else if (!searchQuery.location.empty()) {
        Models::GeoLocation location = searchService_->geocodeAddress(searchQuery.location);
        currentSearchArea_ = Models::SearchArea::fromMiles(location, searchQuery.radiusMiles);
    }
    hasActiveSearch_ = true;

    // Show loading state
    if (resultsDisplay_) {
        resultsDisplay_->showLoading();
    }

    if (searchPanel_) {
        searchPanel_->setSearchEnabled(false);
        searchPanel_->showProgress(true);
    }

    // Perform search with merged query
    searchService_->search(
        searchQuery,
        [this](const Models::SearchResults& results) {
            // This callback is called when search completes
            onSearchComplete(results);
        },
        [this](const Services::SearchProgress& progress) {
            // This callback is called for progress updates
            onSearchProgress(progress);
        }
    );
}

void FranchiseApp::onSearchCancelled() {
    if (searchService_) {
        searchService_->cancelSearch();
    }

    if (searchPanel_) {
        searchPanel_->setSearchEnabled(true);
        searchPanel_->showProgress(false);
    }

    if (resultsDisplay_) {
        resultsDisplay_->clearResults();
    }
}

void FranchiseApp::onSearchProgress(const Services::SearchProgress& progress) {
    if (searchPanel_) {
        searchPanel_->setProgressMessage(progress.currentStep);
    }
}

void FranchiseApp::onSearchComplete(const Models::SearchResults& results) {
    lastResults_ = results;

    if (searchPanel_) {
        searchPanel_->setSearchEnabled(true);
        searchPanel_->showProgress(false);
    }

    // STEP 1: Display results IMMEDIATELY (before scoring optimization)
    // This gives the user instant feedback with raw OSM results
    if (resultsDisplay_) {
        if (results.errorMessage.empty()) {
            resultsDisplay_->showResults(lastResults_);

            // STEP 2: Show optimizing indicator if scoring is enabled
            if (scoringEngine_ && scoringEngine_->hasEnabledRules()) {
                resultsDisplay_->showOptimizing();

                // Force UI update to show results before scoring
                processEvents();

                // STEP 3: Apply scoring adjustments from ScoringEngine
                for (auto& item : lastResults_.items) {
                    if (item.business) {
                        int baseScore = item.business->cateringPotentialScore;
                        int adjustedScore = scoringEngine_->calculateFinalScore(*item.business, baseScore);
                        item.overallScore = adjustedScore;
                        item.business->cateringPotentialScore = adjustedScore;
                        item.aiConfidenceScore = adjustedScore / 100.0;
                    }
                }

                // Re-sort by adjusted score
                std::sort(lastResults_.items.begin(), lastResults_.items.end(),
                    [](const Models::SearchResultItem& a, const Models::SearchResultItem& b) {
                        return a.overallScore > b.overallScore;
                    });

                // STEP 4: Update display with optimized scores
                resultsDisplay_->updateResults(lastResults_);

                // STEP 5: Hide optimizing indicator - scoring complete
                resultsDisplay_->hideOptimizing();
            }
        } else {
            resultsDisplay_->showError(results.errorMessage);
        }
    }
}

void FranchiseApp::onViewDetails(const std::string& id) {
    // Find the item in results
    for (const auto& item : lastResults_.items) {
        if (item.id == id) {
            // Show details dialog or panel
            auto dialog = addChild(std::make_unique<Wt::WMessageBox>(
                "Prospect Details",
                "Details for: " + item.getTitle() + "\n\n" + item.aiSummary,
                Wt::Icon::Information,
                Wt::StandardButton::Ok
            ));
            dialog->show();
            break;
        }
    }
}

void FranchiseApp::onAddToProspects(const std::string& id) {
    // Find the item in search results
    for (const auto& item : lastResults_.items) {
        if (item.id == id) {
            // Check if already saved
            bool alreadySaved = false;
            for (const auto& saved : savedProspects_) {
                if (saved.id == id) {
                    alreadySaved = true;
                    break;
                }
            }

            if (alreadySaved) {
                showToast("Already Saved",
                          item.getTitle() + " is already in your prospects list.",
                          item.overallScore);
                return;
            }

            // Create a copy for saving
            Models::SearchResultItem prospectItem = item;

            // Show toast IMMEDIATELY (non-blocking feedback)
            showToast(item.getTitle(), "Added to My Prospects", item.overallScore);

            // Save to ApiLogicServer (persists to database)
            bool savedToServer = saveProspectToALS(prospectItem);
            if (!savedToServer) {
                std::cerr << "  [App] Warning: Prospect saved locally but failed to persist to server" << std::endl;
            }

            // Add to saved prospects (in-memory)
            savedProspects_.push_back(prospectItem);
            break;
        }
    }
}

void FranchiseApp::onAddSelectedToProspects(const std::vector<std::string>& ids) {
    int addedCount = 0;
    int skippedCount = 0;

    for (const auto& id : ids) {
        // Find the item in search results
        for (const auto& item : lastResults_.items) {
            if (item.id == id) {
                // Check if already saved
                bool alreadySaved = false;
                for (const auto& saved : savedProspects_) {
                    if (saved.id == id) {
                        alreadySaved = true;
                        break;
                    }
                }

                if (alreadySaved) {
                    skippedCount++;
                } else {
                    // Create a copy for saving
                    Models::SearchResultItem prospectItem = item;

                    // Save to ApiLogicServer (persists to database)
                    bool savedToServer = saveProspectToALS(prospectItem);
                    if (!savedToServer) {
                        std::cerr << "  [App] Warning: Prospect saved locally but failed to persist to server" << std::endl;
                    }

                    // Add to saved prospects (in-memory)
                    savedProspects_.push_back(prospectItem);
                    addedCount++;
                }
                break;
            }
        }
    }

    // Show toast with summary
    if (addedCount > 0) {
        std::string message = std::to_string(addedCount) + " prospect" + (addedCount == 1 ? "" : "s") + " added to My Prospects";
        if (skippedCount > 0) {
            message += " (" + std::to_string(skippedCount) + " already saved)";
        }
        showToast("Prospects Added", message);
    } else if (skippedCount > 0) {
        showToast("Already Saved", "All selected prospects were already in your list.");
    }
}

void FranchiseApp::analyzeProspect(Models::SearchResultItem& item) {
    if (!item.business) {
        item.analysisStatus = Models::AnalysisStatus::SKIPPED;
        return;
    }

    // Check if already analyzed
    if (item.analysisStatus == Models::AnalysisStatus::COMPLETED) {
        return;  // Don't re-analyze - saves AI tokens
    }

    item.analysisStatus = Models::AnalysisStatus::IN_PROGRESS;

    // Use AI engine if available for deep analysis
    if (searchService_->isAIEngineConfigured()) {
        auto* aiEngine = searchService_->getAIEngine();
        if (aiEngine) {
            try {
                auto analysis = aiEngine->analyzeBusinessPotentialSync(*item.business);
                if (!analysis.summary.empty()) {
                    item.aiSummary = analysis.summary;
                    item.keyHighlights = analysis.keyHighlights;
                    item.recommendedActions = analysis.recommendedActions;
                    item.matchReason = analysis.matchReason;
                    item.aiConfidenceScore = analysis.confidenceScore;

                    if (analysis.cateringPotentialScore > 0) {
                        item.business->cateringPotentialScore = analysis.cateringPotentialScore;
                    }
                    item.analysisStatus = Models::AnalysisStatus::COMPLETED;
                } else {
                    item.analysisStatus = Models::AnalysisStatus::FAILED;
                    item.analysisError = "Empty analysis response";
                }
            } catch (const std::exception& e) {
                item.analysisStatus = Models::AnalysisStatus::FAILED;
                item.analysisError = e.what();
                std::cerr << "  [App] AI analysis failed: " << e.what() << std::endl;
            }
        } else {
            item.analysisStatus = Models::AnalysisStatus::SKIPPED;
        }
    } else {
        item.analysisStatus = Models::AnalysisStatus::SKIPPED;
    }
}

Models::SearchResultItem* FranchiseApp::findSavedProspect(const std::string& id) {
    for (auto& prospect : savedProspects_) {
        if (prospect.id == id) {
            return &prospect;
        }
    }
    return nullptr;
}

void FranchiseApp::queueForAnalysis(const std::string& prospectId) {
    // Check if already in queue
    for (const auto& queuedId : analysisQueue_) {
        if (queuedId == prospectId) return;
    }

    // Check if already analyzed
    auto* prospect = findSavedProspect(prospectId);
    if (prospect && prospect->analysisStatus == Models::AnalysisStatus::COMPLETED) {
        return;  // Already analyzed, don't waste tokens
    }

    analysisQueue_.push_back(prospectId);

    // Start processing if not already running
    if (!isAnalysisRunning_) {
        processAnalysisQueue();
    }
}

void FranchiseApp::processAnalysisQueue() {
    if (analysisQueue_.empty()) {
        isAnalysisRunning_ = false;
        return;
    }

    isAnalysisRunning_ = true;

    // Get next prospect ID from queue
    std::string prospectId = analysisQueue_.front();
    analysisQueue_.erase(analysisQueue_.begin());

    // Find the prospect
    auto* prospect = findSavedProspect(prospectId);
    if (!prospect) {
        // Prospect not found, skip to next
        processAnalysisQueue();
        return;
    }

    // Skip if already completed
    if (prospect->analysisStatus == Models::AnalysisStatus::COMPLETED) {
        processAnalysisQueue();
        return;
    }

    std::cout << "  [App] Background analysis: " << prospect->getTitle() << std::endl;

    // Perform analysis
    analyzeProspect(*prospect);

    // Update server with analysis results
    if (prospect->analysisStatus == Models::AnalysisStatus::COMPLETED) {
        // Optionally update the prospect on the server with AI analysis data
        // saveProspectToALS(*prospect);  // Uncomment if you want to persist analysis
    }

    // Schedule next item with a small delay to allow UI updates
    Wt::WTimer::singleShot(std::chrono::milliseconds(100), [this] {
        processAnalysisQueue();
    });
}

void FranchiseApp::showToast(const std::string& title, const std::string& message,
                              int score, int durationMs) {
    if (!toastContainer_) return;

    // Create toast element
    auto toast = toastContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    toast->setStyleClass("toast toast-enter");

    // Toast header with title and close button
    auto header = toast->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->setStyleClass("toast-header");

    auto titleText = header->addWidget(std::make_unique<Wt::WText>(title));
    titleText->setStyleClass("toast-title");

    // Score badge if provided
    if (score >= 0) {
        std::string scoreClass = "score-badge ";
        if (score >= 70) scoreClass += "score-high";
        else if (score >= 40) scoreClass += "score-medium";
        else scoreClass += "score-low";

        auto scoreBadge = header->addWidget(std::make_unique<Wt::WText>(std::to_string(score)));
        scoreBadge->setStyleClass(scoreClass);
    }

    auto closeBtn = header->addWidget(std::make_unique<Wt::WText>("✕"));
    closeBtn->setStyleClass("toast-close");
    closeBtn->clicked().connect([toast] {
        toast->addStyleClass("toast-exit");
        // Remove after animation
        Wt::WTimer::singleShot(std::chrono::milliseconds(300), [toast] {
            if (toast->parent()) {
                toast->parent()->removeWidget(toast);
            }
        });
    });

    // Toast body with message
    auto body = toast->addWidget(std::make_unique<Wt::WContainerWidget>());
    body->setStyleClass("toast-body");
    body->addWidget(std::make_unique<Wt::WText>(message));

    // Trigger enter animation
    doJavaScript("setTimeout(function() { " + toast->jsRef() + ".classList.remove('toast-enter'); }, 10);");

    // Auto-remove after duration
    Wt::WTimer::singleShot(std::chrono::milliseconds(durationMs), [this, toast] {
        if (toast->parent()) {
            toast->addStyleClass("toast-exit");
            // Remove after exit animation completes
            Wt::WTimer::singleShot(std::chrono::milliseconds(300), [toast] {
                if (toast->parent()) {
                    toast->parent()->removeWidget(toast);
                }
            });
        }
    });
}

void FranchiseApp::onExportResults() {
    auto dialog = addChild(std::make_unique<Wt::WMessageBox>(
        "Export Results",
        "Results export feature will generate a CSV file with " +
        std::to_string(lastResults_.totalResults) + " prospects.",
        Wt::Icon::Information,
        Wt::StandardButton::Ok
    ));
    dialog->show();
}

void FranchiseApp::onFranchiseeSetupComplete(const Models::Franchisee& franchisee) {
    franchisee_ = franchisee;
    franchisee_.isConfigured = true;

    // Update the sidebar with franchisee info
    updateHeaderWithFranchisee();

    // Save to ApiLogicServer
    if (franchisee_.location.hasValidCoordinates()) {
        std::cout << "  [Setup] Saving store location to ALS..." << std::endl;
        saveStoreLocationToALS();
    }

    // Navigate to AI Search page
    sidebar_->setActiveItem("ai-search");
    setInternalPath("/search", false);
    showAISearchPage();
}

void FranchiseApp::updateHeaderWithFranchisee() {
    if (sidebar_ && franchisee_.isConfigured) {
        // Update the header display name and location
        sidebar_->setUserInfo(
            franchisee_.getDisplayName(),
            franchisee_.getLocationDisplay()
        );

        // Update the franchise details popover with full information
        sidebar_->setFranchiseDetails(
            franchisee_.ownerName.empty() ? "Franchise Owner" : franchisee_.ownerName,
            franchisee_.storeName.empty() ? "My Store" : franchisee_.storeName,
            franchisee_.storeId,
            franchisee_.address,
            franchisee_.phone,
            franchisee_.email
        );
    }
}

void FranchiseApp::showSetupPage() {
    workArea_->clear();
    navigation_->setPageTitle("Store Setup");
    navigation_->setBreadcrumbs({"Home", "Setup"});
    navigation_->setMarketScore(-1);  // Hide market score on non-OSM pages

    auto container = workArea_->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setStyleClass("page-container setup-page");

    // Header
    auto header = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->setStyleClass("page-header");

    auto title = header->addWidget(std::make_unique<Wt::WText>("Welcome to FranchiseAI"));
    title->setStyleClass("page-title");

    auto subtitle = header->addWidget(std::make_unique<Wt::WText>(
        "Let's set up your store location to find catering prospects nearby"
    ));
    subtitle->setStyleClass("page-subtitle");

    // Setup form section
    auto formSection = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    formSection->setStyleClass("settings-section");

    auto formTitle = formSection->addWidget(std::make_unique<Wt::WText>("Store Information"));
    formTitle->setStyleClass("section-title");

    auto formDesc = formSection->addWidget(std::make_unique<Wt::WText>(
        "Enter your Vocelli Pizza store details. This will be the center point for all prospect searches."
    ));
    formDesc->setStyleClass("section-description");

    // Form grid
    auto formGrid = formSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    formGrid->setStyleClass("form-grid");

    // Store Name
    auto nameGroup = formGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    nameGroup->setStyleClass("form-group");
    auto nameLabel = nameGroup->addWidget(std::make_unique<Wt::WText>("Store Name"));
    nameLabel->setStyleClass("form-label");
    auto nameInput = nameGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    nameInput->setPlaceholderText("e.g., Vocelli Pizza - Downtown");
    nameInput->setStyleClass("form-control");

    // Store Address
    auto addressGroup = formGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    addressGroup->setStyleClass("form-group");
    auto addressLabel = addressGroup->addWidget(std::make_unique<Wt::WText>("Store Address"));
    addressLabel->setStyleClass("form-label");
    auto addressInput = addressGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    addressInput->setPlaceholderText("e.g., 123 Main St, Denver, CO 80202");
    addressInput->setStyleClass("form-control");

    // Owner Name
    auto ownerGroup = formGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    ownerGroup->setStyleClass("form-group");
    auto ownerLabel = ownerGroup->addWidget(std::make_unique<Wt::WText>("Owner/Manager Name"));
    ownerLabel->setStyleClass("form-label");
    auto ownerInput = ownerGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    ownerInput->setPlaceholderText("e.g., John Smith");
    ownerInput->setStyleClass("form-control");

    // Phone
    auto phoneGroup = formGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    phoneGroup->setStyleClass("form-group");
    auto phoneLabel = phoneGroup->addWidget(std::make_unique<Wt::WText>("Store Phone"));
    phoneLabel->setStyleClass("form-label");
    auto phoneInput = phoneGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    phoneInput->setPlaceholderText("e.g., (555) 123-4567");
    phoneInput->setStyleClass("form-control");

    // Search Preferences Section
    auto prefsSection = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    prefsSection->setStyleClass("settings-section");

    auto prefsTitle = prefsSection->addWidget(std::make_unique<Wt::WText>("Default Search Preferences"));
    prefsTitle->setStyleClass("section-title");

    auto prefsGrid = prefsSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    prefsGrid->setStyleClass("form-grid");

    // Default Radius
    auto radiusGroup = prefsGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    radiusGroup->setStyleClass("form-group");
    auto radiusLabel = radiusGroup->addWidget(std::make_unique<Wt::WText>("Default Search Radius (miles)"));
    radiusLabel->setStyleClass("form-label");
    auto radiusInput = radiusGroup->addWidget(std::make_unique<Wt::WLineEdit>("5"));
    radiusInput->setStyleClass("form-control");

    // Target Business Types
    auto typesGroup = prefsSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    typesGroup->setStyleClass("form-group");
    auto typesLabel = typesGroup->addWidget(std::make_unique<Wt::WText>("Target Business Types"));
    typesLabel->setStyleClass("form-label");
    auto typesDesc = typesGroup->addWidget(std::make_unique<Wt::WText>(
        "Select the types of businesses you want to target for catering services"
    ));
    typesDesc->setStyleClass("form-help");

    auto checkboxGrid = typesGroup->addWidget(std::make_unique<Wt::WContainerWidget>());
    checkboxGrid->setStyleClass("checkbox-grid");

    // Business type checkboxes
    std::vector<std::pair<std::string, bool>> businessTypes = {
        {"Corporate Offices", true},
        {"Conference Centers", true},
        {"Hotels", true},
        {"Medical Facilities", true},
        {"Educational Institutions", true},
        {"Manufacturing/Industrial", false},
        {"Warehouses/Distribution", false},
        {"Government Offices", false},
        {"Tech Companies", true},
        {"Financial Services", false},
        {"Coworking Spaces", true},
        {"Non-profits", false}
    };

    std::vector<Wt::WCheckBox*> typeCheckboxes;
    for (const auto& [typeName, defaultChecked] : businessTypes) {
        auto checkbox = checkboxGrid->addWidget(std::make_unique<Wt::WCheckBox>(typeName));
        checkbox->setStyleClass("form-checkbox");
        checkbox->setChecked(defaultChecked);
        typeCheckboxes.push_back(checkbox);
    }

    // Employee Size Section
    auto sizeGroup = prefsSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    sizeGroup->setStyleClass("form-group");
    auto sizeLabel = sizeGroup->addWidget(std::make_unique<Wt::WText>("Target Organization Size"));
    sizeLabel->setStyleClass("form-label");

    auto sizeCombo = sizeGroup->addWidget(std::make_unique<Wt::WComboBox>());
    sizeCombo->setStyleClass("form-control");
    for (const auto& range : Models::EmployeeRange::getStandardRanges()) {
        sizeCombo->addItem(range.label);
    }
    sizeCombo->setCurrentIndex(0);

    // Action Buttons
    auto actionsSection = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    actionsSection->setStyleClass("form-actions");

    auto saveBtn = actionsSection->addWidget(std::make_unique<Wt::WPushButton>("Save and Continue to Search"));
    saveBtn->setStyleClass("btn btn-primary btn-lg");

    // Connect save button
    saveBtn->clicked().connect([this, nameInput, addressInput, ownerInput, phoneInput, radiusInput, sizeCombo, typeCheckboxes]() {
        // Validate required fields
        std::string storeName = nameInput->text().toUTF8();
        std::string address = addressInput->text().toUTF8();

        if (storeName.empty() || address.empty()) {
            auto dialog = addChild(std::make_unique<Wt::WMessageBox>(
                "Missing Information",
                "Please enter both store name and address.",
                Wt::Icon::Warning,
                Wt::StandardButton::Ok
            ));
            dialog->show();
            return;
        }

        // Geocode the address
        Models::GeoLocation location = searchService_->geocodeAddress(address);

        // Create franchisee
        Models::Franchisee franchisee;
        franchisee.storeName = storeName;
        franchisee.address = address;
        franchisee.ownerName = ownerInput->text().toUTF8();
        franchisee.phone = phoneInput->text().toUTF8();
        franchisee.location = location;

        // Set search radius
        try {
            franchisee.defaultSearchRadiusMiles = std::stod(radiusInput->text().toUTF8());
        } catch (...) {
            franchisee.defaultSearchRadiusMiles = 5.0;
        }
        franchisee.searchCriteria.radiusMiles = franchisee.defaultSearchRadiusMiles;

        // Set employee range based on combo selection
        auto ranges = Models::EmployeeRange::getStandardRanges();
        int sizeIdx = sizeCombo->currentIndex();
        if (sizeIdx >= 0 && sizeIdx < static_cast<int>(ranges.size())) {
            franchisee.searchCriteria.minEmployees = ranges[sizeIdx].minEmployees;
            franchisee.searchCriteria.maxEmployees = ranges[sizeIdx].maxEmployees;
        }

        // Set business types based on checkboxes
        franchisee.searchCriteria.clearBusinessTypes();
        std::vector<Models::BusinessType> allTypes = {
            Models::BusinessType::CORPORATE_OFFICE,
            Models::BusinessType::CONFERENCE_CENTER,
            Models::BusinessType::HOTEL,
            Models::BusinessType::MEDICAL_FACILITY,
            Models::BusinessType::EDUCATIONAL_INSTITUTION,
            Models::BusinessType::MANUFACTURING,
            Models::BusinessType::WAREHOUSE,
            Models::BusinessType::GOVERNMENT_OFFICE,
            Models::BusinessType::TECH_COMPANY,
            Models::BusinessType::FINANCIAL_SERVICES,
            Models::BusinessType::COWORKING_SPACE,
            Models::BusinessType::NONPROFIT
        };

        for (size_t i = 0; i < typeCheckboxes.size() && i < allTypes.size(); ++i) {
            if (typeCheckboxes[i]->isChecked()) {
                franchisee.searchCriteria.addBusinessType(allTypes[i]);
            }
        }

        // Complete setup
        onFranchiseeSetupComplete(franchisee);
    });
}

void FranchiseApp::showDashboardPage() {
    workArea_->clear();
    navigation_->setPageTitle("Dashboard");
    navigation_->setBreadcrumbs({"Home", "Dashboard"});
    navigation_->setMarketScore(-1);

    auto container = workArea_->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setStyleClass("page-container dashboard-page");

    // Dashboard header
    auto header = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->setStyleClass("page-header");

    auto title = header->addWidget(std::make_unique<Wt::WText>("Welcome to FranchiseAI"));
    title->setStyleClass("page-title");

    auto subtitle = header->addWidget(std::make_unique<Wt::WText>(
        "Your AI-powered prospect discovery dashboard"
    ));
    subtitle->setStyleClass("page-subtitle");

    // Stats cards
    auto statsGrid = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    statsGrid->setStyleClass("stats-grid");

    // Stat cards
    std::vector<std::tuple<std::string, std::string, std::string>> stats = {
        {"156", "Total Prospects", "📊"},
        {"42", "Hot Leads", "🔥"},
        {"89%", "Contact Rate", "📞"},
        {"$12.4K", "Projected Revenue", "💰"}
    };

    for (const auto& [value, label, icon] : stats) {
        auto card = statsGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
        card->setStyleClass("stat-card");

        auto iconText = card->addWidget(std::make_unique<Wt::WText>(icon));
        iconText->setStyleClass("stat-icon");

        auto valueText = card->addWidget(std::make_unique<Wt::WText>(value));
        valueText->setStyleClass("stat-value");

        auto labelText = card->addWidget(std::make_unique<Wt::WText>(label));
        labelText->setStyleClass("stat-label");
    }

    // Quick actions
    auto actionsSection = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    actionsSection->setStyleClass("quick-actions");

    auto actionsTitle = actionsSection->addWidget(std::make_unique<Wt::WText>("Quick Actions"));
    actionsTitle->setStyleClass("section-title");

    auto actionsGrid = actionsSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    actionsGrid->setStyleClass("actions-grid");

    auto searchAction = actionsGrid->addWidget(std::make_unique<Wt::WPushButton>("🔍 Start AI Search"));
    searchAction->setStyleClass("action-btn primary");
    searchAction->clicked().connect([this] {
        onMenuItemSelected("ai-search");
    });

    auto viewProspects = actionsGrid->addWidget(std::make_unique<Wt::WPushButton>("👥 View Prospects"));
    viewProspects->setStyleClass("action-btn secondary");
    viewProspects->clicked().connect([this] {
        onMenuItemSelected("prospects");
    });

    auto viewReports = actionsGrid->addWidget(std::make_unique<Wt::WPushButton>("📈 View Reports"));
    viewReports->setStyleClass("action-btn secondary");
    viewReports->clicked().connect([this] {
        onMenuItemSelected("reports");
    });

    // Hot Prospects section
    auto hotProspectsSection = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    hotProspectsSection->setStyleClass("hot-prospects-section");

    auto hotProspectsHeader = hotProspectsSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    hotProspectsHeader->setStyleClass("section-header");

    auto hotProspectsTitle = hotProspectsHeader->addWidget(std::make_unique<Wt::WText>("🔥 Hot Prospects"));
    hotProspectsTitle->setStyleClass("section-title");

    // Score legend
    auto scoreLegend = hotProspectsHeader->addWidget(std::make_unique<Wt::WContainerWidget>());
    scoreLegend->setStyleClass("score-legend");

    auto legendExcellent = scoreLegend->addWidget(std::make_unique<Wt::WContainerWidget>());
    legendExcellent->setStyleClass("legend-item");
    auto excellentDot = legendExcellent->addWidget(std::make_unique<Wt::WText>(""));
    excellentDot->setStyleClass("legend-dot score-excellent");
    auto excellentLabel = legendExcellent->addWidget(std::make_unique<Wt::WText>("80+"));
    excellentLabel->setStyleClass("legend-label");

    auto legendGood = scoreLegend->addWidget(std::make_unique<Wt::WContainerWidget>());
    legendGood->setStyleClass("legend-item");
    auto goodDot = legendGood->addWidget(std::make_unique<Wt::WText>(""));
    goodDot->setStyleClass("legend-dot score-good");
    auto goodLabel = legendGood->addWidget(std::make_unique<Wt::WText>("60-79"));
    goodLabel->setStyleClass("legend-label");

    auto legendFair = scoreLegend->addWidget(std::make_unique<Wt::WContainerWidget>());
    legendFair->setStyleClass("legend-item");
    auto fairDot = legendFair->addWidget(std::make_unique<Wt::WText>(""));
    fairDot->setStyleClass("legend-dot score-fair");
    auto fairLabel = legendFair->addWidget(std::make_unique<Wt::WText>("40-59"));
    fairLabel->setStyleClass("legend-label");

    auto legendLow = scoreLegend->addWidget(std::make_unique<Wt::WContainerWidget>());
    legendLow->setStyleClass("legend-item");
    auto lowDot = legendLow->addWidget(std::make_unique<Wt::WText>(""));
    lowDot->setStyleClass("legend-dot score-low");
    auto lowLabel = legendLow->addWidget(std::make_unique<Wt::WText>("<40"));
    lowLabel->setStyleClass("legend-label");

    auto viewAllBtn = hotProspectsHeader->addWidget(std::make_unique<Wt::WPushButton>("View All"));
    viewAllBtn->setStyleClass("btn btn-outline btn-sm");
    viewAllBtn->clicked().connect([this] {
        onMenuItemSelected("prospects");
    });

    // Get top 5 prospects from last search results or saved prospects
    std::vector<Models::SearchResultItem> hotProspects;

    // First try to get from last search results
    if (!lastResults_.items.empty()) {
        auto topResults = lastResults_.getTopResults(5);
        hotProspects = topResults;
    } else if (!savedProspects_.empty()) {
        // Fall back to saved prospects
        hotProspects = savedProspects_;
        // Sort by score descending
        std::sort(hotProspects.begin(), hotProspects.end(),
            [](const Models::SearchResultItem& a, const Models::SearchResultItem& b) {
                return a.overallScore > b.overallScore;
            });
        if (hotProspects.size() > 5) {
            hotProspects.resize(5);
        }
    }

    if (hotProspects.empty()) {
        // Show placeholder when no prospects
        auto placeholder = hotProspectsSection->addWidget(std::make_unique<Wt::WContainerWidget>());
        placeholder->setStyleClass("hot-prospects-placeholder");

        auto placeholderIcon = placeholder->addWidget(std::make_unique<Wt::WText>("🔍"));
        placeholderIcon->setStyleClass("placeholder-icon-sm");

        auto placeholderText = placeholder->addWidget(std::make_unique<Wt::WText>(
            "No hot prospects yet. Start an AI Search to discover potential clients in your area."
        ));
        placeholderText->setStyleClass("placeholder-text-sm");
    } else {
        // Create prospect table
        auto prospectTable = hotProspectsSection->addWidget(std::make_unique<Wt::WContainerWidget>());
        prospectTable->setStyleClass("hot-prospects-table");

        // Table header
        auto tableHeader = prospectTable->addWidget(std::make_unique<Wt::WContainerWidget>());
        tableHeader->setStyleClass("prospect-table-header");

        auto headerName = tableHeader->addWidget(std::make_unique<Wt::WText>("Business Name"));
        headerName->setStyleClass("header-cell name-col");

        auto headerScore = tableHeader->addWidget(std::make_unique<Wt::WText>("Score"));
        headerScore->setStyleClass("header-cell score-col");

        auto headerAddress = tableHeader->addWidget(std::make_unique<Wt::WText>("Address"));
        headerAddress->setStyleClass("header-cell address-col");

        auto headerActions = tableHeader->addWidget(std::make_unique<Wt::WText>("Actions"));
        headerActions->setStyleClass("header-cell actions-col");

        // Table body (scrollable)
        auto tableBody = prospectTable->addWidget(std::make_unique<Wt::WContainerWidget>());
        tableBody->setStyleClass("prospect-table-body");

        // Table rows
        for (const auto& prospect : hotProspects) {
            auto row = tableBody->addWidget(std::make_unique<Wt::WContainerWidget>());
            row->setStyleClass("prospect-table-row");

            // Business name
            std::string businessName = prospect.getTitle();
            if (businessName.empty() && prospect.business) {
                businessName = prospect.business->name;
            }
            if (businessName.empty()) {
                businessName = "Unknown Business";
            }
            auto nameCell = row->addWidget(std::make_unique<Wt::WText>(businessName));
            nameCell->setStyleClass("table-cell name-col");

            // Score with color coding
            auto scoreCell = row->addWidget(std::make_unique<Wt::WContainerWidget>());
            scoreCell->setStyleClass("table-cell score-col");

            int score = prospect.overallScore;
            std::string scoreClass = "score-badge";
            if (score >= 80) scoreClass += " score-excellent";
            else if (score >= 60) scoreClass += " score-good";
            else if (score >= 40) scoreClass += " score-fair";
            else scoreClass += " score-low";

            auto scoreBadge = scoreCell->addWidget(std::make_unique<Wt::WText>(std::to_string(score)));
            scoreBadge->setStyleClass(scoreClass);

            // Address
            std::string address = "";
            if (prospect.business) {
                address = prospect.business->address.city;
                if (!prospect.business->address.state.empty()) {
                    if (!address.empty()) address += ", ";
                    address += prospect.business->address.state;
                }
            }
            if (address.empty()) {
                address = prospect.getSubtitle();
            }
            auto addressCell = row->addWidget(std::make_unique<Wt::WText>(address));
            addressCell->setStyleClass("table-cell address-col");

            // Actions
            auto actionsCell = row->addWidget(std::make_unique<Wt::WContainerWidget>());
            actionsCell->setStyleClass("table-cell actions-col");

            // Preview button (popup)
            auto previewBtn = actionsCell->addWidget(std::make_unique<Wt::WPushButton>("👁️"));
            previewBtn->setStyleClass("btn btn-icon btn-preview");
            previewBtn->setToolTip("Preview Details");

            // Capture prospect data for popup
            std::string prospectId = prospect.id;
            std::string fullName = businessName;
            std::string fullAddress = prospect.business ? prospect.business->address.getFullAddress() : "";
            std::string matchReason = prospect.matchReason;
            std::string phone = prospect.business ? prospect.business->contact.primaryPhone : "";
            std::string website = prospect.business ? prospect.business->contact.website : "";
            int prospectScore = score;

            previewBtn->clicked().connect([this, fullName, fullAddress, matchReason, phone, website, prospectScore] {
                // Create popup dialog
                auto dialog = addChild(std::make_unique<Wt::WDialog>("Business Preview"));
                dialog->setStyleClass("preview-dialog");
                dialog->setModal(true);
                dialog->setClosable(true);
                dialog->setResizable(false);

                auto content = dialog->contents();
                content->setStyleClass("preview-content");

                // Business name header
                auto nameHeader = content->addWidget(std::make_unique<Wt::WText>(fullName));
                nameHeader->setStyleClass("preview-name");

                // Score badge
                std::string scoreClass = "score-badge large";
                if (prospectScore >= 80) scoreClass += " score-excellent";
                else if (prospectScore >= 60) scoreClass += " score-good";
                else if (prospectScore >= 40) scoreClass += " score-fair";
                else scoreClass += " score-low";

                auto scoreDisplay = content->addWidget(std::make_unique<Wt::WContainerWidget>());
                scoreDisplay->setStyleClass("preview-score-row");
                auto scoreLabel = scoreDisplay->addWidget(std::make_unique<Wt::WText>("Prospect Score: "));
                auto scoreBadge = scoreDisplay->addWidget(std::make_unique<Wt::WText>(std::to_string(prospectScore)));
                scoreBadge->setStyleClass(scoreClass);

                // Details grid
                auto detailsGrid = content->addWidget(std::make_unique<Wt::WContainerWidget>());
                detailsGrid->setStyleClass("preview-details");

                if (!fullAddress.empty()) {
                    auto addrRow = detailsGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
                    addrRow->setStyleClass("detail-row");
                    addrRow->addWidget(std::make_unique<Wt::WText>("📍 "))->setStyleClass("detail-icon");
                    addrRow->addWidget(std::make_unique<Wt::WText>(fullAddress))->setStyleClass("detail-value");
                }

                if (!phone.empty()) {
                    auto phoneRow = detailsGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
                    phoneRow->setStyleClass("detail-row");
                    phoneRow->addWidget(std::make_unique<Wt::WText>("📞 "))->setStyleClass("detail-icon");
                    phoneRow->addWidget(std::make_unique<Wt::WText>(phone))->setStyleClass("detail-value");
                }

                if (!website.empty()) {
                    auto webRow = detailsGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
                    webRow->setStyleClass("detail-row");
                    webRow->addWidget(std::make_unique<Wt::WText>("🌐 "))->setStyleClass("detail-icon");
                    webRow->addWidget(std::make_unique<Wt::WText>(website))->setStyleClass("detail-value");
                }

                if (!matchReason.empty()) {
                    auto reasonSection = content->addWidget(std::make_unique<Wt::WContainerWidget>());
                    reasonSection->setStyleClass("preview-reason");
                    reasonSection->addWidget(std::make_unique<Wt::WText>("Why This Prospect?"))->setStyleClass("reason-title");
                    reasonSection->addWidget(std::make_unique<Wt::WText>(matchReason))->setStyleClass("reason-text");
                }

                // Dialog footer
                auto footer = dialog->footer();
                footer->setStyleClass("preview-footer");

                auto closeBtn = footer->addWidget(std::make_unique<Wt::WPushButton>("Close"));
                closeBtn->setStyleClass("btn btn-secondary");
                closeBtn->clicked().connect([dialog] {
                    dialog->reject();
                });

                dialog->finished().connect([dialog] {
                    delete dialog;
                });

                dialog->show();
            });

            // Add to Prospects button
            auto addBtn = actionsCell->addWidget(std::make_unique<Wt::WPushButton>("➕"));
            addBtn->setStyleClass("btn btn-icon btn-add");
            addBtn->setToolTip("Add to My Prospects");

            addBtn->clicked().connect([this, prospectId] {
                onAddToProspects(prospectId);
                // Refresh dashboard to update the list
                showDashboardPage();
            });
        }
    }
}

void FranchiseApp::showAISearchPage() {
    workArea_->clear();
    navigation_->setPageTitle("AI Search");
    navigation_->setBreadcrumbs({"Home", "AI Search"});
    navigation_->setMarketScore(-1);

    auto container = workArea_->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setStyleClass("page-container search-page");

    // Franchisee info banner (if configured)
    if (franchisee_.isConfigured) {
        auto franchiseeBanner = container->addWidget(std::make_unique<Wt::WContainerWidget>());
        franchiseeBanner->setStyleClass("franchisee-badge");

        auto storeIcon = franchiseeBanner->addWidget(std::make_unique<Wt::WText>("📍 "));
        auto storeName = franchiseeBanner->addWidget(std::make_unique<Wt::WText>(
            franchisee_.getDisplayName(), Wt::TextFormat::Plain
        ));
        storeName->setStyleClass("store-name");

        auto separator = franchiseeBanner->addWidget(std::make_unique<Wt::WText>(" | "));
        auto storeLocation = franchiseeBanner->addWidget(std::make_unique<Wt::WText>(
            franchisee_.getLocationDisplay(), Wt::TextFormat::Plain
        ));
        storeLocation->setStyleClass("store-location");

        auto changeBtn = franchiseeBanner->addWidget(std::make_unique<Wt::WPushButton>("Change"));
        changeBtn->setStyleClass("btn btn-outline btn-sm");
        changeBtn->clicked().connect([this] {
            showSetupPage();
            setInternalPath("/setup", true);
        });
    }

    // Two-column layout
    auto columnsContainer = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    columnsContainer->setStyleClass("search-columns");

    // Left column - Search panel
    auto leftColumn = columnsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    leftColumn->setStyleClass("search-column left");

    searchPanel_ = leftColumn->addWidget(std::make_unique<Widgets::SearchPanel>());
    searchPanel_->searchRequested().connect(this, &FranchiseApp::onSearchRequested);
    searchPanel_->searchCancelled().connect(this, &FranchiseApp::onSearchCancelled);

    // Pre-populate search panel with current search state or franchisee's location
    Models::SearchQuery defaultQuery;
    if (hasActiveSearch_ && !currentSearchLocation_.empty()) {
        // Use the current search state (synced from Open Street Map or previous search)
        defaultQuery.location = currentSearchLocation_;
        defaultQuery.latitude = currentSearchArea_.center.latitude;
        defaultQuery.longitude = currentSearchArea_.center.longitude;
        defaultQuery.radiusMiles = currentSearchArea_.radiusMiles;
        if (franchisee_.isConfigured) {
            defaultQuery.businessTypes = franchisee_.searchCriteria.businessTypes;
            defaultQuery.minEmployees = franchisee_.searchCriteria.minEmployees;
            defaultQuery.maxEmployees = franchisee_.searchCriteria.maxEmployees;
        }
        defaultQuery.includeOpenStreetMap = true;
        searchPanel_->setSearchQuery(defaultQuery);
    } else if (franchisee_.isConfigured && franchisee_.hasValidLocation()) {
        // Use franchisee location as default
        defaultQuery.location = franchisee_.getFullAddress();
        defaultQuery.latitude = franchisee_.location.latitude;
        defaultQuery.longitude = franchisee_.location.longitude;
        defaultQuery.radiusMiles = franchisee_.searchCriteria.radiusMiles;
        defaultQuery.businessTypes = franchisee_.searchCriteria.businessTypes;
        defaultQuery.minEmployees = franchisee_.searchCriteria.minEmployees;
        defaultQuery.maxEmployees = franchisee_.searchCriteria.maxEmployees;
        defaultQuery.includeOpenStreetMap = franchisee_.searchCriteria.includeOpenStreetMap;
        searchPanel_->setSearchQuery(defaultQuery);
    }

    // Right column - Results display
    auto rightColumn = columnsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    rightColumn->setStyleClass("search-column right");

    resultsDisplay_ = rightColumn->addWidget(std::make_unique<Widgets::ResultsDisplay>());
    resultsDisplay_->viewDetailsRequested().connect(this, &FranchiseApp::onViewDetails);
    resultsDisplay_->addToProspectsRequested().connect(this, &FranchiseApp::onAddToProspects);
    resultsDisplay_->exportRequested().connect(this, &FranchiseApp::onExportResults);

    // Connect bulk add signal for multi-select
    resultsDisplay_->addSelectedRequested().connect([this](const std::vector<std::string>& ids) {
        onAddSelectedToProspects(ids);
    });

    // Restore previous search results if they exist
    if (hasActiveSearch_ && !lastResults_.items.empty()) {
        resultsDisplay_->showResults(lastResults_);
    }
}

void FranchiseApp::showProspectsPage() {
    workArea_->clear();
    navigation_->setPageTitle("My Prospects");
    navigation_->setBreadcrumbs({"Home", "Prospects"});
    navigation_->setMarketScore(-1);

    auto container = workArea_->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setStyleClass("page-container prospects-page");

    auto header = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->setStyleClass("page-header");

    // Tagline - show count if there are saved prospects, otherwise show welcome message
    if (!savedProspects_.empty()) {
        auto tagline = header->addWidget(std::make_unique<Wt::WText>(
            std::to_string(savedProspects_.size()) + " saved prospect" +
            (savedProspects_.size() == 1 ? "" : "s") + " ready for outreach"
        ));
        tagline->setStyleClass("page-tagline");
    } else {
        auto tagline = header->addWidget(std::make_unique<Wt::WText>(
            "Save prospects from your searches to track and manage them here"
        ));
        tagline->setStyleClass("page-tagline");
    }

    if (savedProspects_.empty()) {
        // Show placeholder when no prospects saved
        auto placeholder = container->addWidget(std::make_unique<Wt::WContainerWidget>());
        placeholder->setStyleClass("placeholder-content");

        auto icon = placeholder->addWidget(std::make_unique<Wt::WText>("👥"));
        icon->setStyleClass("placeholder-icon");

        auto text = placeholder->addWidget(std::make_unique<Wt::WText>(
            "Your saved prospects will appear here. Start an AI Search to find new prospects."
        ));
        text->setStyleClass("placeholder-text");

        auto btn = placeholder->addWidget(std::make_unique<Wt::WPushButton>("Start AI Search"));
        btn->setStyleClass("btn btn-primary");
        btn->clicked().connect([this] {
            onMenuItemSelected("ai-search");
        });
    } else {
        // Show saved prospects list
        auto prospectsList = container->addWidget(std::make_unique<Wt::WContainerWidget>());
        prospectsList->setStyleClass("prospects-list");

        for (size_t i = 0; i < savedProspects_.size(); ++i) {
            const auto& prospect = savedProspects_[i];

            auto card = prospectsList->addWidget(std::make_unique<Wt::WContainerWidget>());
            card->setStyleClass("prospect-card");

            // === CARD HEADER: Icon, Name, Address, Score ===
            auto cardHeader = card->addWidget(std::make_unique<Wt::WContainerWidget>());
            cardHeader->setStyleClass("prospect-card-header");

            // Left side of header: Icon + Name/Address
            auto headerLeft = cardHeader->addWidget(std::make_unique<Wt::WContainerWidget>());
            headerLeft->setStyleClass("header-left");

            // Business type icon (greyscale)
            std::string businessIcon = "🏢"; // Default: Corporate Office
            if (prospect.business) {
                switch (prospect.business->type) {
                    case Models::BusinessType::CORPORATE_OFFICE: businessIcon = "🏢"; break;
                    case Models::BusinessType::WAREHOUSE: businessIcon = "🏭"; break;
                    case Models::BusinessType::CONFERENCE_CENTER: businessIcon = "🏛️"; break;
                    case Models::BusinessType::HOTEL: businessIcon = "🏨"; break;
                    case Models::BusinessType::COWORKING_SPACE: businessIcon = "💼"; break;
                    case Models::BusinessType::MEDICAL_FACILITY: businessIcon = "🏥"; break;
                    case Models::BusinessType::EDUCATIONAL_INSTITUTION: businessIcon = "🎓"; break;
                    case Models::BusinessType::GOVERNMENT_OFFICE: businessIcon = "🏛️"; break;
                    case Models::BusinessType::MANUFACTURING: businessIcon = "⚙️"; break;
                    case Models::BusinessType::TECH_COMPANY: businessIcon = "💻"; break;
                    case Models::BusinessType::FINANCIAL_SERVICES: businessIcon = "🏦"; break;
                    case Models::BusinessType::LAW_FIRM: businessIcon = "⚖️"; break;
                    case Models::BusinessType::NONPROFIT: businessIcon = "❤️"; break;
                    default: businessIcon = "🏢"; break;
                }
            }

            auto iconContainer = headerLeft->addWidget(std::make_unique<Wt::WContainerWidget>());
            iconContainer->setStyleClass("prospect-type-icon");
            auto icon = iconContainer->addWidget(std::make_unique<Wt::WText>(businessIcon));
            icon->setStyleClass("type-icon-emoji");

            // Name and address container
            auto nameAddressContainer = headerLeft->addWidget(std::make_unique<Wt::WContainerWidget>());
            nameAddressContainer->setStyleClass("name-address-container");

            auto nameText = nameAddressContainer->addWidget(std::make_unique<Wt::WText>(prospect.getTitle()));
            nameText->setStyleClass("prospect-name");

            if (prospect.business) {
                std::string fullAddress = prospect.business->address.getFullAddress();
                if (!fullAddress.empty() && !prospect.business->address.street1.empty()) {
                    auto addressText = nameAddressContainer->addWidget(std::make_unique<Wt::WText>(fullAddress));
                    addressText->setStyleClass("prospect-address");
                }
            }

            // Right side of header: AI Score bubble
            auto headerRight = cardHeader->addWidget(std::make_unique<Wt::WContainerWidget>());
            headerRight->setStyleClass("header-right");

            int optimizedScore = prospect.overallScore;
            int originalScore = static_cast<int>((prospect.aiConfidenceScore > 0 ? prospect.aiConfidenceScore : prospect.relevanceScore) * 100);
            if (originalScore == 0) originalScore = optimizedScore;

            std::string scoreBubbleClass = "score-bubble clickable";
            if (optimizedScore >= 80) {
                scoreBubbleClass += " score-high";
            } else if (optimizedScore >= 60) {
                scoreBubbleClass += " score-medium";
            } else if (optimizedScore >= 40) {
                scoreBubbleClass += " score-low";
            } else {
                scoreBubbleClass += " score-very-low";
            }

            auto scoreContainer = headerRight->addWidget(std::make_unique<Wt::WContainerWidget>());
            scoreContainer->setStyleClass("score-icon-container clickable-score");

            auto scoreBubble = scoreContainer->addWidget(std::make_unique<Wt::WText>(std::to_string(optimizedScore)));
            scoreBubble->setStyleClass(scoreBubbleClass);

            auto scoreLabel = scoreContainer->addWidget(std::make_unique<Wt::WText>("AI Score"));
            scoreLabel->setStyleClass("score-icon-label");

            // Score details popover (hidden by default)
            auto scorePopover = scoreContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
            scorePopover->setStyleClass("score-popover hidden");

            auto popoverTitle = scorePopover->addWidget(std::make_unique<Wt::WText>("Score Details"));
            popoverTitle->setStyleClass("popover-title");

            auto scoresRow = scorePopover->addWidget(std::make_unique<Wt::WContainerWidget>());
            scoresRow->setStyleClass("scores-comparison");

            auto optimizedLabel = scoresRow->addWidget(std::make_unique<Wt::WText>(
                "Optimized: " + std::to_string(optimizedScore) + "%"
            ));
            optimizedLabel->setStyleClass("score-detail optimized");

            auto originalLabel = scoresRow->addWidget(std::make_unique<Wt::WText>(
                "Original: " + std::to_string(originalScore) + "%"
            ));
            originalLabel->setStyleClass("score-detail original");

            if (optimizedScore != originalScore) {
                auto rulesExplanation = scorePopover->addWidget(std::make_unique<Wt::WContainerWidget>());
                rulesExplanation->setStyleClass("rules-explanation");

                auto rulesTitle = rulesExplanation->addWidget(std::make_unique<Wt::WText>("Applied Rules:"));
                rulesTitle->setStyleClass("rules-title");

                std::string explanation;
                int scoreDiff = optimizedScore - originalScore;
                if (scoreDiff > 0) {
                    explanation = "Score increased by " + std::to_string(scoreDiff) + " points due to: ";
                    if (prospect.business) {
                        if (prospect.business->employeeCount >= 100) explanation += "Large workforce (+10), ";
                        if (prospect.business->hasConferenceRoom) explanation += "Conference facilities (+5), ";
                        if (prospect.business->hasEventSpace) explanation += "Event space (+5), ";
                        if (prospect.business->bbbAccredited) explanation += "BBB accreditation (+3), ";
                        if (prospect.business->googleRating >= 4.5) explanation += "High rating (+5), ";
                    }
                    if (explanation.length() > 2 && explanation.substr(explanation.length() - 2) == ", ") {
                        explanation = explanation.substr(0, explanation.length() - 2);
                    }
                } else {
                    explanation = "Score adjusted by " + std::to_string(scoreDiff) + " points based on market conditions.";
                }

                auto rulesText = rulesExplanation->addWidget(std::make_unique<Wt::WText>(explanation));
                rulesText->setStyleClass("rules-text");
            }

            scoreContainer->clicked().connect([scorePopover] {
                std::string currentClass = scorePopover->styleClass().toUTF8();
                if (currentClass.find("hidden") != std::string::npos) {
                    scorePopover->setStyleClass("score-popover visible");
                } else {
                    scorePopover->setStyleClass("score-popover hidden");
                }
            });

            // === DIVIDING LINE ===
            auto divider = card->addWidget(std::make_unique<Wt::WContainerWidget>());
            divider->setStyleClass("card-divider");

            // === CARD BODY: Single column layout ===
            auto cardBody = card->addWidget(std::make_unique<Wt::WContainerWidget>());
            cardBody->setStyleClass("prospect-card-body");

            // === Demographics + Data Sources + Recommended Actions ===
            if (prospect.business) {
                // Demographics section
                auto demographicsSection = cardBody->addWidget(std::make_unique<Wt::WContainerWidget>());
                demographicsSection->setStyleClass("card-section demographics-section");

                auto demoHeader = demographicsSection->addWidget(std::make_unique<Wt::WContainerWidget>());
                demoHeader->setStyleClass("section-header");

                auto demoIcon = demoHeader->addWidget(std::make_unique<Wt::WText>("👥"));
                demoIcon->setStyleClass("section-icon");

                auto demoLabel = demoHeader->addWidget(std::make_unique<Wt::WText>("Demographics"));
                demoLabel->setStyleClass("section-label");

                // Stat badges container
                auto statsContainer = demographicsSection->addWidget(std::make_unique<Wt::WContainerWidget>());
                statsContainer->setStyleClass("prospect-stats");

                // Business type badge
                auto typeBadge = statsContainer->addWidget(std::make_unique<Wt::WText>(
                    prospect.business->getBusinessTypeString()
                ));
                typeBadge->setStyleClass("stat-badge stat-type");

                // Employee count badge
                int empCount = prospect.business->employeeCount;
                std::string empClass = "stat-badge stat-employees";
                if (empCount >= 100) empClass += " level-high";
                else if (empCount >= 50) empClass += " level-medium";
                else empClass += " level-low";

                auto empBadge = statsContainer->addWidget(std::make_unique<Wt::WText>(
                    std::to_string(empCount) + " employees"
                ));
                empBadge->setStyleClass(empClass);

                // Google rating badge
                if (prospect.business->googleRating > 0) {
                    std::ostringstream ratingStr;
                    ratingStr.precision(1);
                    ratingStr << std::fixed << prospect.business->googleRating;

                    std::string ratingClass = "stat-badge stat-rating";
                    if (prospect.business->googleRating >= 4.5) ratingClass += " level-high";
                    else if (prospect.business->googleRating >= 3.5) ratingClass += " level-medium";
                    else ratingClass += " level-low";

                    auto ratingBadge = statsContainer->addWidget(std::make_unique<Wt::WText>(
                        ratingStr.str() + " rating"
                    ));
                    ratingBadge->setStyleClass(ratingClass);
                }

                // Feature badges
                if (prospect.business->hasConferenceRoom) {
                    auto confBadge = statsContainer->addWidget(std::make_unique<Wt::WText>("Conference Room"));
                    confBadge->setStyleClass("stat-badge stat-feature");
                }

                if (prospect.business->hasEventSpace) {
                    auto eventBadge = statsContainer->addWidget(std::make_unique<Wt::WText>("Event Space"));
                    eventBadge->setStyleClass("stat-badge stat-feature");
                }

                if (prospect.business->bbbAccredited) {
                    auto bbbBadge = statsContainer->addWidget(std::make_unique<Wt::WText>("BBB Accredited"));
                    bbbBadge->setStyleClass("stat-badge stat-verified");
                }

                // Data Sources section
                auto sourcesSection = cardBody->addWidget(std::make_unique<Wt::WContainerWidget>());
                sourcesSection->setStyleClass("card-section sources-section");

                auto sourcesHeader = sourcesSection->addWidget(std::make_unique<Wt::WContainerWidget>());
                sourcesHeader->setStyleClass("section-header");

                auto sourcesIcon = sourcesHeader->addWidget(std::make_unique<Wt::WText>("📊"));
                sourcesIcon->setStyleClass("section-icon");

                auto sourcesLabel = sourcesHeader->addWidget(std::make_unique<Wt::WText>("Data Sources"));
                sourcesLabel->setStyleClass("section-label");

                auto sourceBadgesContainer = sourcesSection->addWidget(std::make_unique<Wt::WContainerWidget>());
                sourceBadgesContainer->setStyleClass("source-badges");

                if (!prospect.sources.empty()) {
                    for (const auto& source : prospect.sources) {
                        auto sourceBadge = sourceBadgesContainer->addWidget(std::make_unique<Wt::WText>(
                            Models::dataSourceToString(source)
                        ));
                        sourceBadge->setStyleClass("source-badge");
                    }
                } else if (prospect.business->source != Models::DataSource::IMPORTED) {
                    auto sourceBadge = sourceBadgesContainer->addWidget(std::make_unique<Wt::WText>(
                        Models::dataSourceToString(prospect.business->source)
                    ));
                    sourceBadge->setStyleClass("source-badge");
                }
            }

            // AI Summary (if available)
            if (!prospect.aiSummary.empty()) {
                auto summaryContainer = cardBody->addWidget(std::make_unique<Wt::WContainerWidget>());
                summaryContainer->setStyleClass("prospect-summary");

                auto summaryLabel = summaryContainer->addWidget(std::make_unique<Wt::WText>("AI Analysis:"));
                summaryLabel->setStyleClass("summary-label");

                auto summaryText = summaryContainer->addWidget(std::make_unique<Wt::WText>(
                    prospect.aiSummary
                ));
                summaryText->setStyleClass("summary-text");
            }

            // Key highlights
            if (!prospect.keyHighlights.empty()) {
                auto highlightsContainer = cardBody->addWidget(std::make_unique<Wt::WContainerWidget>());
                highlightsContainer->setStyleClass("prospect-highlights");

                for (const auto& highlight : prospect.keyHighlights) {
                    auto highlightText = highlightsContainer->addWidget(std::make_unique<Wt::WText>(
                        "• " + highlight
                    ));
                    highlightText->setStyleClass("highlight-item");
                }
            }

            // Recommended actions (collapsible with triangle) - under Data Sources
            if (!prospect.recommendedActions.empty()) {
                auto recActionsContainer = cardBody->addWidget(std::make_unique<Wt::WContainerWidget>());
                recActionsContainer->setStyleClass("prospect-recommended-actions");

                // Header with toggle triangle
                auto recActionsHeader = recActionsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
                recActionsHeader->setStyleClass("recommended-actions-header");

                auto triangle = recActionsHeader->addWidget(std::make_unique<Wt::WText>("▶"));
                triangle->setStyleClass("toggle-triangle");

                auto recActionsLabel = recActionsHeader->addWidget(std::make_unique<Wt::WText>("Recommended Actions"));
                recActionsLabel->setStyleClass("recommended-actions-label");

                auto actionCount = recActionsHeader->addWidget(std::make_unique<Wt::WText>(
                    "(" + std::to_string(prospect.recommendedActions.size()) + ")"
                ));
                actionCount->setStyleClass("actions-count");

                // Collapsible actions list (hidden by default)
                auto recActionsList = recActionsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
                recActionsList->setStyleClass("recommended-actions-list collapsed");

                int actionNum = 1;
                for (const auto& action : prospect.recommendedActions) {
                    auto actionItem = recActionsList->addWidget(std::make_unique<Wt::WContainerWidget>());
                    actionItem->setStyleClass("recommended-action-item");

                    auto actionNumber = actionItem->addWidget(std::make_unique<Wt::WText>(std::to_string(actionNum++) + ". "));
                    actionNumber->setStyleClass("action-number");

                    auto actionText = actionItem->addWidget(std::make_unique<Wt::WText>(action));
                    actionText->setStyleClass("action-text");
                }

                // Toggle click handler
                recActionsHeader->clicked().connect([triangle, recActionsList] {
                    std::string currentClass = recActionsList->styleClass().toUTF8();
                    if (currentClass.find("collapsed") != std::string::npos) {
                        recActionsList->setStyleClass("recommended-actions-list");
                        triangle->setText("▼");
                    } else {
                        recActionsList->setStyleClass("recommended-actions-list collapsed");
                        triangle->setText("▶");
                    }
                });
            }

            // Actions
            auto actionsContainer = card->addWidget(std::make_unique<Wt::WContainerWidget>());
            actionsContainer->setStyleClass("prospect-actions");

            auto removeBtn = actionsContainer->addWidget(std::make_unique<Wt::WPushButton>("Remove"));
            removeBtn->setStyleClass("btn btn-outline btn-sm");

            // Capture index and ID for removal
            size_t prospectIndex = i;
            std::string prospectId = prospect.id;
            removeBtn->clicked().connect([this, prospectIndex, prospectId] {
                if (prospectIndex < savedProspects_.size()) {
                    // Delete from API server
                    deleteProspectFromALS(prospectId);
                    // Remove from in-memory list
                    savedProspects_.erase(savedProspects_.begin() + prospectIndex);
                    showProspectsPage();  // Refresh the page
                }
            });
        }

        // Add search button at bottom
        auto actionsSection = container->addWidget(std::make_unique<Wt::WContainerWidget>());
        actionsSection->setStyleClass("prospects-actions");

        auto searchBtn = actionsSection->addWidget(std::make_unique<Wt::WPushButton>("Find More Prospects"));
        searchBtn->setStyleClass("btn btn-primary");
        searchBtn->clicked().connect([this] {
            onMenuItemSelected("ai-search");
        });
    }
}

void FranchiseApp::showOpenStreetMapPage() {
    workArea_->clear();
    navigation_->setPageTitle("Open Street Map");
    navigation_->setBreadcrumbs({"Home", "Open Street Map"});

    auto container = workArea_->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setStyleClass("page-container openstreetmap-page");

    // Pre-fill with current search location or franchisee location
    std::string defaultLocation = "Denver, CO";
    double defaultRadiusKm = 10.0;
    Models::SearchArea initialSearchArea;

    if (hasActiveSearch_ && currentSearchArea_.center.hasValidCoordinates()) {
        defaultLocation = currentSearchLocation_;
        defaultRadiusKm = currentSearchArea_.radiusKm;
        initialSearchArea = currentSearchArea_;
    } else if (franchisee_.isConfigured && franchisee_.hasValidLocation()) {
        defaultLocation = franchisee_.getFullAddress();
        defaultRadiusKm = franchisee_.searchCriteria.radiusMiles * 1.60934;
        initialSearchArea = Models::SearchArea::fromMiles(
            franchisee_.location,
            franchisee_.searchCriteria.radiusMiles
        );
    } else {
        Models::GeoLocation denverLocation(39.7392, -104.9903, "Denver", "CO");
        initialSearchArea = Models::SearchArea(denverLocation, 10.0);
    }

    // Get initial stats
    auto& osmAPI = searchService_->getOSMAPI();
    auto stats = osmAPI.getAreaStatisticsSync(initialSearchArea);

    // Set market score in navigation header
    navigation_->setMarketScore(stats.marketPotentialScore);

    // Store for updates
    auto currentSearchAreaPtr = std::make_shared<Models::SearchArea>(initialSearchArea);

    // Map with sidebar layout
    auto mapWithSidebar = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    mapWithSidebar->setStyleClass("map-with-sidebar");

    // Map container (left side) with location overlay
    auto mapContainer = mapWithSidebar->addWidget(std::make_unique<Wt::WContainerWidget>());
    mapContainer->setStyleClass("map-container");

    // Location input as overlay (like browser address bar)
    auto locationOverlay = mapContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    locationOverlay->setStyleClass("location-overlay");

    auto locationInput = locationOverlay->addWidget(std::make_unique<Wt::WLineEdit>());
    locationInput->setPlaceholderText("Enter city, address, or location...");
    locationInput->setStyleClass("form-control location-input-overlay");
    locationInput->setText(defaultLocation);

    // Create map div with unique ID
    auto mapDiv = mapContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    mapDiv->setStyleClass("osm-map");
    std::string mapId = mapDiv->id();

    // Store coordinates for JavaScript
    double initLat = initialSearchArea.center.latitude;
    double initLon = initialSearchArea.center.longitude;

    // Build franchisee info for popup (sanitize for JavaScript)
    std::string franchiseeName = franchisee_.storeName.empty() ? "My Franchise" : franchisee_.storeName;
    std::string franchiseeAddress = franchisee_.address;
    std::string franchiseeCity = franchisee_.location.city;
    std::string franchiseeState = franchisee_.location.state;
    std::string franchiseePhone = franchisee_.phone;
    std::string franchiseeEmail = franchisee_.email;

    // Sanitize strings for JavaScript
    auto sanitizeJs = [](std::string& str) {
        for (auto& c : str) {
            if (c == '\'' || c == '"' || c == '\\' || c == '\n' || c == '\r') c = ' ';
        }
    };
    sanitizeJs(franchiseeName);
    sanitizeJs(franchiseeAddress);
    sanitizeJs(franchiseeCity);
    sanitizeJs(franchiseeState);
    sanitizeJs(franchiseePhone);
    sanitizeJs(franchiseeEmail);

    // Build popup content
    std::ostringstream popupContent;
    popupContent << "<div style=\"min-width: 200px;\">"
                 << "<b style=\"font-size: 14px; color: #c41e3a;\">" << franchiseeName << "</b><br>";
    if (!franchiseeAddress.empty()) {
        popupContent << "<span style=\"color: #666;\">" << franchiseeAddress << "</span><br>";
    }
    if (!franchiseeCity.empty() || !franchiseeState.empty()) {
        popupContent << "<span style=\"color: #666;\">" << franchiseeCity;
        if (!franchiseeCity.empty() && !franchiseeState.empty()) popupContent << ", ";
        popupContent << franchiseeState << "</span><br>";
    }
    if (!franchiseePhone.empty()) {
        popupContent << "<br><span style=\"color: #333;\">📞 " << franchiseePhone << "</span><br>";
    }
    if (!franchiseeEmail.empty()) {
        popupContent << "<span style=\"color: #333;\">✉️ " << franchiseeEmail << "</span>";
    }
    popupContent << "</div>";

    // Initialize Leaflet map via JavaScript - load from CDN
    std::ostringstream initMapJs;
    initMapJs << "(function() {"
              // Load Leaflet CSS from CDN
              << "  if (!document.getElementById('leaflet-css')) {"
              << "    var link = document.createElement('link');"
              << "    link.id = 'leaflet-css';"
              << "    link.rel = 'stylesheet';"
              << "    link.href = 'https://unpkg.com/leaflet@1.9.4/dist/leaflet.css';"
              << "    link.integrity = 'sha256-p4NxAoJBhIIN+hmNHrzRCf9tD/miZyoHS5obTRR9BMY=';"
              << "    link.crossOrigin = '';"
              << "    document.head.appendChild(link);"
              << "  }"
              // Load Leaflet JS from CDN
              << "  function loadLeafletJS(callback) {"
              << "    if (typeof L !== 'undefined') { callback(); return; }"
              << "    if (document.getElementById('leaflet-js')) { setTimeout(function() { loadLeafletJS(callback); }, 100); return; }"
              << "    var script = document.createElement('script');"
              << "    script.id = 'leaflet-js';"
              << "    script.src = 'https://unpkg.com/leaflet@1.9.4/dist/leaflet.js';"
              << "    script.integrity = 'sha256-20nQCchB9co0qIjJZRGuk2/Z9VM+kNiyxNV1lvTlZBo=';"
              << "    script.crossOrigin = '';"
              << "    script.onload = callback;"
              << "    document.head.appendChild(script);"
              << "  }"
              // Define init function
              << "  function initOSMMap() {"
              << "    var mapEl = document.getElementById('" << mapId << "');"
              << "    if (!mapEl) { setTimeout(initOSMMap, 100); return; }"
              << "    if (mapEl._leaflet_map) return;"
              << "    if (typeof L === 'undefined') { setTimeout(initOSMMap, 100); return; }"
              // Create the map
              << "    try {"
              << "      var map = L.map('" << mapId << "').setView([" << initLat << ", " << initLon << "], 13);"
              << "      L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {"
              << "        maxZoom: 19,"
              << "        attribution: '&copy; <a href=\"https://www.openstreetmap.org/copyright\">OpenStreetMap</a> contributors'"
              << "      }).addTo(map);"
              << "      mapEl._leaflet_map = map;"
              << "      window.osmMap = map;"
              // Add red pin marker for franchisee location
              << "      var redIcon = L.divIcon({"
              << "        className: 'franchisee-marker',"
              << "        html: '<div style=\"position: relative;\">"
              << "<div style=\"width: 30px; height: 30px; background: linear-gradient(135deg, #ff4444 0%, #cc0000 100%); "
              << "border-radius: 50% 50% 50% 0; transform: rotate(-45deg); "
              << "border: 3px solid #fff; box-shadow: 0 3px 8px rgba(0,0,0,0.4);\">"
              << "</div>"
              << "<div style=\"position: absolute; top: 6px; left: 9px; width: 12px; height: 12px; "
              << "background: #fff; border-radius: 50%; transform: rotate(45deg);\"></div>"
              << "</div>',"
              << "        iconSize: [30, 30],"
              << "        iconAnchor: [15, 30],"
              << "        popupAnchor: [0, -30]"
              << "      });"
              << "      var franchiseeMarker = L.marker([" << initLat << ", " << initLon << "], {icon: redIcon})"
              << "        .addTo(map)"
              << "        .bindPopup('" << popupContent.str() << "');"
              << "      window.osmFranchiseeMarker = franchiseeMarker;"
              << "      setTimeout(function() { map.invalidateSize(); }, 300);"
              << "    } catch(e) { console.error('Map init error:', e); }"
              << "  }"
              << "  loadLeafletJS(function() { setTimeout(initOSMMap, 100); });"
              << "})();";

    doJavaScript(initMapJs.str());

    // Synchronize AI Search prospects to the map
    if (!lastResults_.items.empty()) {
        std::ostringstream addProspectsJs;
        addProspectsJs << "(function() {"
                      << "  function addProspectMarkers() {"
                      << "    if (typeof L === 'undefined' || !window.osmMap) {"
                      << "      setTimeout(addProspectMarkers, 200);"
                      << "      return;"
                      << "    }"
                      << "    var map = window.osmMap;"
                      << "    if (!window.prospectMarkers) window.prospectMarkers = [];"
                      // Clear existing prospect markers
                      << "    window.prospectMarkers.forEach(function(m) { map.removeLayer(m); });"
                      << "    window.prospectMarkers = [];"
                      // Define prospect marker icon (blue/purple)
                      << "    var prospectIcon = L.divIcon({"
                      << "      className: 'prospect-marker',"
                      << "      html: '<div style=\"width: 24px; height: 24px; background: linear-gradient(135deg, #6366f1 0%, #4f46e5 100%); "
                      << "border-radius: 50%; border: 2px solid #fff; box-shadow: 0 2px 6px rgba(0,0,0,0.3); "
                      << "display: flex; align-items: center; justify-content: center; color: #fff; font-size: 11px; font-weight: bold;\">"
                      << "</div>',"
                      << "      iconSize: [24, 24],"
                      << "      iconAnchor: [12, 12],"
                      << "      popupAnchor: [0, -12]"
                      << "    });"
                      // Function to create score-based icon
                      << "    function getScoreIcon(score) {"
                      << "      var color = score >= 80 ? '#22c55e' : score >= 60 ? '#3b82f6' : score >= 40 ? '#f59e0b' : '#94a3b8';"
                      << "      return L.divIcon({"
                      << "        className: 'prospect-marker',"
                      << "        html: '<div style=\"width: 28px; height: 28px; background: ' + color + '; "
                      << "border-radius: 50%; border: 2px solid #fff; box-shadow: 0 2px 6px rgba(0,0,0,0.3); "
                      << "display: flex; align-items: center; justify-content: center; color: #fff; font-size: 11px; font-weight: bold;\">' + score + '</div>',"
                      << "        iconSize: [28, 28],"
                      << "        iconAnchor: [14, 14],"
                      << "        popupAnchor: [0, -14]"
                      << "      });"
                      << "    }";

        // Add markers for each prospect
        for (const auto& item : lastResults_.items) {
            // Check if item has business data with valid coordinates
            if (item.business &&
                item.business->address.latitude != 0.0 &&
                item.business->address.longitude != 0.0) {
                std::string name = item.getTitle();
                std::string address = item.getSubtitle();
                int score = item.overallScore;

                // Sanitize strings for JavaScript
                for (auto& c : name) {
                    if (c == '\'' || c == '"' || c == '\\' || c == '\n' || c == '\r') c = ' ';
                }
                for (auto& c : address) {
                    if (c == '\'' || c == '"' || c == '\\' || c == '\n' || c == '\r') c = ' ';
                }

                addProspectsJs << "    var marker = L.marker(["
                              << item.business->address.latitude << ", " << item.business->address.longitude
                              << "], {icon: getScoreIcon(" << score << ")})"
                              << ".addTo(map)"
                              << ".bindPopup('<div style=\"min-width: 180px;\"><b>" << name << "</b><br>"
                              << "<span style=\"color: #666;\">" << address << "</span><br>"
                              << "<span style=\"font-weight: bold; color: "
                              << (score >= 80 ? "#22c55e" : score >= 60 ? "#3b82f6" : score >= 40 ? "#f59e0b" : "#94a3b8")
                              << ";\">Score: " << score << "</span></div>');"
                              << "    window.prospectMarkers.push(marker);";
            }
        }

        addProspectsJs << "    console.log('Added ' + window.prospectMarkers.length + ' prospect markers to map');"
                      << "  }"
                      << "  setTimeout(addProspectMarkers, 500);"
                      << "})();";

        doJavaScript(addProspectsJs.str());
    }

    // Categories sidebar (right side of map)
    auto mapSidebar = mapWithSidebar->addWidget(std::make_unique<Wt::WContainerWidget>());
    mapSidebar->setStyleClass("map-sidebar");

    auto sidebarHeader = mapSidebar->addWidget(std::make_unique<Wt::WContainerWidget>());
    sidebarHeader->setStyleClass("map-sidebar-header");

    auto categoriesTitle = sidebarHeader->addWidget(std::make_unique<Wt::WText>("Categories"));
    categoriesTitle->setStyleClass("stat-title");

    // Category selector dropdown
    auto categorySelector = mapSidebar->addWidget(std::make_unique<Wt::WContainerWidget>());
    categorySelector->setStyleClass("category-selector");

    auto selectorLabel = categorySelector->addWidget(std::make_unique<Wt::WText>("Add category to view"));
    selectorLabel->setStyleClass("category-selector-label");

    auto categoryDropdown = categorySelector->addWidget(std::make_unique<Wt::WComboBox>());
    categoryDropdown->setStyleClass("category-dropdown");
    categoryDropdown->addItem("-- Select a category --");

    // Category data: display name, api name, count (shared_ptr for callback capture)
    auto categories = std::make_shared<std::vector<std::tuple<std::string, std::string, int>>>(std::vector<std::tuple<std::string, std::string, int>>{
        {"Offices", "offices", stats.offices},
        {"Hotels", "hotels", stats.hotels},
        {"Conference Venues", "conference", stats.conferenceVenues},
        {"Restaurants", "restaurants", stats.restaurants},
        {"Cafes", "cafes", stats.cafes},
        {"Hospitals", "hospitals", stats.hospitals},
        {"Universities", "universities", stats.universities},
        {"Schools", "schools", stats.schools},
        {"Industrial", "industrial", stats.industrialBuildings},
        {"Warehouses", "warehouses", stats.warehouses},
        {"Banks", "banks", stats.banks},
        {"Government", "government", stats.governmentBuildings}
    });

    // Add categories to dropdown
    for (const auto& [displayName, apiName, count] : *categories) {
        categoryDropdown->addItem(displayName + " (" + std::to_string(count) + ")");
    }

    // Pill tray content area
    auto sidebarContent = mapSidebar->addWidget(std::make_unique<Wt::WContainerWidget>());
    sidebarContent->setStyleClass("map-sidebar-content");

    auto pillTray = sidebarContent->addWidget(std::make_unique<Wt::WContainerWidget>());
    pillTray->setStyleClass("category-pill-tray");

    // Empty state message
    auto emptyMessage = pillTray->addWidget(std::make_unique<Wt::WText>("Select categories from the dropdown above to view POIs on the map"));
    emptyMessage->setStyleClass("category-pill-tray-empty");

    // Shared state for active category pills
    struct CategoryPillData {
        std::string displayName;
        std::string apiName;
        int count;
        int poiLimit;
        std::string color;        // Soft pastel color for pill card
        std::string markerColor;  // Deeper vivid color for map markers
        Wt::WContainerWidget* pillWidget;
        Wt::WSlider* limitSlider;
        Wt::WText* limitValueText;
    };
    auto activePills = std::make_shared<std::vector<CategoryPillData>>();

    // Soft muted pastel colors for post-it note cards (less vivid)
    std::vector<std::string> pastelColors = {
        "#FFF5E6", // Soft Peach
        "#E8F5E9", // Soft Mint
        "#E3F2FD", // Soft Blue
        "#FFFDE7", // Soft Yellow
        "#FCE4EC", // Soft Pink
        "#F3E5F5", // Soft Purple
        "#E0F7FA", // Soft Teal
        "#FFF3E0", // Soft Orange
        "#EDE7F6", // Soft Lavender
        "#F1F8E9", // Soft Lime
        "#FFFEF0", // Soft Cream
        "#E8EAF6"  // Soft Indigo
    };

    // Deeper, more vivid marker colors (with black mixed in for depth)
    std::vector<std::string> markerColors = {
        "#CC8844", // Deep Peach/Amber
        "#2E7D32", // Deep Green
        "#1565C0", // Deep Blue
        "#F9A825", // Deep Yellow/Gold
        "#C2185B", // Deep Pink
        "#7B1FA2", // Deep Purple
        "#00838F", // Deep Teal
        "#E65100", // Deep Orange
        "#5E35B1", // Deep Lavender/Violet
        "#558B2F", // Deep Lime
        "#FF8F00", // Deep Amber
        "#303F9F"  // Deep Indigo
    };
    auto usedColorIndex = std::make_shared<int>(0);

    // Helper function to build rich popup HTML for a POI
    auto buildRichPopupHtml = [](const Services::OSMPoi& poi,
                                  const Models::BusinessInfo& bizInfo,
                                  const std::string& categoryName,
                                  const std::string& markerColor) -> std::string {
        std::ostringstream popup;

        // Sanitize strings for JavaScript
        auto sanitize = [](const std::string& str) -> std::string {
            std::string result = str;
            for (auto& c : result) {
                if (c == '\'' || c == '"' || c == '\\' || c == '\n' || c == '\r') c = ' ';
            }
            return result;
        };

        std::string safeName = sanitize(poi.name);
        std::string safeAddress = sanitize(poi.street.empty() ? "" :
            (poi.houseNumber.empty() ? poi.street : poi.houseNumber + " " + poi.street));
        std::string safeCity = sanitize(poi.city);
        std::string safeState = sanitize(poi.state);

        // Get score color based on catering potential
        std::string scoreColor;
        std::string scoreLabel;
        int score = bizInfo.cateringPotentialScore;
        if (score >= 70) {
            scoreColor = "#28a745";  // Green - High potential
            scoreLabel = "High";
        } else if (score >= 40) {
            scoreColor = "#ffc107";  // Yellow - Medium potential
            scoreLabel = "Medium";
        } else {
            scoreColor = "#6c757d";  // Gray - Low potential
            scoreLabel = "Low";
        }

        // Build popup HTML - simplified clean design with padding
        popup << "<div class=\"poi-popup\" style=\"min-width: 260px; max-width: 300px; padding: 12px; font-family: -apple-system, BlinkMacSystemFont, sans-serif;\">";

        // Header with name and category badge
        popup << "<div style=\"display: flex; justify-content: space-between; align-items: flex-start; margin-bottom: 8px;\">";
        popup << "<h4 style=\"margin: 0; font-size: 15px; color: #333; font-weight: 600;\">" << safeName << "</h4>";
        popup << "<span style=\"background: " << markerColor << "; color: #fff; padding: 2px 8px; border-radius: 12px; font-size: 10px; font-weight: 500; white-space: nowrap; margin-left: 8px;\">" << categoryName << "</span>";
        popup << "</div>";

        // Address
        if (!safeAddress.empty() || !safeCity.empty()) {
            popup << "<div style=\"color: #666; font-size: 12px; margin-bottom: 10px;\">";
            if (!safeAddress.empty()) popup << safeAddress;
            if (!safeAddress.empty() && !safeCity.empty()) popup << ", ";
            if (!safeCity.empty()) popup << safeCity;
            if (!safeState.empty()) popup << ", " << safeState;
            popup << "</div>";
        }

        // Scoring section - simplified inline
        popup << "<div style=\"display: flex; align-items: center; gap: 10px; margin-bottom: 10px;\">";
        popup << "<span style=\"font-size: 11px; color: #666;\">Potential:</span>";
        popup << "<div style=\"flex: 1; background: #e9ecef; border-radius: 4px; height: 6px; overflow: hidden;\">";
        popup << "<div style=\"background: " << scoreColor << "; width: " << score << "%; height: 100%;\"></div>";
        popup << "</div>";
        popup << "<span style=\"background: " << scoreColor << "; color: #fff; padding: 2px 8px; border-radius: 10px; font-size: 11px; font-weight: 600;\">" << score << "</span>";
        popup << "</div>";

        // Business details - simple inline
        popup << "<div style=\"font-size: 12px; color: #555; margin-bottom: 8px;\">";
        popup << "<strong>Type:</strong> " << bizInfo.getBusinessTypeString();
        if (bizInfo.employeeCount > 0) {
            popup << " &nbsp;•&nbsp; <strong>Size:</strong> " << bizInfo.employeeCount << " employees";
        } else if (bizInfo.estimatedEmployeesOnSite > 0) {
            popup << " &nbsp;•&nbsp; <strong>Size:</strong> ~" << bizInfo.estimatedEmployeesOnSite << " on-site";
        }
        popup << "</div>";

        // Features/Amenities row - simplified tags
        bool hasFeatures = bizInfo.hasConferenceRoom || bizInfo.hasEventSpace || bizInfo.regularMeetings || bizInfo.isVerified;
        if (hasFeatures) {
            popup << "<div style=\"display: flex; flex-wrap: wrap; gap: 4px; margin-bottom: 10px;\">";
            if (bizInfo.hasConferenceRoom) {
                popup << "<span style=\"background: #e3f2fd; color: #1565c0; padding: 2px 6px; border-radius: 3px; font-size: 10px;\">Conference Room</span>";
            }
            if (bizInfo.hasEventSpace) {
                popup << "<span style=\"background: #fce4ec; color: #c2185b; padding: 2px 6px; border-radius: 3px; font-size: 10px;\">Event Space</span>";
            }
            if (bizInfo.regularMeetings) {
                popup << "<span style=\"background: #e8f5e9; color: #2e7d32; padding: 2px 6px; border-radius: 3px; font-size: 10px;\">Regular Meetings</span>";
            }
            if (bizInfo.isVerified) {
                popup << "<span style=\"background: #fff3e0; color: #e65100; padding: 2px 6px; border-radius: 3px; font-size: 10px;\">Verified</span>";
            }
            popup << "</div>";
        }

        // Marketing insight - simplified
        popup << "<div style=\"font-size: 11px; color: #666; font-style: italic; border-top: 1px solid #eee; padding-top: 8px;\">";
        popup << "💡 ";

        // Generate contextual insight based on business type
        switch (bizInfo.type) {
            case Models::BusinessType::CORPORATE_OFFICE:
            case Models::BusinessType::TECH_COMPANY:
                popup << "Great for recurring lunch catering and team meetings. Target office managers for weekly orders.";
                break;
            case Models::BusinessType::CONFERENCE_CENTER:
            case Models::BusinessType::HOTEL:
                popup << "High-volume opportunity for events and conferences. Build relationship with event planners.";
                break;
            case Models::BusinessType::MEDICAL_FACILITY:
                popup << "Staff appreciation meals and medical conference catering. Regular scheduling potential.";
                break;
            case Models::BusinessType::EDUCATIONAL_INSTITUTION:
                popup << "Faculty meetings, graduation events, and parent nights. Seasonal opportunities.";
                break;
            case Models::BusinessType::WAREHOUSE:
            case Models::BusinessType::MANUFACTURING:
                popup << "Employee meal programs and shift catering. Volume discounts for large orders.";
                break;
            case Models::BusinessType::GOVERNMENT_OFFICE:
                popup << "Government contract potential. Public meetings and civic events.";
                break;
            case Models::BusinessType::FINANCIAL_SERVICES:
                popup << "Client meetings and quarterly reviews. Premium catering opportunities.";
                break;
            default:
                popup << "Potential catering client. Contact for needs assessment.";
                break;
        }
        popup << "</div>";

        // Contact info footer (if available) - simplified inline
        if (!poi.phone.empty() || !poi.website.empty() || !poi.email.empty()) {
            popup << "<div style=\"margin-top: 8px; font-size: 11px;\">";
            bool first = true;
            if (!poi.phone.empty()) {
                std::string safePhone = sanitize(poi.phone);
                popup << "<a href=\"tel:" << safePhone << "\" style=\"color: #1976d2; text-decoration: none;\">📞 " << safePhone << "</a>";
                first = false;
            }
            if (!poi.website.empty()) {
                std::string safeWebsite = sanitize(poi.website);
                if (!first) popup << " &nbsp;•&nbsp; ";
                popup << "<a href=\"" << safeWebsite << "\" target=\"_blank\" style=\"color: #1976d2; text-decoration: none;\">🌐 Website</a>";
                first = false;
            }
            if (!poi.email.empty()) {
                std::string safeEmail = sanitize(poi.email);
                if (!first) popup << " &nbsp;•&nbsp; ";
                popup << "<a href=\"mailto:" << safeEmail << "\" style=\"color: #1976d2; text-decoration: none;\">✉️ Email</a>";
            }
            popup << "</div>";
        }

        // Opening hours (if available)
        if (!poi.openingHours.empty()) {
            std::string safeHours = sanitize(poi.openingHours);
            popup << "<div style=\"margin-top: 6px; font-size: 10px; color: #888;\">🕐 " << safeHours << "</div>";
        }

        popup << "</div>";

        return popup.str();
    };

    // Function to refresh all POI markers
    auto refreshMarkers = std::make_shared<std::function<void()>>();
    *refreshMarkers = [this, activePills, currentSearchAreaPtr, buildRichPopupHtml]() {
        // Clear existing markers
        std::ostringstream clearMarkersJs;
        clearMarkersJs << "if (window.osmMarkers) {"
                      << "  window.osmMarkers.forEach(function(m) { m.remove(); });"
                      << "}"
                      << "window.osmMarkers = [];";
        doJavaScript(clearMarkersJs.str());

        // Add markers for each active category
        for (auto& pill : *activePills) {
            // Read current slider value directly
            int currentLimit = pill.limitSlider ? pill.limitSlider->value() : pill.poiLimit;

            auto& osmAPI = searchService_->getOSMAPI();
            auto pois = osmAPI.searchByCategorySync(*currentSearchAreaPtr, pill.apiName);

            int markerCount = 0;
            for (const auto& poi : pois) {
                if (markerCount >= currentLimit) break;

                // Convert POI to BusinessInfo for scoring and insights
                Models::BusinessInfo bizInfo = osmAPI.poiToBusinessInfo(poi);

                // Build rich popup HTML
                std::string popupHtml = buildRichPopupHtml(poi, bizInfo, pill.displayName, pill.markerColor);

                // Escape the popup HTML for JavaScript string
                std::string escapedPopup;
                for (char c : popupHtml) {
                    if (c == '\'') escapedPopup += "\\'";
                    else if (c == '\n') escapedPopup += " ";
                    else if (c == '\r') continue;
                    else escapedPopup += c;
                }

                // Create colored circle marker with deep vivid color
                std::ostringstream addMarkerJs;
                addMarkerJs << "if (window.osmMap && typeof L !== 'undefined') {"
                           << "  var markerIcon = L.divIcon({"
                           << "    className: 'custom-marker',"
                           << "    html: '<div style=\"background-color: " << pill.markerColor << "; "
                           << "      width: 22px; height: 22px; border-radius: 50%; "
                           << "      border: 2px solid rgba(0,0,0,0.5); cursor: pointer; "
                           << "      box-shadow: 0 2px 4px rgba(0,0,0,0.4), inset 0 1px 2px rgba(255,255,255,0.3);\"></div>',"
                           << "    iconSize: [22, 22],"
                           << "    iconAnchor: [11, 11],"
                           << "    popupAnchor: [0, -11]"
                           << "  });"
                           << "  var marker = L.marker([" << poi.latitude << ", " << poi.longitude << "], {icon: markerIcon})"
                           << "    .addTo(window.osmMap)"
                           << "    .bindPopup('" << escapedPopup << "', {maxWidth: 350, className: 'rich-popup'});"
                           << "  if (!window.osmMarkers) window.osmMarkers = [];"
                           << "  window.osmMarkers.push(marker);"
                           << "}";
                doJavaScript(addMarkerJs.str());
                markerCount++;
            }
        }
    };

    // Function to update empty state visibility
    auto updateEmptyState = [emptyMessage, activePills]() {
        if (activePills->empty()) {
            emptyMessage->setHidden(false);
        } else {
            emptyMessage->setHidden(true);
        }
    };

    // Function to create a pill card for a category
    auto createPill = std::make_shared<std::function<void(const std::string&, const std::string&, int)>>();
    *createPill = [this, pillTray, activePills, refreshMarkers, updateEmptyState, createPill, pastelColors, markerColors, usedColorIndex](
        const std::string& displayName, const std::string& apiName, int count) {

        // Check if already added
        for (const auto& pill : *activePills) {
            if (pill.apiName == apiName) return;
        }

        // Get next colors from palette (soft for pill, deep for markers)
        int colorIdx = *usedColorIndex % pastelColors.size();
        std::string pillColor = pastelColors[colorIdx];
        std::string pillMarkerColor = markerColors[colorIdx];
        (*usedColorIndex)++;

        auto pillCard = pillTray->addWidget(std::make_unique<Wt::WContainerWidget>());
        pillCard->setStyleClass("category-pill");

        // Apply pastel background color like a post-it note
        pillCard->decorationStyle().setBackgroundColor(Wt::WColor(pillColor));

        // Header with name and count
        auto pillHeader = pillCard->addWidget(std::make_unique<Wt::WContainerWidget>());
        pillHeader->setStyleClass("category-pill-header");

        auto pillName = pillHeader->addWidget(std::make_unique<Wt::WText>(displayName));
        pillName->setStyleClass("category-pill-name");

        auto pillCount = pillHeader->addWidget(std::make_unique<Wt::WText>(std::to_string(count) + " total"));
        pillCount->setStyleClass("category-pill-count");

        // Remove button
        auto removeBtn = pillCard->addWidget(std::make_unique<Wt::WPushButton>("×"));
        removeBtn->setStyleClass("category-pill-remove");

        // POI limit controls with slider
        auto pillControls = pillCard->addWidget(std::make_unique<Wt::WContainerWidget>());
        pillControls->setStyleClass("category-pill-controls");

        auto limitLabel = pillControls->addWidget(std::make_unique<Wt::WText>("POIs:"));

        auto sliderContainer = pillControls->addWidget(std::make_unique<Wt::WContainerWidget>());
        sliderContainer->setStyleClass("category-pill-slider-container");

        // Slider from 0 to total count (or max 100)
        int maxValue = std::min(count, 100);
        int defaultValue = std::min(10, maxValue);

        auto limitSlider = sliderContainer->addWidget(std::make_unique<Wt::WSlider>(Wt::Orientation::Horizontal));
        limitSlider->setStyleClass("category-pill-slider");
        limitSlider->setNativeControl(true);
        limitSlider->setMinimum(0);
        limitSlider->setMaximum(maxValue);
        limitSlider->setValue(defaultValue);
        limitSlider->resize(Wt::WLength::Auto, 24);

        auto limitValueText = sliderContainer->addWidget(std::make_unique<Wt::WText>(std::to_string(defaultValue)));
        limitValueText->setStyleClass("category-pill-slider-value");

        // Add JavaScript for real-time slider value display update
        std::string sliderId = limitSlider->id();
        std::string valueTextId = limitValueText->id();
        std::ostringstream realTimeJs;
        realTimeJs << "setTimeout(function() {"
                   << "  var slider = document.getElementById('" << sliderId << "');"
                   << "  var valueText = document.getElementById('" << valueTextId << "');"
                   << "  if (slider && valueText) {"
                   << "    var input = slider.querySelector('input[type=\"range\"]') || slider;"
                   << "    input.addEventListener('input', function() {"
                   << "      valueText.textContent = this.value;"
                   << "    });"
                   << "  }"
                   << "}, 100);";
        doJavaScript(realTimeJs.str());

        // Add to active pills
        CategoryPillData pillData;
        pillData.displayName = displayName;
        pillData.apiName = apiName;
        pillData.count = count;
        pillData.poiLimit = defaultValue;
        pillData.color = pillColor;
        pillData.markerColor = pillMarkerColor;
        pillData.pillWidget = pillCard;
        pillData.limitSlider = limitSlider;
        pillData.limitValueText = limitValueText;
        activePills->push_back(pillData);

        updateEmptyState();
        // Note: POI markers are refreshed when user clicks "Analyze Area"

        // Handle slider value change (fires on release)
        limitSlider->valueChanged().connect([activePills, apiName, limitSlider, limitValueText]() {
            int newLimit = limitSlider->value();
            limitValueText->setText(std::to_string(newLimit));
            for (auto& pill : *activePills) {
                if (pill.apiName == apiName) {
                    pill.poiLimit = newLimit;
                    break;
                }
            }
        });

        // Handle slider moved (fires while dragging for real-time updates)
        limitSlider->sliderMoved().connect([activePills, apiName, limitValueText](int newLimit) {
            limitValueText->setText(std::to_string(newLimit));
            for (auto& pill : *activePills) {
                if (pill.apiName == apiName) {
                    pill.poiLimit = newLimit;
                    break;
                }
            }
        });

        // Handle remove
        removeBtn->clicked().connect([activePills, pillCard, apiName, updateEmptyState]() {
            // Remove from active pills
            activePills->erase(
                std::remove_if(activePills->begin(), activePills->end(),
                    [&apiName](const CategoryPillData& p) { return p.apiName == apiName; }),
                activePills->end()
            );
            // Remove widget
            pillCard->removeFromParent();
            updateEmptyState();
            // Note: POI markers are refreshed when user clicks "Analyze Area"
        });
    };

    // Handle dropdown selection
    categoryDropdown->changed().connect([categoryDropdown, categories, createPill]() {
        int idx = categoryDropdown->currentIndex();
        if (idx <= 0) return;  // Skip "Select a category" option

        const auto& [displayName, apiName, count] = (*categories)[idx - 1];
        (*createPill)(displayName, apiName, count);

        // Reset dropdown
        categoryDropdown->setCurrentIndex(0);
    });

    // Sidebar footer with action controls
    auto sidebarFooter = mapSidebar->addWidget(std::make_unique<Wt::WContainerWidget>());
    sidebarFooter->setStyleClass("sidebar-footer");

    // Top row: Radius and Analyze button
    auto footerControls = sidebarFooter->addWidget(std::make_unique<Wt::WContainerWidget>());
    footerControls->setStyleClass("sidebar-footer-controls");

    auto radiusSelect = footerControls->addWidget(std::make_unique<Wt::WComboBox>());
    radiusSelect->setStyleClass("form-control radius-select-footer");
    radiusSelect->addItem("5 km");
    radiusSelect->addItem("10 km");
    radiusSelect->addItem("25 km");
    radiusSelect->addItem("40 km");
    radiusSelect->addItem("50 km");

    // Set default selection based on defaultRadiusKm
    if (defaultRadiusKm <= 5) radiusSelect->setCurrentIndex(0);
    else if (defaultRadiusKm <= 10) radiusSelect->setCurrentIndex(1);
    else if (defaultRadiusKm <= 25) radiusSelect->setCurrentIndex(2);
    else if (defaultRadiusKm <= 40) radiusSelect->setCurrentIndex(3);
    else radiusSelect->setCurrentIndex(4);

    auto analyzeBtn = footerControls->addWidget(std::make_unique<Wt::WPushButton>("Analyze Area"));
    analyzeBtn->setStyleClass("btn btn-primary analyze-btn-footer");

    // Bottom row: Info and Clear All
    auto footerBottom = sidebarFooter->addWidget(std::make_unique<Wt::WContainerWidget>());
    footerBottom->setStyleClass("sidebar-footer-bottom");

    auto footerInfo = footerBottom->addWidget(std::make_unique<Wt::WText>("Data: OpenStreetMap"));
    footerInfo->setStyleClass("sidebar-footer-info");

    auto clearAllBtn = footerBottom->addWidget(std::make_unique<Wt::WPushButton>("Clear All"));
    clearAllBtn->setStyleClass("btn-clear-all");
    clearAllBtn->clicked().connect([this, activePills, pillTray, emptyMessage, refreshMarkers]() {
        // Remove all pill widgets
        for (const auto& pill : *activePills) {
            if (pill.pillWidget) {
                pill.pillWidget->removeFromParent();
            }
        }
        activePills->clear();
        emptyMessage->setHidden(false);

        // Clear markers
        std::ostringstream clearMarkersJs;
        clearMarkersJs << "if (window.osmMarkers) {"
                      << "  window.osmMarkers.forEach(function(m) { m.remove(); });"
                      << "}"
                      << "window.osmMarkers = [];";
        doJavaScript(clearMarkersJs.str());
    });

    // Area Summary footer at bottom
    auto summaryFooter = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    summaryFooter->setStyleClass("area-summary-footer");

    auto summaryTitle = summaryFooter->addWidget(std::make_unique<Wt::WText>("Area Summary"));
    summaryTitle->setStyleClass("stat-title");

    auto summaryStats = summaryFooter->addWidget(std::make_unique<Wt::WContainerWidget>());
    summaryStats->setStyleClass("summary-stats");

    // POIs stat
    auto poisItem = summaryStats->addWidget(std::make_unique<Wt::WContainerWidget>());
    poisItem->setStyleClass("summary-stat-item");
    auto poisLabel = poisItem->addWidget(std::make_unique<Wt::WText>("POIs"));
    poisLabel->setStyleClass("stat-label");
    auto totalPoisText = poisItem->addWidget(std::make_unique<Wt::WText>(std::to_string(stats.totalPois)));
    totalPoisText->setStyleClass("stat-value");

    // Density stat
    auto densityItem = summaryStats->addWidget(std::make_unique<Wt::WContainerWidget>());
    densityItem->setStyleClass("summary-stat-item");
    auto densityLabel = densityItem->addWidget(std::make_unique<Wt::WText>("Density"));
    densityLabel->setStyleClass("stat-label");
    std::ostringstream densityStr;
    densityStr << std::fixed << std::setprecision(1) << stats.businessDensityPerSqKm << "/km²";
    auto densityText = densityItem->addWidget(std::make_unique<Wt::WText>(densityStr.str()));
    densityText->setStyleClass("stat-value");

    // Location stat
    auto locationItem = summaryStats->addWidget(std::make_unique<Wt::WContainerWidget>());
    locationItem->setStyleClass("summary-stat-item");
    auto locationLabel = locationItem->addWidget(std::make_unique<Wt::WText>("Location"));
    locationLabel->setStyleClass("stat-label");
    auto locationText = locationItem->addWidget(std::make_unique<Wt::WText>(defaultLocation));
    locationText->setStyleClass("stat-value");

    // Radius stat
    auto radiusItem = summaryStats->addWidget(std::make_unique<Wt::WContainerWidget>());
    radiusItem->setStyleClass("summary-stat-item");
    auto radiusLabel = radiusItem->addWidget(std::make_unique<Wt::WText>("Radius"));
    radiusLabel->setStyleClass("stat-label");
    std::ostringstream radiusStr;
    radiusStr << std::fixed << std::setprecision(0) << defaultRadiusKm << " km";
    auto radiusText = radiusItem->addWidget(std::make_unique<Wt::WText>(radiusStr.str()));
    radiusText->setStyleClass("stat-value");

    // Helper function to get radius from dropdown selection
    auto getRadiusFromSelect = [](int index) -> double {
        switch (index) {
            case 0: return 5.0;
            case 1: return 10.0;
            case 2: return 25.0;
            case 3: return 40.0;
            case 4: return 50.0;
            default: return 10.0;
        }
    };

    // Connect analyze button
    analyzeBtn->clicked().connect([this, locationInput, radiusSelect, currentSearchAreaPtr,
                                   totalPoisText, densityText, locationText,
                                   radiusText, categoryDropdown, categories, refreshMarkers, getRadiusFromSelect]() {
        std::string location = locationInput->text().toUTF8();
        double radiusKm = getRadiusFromSelect(radiusSelect->currentIndex());

        if (location.empty()) {
            location = "Denver, CO";
        }

        // Geocode the location
        Models::GeoLocation geoLocation = searchService_->geocodeAddress(location);
        Models::SearchArea searchArea(geoLocation, radiusKm);

        // Update shared state
        currentSearchLocation_ = location;
        currentSearchArea_ = searchArea;
        hasActiveSearch_ = true;
        *currentSearchAreaPtr = searchArea;

        // Update map to new location via JavaScript
        std::ostringstream panMapJs;
        panMapJs << "if (window.osmMap) {"
                 << "  window.osmMap.setView([" << geoLocation.latitude << ", " << geoLocation.longitude << "], 13);"
                 << "}";
        doJavaScript(panMapJs.str());

        // Get new stats
        auto& osmAPI = searchService_->getOSMAPI();
        auto newStats = osmAPI.getAreaStatisticsSync(searchArea);

        // Update market score in navigation header
        navigation_->setMarketScore(newStats.marketPotentialScore);

        // Update summary stats
        totalPoisText->setText(std::to_string(newStats.totalPois));

        std::ostringstream newDensityStr;
        newDensityStr << std::fixed << std::setprecision(1) << newStats.businessDensityPerSqKm << "/km²";
        densityText->setText(newDensityStr.str());

        locationText->setText(location);

        std::ostringstream newRadiusStr;
        newRadiusStr << std::fixed << std::setprecision(0) << radiusKm << " km";
        radiusText->setText(newRadiusStr.str());

        // Update category counts in shared data and dropdown
        *categories = {
            {"Offices", "offices", newStats.offices},
            {"Hotels", "hotels", newStats.hotels},
            {"Conference Venues", "conference", newStats.conferenceVenues},
            {"Restaurants", "restaurants", newStats.restaurants},
            {"Cafes", "cafes", newStats.cafes},
            {"Hospitals", "hospitals", newStats.hospitals},
            {"Universities", "universities", newStats.universities},
            {"Schools", "schools", newStats.schools},
            {"Industrial", "industrial", newStats.industrialBuildings},
            {"Warehouses", "warehouses", newStats.warehouses},
            {"Banks", "banks", newStats.banks},
            {"Government", "government", newStats.governmentBuildings}
        };

        // Rebuild dropdown with new counts
        categoryDropdown->clear();
        categoryDropdown->addItem("-- Select a category --");
        for (const auto& [displayName, apiName, count] : *categories) {
            categoryDropdown->addItem(displayName + " (" + std::to_string(count) + ")");
        }

        // Refresh POI markers for active category pills with new location
        (*refreshMarkers)();
    });

    // Add blur event to location input to recenter map and refresh POIs
    locationInput->blurred().connect([this, locationInput, radiusSelect, currentSearchAreaPtr, refreshMarkers, getRadiusFromSelect]() {
        std::string location = locationInput->text().toUTF8();
        if (location.empty()) return;

        // Geocode and recenter map
        Models::GeoLocation geoLocation = searchService_->geocodeAddress(location);
        if (geoLocation.hasValidCoordinates()) {
            double radiusKm = getRadiusFromSelect(radiusSelect->currentIndex());
            Models::SearchArea searchArea(geoLocation, radiusKm);

            // Update shared state
            currentSearchLocation_ = location;
            currentSearchArea_ = searchArea;
            *currentSearchAreaPtr = searchArea;

            std::ostringstream panMapJs;
            panMapJs << "if (window.osmMap) {"
                     << "  window.osmMap.setView([" << geoLocation.latitude << ", " << geoLocation.longitude << "], 13);"
                     << "}";
            doJavaScript(panMapJs.str());

            // Refresh POI markers for the new location
            (*refreshMarkers)();
        }
    });
}

void FranchiseApp::showReportsPage() {
    workArea_->clear();
    navigation_->setPageTitle("Reports");
    navigation_->setBreadcrumbs({"Home", "Reports"});
    navigation_->setMarketScore(-1);

    auto container = workArea_->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setStyleClass("page-container reports-page");

    auto header = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->setStyleClass("page-header");

    auto tagline = header->addWidget(std::make_unique<Wt::WText>(
        "Detailed reports and analytics for your prospect discovery efforts"
    ));
    tagline->setStyleClass("page-tagline");

    auto placeholder = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    placeholder->setStyleClass("placeholder-content");

    auto icon = placeholder->addWidget(std::make_unique<Wt::WText>("📈"));
    icon->setStyleClass("placeholder-icon");

    auto text = placeholder->addWidget(std::make_unique<Wt::WText>(
        "Coming soon: Track your outreach performance and conversion metrics."
    ));
    text->setStyleClass("placeholder-text");
}

void FranchiseApp::showSettingsPage() {
    workArea_->clear();
    navigation_->setPageTitle("Settings");
    navigation_->setBreadcrumbs({"Home", "Settings"});
    navigation_->setMarketScore(-1);

    auto container = workArea_->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setStyleClass("page-container settings-page");

    auto header = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->setStyleClass("page-header");

    auto title = header->addWidget(std::make_unique<Wt::WText>("Settings"));
    title->setStyleClass("page-title");

    auto subtitle = header->addWidget(std::make_unique<Wt::WText>(
        "Configure your store, API keys, and application preferences"
    ));
    subtitle->setStyleClass("page-subtitle");

    // Get current configuration
    auto& appConfig = AppConfig::instance();

    // ===========================================
    // Tab Navigation
    // ===========================================
    auto tabContainer = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    tabContainer->setStyleClass("settings-tabs");

    auto tabNav = tabContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    tabNav->setStyleClass("tab-nav");

    // Create tab buttons
    auto tabFranchisee = tabNav->addWidget(std::make_unique<Wt::WText>("Franchisee Information"));
    tabFranchisee->setStyleClass("tab-btn active");

    auto tabMarketing = tabNav->addWidget(std::make_unique<Wt::WText>("Marketing"));
    tabMarketing->setStyleClass("tab-btn");

    auto tabAI = tabNav->addWidget(std::make_unique<Wt::WText>("AI Configuration"));
    tabAI->setStyleClass("tab-btn");

    auto tabData = tabNav->addWidget(std::make_unique<Wt::WText>("Data Sources"));
    tabData->setStyleClass("tab-btn");

    auto tabBranding = tabNav->addWidget(std::make_unique<Wt::WText>("Branding"));
    tabBranding->setStyleClass("tab-btn");

    // Tab content container
    auto tabContent = tabContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    tabContent->setStyleClass("tab-content");

    // ===========================================
    // Tab 1: Franchisee Information
    // ===========================================
    auto franchiseePanel = tabContent->addWidget(std::make_unique<Wt::WContainerWidget>());
    franchiseePanel->setStyleClass("tab-panel active");
    franchiseePanel->setId("tab-franchisee");

    // Franchisee Information Section
    auto franchiseeSection = franchiseePanel->addWidget(std::make_unique<Wt::WContainerWidget>());
    franchiseeSection->setStyleClass("settings-section");

    auto franchiseeTitle = franchiseeSection->addWidget(std::make_unique<Wt::WText>("Franchisee Information"));
    franchiseeTitle->setStyleClass("section-title");

    auto franchiseeDesc = franchiseeSection->addWidget(std::make_unique<Wt::WText>(
        "Enter your franchise store details. This will be the center point for all prospect searches."
    ));
    franchiseeDesc->setStyleClass("section-description");

    auto franchiseeFormGrid = franchiseeSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    franchiseeFormGrid->setStyleClass("form-grid");

    // Store Name - combo box with existing stores + new store option
    auto nameGroup = franchiseeFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    nameGroup->setStyleClass("form-group");
    nameGroup->addWidget(std::make_unique<Wt::WText>("Store Name"))->setStyleClass("form-label");

    auto storeCombo = nameGroup->addWidget(std::make_unique<Wt::WComboBox>());
    storeCombo->setStyleClass("form-control");
    storeCombo->addItem("-- New Store --");

    // Load available stores and populate combo
    loadAvailableStores();
    int selectedIndex = 0;
    for (size_t i = 0; i < availableStores_.size(); ++i) {
        const auto& store = availableStores_[i];
        storeCombo->addItem(store.storeName);
        if (store.id == currentStoreLocationId_) {
            selectedIndex = static_cast<int>(i) + 1;  // +1 for "New Store" option
        }
    }
    storeCombo->setCurrentIndex(selectedIndex);

    // Text input for new store name (shown only when "New Store" selected)
    auto nameInput = nameGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    nameInput->setPlaceholderText("Enter new store name...");
    nameInput->setStyleClass("form-control");
    nameInput->setMargin(5, Wt::Side::Top);
    nameInput->setHidden(selectedIndex != 0);  // Hide if existing store selected

    // Store Address - full width row
    auto addressGroup = franchiseeFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    addressGroup->setStyleClass("form-group full-width");
    addressGroup->addWidget(std::make_unique<Wt::WText>("Street Address"))->setStyleClass("form-label");
    auto addressInput = addressGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    addressInput->setPlaceholderText("e.g., 123 Main St, Suite 200");
    addressInput->setStyleClass("form-control");
    if (franchisee_.isConfigured) addressInput->setText(franchisee_.address);

    // City, State, Zip on second row
    auto locationRow = franchiseeFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    locationRow->setStyleClass("form-row address-row");

    // City
    auto cityGroup = locationRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    cityGroup->setStyleClass("form-group form-group-city");
    cityGroup->addWidget(std::make_unique<Wt::WText>("City"))->setStyleClass("form-label");
    auto cityInput = cityGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    cityInput->setPlaceholderText("e.g., Denver");
    cityInput->setStyleClass("form-control");
    if (franchisee_.isConfigured) cityInput->setText(franchisee_.location.city);

    // State dropdown with full names (stores state codes)
    auto stateGroup = locationRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    stateGroup->setStyleClass("form-group form-group-state");
    stateGroup->addWidget(std::make_unique<Wt::WText>("State"))->setStyleClass("form-label");
    auto stateCombo = stateGroup->addWidget(std::make_unique<Wt::WComboBox>());
    stateCombo->setStyleClass("form-control");

    // State names and codes - display name but store code
    std::vector<std::pair<std::string, std::string>> states = {
        {"Select State", ""}, {"Alabama", "AL"}, {"Alaska", "AK"}, {"Arizona", "AZ"},
        {"Arkansas", "AR"}, {"California", "CA"}, {"Colorado", "CO"}, {"Connecticut", "CT"},
        {"Delaware", "DE"}, {"Florida", "FL"}, {"Georgia", "GA"}, {"Hawaii", "HI"},
        {"Idaho", "ID"}, {"Illinois", "IL"}, {"Indiana", "IN"}, {"Iowa", "IA"},
        {"Kansas", "KS"}, {"Kentucky", "KY"}, {"Louisiana", "LA"}, {"Maine", "ME"},
        {"Maryland", "MD"}, {"Massachusetts", "MA"}, {"Michigan", "MI"}, {"Minnesota", "MN"},
        {"Mississippi", "MS"}, {"Missouri", "MO"}, {"Montana", "MT"}, {"Nebraska", "NE"},
        {"Nevada", "NV"}, {"New Hampshire", "NH"}, {"New Jersey", "NJ"}, {"New Mexico", "NM"},
        {"New York", "NY"}, {"North Carolina", "NC"}, {"North Dakota", "ND"}, {"Ohio", "OH"},
        {"Oklahoma", "OK"}, {"Oregon", "OR"}, {"Pennsylvania", "PA"}, {"Rhode Island", "RI"},
        {"South Carolina", "SC"}, {"South Dakota", "SD"}, {"Tennessee", "TN"}, {"Texas", "TX"},
        {"Utah", "UT"}, {"Vermont", "VT"}, {"Virginia", "VA"}, {"Washington", "WA"},
        {"West Virginia", "WV"}, {"Wisconsin", "WI"}, {"Wyoming", "WY"}
    };

    for (const auto& s : states) {
        stateCombo->addItem(s.first);
    }

    // Helper lambda to get state code from combo selection
    auto getStateCode = [states](int idx) -> std::string {
        if (idx >= 0 && idx < static_cast<int>(states.size())) return states[idx].second;
        return "";
    };

    // Helper lambda to find index by state code
    auto findStateIndex = [states](const std::string& code) -> int {
        for (size_t i = 0; i < states.size(); ++i) {
            if (states[i].second == code) return static_cast<int>(i);
        }
        return 0;
    };

    // Set initial value if configured
    if (franchisee_.isConfigured && !franchisee_.location.state.empty()) {
        stateCombo->setCurrentIndex(findStateIndex(franchisee_.location.state));
    }

    // Zip Code
    auto zipGroup = locationRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    zipGroup->setStyleClass("form-group form-group-zip");
    zipGroup->addWidget(std::make_unique<Wt::WText>("Zip Code"))->setStyleClass("form-label");
    auto zipInput = zipGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    zipInput->setPlaceholderText("e.g., 80202");
    zipInput->setStyleClass("form-control");
    if (franchisee_.isConfigured) zipInput->setText(franchisee_.location.postalCode);

    // Owner Name
    auto ownerGroup = franchiseeFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    ownerGroup->setStyleClass("form-group");
    ownerGroup->addWidget(std::make_unique<Wt::WText>("Owner/Manager Name"))->setStyleClass("form-label");
    auto ownerInput = ownerGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    ownerInput->setPlaceholderText("e.g., John Smith");
    ownerInput->setStyleClass("form-control");
    if (franchisee_.isConfigured) ownerInput->setText(franchisee_.ownerName);

    // Phone
    auto phoneGroup = franchiseeFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    phoneGroup->setStyleClass("form-group");
    phoneGroup->addWidget(std::make_unique<Wt::WText>("Store Phone"))->setStyleClass("form-label");
    auto phoneInput = phoneGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    phoneInput->setPlaceholderText("e.g., (555) 123-4567");
    phoneInput->setStyleClass("form-control");
    if (franchisee_.isConfigured) phoneInput->setText(franchisee_.phone);

    // Handle store selection change
    storeCombo->changed().connect([this, storeCombo, nameInput, addressInput, cityInput, stateCombo, findStateIndex, zipInput, ownerInput, phoneInput] {
        int idx = storeCombo->currentIndex();
        if (idx == 0) {
            // New Store - show name input and clear fields
            currentStoreLocationId_ = "";
            nameInput->setHidden(false);
            nameInput->setText("");
            addressInput->setText("");
            cityInput->setText("");
            stateCombo->setCurrentIndex(0);
            zipInput->setText("");
            ownerInput->setText("");
            phoneInput->setText("");
        } else if (idx > 0 && static_cast<size_t>(idx - 1) < availableStores_.size()) {
            // Existing store - hide name input and load store data
            nameInput->setHidden(true);
            const auto& store = availableStores_[idx - 1];
            selectStoreById(store.id);

            // Update form fields with separate address components
            addressInput->setText(store.addressLine1);
            cityInput->setText(store.city);
            stateCombo->setCurrentIndex(findStateIndex(store.stateProvince));
            zipInput->setText(store.postalCode);
            ownerInput->setText(franchisee_.ownerName);
            phoneInput->setText(store.phone);
        }
    });

    // ===========================================
    // Tab 2: Marketing
    // ===========================================
    auto marketingPanel = tabContent->addWidget(std::make_unique<Wt::WContainerWidget>());
    marketingPanel->setStyleClass("tab-panel");
    marketingPanel->setId("tab-marketing");

    auto marketingSection = marketingPanel->addWidget(std::make_unique<Wt::WContainerWidget>());
    marketingSection->setStyleClass("settings-section");

    marketingSection->addWidget(std::make_unique<Wt::WText>("Search Preferences"))->setStyleClass("section-title");
    marketingSection->addWidget(std::make_unique<Wt::WText>(
        "Configure your default search parameters for finding prospects."
    ))->setStyleClass("section-description");

    auto prefsGrid = marketingSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    prefsGrid->setStyleClass("form-grid");

    // Default Radius
    auto radiusGroup = prefsGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    radiusGroup->setStyleClass("form-group");
    radiusGroup->addWidget(std::make_unique<Wt::WText>("Default Search Radius (miles)"))->setStyleClass("form-label");
    auto radiusInput = radiusGroup->addWidget(std::make_unique<Wt::WLineEdit>("5"));
    radiusInput->setStyleClass("form-control");
    if (franchisee_.isConfigured) {
        radiusInput->setText(std::to_string(static_cast<int>(franchisee_.defaultSearchRadiusMiles)));
    }

    // Target Business Types
    auto typesGroup = marketingSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    typesGroup->setStyleClass("form-group");
    typesGroup->addWidget(std::make_unique<Wt::WText>("Target Business Types"))->setStyleClass("form-label");
    typesGroup->addWidget(std::make_unique<Wt::WText>(
        "Select the types of businesses you want to target for catering services"
    ))->setStyleClass("form-help");

    auto checkboxGrid = typesGroup->addWidget(std::make_unique<Wt::WContainerWidget>());
    checkboxGrid->setStyleClass("checkbox-grid");

    // Map business type names to their enum values for checking saved preferences
    std::vector<std::pair<std::string, Models::BusinessType>> businessTypeMap = {
        {"Corporate Offices", Models::BusinessType::CORPORATE_OFFICE},
        {"Conference Centers", Models::BusinessType::CONFERENCE_CENTER},
        {"Hotels", Models::BusinessType::HOTEL},
        {"Medical Facilities", Models::BusinessType::MEDICAL_FACILITY},
        {"Educational Institutions", Models::BusinessType::EDUCATIONAL_INSTITUTION},
        {"Manufacturing/Industrial", Models::BusinessType::MANUFACTURING},
        {"Warehouses/Distribution", Models::BusinessType::WAREHOUSE},
        {"Government Offices", Models::BusinessType::GOVERNMENT_OFFICE},
        {"Tech Companies", Models::BusinessType::TECH_COMPANY},
        {"Financial Services", Models::BusinessType::FINANCIAL_SERVICES},
        {"Coworking Spaces", Models::BusinessType::COWORKING_SPACE},
        {"Non-profits", Models::BusinessType::NONPROFIT}
    };

    // Default checked states (used when no saved preferences)
    std::vector<bool> defaultChecked = {true, true, true, true, true, false, false, false, true, false, true, false};

    std::vector<Wt::WCheckBox*> typeCheckboxes;
    for (size_t i = 0; i < businessTypeMap.size(); ++i) {
        const auto& [typeName, typeEnum] = businessTypeMap[i];
        auto checkbox = checkboxGrid->addWidget(std::make_unique<Wt::WCheckBox>(typeName));
        checkbox->setStyleClass("form-checkbox");

        // Check if this type is in saved preferences, otherwise use default
        if (franchisee_.isConfigured && !franchisee_.searchCriteria.businessTypes.empty()) {
            checkbox->setChecked(franchisee_.searchCriteria.hasBusinessType(typeEnum));
        } else {
            checkbox->setChecked(defaultChecked[i]);
        }
        typeCheckboxes.push_back(checkbox);
    }

    // Employee Size
    auto sizeGroup = marketingSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    sizeGroup->setStyleClass("form-group");
    sizeGroup->addWidget(std::make_unique<Wt::WText>("Target Organization Size"))->setStyleClass("form-label");
    auto sizeCombo = sizeGroup->addWidget(std::make_unique<Wt::WComboBox>());
    sizeCombo->setStyleClass("form-control");
    auto employeeRanges = Models::EmployeeRange::getStandardRanges();
    for (const auto& range : employeeRanges) {
        sizeCombo->addItem(range.label);
    }

    // Set combo to saved employee range if configured
    int selectedSizeIndex = 0;
    if (franchisee_.isConfigured) {
        for (size_t i = 0; i < employeeRanges.size(); ++i) {
            if (employeeRanges[i].minEmployees == franchisee_.searchCriteria.minEmployees &&
                employeeRanges[i].maxEmployees == franchisee_.searchCriteria.maxEmployees) {
                selectedSizeIndex = static_cast<int>(i);
                break;
            }
        }
    }
    sizeCombo->setCurrentIndex(selectedSizeIndex);

    // ===========================================
    // Tab 3: AI Configuration
    // ===========================================
    auto aiPanel = tabContent->addWidget(std::make_unique<Wt::WContainerWidget>());
    aiPanel->setStyleClass("tab-panel");
    aiPanel->setId("tab-ai");

    auto aiSection = aiPanel->addWidget(std::make_unique<Wt::WContainerWidget>());
    aiSection->setStyleClass("settings-section");

    aiSection->addWidget(std::make_unique<Wt::WText>("AI Configuration"))->setStyleClass("section-title");
    aiSection->addWidget(std::make_unique<Wt::WText>(
        "Configure your AI provider for intelligent prospect analysis and recommendations."
    ))->setStyleClass("section-description");

    // AI Status
    auto aiStatusContainer = aiSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    aiStatusContainer->setStyleClass("api-status-container");

    bool aiConfigured = searchService_->isAIEngineConfigured();
    std::string aiStatusText = aiConfigured ? "AI Engine: Configured" : "AI Engine: Not Configured";
    if (aiConfigured) {
        auto provider = searchService_->getAIProvider();
        if (provider == Services::AIProvider::OPENAI) {
            aiStatusText = "AI Engine: OpenAI (" + appConfig.getOpenAIModel() + ")";
        } else if (provider == Services::AIProvider::GEMINI) {
            aiStatusText = "AI Engine: Google Gemini";
        }
    }
    auto aiStatus = aiStatusContainer->addWidget(std::make_unique<Wt::WText>(aiStatusText));
    aiStatus->setStyleClass(aiConfigured ? "status-indicator status-configured" : "status-indicator status-not-configured");

    auto aiFormGrid = aiSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    aiFormGrid->setStyleClass("form-grid");

    // OpenAI API Key
    auto openaiGroup = aiFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    openaiGroup->setStyleClass("form-group");
    openaiGroup->addWidget(std::make_unique<Wt::WText>("OpenAI API Key"))->setStyleClass("form-label");
    auto openaiInput = openaiGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    openaiInput->setPlaceholderText(appConfig.hasOpenAIKey() ? "sk-****...****(configured)" : "sk-...");
    openaiInput->setStyleClass("form-control");
    openaiInput->setAttributeValue("type", "password");
    openaiGroup->addWidget(std::make_unique<Wt::WText>("Get your API key from platform.openai.com"))->setStyleClass("form-help");

    // OpenAI Model Selection
    auto modelGroup = aiFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    modelGroup->setStyleClass("form-group");
    modelGroup->addWidget(std::make_unique<Wt::WText>("OpenAI Model"))->setStyleClass("form-label");
    auto modelSelect = modelGroup->addWidget(std::make_unique<Wt::WComboBox>());
    modelSelect->setStyleClass("form-control");
    modelSelect->addItem("gpt-4o (Recommended)");
    modelSelect->addItem("gpt-4o-mini (Faster, Lower Cost)");
    modelSelect->addItem("gpt-4-turbo");
    modelSelect->addItem("gpt-4");
    modelSelect->addItem("gpt-3.5-turbo");

    std::string currentModel = appConfig.getOpenAIModel();
    if (currentModel == "gpt-4o-mini") modelSelect->setCurrentIndex(1);
    else if (currentModel == "gpt-4-turbo") modelSelect->setCurrentIndex(2);
    else if (currentModel == "gpt-4") modelSelect->setCurrentIndex(3);
    else if (currentModel == "gpt-3.5-turbo") modelSelect->setCurrentIndex(4);
    else modelSelect->setCurrentIndex(0);

    // Gemini API Key
    auto geminiGroup = aiFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    geminiGroup->setStyleClass("form-group");
    geminiGroup->addWidget(std::make_unique<Wt::WText>("Google Gemini API Key (Alternative)"))->setStyleClass("form-label");
    auto geminiInput = geminiGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    geminiInput->setPlaceholderText(appConfig.hasGeminiKey() ? "AIza****...****(configured)" : "AIza...");
    geminiInput->setStyleClass("form-control");
    geminiInput->setAttributeValue("type", "password");
    geminiGroup->addWidget(std::make_unique<Wt::WText>("Used if OpenAI is not configured"))->setStyleClass("form-help");

    // --- Scoring Optimization Section ---
    auto scoringSection = aiPanel->addWidget(std::make_unique<Wt::WContainerWidget>());
    scoringSection->setStyleClass("settings-section");
    scoringSection->setAttributeValue("style", "margin-top: 24px; border-top: 1px solid #e5e7eb; padding-top: 20px;");

    scoringSection->addWidget(std::make_unique<Wt::WText>("Scoring Optimization"))->setStyleClass("section-title");
    scoringSection->addWidget(std::make_unique<Wt::WText>(
        "Adjust how prospects are scored. Enable/disable rules and customize point values."
    ))->setStyleClass("section-description");

    // Two-column container for Penalties and Bonuses panels
    auto panelsContainer = scoringSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    panelsContainer->setStyleClass("scoring-panels-container");

    // Store slider/checkbox pointers for save handler
    std::vector<std::pair<std::string, Wt::WSlider*>> penaltySliders;
    std::vector<std::pair<std::string, Wt::WCheckBox*>> penaltyChecks;
    std::vector<std::pair<std::string, Wt::WSlider*>> bonusSliders;
    std::vector<std::pair<std::string, Wt::WCheckBox*>> bonusChecks;

    // ========== PENALTIES PANEL ==========
    auto penaltiesPanel = panelsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    penaltiesPanel->setStyleClass("scoring-panel penalties");

    // Penalties header
    auto penaltiesHeader = penaltiesPanel->addWidget(std::make_unique<Wt::WContainerWidget>());
    penaltiesHeader->setStyleClass("panel-header");

    auto penaltyIcon = penaltiesHeader->addWidget(std::make_unique<Wt::WText>("↓"));
    penaltyIcon->setStyleClass("panel-icon");

    auto penaltyTitleContainer = penaltiesHeader->addWidget(std::make_unique<Wt::WContainerWidget>());
    penaltyTitleContainer->addWidget(std::make_unique<Wt::WText>("Penalties"))->setStyleClass("panel-title");
    penaltyTitleContainer->addWidget(std::make_unique<Wt::WText>("Reduce prospect scores"))->setStyleClass("panel-subtitle");

    // Penalties GridBag
    auto penaltiesGrid = penaltiesPanel->addWidget(std::make_unique<Wt::WContainerWidget>());
    penaltiesGrid->setStyleClass("scoring-grid");

    // Grid header row
    auto penaltyHeaderRow = penaltiesGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    penaltyHeaderRow->setStyleClass("scoring-grid-header");
    penaltyHeaderRow->addWidget(std::make_unique<Wt::WText>(""));  // checkbox column
    penaltyHeaderRow->addWidget(std::make_unique<Wt::WText>("Rule"));
    penaltyHeaderRow->addWidget(std::make_unique<Wt::WText>("Adjustment"));
    penaltyHeaderRow->addWidget(std::make_unique<Wt::WText>("Points"));

    // Penalty rules
    for (const auto* rule : scoringEngine_->getPenaltyRules()) {
        auto ruleRow = penaltiesGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
        ruleRow->setStyleClass("scoring-grid-row");

        // Checkbox cell
        auto checkCell = ruleRow->addWidget(std::make_unique<Wt::WContainerWidget>());
        checkCell->setStyleClass("cell-checkbox");
        auto enableCheck = checkCell->addWidget(std::make_unique<Wt::WCheckBox>());
        enableCheck->setChecked(rule->enabled);
        penaltyChecks.push_back({rule->id, enableCheck});

        // Name cell with description
        auto nameCell = ruleRow->addWidget(std::make_unique<Wt::WContainerWidget>());
        nameCell->setStyleClass("cell-name");
        nameCell->addWidget(std::make_unique<Wt::WText>(rule->name));
        auto descText = nameCell->addWidget(std::make_unique<Wt::WText>(rule->description));
        descText->setStyleClass("rule-description");

        // Slider cell with range labels
        auto sliderCell = ruleRow->addWidget(std::make_unique<Wt::WContainerWidget>());
        sliderCell->setStyleClass("cell-slider");

        auto sliderWrapper = sliderCell->addWidget(std::make_unique<Wt::WContainerWidget>());
        sliderWrapper->setStyleClass("slider-with-range");

        auto minLabel = sliderWrapper->addWidget(std::make_unique<Wt::WText>(std::to_string(rule->minPoints)));
        minLabel->setStyleClass("slider-range-label");

        auto slider = sliderWrapper->addWidget(std::make_unique<Wt::WSlider>(Wt::Orientation::Horizontal));
        slider->setNativeControl(true);
        slider->setMinimum(rule->minPoints);
        slider->setMaximum(rule->maxPoints);
        slider->setValue(rule->currentPoints);
        slider->setStyleClass("scoring-slider");
        slider->resize(Wt::WLength::Auto, 24);
        penaltySliders.push_back({rule->id, slider});

        auto maxLabel = sliderWrapper->addWidget(std::make_unique<Wt::WText>(std::to_string(rule->maxPoints)));
        maxLabel->setStyleClass("slider-range-label");

        // Points cell
        auto pointsCell = ruleRow->addWidget(std::make_unique<Wt::WContainerWidget>());
        pointsCell->setStyleClass("cell-points");
        auto pointsLabel = pointsCell->addWidget(std::make_unique<Wt::WText>(std::to_string(rule->currentPoints)));

        // Update display when slider changes
        slider->valueChanged().connect([pointsLabel](int value) {
            pointsLabel->setText(std::to_string(value));
        });
    }

    // ========== BONUSES PANEL ==========
    auto bonusesPanel = panelsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    bonusesPanel->setStyleClass("scoring-panel bonuses");

    // Bonuses header
    auto bonusesHeader = bonusesPanel->addWidget(std::make_unique<Wt::WContainerWidget>());
    bonusesHeader->setStyleClass("panel-header");

    auto bonusIcon = bonusesHeader->addWidget(std::make_unique<Wt::WText>("↑"));
    bonusIcon->setStyleClass("panel-icon");

    auto bonusTitleContainer = bonusesHeader->addWidget(std::make_unique<Wt::WContainerWidget>());
    bonusTitleContainer->addWidget(std::make_unique<Wt::WText>("Bonuses"))->setStyleClass("panel-title");
    bonusTitleContainer->addWidget(std::make_unique<Wt::WText>("Increase prospect scores"))->setStyleClass("panel-subtitle");

    // Bonuses GridBag
    auto bonusesGrid = bonusesPanel->addWidget(std::make_unique<Wt::WContainerWidget>());
    bonusesGrid->setStyleClass("scoring-grid");

    // Grid header row
    auto bonusHeaderRow = bonusesGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    bonusHeaderRow->setStyleClass("scoring-grid-header");
    bonusHeaderRow->addWidget(std::make_unique<Wt::WText>(""));  // checkbox column
    bonusHeaderRow->addWidget(std::make_unique<Wt::WText>("Rule"));
    bonusHeaderRow->addWidget(std::make_unique<Wt::WText>("Adjustment"));
    bonusHeaderRow->addWidget(std::make_unique<Wt::WText>("Points"));

    // Bonus rules
    for (const auto* rule : scoringEngine_->getBonusRules()) {
        auto ruleRow = bonusesGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
        ruleRow->setStyleClass("scoring-grid-row");

        // Checkbox cell
        auto checkCell = ruleRow->addWidget(std::make_unique<Wt::WContainerWidget>());
        checkCell->setStyleClass("cell-checkbox");
        auto enableCheck = checkCell->addWidget(std::make_unique<Wt::WCheckBox>());
        enableCheck->setChecked(rule->enabled);
        bonusChecks.push_back({rule->id, enableCheck});

        // Name cell with description
        auto nameCell = ruleRow->addWidget(std::make_unique<Wt::WContainerWidget>());
        nameCell->setStyleClass("cell-name");
        nameCell->addWidget(std::make_unique<Wt::WText>(rule->name));
        auto descText = nameCell->addWidget(std::make_unique<Wt::WText>(rule->description));
        descText->setStyleClass("rule-description");

        // Slider cell with range labels
        auto sliderCell = ruleRow->addWidget(std::make_unique<Wt::WContainerWidget>());
        sliderCell->setStyleClass("cell-slider");

        auto sliderWrapper = sliderCell->addWidget(std::make_unique<Wt::WContainerWidget>());
        sliderWrapper->setStyleClass("slider-with-range");

        auto minLabel = sliderWrapper->addWidget(std::make_unique<Wt::WText>(std::to_string(rule->minPoints)));
        minLabel->setStyleClass("slider-range-label");

        auto slider = sliderWrapper->addWidget(std::make_unique<Wt::WSlider>(Wt::Orientation::Horizontal));
        slider->setNativeControl(true);
        slider->setMinimum(rule->minPoints);
        slider->setMaximum(rule->maxPoints);
        slider->setValue(rule->currentPoints);
        slider->setStyleClass("scoring-slider");
        slider->resize(Wt::WLength::Auto, 24);
        bonusSliders.push_back({rule->id, slider});

        auto maxLabel = sliderWrapper->addWidget(std::make_unique<Wt::WText>(std::to_string(rule->maxPoints)));
        maxLabel->setStyleClass("slider-range-label");

        // Points cell
        auto pointsCell = ruleRow->addWidget(std::make_unique<Wt::WContainerWidget>());
        pointsCell->setStyleClass("cell-points");
        auto pointsLabel = pointsCell->addWidget(std::make_unique<Wt::WText>("+" + std::to_string(rule->currentPoints)));

        // Update display when slider changes
        slider->valueChanged().connect([pointsLabel](int value) {
            pointsLabel->setText("+" + std::to_string(value));
        });
    }

    // Reset to defaults button (below both panels)
    auto resetBtn = scoringSection->addWidget(std::make_unique<Wt::WPushButton>("Reset to Defaults"));
    resetBtn->setStyleClass("btn btn-outline btn-sm");
    resetBtn->setAttributeValue("style", "margin-top: 16px;");
    resetBtn->clicked().connect([this, penaltySliders, penaltyChecks, bonusSliders, bonusChecks]() {
        scoringEngine_->resetAllToDefaults();
        // Refresh the page to update UI
        showSettingsPage();
        setInternalPath("/settings", true);
    });

    // ===========================================
    // Tab 4: Data Sources
    // ===========================================
    auto dataPanel = tabContent->addWidget(std::make_unique<Wt::WContainerWidget>());
    dataPanel->setStyleClass("tab-panel");
    dataPanel->setId("tab-data");

    auto apiSection = dataPanel->addWidget(std::make_unique<Wt::WContainerWidget>());
    apiSection->setStyleClass("settings-section");

    apiSection->addWidget(std::make_unique<Wt::WText>("Data Source APIs"))->setStyleClass("section-title");
    apiSection->addWidget(std::make_unique<Wt::WText>(
        "Configure API keys for business data sources. OpenStreetMap is always available (no key required)."
    ))->setStyleClass("section-description");

    auto apiFormGrid = apiSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    apiFormGrid->setStyleClass("form-grid");

    // Google API Key
    auto googleGroup = apiFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    googleGroup->setStyleClass("form-group");
    googleGroup->addWidget(std::make_unique<Wt::WText>("Google Places API Key"))->setStyleClass("form-label");
    auto googleInput = googleGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    googleInput->setPlaceholderText(appConfig.hasGoogleKey() ? "AIza****...****(configured)" : "AIza...");
    googleInput->setStyleClass("form-control");
    googleInput->setAttributeValue("type", "password");

    // BBB API Key
    auto bbbGroup = apiFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    bbbGroup->setStyleClass("form-group");
    bbbGroup->addWidget(std::make_unique<Wt::WText>("BBB API Key"))->setStyleClass("form-label");
    auto bbbInput = bbbGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    bbbInput->setPlaceholderText(appConfig.hasBBBKey() ? "****...****(configured)" : "Enter BBB API key");
    bbbInput->setStyleClass("form-control");
    bbbInput->setAttributeValue("type", "password");

    // Census API Key
    auto censusGroup = apiFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    censusGroup->setStyleClass("form-group");
    censusGroup->addWidget(std::make_unique<Wt::WText>("Census/Demographics API Key"))->setStyleClass("form-label");
    auto censusInput = censusGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    censusInput->setPlaceholderText(appConfig.hasCensusKey() ? "****...****(configured)" : "Enter Census API key");
    censusInput->setStyleClass("form-control");
    censusInput->setAttributeValue("type", "password");

    // ===========================================
    // Tab 5: Branding
    // ===========================================
    auto brandingPanel = tabContent->addWidget(std::make_unique<Wt::WContainerWidget>());
    brandingPanel->setStyleClass("tab-panel");
    brandingPanel->setId("tab-branding");

    auto brandingSection = brandingPanel->addWidget(std::make_unique<Wt::WContainerWidget>());
    brandingSection->setStyleClass("settings-section");

    brandingSection->addWidget(std::make_unique<Wt::WText>("Logo Configuration"))->setStyleClass("section-title");
    brandingSection->addWidget(std::make_unique<Wt::WText>(
        "Customize your sidebar logo. Paste a URL to an image or upload a logo file."
    ))->setStyleClass("section-description");

    // Current Logo Preview
    auto previewContainer = brandingSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    previewContainer->setStyleClass("logo-preview-container");

    auto previewLabel = previewContainer->addWidget(std::make_unique<Wt::WText>("Current Logo:"));
    previewLabel->setStyleClass("form-label");

    auto logoPreview = previewContainer->addWidget(std::make_unique<Wt::WImage>(appConfig.getBrandLogoPath()));
    logoPreview->setStyleClass("logo-preview");
    logoPreview->setAlternateText("Logo Preview");

    // Logo URL Input
    auto logoFormGrid = brandingSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    logoFormGrid->setStyleClass("form-grid");

    auto logoUrlGroup = logoFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    logoUrlGroup->setStyleClass("form-group full-width");
    logoUrlGroup->addWidget(std::make_unique<Wt::WText>("Logo URL"))->setStyleClass("form-label");
    auto logoUrlInput = logoUrlGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    logoUrlInput->setPlaceholderText("https://example.com/logo.png");
    logoUrlInput->setStyleClass("form-control");
    if (appConfig.hasCustomLogo()) {
        logoUrlInput->setText(appConfig.getBrandLogoPath());
    }
    logoUrlGroup->addWidget(std::make_unique<Wt::WText>("Enter a direct URL to your logo image (PNG, JPG, SVG)"))->setStyleClass("form-help");

    // Preview update button
    auto previewBtnContainer = brandingSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    previewBtnContainer->setStyleClass("preview-btn-container");
    auto previewBtn = previewBtnContainer->addWidget(std::make_unique<Wt::WPushButton>("Preview Logo"));
    previewBtn->setStyleClass("btn btn-secondary");

    // Connect preview button
    previewBtn->clicked().connect([logoUrlInput, logoPreview] {
        std::string newUrl = logoUrlInput->text().toUTF8();
        if (!newUrl.empty()) {
            logoPreview->setImageLink(Wt::WLink(newUrl));
        }
    });

    // Reset to default button
    auto logoResetBtn = previewBtnContainer->addWidget(std::make_unique<Wt::WPushButton>("Reset to Default"));
    logoResetBtn->setStyleClass("btn btn-outline");

    logoResetBtn->clicked().connect([this, logoUrlInput, logoPreview] {
        auto& appConfig = AppConfig::instance();
        logoUrlInput->setText("");
        logoPreview->setImageLink(Wt::WLink(AppConfig::getDefaultLogoUrl()));
    });

    // ===========================================
    // Tab Switching Logic
    // ===========================================
    tabFranchisee->clicked().connect([tabFranchisee, tabMarketing, tabAI, tabData, tabBranding,
                                       franchiseePanel, marketingPanel, aiPanel, dataPanel, brandingPanel] {
        tabFranchisee->setStyleClass("tab-btn active");
        tabMarketing->setStyleClass("tab-btn");
        tabAI->setStyleClass("tab-btn");
        tabData->setStyleClass("tab-btn");
        tabBranding->setStyleClass("tab-btn");
        franchiseePanel->setStyleClass("tab-panel active");
        marketingPanel->setStyleClass("tab-panel");
        aiPanel->setStyleClass("tab-panel");
        dataPanel->setStyleClass("tab-panel");
        brandingPanel->setStyleClass("tab-panel");
    });

    tabMarketing->clicked().connect([tabFranchisee, tabMarketing, tabAI, tabData, tabBranding,
                                      franchiseePanel, marketingPanel, aiPanel, dataPanel, brandingPanel] {
        tabFranchisee->setStyleClass("tab-btn");
        tabMarketing->setStyleClass("tab-btn active");
        tabAI->setStyleClass("tab-btn");
        tabData->setStyleClass("tab-btn");
        tabBranding->setStyleClass("tab-btn");
        franchiseePanel->setStyleClass("tab-panel");
        marketingPanel->setStyleClass("tab-panel active");
        aiPanel->setStyleClass("tab-panel");
        dataPanel->setStyleClass("tab-panel");
        brandingPanel->setStyleClass("tab-panel");
    });

    tabAI->clicked().connect([tabFranchisee, tabMarketing, tabAI, tabData, tabBranding,
                               franchiseePanel, marketingPanel, aiPanel, dataPanel, brandingPanel] {
        tabFranchisee->setStyleClass("tab-btn");
        tabMarketing->setStyleClass("tab-btn");
        tabAI->setStyleClass("tab-btn active");
        tabData->setStyleClass("tab-btn");
        tabBranding->setStyleClass("tab-btn");
        franchiseePanel->setStyleClass("tab-panel");
        marketingPanel->setStyleClass("tab-panel");
        aiPanel->setStyleClass("tab-panel active");
        dataPanel->setStyleClass("tab-panel");
        brandingPanel->setStyleClass("tab-panel");
    });

    tabData->clicked().connect([tabFranchisee, tabMarketing, tabAI, tabData, tabBranding,
                                 franchiseePanel, marketingPanel, aiPanel, dataPanel, brandingPanel] {
        tabFranchisee->setStyleClass("tab-btn");
        tabMarketing->setStyleClass("tab-btn");
        tabAI->setStyleClass("tab-btn");
        tabData->setStyleClass("tab-btn active");
        tabBranding->setStyleClass("tab-btn");
        franchiseePanel->setStyleClass("tab-panel");
        marketingPanel->setStyleClass("tab-panel");
        aiPanel->setStyleClass("tab-panel");
        dataPanel->setStyleClass("tab-panel active");
        brandingPanel->setStyleClass("tab-panel");
    });

    tabBranding->clicked().connect([tabFranchisee, tabMarketing, tabAI, tabData, tabBranding,
                                     franchiseePanel, marketingPanel, aiPanel, dataPanel, brandingPanel] {
        tabFranchisee->setStyleClass("tab-btn");
        tabMarketing->setStyleClass("tab-btn");
        tabAI->setStyleClass("tab-btn");
        tabData->setStyleClass("tab-btn");
        tabBranding->setStyleClass("tab-btn active");
        franchiseePanel->setStyleClass("tab-panel");
        marketingPanel->setStyleClass("tab-panel");
        aiPanel->setStyleClass("tab-panel");
        dataPanel->setStyleClass("tab-panel");
        brandingPanel->setStyleClass("tab-panel active");
    });

    // ===========================================
    // Action Buttons
    // ===========================================
    auto actionsSection = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    actionsSection->setStyleClass("form-actions");

    auto saveBtn = actionsSection->addWidget(std::make_unique<Wt::WPushButton>("Save All Settings"));
    saveBtn->setStyleClass("btn btn-primary");

    auto statusMessage = actionsSection->addWidget(std::make_unique<Wt::WText>(""));
    statusMessage->setStyleClass("settings-status-message");
    statusMessage->setHidden(true);

    // Connect save button - saves ALL tabs
    saveBtn->clicked().connect([this, saveBtn, storeCombo, nameInput, addressInput, cityInput, stateCombo, getStateCode, zipInput,
                                ownerInput, phoneInput, radiusInput,
                                sizeCombo, typeCheckboxes, openaiInput, modelSelect, geminiInput,
                                googleInput, bbbInput, censusInput, logoUrlInput, statusMessage, aiStatus,
                                penaltySliders, penaltyChecks, bonusSliders, bonusChecks]() {
        std::cout << "  [Settings] Save button clicked" << std::endl;
        auto& appConfig = AppConfig::instance();
        bool changed = false;

        // === Save Store Setup ===
        // Get store name from combo (existing) or text input (new store)
        int storeIdx = storeCombo->currentIndex();
        std::string storeName;
        if (storeIdx == 0) {
            // New store - get name from text input
            storeName = nameInput->text().toUTF8();
        } else if (storeIdx > 0 && static_cast<size_t>(storeIdx - 1) < availableStores_.size()) {
            // Existing store - get name from combo
            storeName = availableStores_[storeIdx - 1].storeName;
        }

        // Get address components
        std::string streetAddress = addressInput->text().toUTF8();
        std::string city = cityInput->text().toUTF8();
        std::string state = getStateCode(stateCombo->currentIndex());
        std::string zipCode = zipInput->text().toUTF8();

        // Build full address for geocoding
        std::string fullAddress = streetAddress;
        if (!city.empty()) {
            fullAddress += ", " + city;
        }
        if (!state.empty()) {
            fullAddress += ", " + state;
        }
        if (!zipCode.empty()) {
            fullAddress += " " + zipCode;
        }

        std::cout << "  [Settings] Store name: '" << storeName << "'" << std::endl;
        std::cout << "  [Settings] Full Address: '" << fullAddress << "'" << std::endl;

        bool geocodeSuccess = false;
        if (!storeName.empty() && !streetAddress.empty()) {
            std::cout << "  [Settings] Geocoding address..." << std::endl;
            Models::GeoLocation location = searchService_->geocodeAddress(fullAddress);

            franchisee_.storeName = storeName;
            franchisee_.address = streetAddress;
            franchisee_.ownerName = ownerInput->text().toUTF8();
            franchisee_.phone = phoneInput->text().toUTF8();

            // Store location details from separate fields (overriding geocoded values for accuracy)
            location.city = city;
            location.state = state;
            location.postalCode = zipCode;
            franchisee_.location = location;

            // Check if geocoding was successful
            geocodeSuccess = location.hasValidCoordinates();
            std::cout << "  [Settings] Geocode success: " << (geocodeSuccess ? "yes" : "no") << std::endl;
            std::cout << "  [Settings] Lat/Lng: " << location.latitude << ", " << location.longitude << std::endl;

            try {
                franchisee_.defaultSearchRadiusMiles = std::stod(radiusInput->text().toUTF8());
            } catch (...) {
                franchisee_.defaultSearchRadiusMiles = 5.0;
            }
            franchisee_.searchCriteria.radiusMiles = franchisee_.defaultSearchRadiusMiles;

            auto ranges = Models::EmployeeRange::getStandardRanges();
            int sizeIdx = sizeCombo->currentIndex();
            if (sizeIdx >= 0 && sizeIdx < static_cast<int>(ranges.size())) {
                franchisee_.searchCriteria.minEmployees = ranges[sizeIdx].minEmployees;
                franchisee_.searchCriteria.maxEmployees = ranges[sizeIdx].maxEmployees;
            }

            franchisee_.searchCriteria.clearBusinessTypes();
            std::vector<Models::BusinessType> allTypes = {
                Models::BusinessType::CORPORATE_OFFICE, Models::BusinessType::CONFERENCE_CENTER,
                Models::BusinessType::HOTEL, Models::BusinessType::MEDICAL_FACILITY,
                Models::BusinessType::EDUCATIONAL_INSTITUTION, Models::BusinessType::MANUFACTURING,
                Models::BusinessType::WAREHOUSE, Models::BusinessType::GOVERNMENT_OFFICE,
                Models::BusinessType::TECH_COMPANY, Models::BusinessType::FINANCIAL_SERVICES,
                Models::BusinessType::COWORKING_SPACE, Models::BusinessType::NONPROFIT
            };

            for (size_t i = 0; i < typeCheckboxes.size() && i < allTypes.size(); ++i) {
                if (typeCheckboxes[i]->isChecked()) {
                    franchisee_.searchCriteria.addBusinessType(allTypes[i]);
                }
            }

            franchisee_.isConfigured = geocodeSuccess;  // Only mark configured if geocoding succeeded

            // Update sidebar with all franchisee details (header + popover)
            updateHeaderWithFranchisee();

            // Sync location across all views (AI Search, Open Street Map)
            if (geocodeSuccess) {
                currentSearchLocation_ = franchisee_.getFullAddress();
                currentSearchArea_ = franchisee_.createSearchArea();
                // Don't set hasActiveSearch_ here - let the search trigger that
            }

            // Save franchisee and store location to ApiLogicServer
            if (geocodeSuccess) {
                std::cout << "  [Settings] Saving to ALS..." << std::endl;
                saveFranchiseeToALS();      // Save franchisee first (for linking)
                saveStoreLocationToALS();   // Then save store location
            } else {
                std::cout << "  [Settings] Skipping ALS save - geocode failed" << std::endl;
            }

            changed = true;
        } else {
            std::cout << "  [Settings] Skipping save - store name or address empty" << std::endl;
        }

        // === Save AI Configuration ===
        std::string openaiKey = openaiInput->text().toUTF8();
        if (!openaiKey.empty()) {
            appConfig.setOpenAIApiKey(openaiKey);
            changed = true;
        }

        std::vector<std::string> models = {"gpt-4o", "gpt-4o-mini", "gpt-4-turbo", "gpt-4", "gpt-3.5-turbo"};
        int modelIdx = modelSelect->currentIndex();
        if (modelIdx >= 0 && modelIdx < static_cast<int>(models.size())) {
            appConfig.setOpenAIModel(models[modelIdx]);
        }

        std::string geminiKey = geminiInput->text().toUTF8();
        if (!geminiKey.empty()) {
            appConfig.setGeminiApiKey(geminiKey);
            changed = true;
        }

        // === Save Scoring Optimization ===
        for (const auto& [ruleId, slider] : penaltySliders) {
            scoringEngine_->setRulePoints(ruleId, slider->value());
        }
        for (const auto& [ruleId, checkbox] : penaltyChecks) {
            scoringEngine_->setRuleEnabled(ruleId, checkbox->isChecked());
        }
        for (const auto& [ruleId, slider] : bonusSliders) {
            scoringEngine_->setRulePoints(ruleId, slider->value());
        }
        for (const auto& [ruleId, checkbox] : bonusChecks) {
            scoringEngine_->setRuleEnabled(ruleId, checkbox->isChecked());
        }
        // Persist scoring rules to ApiLogicServer
        saveScoringRulesToALS();

        // === Save Data Sources ===
        std::string googleKey = googleInput->text().toUTF8();
        if (!googleKey.empty()) {
            appConfig.setGoogleApiKey(googleKey);
            changed = true;
        }

        std::string bbbKey = bbbInput->text().toUTF8();
        if (!bbbKey.empty()) {
            appConfig.setBBBApiKey(bbbKey);
            changed = true;
        }

        std::string censusKey = censusInput->text().toUTF8();
        if (!censusKey.empty()) {
            appConfig.setCensusApiKey(censusKey);
            changed = true;
        }

        // === Save Branding ===
        std::string logoUrl = logoUrlInput->text().toUTF8();
        std::string currentLogoPath = appConfig.hasCustomLogo() ? appConfig.getBrandLogoPath() : "";
        if (logoUrl != currentLogoPath) {
            appConfig.setBrandLogoPath(logoUrl);
            // Update the sidebar logo
            if (sidebar_) {
                sidebar_->setLogoUrl(logoUrl.empty() ? AppConfig::getDefaultLogoUrl() : logoUrl);
            }
            changed = true;
        }

        if (changed) {
            appConfig.saveToFile("config/app_config.json");

            if (appConfig.hasOpenAIKey()) {
                searchService_->setAIProvider(Services::AIProvider::OPENAI, appConfig.getOpenAIApiKey());
                aiStatus->setText("AI Engine: OpenAI (" + appConfig.getOpenAIModel() + ")");
                aiStatus->setStyleClass("status-indicator status-configured");
            } else if (appConfig.hasGeminiKey()) {
                searchService_->setAIProvider(Services::AIProvider::GEMINI, appConfig.getGeminiApiKey());
                aiStatus->setText("AI Engine: Google Gemini");
                aiStatus->setStyleClass("status-indicator status-configured");
            }

            if (!geocodeSuccess && !streetAddress.empty()) {
                statusMessage->setText("Settings saved, but address could not be geocoded. Check the address and try again.");
                statusMessage->setStyleClass("settings-status-message status-warning");
            } else {
                statusMessage->setText("✓ All settings saved successfully!");
                statusMessage->setStyleClass("settings-status-message status-success");
            }
            statusMessage->setHidden(false);

            // Hide save button, show status message - toggle visibility to prevent layout shift
            std::string buttonId = saveBtn->id();
            std::string messageId = statusMessage->id();

            // Immediately hide button and show message
            std::ostringstream hideButtonJs;
            hideButtonJs << "var btn = document.getElementById('" << buttonId << "');"
                         << "var msg = document.getElementById('" << messageId << "');"
                         << "if (btn) { btn.style.display = 'none'; }"
                         << "if (msg) { msg.style.opacity = '1'; msg.style.display = 'inline-block'; }";
            doJavaScript(hideButtonJs.str());

            // After 4 seconds, fade out message and show button again
            std::ostringstream fadeJs;
            fadeJs << "setTimeout(function() {"
                   << "  var btn = document.getElementById('" << buttonId << "');"
                   << "  var msg = document.getElementById('" << messageId << "');"
                   << "  if (msg) {"
                   << "    msg.style.transition = 'opacity 0.5s ease-out';"
                   << "    msg.style.opacity = '0';"
                   << "    setTimeout(function() {"
                   << "      if (msg) { msg.style.display = 'none'; }"
                   << "      if (btn) { btn.style.display = 'inline-block'; }"
                   << "    }, 500);"
                   << "  }"
                   << "}, 4000);";
            doJavaScript(fadeJs.str());

            // Clear password fields
            openaiInput->setText("");
            geminiInput->setText("");
            googleInput->setText("");
            bbbInput->setText("");
            censusInput->setText("");
        } else {
            statusMessage->setText("No changes to save.");
            statusMessage->setStyleClass("settings-status-message status-info");
            statusMessage->setHidden(false);
        }
    });
}

void FranchiseApp::showAuditTrailPage() {
    // Admin-only page - check role
    if (currentUser_.role != "admin") {
        std::cout << "[FranchiseApp] Non-admin user attempted to access Audit Trail" << std::endl;
        showDashboardPage();
        return;
    }

    workArea_->clear();
    navigation_->setPageTitle("Audit Trail");
    navigation_->setBreadcrumbs({"Home", "Admin", "Audit Trail"});
    navigation_->setMarketScore(-1);

    // Add the AuditTrailPage widget
    workArea_->addWidget(std::make_unique<Widgets::AuditTrailPage>());
}

void FranchiseApp::loadStoreLocationFromALS() {
    std::cout << "  [App] Loading store location from ALS..." << std::endl;

    // First, get the saved current_store_id from app_config
    std::string savedStoreId = alsClient_->getAppConfigValue("current_store_id");
    std::cout << "  [App] current_store_id from AppConfig: '" << savedStoreId << "'" << std::endl;

    if (!savedStoreId.empty()) {
        // IMPORTANT: Set the member variable from AppConfig cache FIRST
        // This ensures PATCH (not POST) on save, even if fetch fails
        currentStoreLocationId_ = savedStoreId;
        std::cout << "  [App] Set currentStoreLocationId_ = " << currentStoreLocationId_ << std::endl;

        // Now fetch the full store data to populate the UI
        std::cout << "  [App] Fetching StoreLocation by ID: " << savedStoreId << std::endl;
        auto response = alsClient_->getStoreLocation(savedStoreId);
        std::cout << "  [App] StoreLocation response success: " << (response.success ? "true" : "false") << std::endl;

        if (response.success) {
            auto loc = Services::StoreLocationDTO::fromJson(response.body);
            std::cout << "  [App] Parsed StoreLocation: id='" << loc.id << "', name='" << loc.storeName << "'" << std::endl;
            if (!loc.id.empty()) {
                franchisee_.storeId = loc.id;
                franchisee_.storeName = loc.storeName;
                franchisee_.address = loc.addressLine1;
                franchisee_.location.city = loc.city;
                franchisee_.location.state = loc.stateProvince;
                franchisee_.location.postalCode = loc.postalCode;
                franchisee_.location.latitude = loc.latitude;
                franchisee_.location.longitude = loc.longitude;
                franchisee_.location.isValid = true;  // Mark location as valid for hasValidCoordinates()
                franchisee_.defaultSearchRadiusMiles = loc.defaultSearchRadiusMiles;
                franchisee_.phone = loc.phone;
                franchisee_.email = loc.email;
                franchisee_.isConfigured = true;

                // Load search criteria
                franchisee_.searchCriteria.radiusMiles = loc.defaultSearchRadiusMiles;
                franchisee_.searchCriteria.minEmployees = loc.minEmployees;
                franchisee_.searchCriteria.maxEmployees = loc.maxEmployees;
                franchisee_.searchCriteria.includeOpenStreetMap = loc.includeOpenStreetMap;
                franchisee_.searchCriteria.includeGooglePlaces = loc.includeGooglePlaces;
                franchisee_.searchCriteria.includeBBB = loc.includeBBB;

                // Parse business types from comma-separated string
                if (!loc.targetBusinessTypes.empty()) {
                    franchisee_.searchCriteria.clearBusinessTypes();
                    std::string types = loc.targetBusinessTypes;
                    size_t pos = 0;
                    while ((pos = types.find(',')) != std::string::npos || !types.empty()) {
                        std::string token;
                        if (pos != std::string::npos) {
                            token = types.substr(0, pos);
                            types.erase(0, pos + 1);
                        } else {
                            token = types;
                            types.clear();
                        }
                        try {
                            int typeInt = std::stoi(token);
                            franchisee_.searchCriteria.addBusinessType(static_cast<Models::BusinessType>(typeInt));
                        } catch (...) {}
                    }
                }

                std::cout << "  [App] Store location loaded successfully: " << loc.storeName
                          << " at " << loc.latitude << ", " << loc.longitude << std::endl;
                std::cout << "  [App] Search criteria loaded: minEmp=" << franchisee_.searchCriteria.minEmployees
                          << ", maxEmp=" << franchisee_.searchCriteria.maxEmployees
                          << ", types=" << franchisee_.searchCriteria.businessTypes.size() << std::endl;

                // Sync location across all views (AI Search, Open Street Map)
                currentSearchLocation_ = franchisee_.getFullAddress();
                currentSearchArea_ = franchisee_.createSearchArea();

                // Load prospects linked to this store
                loadProspectsFromALS();

                return;
            }
        } else {
            std::cout << "  [App] Failed to fetch StoreLocation: " << response.body << std::endl;
        }
    } else {
        std::cout << "  [App] No current_store_id found in AppConfig" << std::endl;
    }

    // No saved store - franchisee remains unconfigured
    // User will need to select a store from the Settings page
    std::cout << "  [App] No store location configured" << std::endl;
    franchisee_.isConfigured = false;
}

bool FranchiseApp::saveStoreLocationToALS() {
    std::cout << "  [App] Saving store location to ApiLogicServer..." << std::endl;

    Services::StoreLocationDTO dto;
    dto.id = currentStoreLocationId_;  // Empty for new, set for update
    dto.franchiseeId = currentFranchiseeId_;  // Link to current franchisee
    dto.storeName = franchisee_.storeName;
    dto.addressLine1 = franchisee_.address;
    dto.city = franchisee_.location.city;
    dto.stateProvince = franchisee_.location.state;
    dto.postalCode = franchisee_.location.postalCode;
    dto.latitude = franchisee_.location.latitude;
    dto.longitude = franchisee_.location.longitude;
    dto.defaultSearchRadiusMiles = franchisee_.defaultSearchRadiusMiles;
    dto.phone = franchisee_.phone;
    dto.email = franchisee_.email;
    dto.geocodeSource = "nominatim";
    dto.isPrimary = true;
    dto.isActive = true;

    // Search criteria
    dto.minEmployees = franchisee_.searchCriteria.minEmployees;
    dto.maxEmployees = franchisee_.searchCriteria.maxEmployees;
    dto.includeOpenStreetMap = franchisee_.searchCriteria.includeOpenStreetMap;
    dto.includeGooglePlaces = franchisee_.searchCriteria.includeGooglePlaces;
    dto.includeBBB = franchisee_.searchCriteria.includeBBB;

    // Convert business types to comma-separated string
    std::string types;
    for (const auto& bt : franchisee_.searchCriteria.businessTypes) {
        if (!types.empty()) types += ",";
        types += std::to_string(static_cast<int>(bt));
    }
    dto.targetBusinessTypes = types;

    std::cout << "  [App] Saving search criteria: minEmp=" << dto.minEmployees
              << ", maxEmp=" << dto.maxEmployees
              << ", types=" << dto.targetBusinessTypes << std::endl;

    auto response = alsClient_->saveStoreLocation(dto);

    if (response.success) {
        // Parse the response to get the ID if this was a create
        if (currentStoreLocationId_.empty()) {
            auto created = Services::StoreLocationDTO::fromJson(response.body);
            if (!created.id.empty()) {
                currentStoreLocationId_ = created.id;
                franchisee_.storeId = created.id;
            }
        }

        // Save the current store ID to app_config so it loads on next startup
        if (!currentStoreLocationId_.empty()) {
            alsClient_->setAppConfigValue("current_store_id", currentStoreLocationId_);
        }

        return true;
    } else {
        std::cerr << "  [App] Failed to save to ALS: " << response.errorMessage << std::endl;
        return false;
    }
}

std::vector<Services::StoreLocationDTO> FranchiseApp::loadAvailableStores() {
    auto response = alsClient_->getStoreLocations();

    if (response.success) {
        availableStores_ = Services::ApiLogicServerClient::parseStoreLocations(response);
    }

    return availableStores_;
}

void FranchiseApp::selectStoreById(const std::string& storeId) {
    // Validate storeId to prevent empty UUID queries
    if (storeId.empty()) {
        std::cerr << "  [App] selectStoreById: empty storeId, ignoring" << std::endl;
        return;
    }

    // Find the store in cached list or load it
    Services::StoreLocationDTO selectedStore;
    bool found = false;

    for (const auto& store : availableStores_) {
        if (store.id == storeId) {
            selectedStore = store;
            found = true;
            break;
        }
    }

    if (!found) {
        // Load directly from API
        auto response = alsClient_->getStoreLocation(storeId);
        if (response.success) {
            selectedStore = Services::StoreLocationDTO::fromJson(response.body);
            found = !selectedStore.id.empty();
        }
    }

    if (found) {
        currentStoreLocationId_ = selectedStore.id;
        franchisee_.storeId = selectedStore.id;
        franchisee_.storeName = selectedStore.storeName;
        franchisee_.address = selectedStore.addressLine1;
        franchisee_.location.city = selectedStore.city;
        franchisee_.location.state = selectedStore.stateProvince;
        franchisee_.location.postalCode = selectedStore.postalCode;
        franchisee_.location.latitude = selectedStore.latitude;
        franchisee_.location.longitude = selectedStore.longitude;
        franchisee_.location.isValid = true;
        franchisee_.defaultSearchRadiusMiles = selectedStore.defaultSearchRadiusMiles;
        franchisee_.phone = selectedStore.phone;
        franchisee_.email = selectedStore.email;
        franchisee_.isConfigured = true;

        // Load search criteria from selected store
        franchisee_.searchCriteria.radiusMiles = selectedStore.defaultSearchRadiusMiles;
        franchisee_.searchCriteria.minEmployees = selectedStore.minEmployees;
        franchisee_.searchCriteria.maxEmployees = selectedStore.maxEmployees;
        franchisee_.searchCriteria.includeOpenStreetMap = selectedStore.includeOpenStreetMap;
        franchisee_.searchCriteria.includeGooglePlaces = selectedStore.includeGooglePlaces;
        franchisee_.searchCriteria.includeBBB = selectedStore.includeBBB;

        // Parse business types from comma-separated string
        if (!selectedStore.targetBusinessTypes.empty()) {
            franchisee_.searchCriteria.clearBusinessTypes();
            std::string types = selectedStore.targetBusinessTypes;
            size_t pos = 0;
            while ((pos = types.find(',')) != std::string::npos || !types.empty()) {
                std::string token;
                if (pos != std::string::npos) {
                    token = types.substr(0, pos);
                    types.erase(0, pos + 1);
                } else {
                    token = types;
                    types.clear();
                }
                try {
                    int typeInt = std::stoi(token);
                    franchisee_.searchCriteria.addBusinessType(static_cast<Models::BusinessType>(typeInt));
                } catch (...) {}
            }
        }

        // Save as current store
        alsClient_->setAppConfigValue("current_store_id", storeId);

        // Update sidebar with full franchisee details
        updateHeaderWithFranchisee();

        // Sync location across all views (AI Search, Open Street Map)
        currentSearchLocation_ = franchisee_.getFullAddress();
        currentSearchArea_ = franchisee_.createSearchArea();

        // Load prospects linked to this store
        loadProspectsFromALS();

        std::cout << "  [App] Selected store: " << selectedStore.storeName
                  << " at " << selectedStore.city << ", " << selectedStore.stateProvince << std::endl;
        std::cout << "  [App] Search criteria loaded: minEmp=" << franchisee_.searchCriteria.minEmployees
                  << ", maxEmp=" << franchisee_.searchCriteria.maxEmployees << std::endl;
    }
}

// ============================================================================
// Franchisee ALS Integration
// ============================================================================

void FranchiseApp::loadFranchiseeFromALS() {
    std::cout << "  [App] Loading franchisee from ALS..." << std::endl;

    // Get the saved current_franchisee_id from app_config
    std::string savedFranchiseeId = alsClient_->getAppConfigValue("current_franchisee_id");
    std::cout << "  [App] current_franchisee_id from AppConfig: '" << savedFranchiseeId << "'" << std::endl;

    if (!savedFranchiseeId.empty()) {
        // IMPORTANT: Set the member variable from AppConfig cache FIRST
        // This ensures PATCH (not POST) on save, even if fetch fails
        currentFranchiseeId_ = savedFranchiseeId;
        std::cout << "  [App] Set currentFranchiseeId_ = " << currentFranchiseeId_ << std::endl;

        // Now fetch the full franchisee data to populate the UI
        std::cout << "  [App] Fetching Franchisee by ID: " << savedFranchiseeId << std::endl;
        auto response = alsClient_->getFranchisee(savedFranchiseeId);
        std::cout << "  [App] Franchisee response success: " << (response.success ? "true" : "false") << std::endl;

        if (response.success) {
            auto dto = Services::FranchiseeDTO::fromJson(response.body);
            if (!dto.id.empty()) {
                // Update franchisee_ with loaded data
                franchisee_.ownerName = dto.ownerFirstName;
                if (!dto.ownerLastName.empty()) {
                    franchisee_.ownerName += " " + dto.ownerLastName;
                }
                franchisee_.phone = dto.phone;
                franchisee_.email = dto.email;
                // Address info can be used if store location isn't set
                if (franchisee_.address.empty()) {
                    franchisee_.address = dto.addressLine1;
                    franchisee_.location.city = dto.city;
                    franchisee_.location.state = dto.stateProvince;
                    franchisee_.location.postalCode = dto.postalCode;
                    franchisee_.location.latitude = dto.latitude;
                    franchisee_.location.longitude = dto.longitude;
                    franchisee_.location.isValid = true;  // Mark location as valid
                }
                std::cout << "  [App] Loaded franchisee: " << dto.businessName << std::endl;
                return;
            } else {
                std::cout << "  [App] Franchisee DTO has empty ID" << std::endl;
            }
        } else {
            std::cout << "  [App] Failed to fetch Franchisee" << std::endl;
        }
    } else {
        std::cout << "  [App] No current_franchisee_id found in AppConfig" << std::endl;
    }

    // No saved franchisee - will be created when user saves settings
    std::cout << "  [App] No franchisee configured" << std::endl;
}

bool FranchiseApp::saveFranchiseeToALS() {
    std::cout << "  [App] Saving franchisee to ApiLogicServer..." << std::endl;

    Services::FranchiseeDTO dto;
    dto.id = currentFranchiseeId_;  // Empty for new, set for update
    dto.businessName = franchisee_.storeName.empty() ? franchisee_.franchiseName : franchisee_.storeName;

    // Parse owner name into first/last
    std::string ownerName = franchisee_.ownerName;
    size_t spacePos = ownerName.find(' ');
    if (spacePos != std::string::npos) {
        dto.ownerFirstName = ownerName.substr(0, spacePos);
        dto.ownerLastName = ownerName.substr(spacePos + 1);
    } else {
        dto.ownerFirstName = ownerName;
    }

    dto.phone = franchisee_.phone;
    dto.email = franchisee_.email;
    dto.addressLine1 = franchisee_.address;
    dto.city = franchisee_.location.city;
    dto.stateProvince = franchisee_.location.state;
    dto.postalCode = franchisee_.location.postalCode;
    dto.latitude = franchisee_.location.latitude;
    dto.longitude = franchisee_.location.longitude;
    dto.isActive = true;

    auto response = alsClient_->saveFranchisee(dto);

    if (response.success) {
        // Parse the response to get the ID if this was a create
        if (currentFranchiseeId_.empty()) {
            auto created = Services::FranchiseeDTO::fromJson(response.body);
            if (!created.id.empty()) {
                currentFranchiseeId_ = created.id;
                std::cout << "  [App] Created franchisee with ID: " << currentFranchiseeId_ << std::endl;
            }
        }

        // Save the current franchisee ID to app_config so it loads on next startup
        if (!currentFranchiseeId_.empty()) {
            alsClient_->setAppConfigValue("current_franchisee_id", currentFranchiseeId_);
        }

        return true;
    } else {
        std::cerr << "  [App] Failed to save franchisee to ALS: " << response.errorMessage << std::endl;
        return false;
    }
}

std::vector<Services::FranchiseeDTO> FranchiseApp::loadAvailableFranchisees() {
    auto response = alsClient_->getFranchisees();

    if (response.success) {
        availableFranchisees_ = Services::ApiLogicServerClient::parseFranchisees(response);
    }

    return availableFranchisees_;
}

void FranchiseApp::selectFranchiseeById(const std::string& franchiseeId) {
    // Validate franchiseeId to prevent empty UUID queries
    if (franchiseeId.empty()) {
        std::cerr << "  [App] selectFranchiseeById: empty franchiseeId, ignoring" << std::endl;
        return;
    }

    // Find the franchisee in cached list or load it
    Services::FranchiseeDTO selectedFranchisee;
    bool found = false;

    for (const auto& f : availableFranchisees_) {
        if (f.id == franchiseeId) {
            selectedFranchisee = f;
            found = true;
            break;
        }
    }

    if (!found) {
        // Load directly from API
        auto response = alsClient_->getFranchisee(franchiseeId);
        if (response.success) {
            selectedFranchisee = Services::FranchiseeDTO::fromJson(response.body);
            found = !selectedFranchisee.id.empty();
        }
    }

    if (found) {
        currentFranchiseeId_ = selectedFranchisee.id;
        franchisee_.ownerName = selectedFranchisee.ownerFirstName;
        if (!selectedFranchisee.ownerLastName.empty()) {
            franchisee_.ownerName += " " + selectedFranchisee.ownerLastName;
        }
        franchisee_.phone = selectedFranchisee.phone;
        franchisee_.email = selectedFranchisee.email;

        // Save as current franchisee
        alsClient_->setAppConfigValue("current_franchisee_id", franchiseeId);

        // Update sidebar
        sidebar_->setUserInfo(
            franchisee_.ownerName.empty() ? "Franchise Owner" : franchisee_.ownerName,
            franchisee_.storeName
        );
    }
}

// ============================================================================
// Scoring Rules ALS Integration
// ============================================================================

void FranchiseApp::loadScoringRulesFromALS() {
    std::cout << "  [App] Loading scoring rules from ALS..." << std::endl;

    auto response = alsClient_->getScoringRules();
    if (!response.success) {
        std::cout << "  [App] No scoring rules found in ALS, using defaults" << std::endl;
        return;
    }

    auto rules = Services::ApiLogicServerClient::parseScoringRules(response);
    if (rules.empty()) {
        std::cout << "  [App] No scoring rules returned, using defaults" << std::endl;
        return;
    }

    std::cout << "  [App] Loaded " << rules.size() << " scoring rules from ALS" << std::endl;

    // Clear and rebuild the ruleId -> UUID mapping
    scoringRuleDbIds_.clear();

    // Update the scoring engine with loaded rules
    for (const auto& dto : rules) {
        if (dto.ruleId.empty()) continue;

        // Store the database UUID for this ruleId
        if (!dto.id.empty()) {
            scoringRuleDbIds_[dto.ruleId] = dto.id;
            std::cout << "  [App] Cached rule UUID: " << dto.ruleId << " -> " << dto.id << std::endl;
        }

        // Update existing rule in scoring engine
        scoringEngine_->setRuleEnabled(dto.ruleId, dto.enabled);
        scoringEngine_->setRulePoints(dto.ruleId, dto.currentPoints);

        std::cout << "  [App] Updated rule: " << dto.ruleId
                  << " enabled=" << (dto.enabled ? "true" : "false")
                  << " points=" << dto.currentPoints << std::endl;
    }
}

bool FranchiseApp::saveScoringRulesToALS() {
    std::cout << "  [App] Saving scoring rules to ApiLogicServer..." << std::endl;

    const auto& rules = scoringEngine_->getRules();
    bool allSuccess = true;

    for (const auto& rule : rules) {
        Services::ScoringRuleDTO dto;
        dto.ruleId = rule.id;
        dto.name = rule.name;
        dto.description = rule.description;
        dto.isPenalty = rule.isPenalty;
        dto.enabled = rule.enabled;
        dto.defaultPoints = rule.defaultPoints;
        dto.currentPoints = rule.currentPoints;
        dto.minPoints = rule.minPoints;
        dto.maxPoints = rule.maxPoints;

        // Look up the database UUID from our cached mapping
        auto it = scoringRuleDbIds_.find(rule.id);
        if (it != scoringRuleDbIds_.end()) {
            dto.id = it->second;  // Use existing UUID for PATCH
            std::cout << "  [App] Using existing UUID for rule " << rule.id << ": " << dto.id << std::endl;
        } else {
            std::cout << "  [App] No existing UUID found for rule " << rule.id << ", will create new" << std::endl;
        }

        auto response = alsClient_->saveScoringRule(dto);
        if (!response.success) {
            std::cerr << "  [App] Failed to save scoring rule: " << rule.id
                      << " - " << response.errorMessage << std::endl;
            allSuccess = false;
        } else {
            std::cout << "  [App] Saved scoring rule: " << rule.id << std::endl;
            // If this was a new rule, cache the generated UUID for future saves
            if (it == scoringRuleDbIds_.end() && !dto.id.empty()) {
                scoringRuleDbIds_[rule.id] = dto.id;
            }
        }
    }

    return allSuccess;
}

// ============================================================================
// Prospect Persistence Methods
// ============================================================================

void FranchiseApp::loadProspectsFromALS() {
    if (currentFranchiseeId_.empty()) {
        std::cout << "  [App] Cannot load prospects - no franchisee selected" << std::endl;
        savedProspects_.clear();
        return;
    }

    std::cout << "  [App] Loading prospects for franchisee: " << currentFranchiseeId_ << std::endl;

    // Use the Franchisee/{id}/ProspectList endpoint
    auto response = alsClient_->getProspectsForFranchisee(currentFranchiseeId_);
    if (!response.success) {
        std::cerr << "  [App] Failed to load prospects: " << response.errorMessage << std::endl;
        return;
    }

    auto prospectDTOs = Services::ApiLogicServerClient::parseProspects(response);
    savedProspects_.clear();

    for (const auto& dto : prospectDTOs) {
        savedProspects_.push_back(dtoToProspectItem(dto));
    }

    std::cout << "  [App] Loaded " << savedProspects_.size() << " prospects from database" << std::endl;
}

bool FranchiseApp::saveProspectToALS(const Models::SearchResultItem& item) {
    if (currentFranchiseeId_.empty()) {
        std::cerr << "  [App] Cannot save prospect - no franchisee selected" << std::endl;
        return false;
    }

    Services::ProspectDTO dto = prospectItemToDTO(item);
    dto.franchiseeId = currentFranchiseeId_;

    // Set status for new prospects
    if (dto.status.empty()) {
        dto.status = "new";
    }

    auto response = alsClient_->saveProspect(dto);
    if (!response.success) {
        std::cerr << "  [App] Failed to save prospect: " << response.errorMessage << std::endl;
        return false;
    }

    std::cout << "  [App] Saved prospect to database: " << item.getTitle() << std::endl;
    return true;
}

bool FranchiseApp::deleteProspectFromALS(const std::string& prospectId) {
    if (prospectId.empty()) {
        return false;
    }

    auto response = alsClient_->deleteProspect(prospectId);
    if (!response.success) {
        std::cerr << "  [App] Failed to delete prospect: " << response.errorMessage << std::endl;
        return false;
    }

    std::cout << "  [App] Deleted prospect from database: " << prospectId << std::endl;
    return true;
}

Services::ProspectDTO FranchiseApp::prospectItemToDTO(const Models::SearchResultItem& item) {
    Services::ProspectDTO dto;

    // Copy ID if available (for updates)
    dto.id = item.id;

    if (item.business) {
        dto.businessName = item.business->name;
        dto.businessType = item.business->category;
        dto.addressLine1 = item.business->address.street1;
        dto.addressLine2 = item.business->address.street2;
        dto.city = item.business->address.city;
        dto.stateProvince = item.business->address.state;
        dto.postalCode = item.business->address.zipCode;
        dto.countryCode = item.business->address.country.empty() ? "US" : item.business->address.country;
        dto.latitude = item.business->address.latitude;
        dto.longitude = item.business->address.longitude;
        dto.primaryPhone = item.business->contact.primaryPhone;
        dto.email = item.business->contact.email;
        dto.website = item.business->contact.website;
        dto.employeeCount = item.business->employeeCount;
        dto.dataSource = Models::dataSourceToString(item.business->source);

        // Map employee count to range
        if (dto.employeeCount <= 10) dto.employeeCountRange = "1-10";
        else if (dto.employeeCount <= 50) dto.employeeCountRange = "11-50";
        else if (dto.employeeCount <= 200) dto.employeeCountRange = "51-200";
        else if (dto.employeeCount <= 500) dto.employeeCountRange = "201-500";
        else dto.employeeCountRange = "500+";
    }

    // AI and scoring fields
    dto.aiScore = static_cast<int>(item.aiConfidenceScore * 100);
    dto.optimizedScore = item.overallScore;
    dto.relevanceScore = item.relevanceScore;
    dto.aiSummary = item.aiSummary;

    // Convert key highlights to comma-separated string
    if (!item.keyHighlights.empty()) {
        std::string highlights;
        for (const auto& h : item.keyHighlights) {
            if (!highlights.empty()) highlights += "|";
            highlights += h;
        }
        dto.keyHighlights = highlights;
    }

    // Convert recommended actions to comma-separated string
    if (!item.recommendedActions.empty()) {
        std::string actions;
        for (const auto& a : item.recommendedActions) {
            if (!actions.empty()) actions += "|";
            actions += a;
        }
        dto.recommendedActions = actions;
    }

    // Convert data sources to comma-separated string
    if (!item.sources.empty()) {
        std::string sources;
        for (const auto& s : item.sources) {
            if (!sources.empty()) sources += ",";
            sources += Models::dataSourceToString(s);
        }
        dto.dataSources = sources;
    }

    dto.status = "new";

    return dto;
}

Models::SearchResultItem FranchiseApp::dtoToProspectItem(const Services::ProspectDTO& dto) {
    Models::SearchResultItem item;

    item.id = dto.id;
    item.resultType = Models::SearchResultType::BUSINESS;

    // Create business info
    auto business = std::make_shared<Models::BusinessInfo>();
    business->id = dto.id;
    business->name = dto.businessName;
    business->category = dto.businessType;
    business->address.street1 = dto.addressLine1;
    business->address.street2 = dto.addressLine2;
    business->address.city = dto.city;
    business->address.state = dto.stateProvince;
    business->address.zipCode = dto.postalCode;
    business->address.country = dto.countryCode;
    business->address.latitude = dto.latitude;
    business->address.longitude = dto.longitude;
    business->contact.primaryPhone = dto.primaryPhone;
    business->contact.secondaryPhone = dto.secondaryPhone;
    business->contact.email = dto.email;
    business->contact.website = dto.website;
    business->employeeCount = dto.employeeCount;
    business->yearEstablished = dto.yearEstablished;

    // Parse data source
    if (dto.dataSource == "OpenStreetMap") {
        business->source = Models::DataSource::OPENSTREETMAP;
    } else if (dto.dataSource == "Google My Business" || dto.dataSource == "GooglePlaces") {
        business->source = Models::DataSource::GOOGLE_MY_BUSINESS;
    } else if (dto.dataSource == "Better Business Bureau" || dto.dataSource == "BBB") {
        business->source = Models::DataSource::BBB;
    } else {
        business->source = Models::DataSource::IMPORTED;
    }

    item.business = business;

    // Restore AI and scoring fields
    item.aiConfidenceScore = dto.aiScore / 100.0;
    item.overallScore = dto.optimizedScore;
    item.relevanceScore = dto.relevanceScore;
    item.aiSummary = dto.aiSummary;

    // Parse key highlights from pipe-separated string
    if (!dto.keyHighlights.empty()) {
        std::string highlights = dto.keyHighlights;
        size_t pos = 0;
        while ((pos = highlights.find('|')) != std::string::npos || !highlights.empty()) {
            std::string token;
            if (pos != std::string::npos) {
                token = highlights.substr(0, pos);
                highlights.erase(0, pos + 1);
            } else {
                token = highlights;
                highlights.clear();
            }
            if (!token.empty()) {
                item.keyHighlights.push_back(token);
            }
        }
    }

    // Parse recommended actions from pipe-separated string
    if (!dto.recommendedActions.empty()) {
        std::string actions = dto.recommendedActions;
        size_t pos = 0;
        while ((pos = actions.find('|')) != std::string::npos || !actions.empty()) {
            std::string token;
            if (pos != std::string::npos) {
                token = actions.substr(0, pos);
                actions.erase(0, pos + 1);
            } else {
                token = actions;
                actions.clear();
            }
            if (!token.empty()) {
                item.recommendedActions.push_back(token);
            }
        }
    }

    // Parse data sources from comma-separated string
    if (!dto.dataSources.empty()) {
        std::string sources = dto.dataSources;
        size_t pos = 0;
        while ((pos = sources.find(',')) != std::string::npos || !sources.empty()) {
            std::string token;
            if (pos != std::string::npos) {
                token = sources.substr(0, pos);
                sources.erase(0, pos + 1);
            } else {
                token = sources;
                sources.clear();
            }
            if (!token.empty()) {
                if (token == "OpenStreetMap") {
                    item.sources.push_back(Models::DataSource::OPENSTREETMAP);
                } else if (token == "Google My Business" || token == "GooglePlaces") {
                    item.sources.push_back(Models::DataSource::GOOGLE_MY_BUSINESS);
                } else if (token == "Better Business Bureau" || token == "BBB") {
                    item.sources.push_back(Models::DataSource::BBB);
                } else if (token == "Demographics") {
                    item.sources.push_back(Models::DataSource::DEMOGRAPHICS);
                } else {
                    item.sources.push_back(Models::DataSource::IMPORTED);
                }
            }
        }
    }

    return item;
}

std::unique_ptr<Wt::WApplication> createFranchiseApp(const Wt::WEnvironment& env) {
    return std::make_unique<FranchiseApp>(env);
}

} // namespace FranchiseAI
