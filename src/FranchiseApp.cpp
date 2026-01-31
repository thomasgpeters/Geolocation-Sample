#include "FranchiseApp.h"
#include "AppConfig.h"
#include "models/GeoLocation.h"
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
#include <sstream>
#include <iomanip>

namespace FranchiseAI {

FranchiseApp::FranchiseApp(const Wt::WEnvironment& env)
    : Wt::WApplication(env)
{
    setTitle("FranchiseAI - Prospect Search");

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

    // Load styles
    loadStyleSheet();

    // Setup UI
    setupUI();

    // Setup routing
    setupRouting();

    // Show setup page if franchisee not configured, otherwise show AI Search
    if (!franchisee_.isConfigured) {
        sidebar_->setActiveItem("setup");
        showSetupPage();
    } else {
        sidebar_->setActiveItem("ai-search");
        showAISearchPage();
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

void FranchiseApp::setupUI() {
    // Main container - full viewport layout
    mainContainer_ = root()->addWidget(std::make_unique<Wt::WContainerWidget>());
    mainContainer_->setStyleClass("app-container");

    // Sidebar
    sidebar_ = mainContainer_->addWidget(std::make_unique<Widgets::Sidebar>());
    sidebar_->setUserInfo("John Smith", "ABC Catering Franchise");

    // Connect sidebar signals
    sidebar_->itemSelected().connect(this, &FranchiseApp::onMenuItemSelected);

    // Content area (navigation + work area)
    contentArea_ = mainContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    contentArea_->setStyleClass("content-area");

    // Top navigation
    navigation_ = contentArea_->addWidget(std::make_unique<Widgets::Navigation>());
    navigation_->quickSearchSubmitted().connect(this, &FranchiseApp::onQuickSearch);

    // Work area
    workArea_ = contentArea_->addWidget(std::make_unique<Wt::WContainerWidget>());
    workArea_->setStyleClass("work-area");
}

void FranchiseApp::setupRouting() {
    // Internal path handling
    internalPathChanged().connect([this] {
        std::string path = internalPath();

        if (path == "/setup") {
            sidebar_->setActiveItem("setup");
            showSetupPage();
        } else if (path == "/dashboard") {
            sidebar_->setActiveItem("dashboard");
            showDashboardPage();
        } else if (path == "/search" || path == "/ai-search") {
            sidebar_->setActiveItem("ai-search");
            showAISearchPage();
        } else if (path == "/prospects") {
            sidebar_->setActiveItem("prospects");
            showProspectsPage();
        } else if (path == "/demographics") {
            sidebar_->setActiveItem("demographics");
            showDemographicsPage();
        } else if (path == "/reports") {
            sidebar_->setActiveItem("reports");
            showReportsPage();
        } else if (path == "/settings") {
            sidebar_->setActiveItem("settings");
            showSettingsPage();
        } else {
            // Default to setup if not configured, otherwise search
            if (!franchisee_.isConfigured) {
                sidebar_->setActiveItem("setup");
                showSetupPage();
            } else {
                sidebar_->setActiveItem("ai-search");
                showAISearchPage();
            }
        }
    });
}

void FranchiseApp::onMenuItemSelected(const std::string& itemId) {
    currentPage_ = itemId;

    // Map menu item IDs to internal paths
    // The internalPathChanged handler will show the appropriate page
    if (itemId == "setup") {
        setInternalPath("/setup", false);
        showSetupPage();
    } else if (itemId == "dashboard") {
        setInternalPath("/dashboard", false);
        showDashboardPage();
    } else if (itemId == "ai-search") {
        setInternalPath("/search", false);
        showAISearchPage();
    } else if (itemId == "prospects") {
        setInternalPath("/prospects", false);
        showProspectsPage();
    } else if (itemId == "demographics") {
        setInternalPath("/demographics", false);
        showDemographicsPage();
    } else if (itemId == "reports") {
        setInternalPath("/reports", false);
        showReportsPage();
    } else if (itemId == "settings") {
        setInternalPath("/settings", false);
        showSettingsPage();
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

    // Store the search context for syncing with Demographics page
    currentSearchLocation_ = query.location;
    if (query.latitude != 0 && query.longitude != 0) {
        Models::GeoLocation location(query.latitude, query.longitude);
        location.formattedAddress = query.location;
        currentSearchArea_ = Models::SearchArea::fromMiles(location, query.radiusMiles);
    } else if (!query.location.empty()) {
        Models::GeoLocation location = searchService_->geocodeAddress(query.location);
        currentSearchArea_ = Models::SearchArea::fromMiles(location, query.radiusMiles);
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

    // Perform search
    searchService_->search(
        query,
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

    if (resultsDisplay_) {
        if (results.errorMessage.empty()) {
            resultsDisplay_->showResults(results);
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
    // Find the item and add to prospects list
    for (const auto& item : lastResults_.items) {
        if (item.id == id) {
            // Show confirmation
            auto dialog = addChild(std::make_unique<Wt::WMessageBox>(
                "Added to Prospects",
                item.getTitle() + " has been added to your prospects list.",
                Wt::Icon::Information,
                Wt::StandardButton::Ok
            ));
            dialog->show();
            break;
        }
    }
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

    // Navigate to AI Search page
    sidebar_->setActiveItem("ai-search");
    setInternalPath("/search", false);
    showAISearchPage();
}

void FranchiseApp::updateHeaderWithFranchisee() {
    if (sidebar_ && franchisee_.isConfigured) {
        sidebar_->setUserInfo(
            franchisee_.getDisplayName(),
            franchisee_.getLocationDisplay()
        );
    }
}

void FranchiseApp::showSetupPage() {
    workArea_->clear();
    navigation_->setPageTitle("Store Setup");
    navigation_->setBreadcrumbs({"Home", "Setup"});
    navigation_->setMarketScore(-1);  // Hide market score on non-demographics pages

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
        // Use the current search state (synced from Demographics or previous search)
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
        defaultQuery.location = franchisee_.address;
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

    auto title = header->addWidget(std::make_unique<Wt::WText>("My Prospects"));
    title->setStyleClass("page-title");

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
}

void FranchiseApp::showDemographicsPage() {
    workArea_->clear();
    navigation_->setPageTitle("Demographics");
    navigation_->setBreadcrumbs({"Home", "Demographics"});

    auto container = workArea_->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setStyleClass("page-container demographics-page");

    // Pre-fill with current search location or franchisee location
    std::string defaultLocation = "Denver, CO";
    double defaultRadiusKm = 10.0;
    Models::SearchArea initialSearchArea;

    if (hasActiveSearch_ && currentSearchArea_.center.hasValidCoordinates()) {
        defaultLocation = currentSearchLocation_;
        defaultRadiusKm = currentSearchArea_.radiusKm;
        initialSearchArea = currentSearchArea_;
    } else if (franchisee_.isConfigured && franchisee_.hasValidLocation()) {
        defaultLocation = franchisee_.address;
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
    mapDiv->setStyleClass("demographics-map");
    std::string mapId = mapDiv->id();

    // Store coordinates for JavaScript
    double initLat = initialSearchArea.center.latitude;
    double initLon = initialSearchArea.center.longitude;

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
              << "  function initDemographicsMap() {"
              << "    var mapEl = document.getElementById('" << mapId << "');"
              << "    if (!mapEl) { setTimeout(initDemographicsMap, 100); return; }"
              << "    if (mapEl._leaflet_map) return;"
              << "    if (typeof L === 'undefined') { setTimeout(initDemographicsMap, 100); return; }"
              // Create the map
              << "    try {"
              << "      var map = L.map('" << mapId << "').setView([" << initLat << ", " << initLon << "], 13);"
              << "      L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {"
              << "        maxZoom: 19,"
              << "        attribution: '&copy; <a href=\"https://www.openstreetmap.org/copyright\">OpenStreetMap</a> contributors'"
              << "      }).addTo(map);"
              << "      mapEl._leaflet_map = map;"
              << "      window.demographicsMap = map;"
              << "      setTimeout(function() { map.invalidateSize(); }, 300);"
              << "    } catch(e) { console.error('Map init error:', e); }"
              << "  }"
              << "  loadLeafletJS(function() { setTimeout(initDemographicsMap, 100); });"
              << "})();";

    doJavaScript(initMapJs.str());

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

    // Function to refresh all POI markers
    auto refreshMarkers = std::make_shared<std::function<void()>>();
    *refreshMarkers = [this, activePills, currentSearchAreaPtr]() {
        // Clear existing markers
        std::ostringstream clearMarkersJs;
        clearMarkersJs << "if (window.demographicsMarkers) {"
                      << "  window.demographicsMarkers.forEach(function(m) { m.remove(); });"
                      << "}"
                      << "window.demographicsMarkers = [];";
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

                std::string safeName = poi.name;
                for (auto& c : safeName) {
                    if (c == '\'' || c == '"' || c == '\\') c = ' ';
                }

                // Create colored circle marker with deep vivid color
                std::ostringstream addMarkerJs;
                addMarkerJs << "if (window.demographicsMap && typeof L !== 'undefined') {"
                           << "  var markerIcon = L.divIcon({"
                           << "    className: 'custom-marker',"
                           << "    html: '<div style=\"background-color: " << pill.markerColor << "; "
                           << "      width: 22px; height: 22px; border-radius: 50%; "
                           << "      border: 2px solid rgba(0,0,0,0.5); "
                           << "      box-shadow: 0 2px 4px rgba(0,0,0,0.4), inset 0 1px 2px rgba(255,255,255,0.3);\"></div>',"
                           << "    iconSize: [22, 22],"
                           << "    iconAnchor: [11, 11]"
                           << "  });"
                           << "  var marker = L.marker([" << poi.latitude << ", " << poi.longitude << "], {icon: markerIcon})"
                           << "    .addTo(window.demographicsMap)"
                           << "    .bindPopup('<b>" << safeName << "</b><br><small>" << pill.displayName << "</small>');"
                           << "  if (!window.demographicsMarkers) window.demographicsMarkers = [];"
                           << "  window.demographicsMarkers.push(marker);"
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
        clearMarkersJs << "if (window.demographicsMarkers) {"
                      << "  window.demographicsMarkers.forEach(function(m) { m.remove(); });"
                      << "}"
                      << "window.demographicsMarkers = [];";
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
        panMapJs << "if (window.demographicsMap) {"
                 << "  window.demographicsMap.setView([" << geoLocation.latitude << ", " << geoLocation.longitude << "], 13);"
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
            panMapJs << "if (window.demographicsMap) {"
                     << "  window.demographicsMap.setView([" << geoLocation.latitude << ", " << geoLocation.longitude << "], 13);"
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

    auto title = header->addWidget(std::make_unique<Wt::WText>("Reports & Analytics"));
    title->setStyleClass("page-title");

    auto placeholder = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    placeholder->setStyleClass("placeholder-content");

    auto icon = placeholder->addWidget(std::make_unique<Wt::WText>("📈"));
    icon->setStyleClass("placeholder-icon");

    auto text = placeholder->addWidget(std::make_unique<Wt::WText>(
        "Detailed reports and analytics for your prospect discovery efforts."
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
        "Configure API keys and application preferences"
    ));
    subtitle->setStyleClass("page-subtitle");

    // Get current configuration
    auto& appConfig = AppConfig::instance();

    // ===========================================
    // AI Configuration Section (OpenAI / Gemini)
    // ===========================================
    auto aiSection = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    aiSection->setStyleClass("settings-section");

    auto aiSectionTitle = aiSection->addWidget(std::make_unique<Wt::WText>("AI Configuration"));
    aiSectionTitle->setStyleClass("section-title");

    auto aiSectionDesc = aiSection->addWidget(std::make_unique<Wt::WText>(
        "Configure your AI provider for intelligent prospect analysis and recommendations."
    ));
    aiSectionDesc->setStyleClass("section-description");

    // AI Provider status indicator
    auto aiStatusContainer = aiSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    aiStatusContainer->setStyleClass("api-status-container");

    bool aiConfigured = searchService_->isAIEngineConfigured();
    std::string aiStatusClass = aiConfigured ? "status-indicator status-configured" : "status-indicator status-not-configured";
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
    aiStatus->setStyleClass(aiStatusClass);

    // Form grid for AI settings
    auto aiFormGrid = aiSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    aiFormGrid->setStyleClass("form-grid");

    // OpenAI API Key
    auto openaiGroup = aiFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    openaiGroup->setStyleClass("form-group");
    auto openaiLabel = openaiGroup->addWidget(std::make_unique<Wt::WText>("OpenAI API Key"));
    openaiLabel->setStyleClass("form-label");
    auto openaiInput = openaiGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    openaiInput->setPlaceholderText("sk-...");
    openaiInput->setStyleClass("form-control");
    openaiInput->setAttributeValue("type", "password");
    if (appConfig.hasOpenAIKey()) {
        // Show masked key
        openaiInput->setPlaceholderText("sk-****...****(configured)");
    }
    auto openaiHelp = openaiGroup->addWidget(std::make_unique<Wt::WText>(
        "Get your API key from platform.openai.com"
    ));
    openaiHelp->setStyleClass("form-help");

    // OpenAI Model Selection
    auto modelGroup = aiFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    modelGroup->setStyleClass("form-group");
    auto modelLabel = modelGroup->addWidget(std::make_unique<Wt::WText>("OpenAI Model"));
    modelLabel->setStyleClass("form-label");
    auto modelSelect = modelGroup->addWidget(std::make_unique<Wt::WComboBox>());
    modelSelect->setStyleClass("form-control");
    modelSelect->addItem("gpt-4o (Recommended)");
    modelSelect->addItem("gpt-4o-mini (Faster, Lower Cost)");
    modelSelect->addItem("gpt-4-turbo");
    modelSelect->addItem("gpt-4");
    modelSelect->addItem("gpt-3.5-turbo");

    // Set current model selection
    std::string currentModel = appConfig.getOpenAIModel();
    if (currentModel == "gpt-4o-mini") modelSelect->setCurrentIndex(1);
    else if (currentModel == "gpt-4-turbo") modelSelect->setCurrentIndex(2);
    else if (currentModel == "gpt-4") modelSelect->setCurrentIndex(3);
    else if (currentModel == "gpt-3.5-turbo") modelSelect->setCurrentIndex(4);
    else modelSelect->setCurrentIndex(0);

    // Gemini API Key (alternative)
    auto geminiGroup = aiFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    geminiGroup->setStyleClass("form-group");
    auto geminiLabel = geminiGroup->addWidget(std::make_unique<Wt::WText>("Google Gemini API Key (Alternative)"));
    geminiLabel->setStyleClass("form-label");
    auto geminiInput = geminiGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    geminiInput->setPlaceholderText("AIza...");
    geminiInput->setStyleClass("form-control");
    geminiInput->setAttributeValue("type", "password");
    if (appConfig.hasGeminiKey()) {
        geminiInput->setPlaceholderText("AIza****...****(configured)");
    }
    auto geminiHelp = geminiGroup->addWidget(std::make_unique<Wt::WText>(
        "Get your API key from makersuite.google.com (used if OpenAI is not configured)"
    ));
    geminiHelp->setStyleClass("form-help");

    // ===========================================
    // Data Sources API Configuration Section
    // ===========================================
    auto apiSection = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    apiSection->setStyleClass("settings-section");

    auto apiSectionTitle = apiSection->addWidget(std::make_unique<Wt::WText>("Data Source APIs"));
    apiSectionTitle->setStyleClass("section-title");

    auto apiSectionDesc = apiSection->addWidget(std::make_unique<Wt::WText>(
        "Configure API keys for business data sources. OpenStreetMap is always available (no key required)."
    ));
    apiSectionDesc->setStyleClass("section-description");

    auto apiFormGrid = apiSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    apiFormGrid->setStyleClass("form-grid");

    // Google API Key
    auto googleGroup = apiFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    googleGroup->setStyleClass("form-group");
    auto googleLabel = googleGroup->addWidget(std::make_unique<Wt::WText>("Google Places API Key"));
    googleLabel->setStyleClass("form-label");
    auto googleInput = googleGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    googleInput->setPlaceholderText("AIza...");
    googleInput->setStyleClass("form-control");
    googleInput->setAttributeValue("type", "password");
    if (appConfig.hasGoogleKey()) {
        googleInput->setPlaceholderText("AIza****...****(configured)");
    }

    // BBB API Key
    auto bbbGroup = apiFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    bbbGroup->setStyleClass("form-group");
    auto bbbLabel = bbbGroup->addWidget(std::make_unique<Wt::WText>("BBB API Key"));
    bbbLabel->setStyleClass("form-label");
    auto bbbInput = bbbGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    bbbInput->setPlaceholderText("Enter BBB API key");
    bbbInput->setStyleClass("form-control");
    bbbInput->setAttributeValue("type", "password");
    if (appConfig.hasBBBKey()) {
        bbbInput->setPlaceholderText("****...****(configured)");
    }

    // Census API Key
    auto censusGroup = apiFormGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
    censusGroup->setStyleClass("form-group");
    auto censusLabel = censusGroup->addWidget(std::make_unique<Wt::WText>("Census/Demographics API Key"));
    censusLabel->setStyleClass("form-label");
    auto censusInput = censusGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    censusInput->setPlaceholderText("Enter Census API key");
    censusInput->setStyleClass("form-control");
    censusInput->setAttributeValue("type", "password");
    if (appConfig.hasCensusKey()) {
        censusInput->setPlaceholderText("****...****(configured)");
    }

    // ===========================================
    // Search Preferences Section
    // ===========================================
    auto prefsSection = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    prefsSection->setStyleClass("settings-section");

    auto prefsSectionTitle = prefsSection->addWidget(std::make_unique<Wt::WText>("Search Preferences"));
    prefsSectionTitle->setStyleClass("section-title");

    auto prefsSectionDesc = prefsSection->addWidget(std::make_unique<Wt::WText>(
        "Set your default search radius and preferred business types."
    ));
    prefsSectionDesc->setStyleClass("section-description");

    // ===========================================
    // Franchise Profile Section
    // ===========================================
    auto profileSection = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    profileSection->setStyleClass("settings-section");

    auto profileSectionTitle = profileSection->addWidget(std::make_unique<Wt::WText>("Franchise Profile"));
    profileSectionTitle->setStyleClass("section-title");

    auto profileSectionDesc = profileSection->addWidget(std::make_unique<Wt::WText>(
        "Update your franchise location and contact information."
    ));
    profileSectionDesc->setStyleClass("section-description");

    if (franchisee_.isConfigured) {
        auto profileInfo = profileSection->addWidget(std::make_unique<Wt::WContainerWidget>());
        profileInfo->setStyleClass("profile-info");

        auto storeNameText = profileInfo->addWidget(std::make_unique<Wt::WText>(
            "Store: " + franchisee_.storeName
        ));
        storeNameText->setStyleClass("profile-field");

        auto addressText = profileInfo->addWidget(std::make_unique<Wt::WText>(
            "Address: " + franchisee_.address
        ));
        addressText->setStyleClass("profile-field");

        auto editProfileBtn = profileSection->addWidget(std::make_unique<Wt::WPushButton>("Edit Profile"));
        editProfileBtn->setStyleClass("btn btn-outline");
        editProfileBtn->clicked().connect([this]() {
            showSetupPage();
        });
    }

    // ===========================================
    // Action Buttons
    // ===========================================
    auto actionsSection = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    actionsSection->setStyleClass("form-actions");

    auto saveBtn = actionsSection->addWidget(std::make_unique<Wt::WPushButton>("Save Settings"));
    saveBtn->setStyleClass("btn btn-primary");

    // Status message area
    auto statusMessage = actionsSection->addWidget(std::make_unique<Wt::WText>(""));
    statusMessage->setStyleClass("settings-status-message");
    statusMessage->setHidden(true);

    // Connect save button
    saveBtn->clicked().connect([this, openaiInput, modelSelect, geminiInput, googleInput, bbbInput, censusInput, statusMessage, aiStatus]() {
        auto& appConfig = AppConfig::instance();
        bool changed = false;

        // Update OpenAI API Key if provided
        std::string openaiKey = openaiInput->text().toUTF8();
        if (!openaiKey.empty()) {
            appConfig.setOpenAIApiKey(openaiKey);
            changed = true;
        }

        // Update OpenAI Model
        std::vector<std::string> models = {"gpt-4o", "gpt-4o-mini", "gpt-4-turbo", "gpt-4", "gpt-3.5-turbo"};
        int modelIdx = modelSelect->currentIndex();
        if (modelIdx >= 0 && modelIdx < static_cast<int>(models.size())) {
            appConfig.setOpenAIModel(models[modelIdx]);
            changed = true;
        }

        // Update Gemini API Key if provided
        std::string geminiKey = geminiInput->text().toUTF8();
        if (!geminiKey.empty()) {
            appConfig.setGeminiApiKey(geminiKey);
            changed = true;
        }

        // Update Google API Key if provided
        std::string googleKey = googleInput->text().toUTF8();
        if (!googleKey.empty()) {
            appConfig.setGoogleApiKey(googleKey);
            changed = true;
        }

        // Update BBB API Key if provided
        std::string bbbKey = bbbInput->text().toUTF8();
        if (!bbbKey.empty()) {
            appConfig.setBBBApiKey(bbbKey);
            changed = true;
        }

        // Update Census API Key if provided
        std::string censusKey = censusInput->text().toUTF8();
        if (!censusKey.empty()) {
            appConfig.setCensusApiKey(censusKey);
            changed = true;
        }

        if (changed) {
            // Save to config file
            appConfig.saveToFile("config/app_config.json");

            // Update the search service with new AI configuration
            if (appConfig.hasOpenAIKey()) {
                searchService_->setAIProvider(
                    Services::AIProvider::OPENAI,
                    appConfig.getOpenAIApiKey()
                );
                aiStatus->setText("AI Engine: OpenAI (" + appConfig.getOpenAIModel() + ")");
                aiStatus->setStyleClass("status-indicator status-configured");
            } else if (appConfig.hasGeminiKey()) {
                searchService_->setAIProvider(
                    Services::AIProvider::GEMINI,
                    appConfig.getGeminiApiKey()
                );
                aiStatus->setText("AI Engine: Google Gemini");
                aiStatus->setStyleClass("status-indicator status-configured");
            }

            // Show success message
            statusMessage->setText("Settings saved successfully!");
            statusMessage->setStyleClass("settings-status-message status-success");
            statusMessage->setHidden(false);

            // Clear input fields (they now show as configured)
            openaiInput->setText("");
            geminiInput->setText("");
            googleInput->setText("");
            bbbInput->setText("");
            censusInput->setText("");

            // Update placeholders
            if (appConfig.hasOpenAIKey()) {
                openaiInput->setPlaceholderText("sk-****...****(configured)");
            }
            if (appConfig.hasGeminiKey()) {
                geminiInput->setPlaceholderText("AIza****...****(configured)");
            }
            if (appConfig.hasGoogleKey()) {
                googleInput->setPlaceholderText("AIza****...****(configured)");
            }
            if (appConfig.hasBBBKey()) {
                bbbInput->setPlaceholderText("****...****(configured)");
            }
            if (appConfig.hasCensusKey()) {
                censusInput->setPlaceholderText("****...****(configured)");
            }
        } else {
            statusMessage->setText("No changes to save.");
            statusMessage->setStyleClass("settings-status-message status-info");
            statusMessage->setHidden(false);
        }
    });
}

std::unique_ptr<Wt::WApplication> createFranchiseApp(const Wt::WEnvironment& env) {
    return std::make_unique<FranchiseApp>(env);
}

} // namespace FranchiseAI
