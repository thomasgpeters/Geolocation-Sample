#include "FranchiseApp.h"
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
#include <Wt/WLeafletMap.h>
#include <Wt/Json/Object.h>
#include <Wt/Json/Array.h>
#include <Wt/Json/Value.h>
#include <sstream>
#include <iomanip>

namespace FranchiseAI {

FranchiseApp::FranchiseApp(const Wt::WEnvironment& env)
    : Wt::WApplication(env)
{
    setTitle("FranchiseAI - Prospect Search");

    // Initialize search service
    Services::AISearchConfig config;
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

    // Header
    auto header = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->setStyleClass("page-header");

    auto title = header->addWidget(std::make_unique<Wt::WText>("Demographics Map View"));
    title->setStyleClass("page-title");

    auto subtitle = header->addWidget(std::make_unique<Wt::WText>(
        "View business locations on the map. Select categories to filter markers."
    ));
    subtitle->setStyleClass("page-subtitle");

    // Search section
    auto searchSection = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    searchSection->setStyleClass("settings-section compact");

    auto searchRow = searchSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    searchRow->setStyleClass("actions-grid");

    auto locationInput = searchRow->addWidget(std::make_unique<Wt::WLineEdit>());
    locationInput->setPlaceholderText("e.g., Denver, CO");
    locationInput->setStyleClass("form-control");

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
    locationInput->setText(defaultLocation);

    auto radiusInput = searchRow->addWidget(std::make_unique<Wt::WLineEdit>(
        std::to_string(static_cast<int>(defaultRadiusKm))
    ));
    radiusInput->setStyleClass("form-control");
    radiusInput->setWidth(80);

    auto radiusLabel = searchRow->addWidget(std::make_unique<Wt::WText>("km"));
    radiusLabel->setStyleClass("input-suffix");

    auto analyzeBtn = searchRow->addWidget(std::make_unique<Wt::WPushButton>("Update Map"));
    analyzeBtn->setStyleClass("btn btn-primary");

    // Two-column layout: Map on left, Category sidebar on right
    auto columnsContainer = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    columnsContainer->setStyleClass("demographics-columns");

    // Left column - Map
    auto mapColumn = columnsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    mapColumn->setStyleClass("map-column");

    // Create Leaflet map with options
    Wt::Json::Object options;
    options["center"] = Wt::Json::Array({
        Wt::Json::Value(initialSearchArea.center.latitude),
        Wt::Json::Value(initialSearchArea.center.longitude)
    });
    options["zoom"] = Wt::Json::Value(12);

    auto map = mapColumn->addWidget(std::make_unique<Wt::WLeafletMap>(options));
    map->setStyleClass("demographics-map");

    // Add OpenStreetMap tile layer using the proper API
    Wt::Json::Object tileOptions;
    tileOptions["attribution"] = Wt::Json::Value("&copy; OpenStreetMap contributors");
    map->addTileLayer(
        "https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png",
        tileOptions
    );

    // Right column - Category sidebar
    auto sidebarColumn = columnsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    sidebarColumn->setStyleClass("category-sidebar");

    auto sidebarTitle = sidebarColumn->addWidget(std::make_unique<Wt::WText>("Filter Categories"));
    sidebarTitle->setStyleClass("sidebar-title");

    // Get initial stats
    auto& osmAPI = searchService_->getOSMAPI();
    auto stats = osmAPI.getAreaStatisticsSync(initialSearchArea);

    // Store current search area and checkboxes for updates
    auto currentSearchAreaPtr = std::make_shared<Models::SearchArea>(initialSearchArea);
    auto categoryCheckboxes = std::make_shared<std::map<std::string, Wt::WCheckBox*>>();

    // Category data with stats
    std::vector<std::tuple<std::string, int, bool>> categories = {
        {"Offices", stats.offices, true},
        {"Hotels", stats.hotels, true},
        {"Conference", stats.conferenceVenues, true},
        {"Hospitals", stats.hospitals, false},
        {"Universities", stats.universities, false},
        {"Schools", stats.schools, false},
        {"Industrial", stats.industrialBuildings, false},
        {"Warehouses", stats.warehouses, false},
        {"Banks", stats.banks, false},
        {"Government", stats.governmentBuildings, false},
        {"Restaurants", stats.restaurants, false},
        {"Cafes", stats.cafes, false}
    };

    // Market score display
    auto scoreContainer = sidebarColumn->addWidget(std::make_unique<Wt::WContainerWidget>());
    scoreContainer->setStyleClass("market-score-box");
    auto scoreLabel = scoreContainer->addWidget(std::make_unique<Wt::WText>("Market Score"));
    scoreLabel->setStyleClass("score-label-small");
    auto scoreValue = scoreContainer->addWidget(std::make_unique<Wt::WText>(
        std::to_string(stats.marketPotentialScore) + "/100"
    ));
    scoreValue->setStyleClass("score-value-large");

    // Select All / None buttons
    auto selectBtns = sidebarColumn->addWidget(std::make_unique<Wt::WContainerWidget>());
    selectBtns->setStyleClass("select-buttons");
    auto selectAllBtn = selectBtns->addWidget(std::make_unique<Wt::WPushButton>("All"));
    selectAllBtn->setStyleClass("btn btn-sm btn-outline");
    auto selectNoneBtn = selectBtns->addWidget(std::make_unique<Wt::WPushButton>("None"));
    selectNoneBtn->setStyleClass("btn btn-sm btn-outline");

    // Category checkboxes container
    auto categoriesContainer = sidebarColumn->addWidget(std::make_unique<Wt::WContainerWidget>());
    categoriesContainer->setStyleClass("categories-list");

    // Store markers so we can manage them
    auto markers = std::make_shared<std::vector<Wt::WLeafletMap::LeafletMarker*>>();

    // Function to update map markers based on selected categories
    auto updateMapMarkers = [this, map, currentSearchAreaPtr, categoryCheckboxes, markers]() {
        // Remove existing markers
        for (auto* marker : *markers) {
            map->removeMarker(marker);
        }
        markers->clear();

        auto& osmAPI = searchService_->getOSMAPI();

        // Collect POIs from selected categories
        for (const auto& [category, checkbox] : *categoryCheckboxes) {
            if (checkbox->isChecked()) {
                auto pois = osmAPI.searchByCategorySync(*currentSearchAreaPtr, category);
                for (const auto& poi : pois) {
                    auto marker = std::make_unique<Wt::WLeafletMap::LeafletMarker>(
                        Wt::WLeafletMap::Coordinate(poi.latitude, poi.longitude)
                    );
                    auto* markerPtr = marker.get();
                    map->addMarker(std::move(marker));
                    markers->push_back(markerPtr);
                }
            }
        }
    };

    // Create checkboxes for each category
    for (const auto& [name, count, defaultChecked] : categories) {
        auto checkboxRow = categoriesContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
        checkboxRow->setStyleClass("category-checkbox-row");

        std::string labelText = name + " (" + std::to_string(count) + ")";
        auto checkbox = checkboxRow->addWidget(std::make_unique<Wt::WCheckBox>(labelText));
        checkbox->setStyleClass("category-checkbox");
        checkbox->setChecked(defaultChecked);

        std::string categoryName = name;
        (*categoryCheckboxes)[categoryName] = checkbox;

        // Update map when checkbox changes
        checkbox->changed().connect([updateMapMarkers]() {
            updateMapMarkers();
        });
    }

    // Select All button handler
    selectAllBtn->clicked().connect([categoryCheckboxes, updateMapMarkers]() {
        for (auto& [name, checkbox] : *categoryCheckboxes) {
            checkbox->setChecked(true);
        }
        updateMapMarkers();
    });

    // Select None button handler
    selectNoneBtn->clicked().connect([categoryCheckboxes, updateMapMarkers]() {
        for (auto& [name, checkbox] : *categoryCheckboxes) {
            checkbox->setChecked(false);
        }
        updateMapMarkers();
    });

    // Summary stats
    auto summaryContainer = sidebarColumn->addWidget(std::make_unique<Wt::WContainerWidget>());
    summaryContainer->setStyleClass("summary-stats-box");

    auto totalPoisText = summaryContainer->addWidget(std::make_unique<Wt::WText>(
        "Total POIs: " + std::to_string(stats.totalPois)
    ));
    totalPoisText->setStyleClass("summary-stat");

    std::ostringstream densityStr;
    densityStr << std::fixed << std::setprecision(1) << stats.businessDensityPerSqKm;
    auto densityText = summaryContainer->addWidget(std::make_unique<Wt::WText>(
        "Density: " + densityStr.str() + "/km²"
    ));
    densityText->setStyleClass("summary-stat");

    // Attribution
    auto attribution = sidebarColumn->addWidget(std::make_unique<Wt::WText>(
        "Data: OpenStreetMap"
    ));
    attribution->setStyleClass("attribution-small");

    // Initial marker load
    updateMapMarkers();

    // Connect analyze button to update the map and stats
    analyzeBtn->clicked().connect([this, locationInput, radiusInput, map, currentSearchAreaPtr,
                                   categoryCheckboxes, scoreValue, totalPoisText, densityText,
                                   categoriesContainer, updateMapMarkers]() {
        std::string location = locationInput->text().toUTF8();
        double radiusKm = 10.0;
        try {
            radiusKm = std::stod(radiusInput->text().toUTF8());
        } catch (...) {
            radiusKm = 10.0;
        }

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

        // Update map center
        Wt::WLeafletMap::Coordinate newCenter(geoLocation.latitude, geoLocation.longitude);
        map->panTo(newCenter);

        // Get new stats
        auto& osmAPI = searchService_->getOSMAPI();
        auto newStats = osmAPI.getAreaStatisticsSync(searchArea);

        // Update score
        scoreValue->setText(std::to_string(newStats.marketPotentialScore) + "/100");

        // Update summary
        totalPoisText->setText("Total POIs: " + std::to_string(newStats.totalPois));
        std::ostringstream densityStr;
        densityStr << std::fixed << std::setprecision(1) << newStats.businessDensityPerSqKm;
        densityText->setText("Density: " + densityStr.str() + "/km²");

        // Update category counts in checkboxes
        std::map<std::string, int> newCounts = {
            {"Offices", newStats.offices},
            {"Hotels", newStats.hotels},
            {"Conference", newStats.conferenceVenues},
            {"Hospitals", newStats.hospitals},
            {"Universities", newStats.universities},
            {"Schools", newStats.schools},
            {"Industrial", newStats.industrialBuildings},
            {"Warehouses", newStats.warehouses},
            {"Banks", newStats.banks},
            {"Government", newStats.governmentBuildings},
            {"Restaurants", newStats.restaurants},
            {"Cafes", newStats.cafes}
        };

        for (auto& [name, checkbox] : *categoryCheckboxes) {
            if (newCounts.count(name)) {
                checkbox->setText(name + " (" + std::to_string(newCounts[name]) + ")");
            }
        }

        // Update markers
        updateMapMarkers();
    });
}

void FranchiseApp::showReportsPage() {
    workArea_->clear();
    navigation_->setPageTitle("Reports");
    navigation_->setBreadcrumbs({"Home", "Reports"});

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

    auto container = workArea_->addWidget(std::make_unique<Wt::WContainerWidget>());
    container->setStyleClass("page-container settings-page");

    auto header = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->setStyleClass("page-header");

    auto title = header->addWidget(std::make_unique<Wt::WText>("Settings"));
    title->setStyleClass("page-title");

    // Settings sections
    auto section1 = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    section1->setStyleClass("settings-section");

    auto section1Title = section1->addWidget(std::make_unique<Wt::WText>("API Configuration"));
    section1Title->setStyleClass("section-title");

    auto section1Desc = section1->addWidget(std::make_unique<Wt::WText>(
        "Configure your Google My Business and BBB API keys for live data."
    ));
    section1Desc->setStyleClass("section-description");

    auto section2 = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    section2->setStyleClass("settings-section");

    auto section2Title = section2->addWidget(std::make_unique<Wt::WText>("Search Preferences"));
    section2Title->setStyleClass("section-title");

    auto section2Desc = section2->addWidget(std::make_unique<Wt::WText>(
        "Set your default search radius and preferred business types."
    ));
    section2Desc->setStyleClass("section-description");

    auto section3 = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    section3->setStyleClass("settings-section");

    auto section3Title = section3->addWidget(std::make_unique<Wt::WText>("Franchise Profile"));
    section3Title->setStyleClass("section-title");

    auto section3Desc = section3->addWidget(std::make_unique<Wt::WText>(
        "Update your franchise location and contact information."
    ));
    section3Desc->setStyleClass("section-description");
}

std::unique_ptr<Wt::WApplication> createFranchiseApp(const Wt::WEnvironment& env) {
    return std::make_unique<FranchiseApp>(env);
}

} // namespace FranchiseAI
