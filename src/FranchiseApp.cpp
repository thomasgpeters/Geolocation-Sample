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
        showSetupPage();
    } else {
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
            showSetupPage();
        } else if (path == "/dashboard") {
            showDashboardPage();
        } else if (path == "/search" || path == "/ai-search") {
            showAISearchPage();
        } else if (path == "/prospects") {
            showProspectsPage();
        } else if (path == "/demographics") {
            showDemographicsPage();
        } else if (path == "/reports") {
            showReportsPage();
        } else if (path == "/settings") {
            showSettingsPage();
        } else {
            // Default to setup if not configured, otherwise search
            if (!franchisee_.isConfigured) {
                showSetupPage();
            } else {
                showAISearchPage();
            }
        }
    });
}

void FranchiseApp::onMenuItemSelected(const std::string& itemId) {
    currentPage_ = itemId;

    if (itemId == "setup") {
        showSetupPage();
        setInternalPath("/setup", true);
    } else if (itemId == "dashboard") {
        showDashboardPage();
        setInternalPath("/dashboard", true);
    } else if (itemId == "ai-search") {
        showAISearchPage();
        setInternalPath("/search", true);
    } else if (itemId == "prospects") {
        showProspectsPage();
        setInternalPath("/prospects", true);
    } else if (itemId == "demographics") {
        showDemographicsPage();
        setInternalPath("/demographics", true);
    } else if (itemId == "reports") {
        showReportsPage();
        setInternalPath("/reports", true);
    } else if (itemId == "settings") {
        showSettingsPage();
        setInternalPath("/settings", true);
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
    showAISearchPage();
    setInternalPath("/search", true);
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

    // Pre-populate search panel with franchisee's location and criteria
    if (franchisee_.isConfigured && franchisee_.hasValidLocation()) {
        Models::SearchQuery defaultQuery;
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

    auto header = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->setStyleClass("page-header");

    auto title = header->addWidget(std::make_unique<Wt::WText>("Demographics Analysis"));
    title->setStyleClass("page-title");

    auto subtitle = header->addWidget(std::make_unique<Wt::WText>(
        "Explore OpenStreetMap data and market potential in your service area"
    ));
    subtitle->setStyleClass("page-subtitle");

    // Search section
    auto searchSection = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    searchSection->setStyleClass("settings-section");

    auto searchLabel = searchSection->addWidget(std::make_unique<Wt::WText>("Enter a location to analyze:"));
    searchLabel->setStyleClass("section-title");

    auto searchRow = searchSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    searchRow->setStyleClass("actions-grid");

    auto locationInput = searchRow->addWidget(std::make_unique<Wt::WLineEdit>());
    locationInput->setPlaceholderText("e.g., Denver, CO or New York, NY");
    locationInput->setStyleClass("form-control");
    locationInput->setText("Denver, CO");

    auto radiusInput = searchRow->addWidget(std::make_unique<Wt::WLineEdit>("10"));
    radiusInput->setStyleClass("form-control");

    auto analyzeBtn = searchRow->addWidget(std::make_unique<Wt::WPushButton>("Analyze Area (km radius)"));
    analyzeBtn->setStyleClass("btn btn-primary");

    // Results container
    auto resultsContainer = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    resultsContainer->setStyleClass("demographics-results");

    // Drill-down container for business list (initially hidden)
    auto drillDownContainer = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    drillDownContainer->setStyleClass("settings-section drilldown-container hidden");

    // Store current search area for drill-down queries
    auto currentSearchArea = std::make_shared<Models::SearchArea>();

    // Function to show drill-down business list
    auto showDrillDownFunc = [this, drillDownContainer, currentSearchArea](const std::string& category, int count) {
        drillDownContainer->clear();
        drillDownContainer->setStyleClass("settings-section drilldown-container");

        // Header with back button
        auto headerRow = drillDownContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
        headerRow->setStyleClass("drilldown-header");

        auto backBtn = headerRow->addWidget(std::make_unique<Wt::WPushButton>("< Back to Overview"));
        backBtn->setStyleClass("btn btn-outline");
        backBtn->clicked().connect([drillDownContainer] {
            drillDownContainer->setStyleClass("settings-section drilldown-container hidden");
        });

        auto titleText = headerRow->addWidget(std::make_unique<Wt::WText>(
            category + " (" + std::to_string(count) + " found)", Wt::TextFormat::Plain
        ));
        titleText->setStyleClass("section-title");

        // Fetch businesses for this category
        auto& osmAPI = searchService_->getOSMAPI();
        auto pois = osmAPI.searchByCategorySync(*currentSearchArea, category);

        if (pois.empty()) {
            auto emptyText = drillDownContainer->addWidget(std::make_unique<Wt::WText>("No businesses found in this category."));
            emptyText->setStyleClass("section-description");
            return;
        }

        // Business list
        auto listContainer = drillDownContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
        listContainer->setStyleClass("business-list");

        for (const auto& poi : pois) {
            auto itemCard = listContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
            itemCard->setStyleClass("business-item-card");

            // Business name
            auto nameText = itemCard->addWidget(std::make_unique<Wt::WText>(poi.name, Wt::TextFormat::Plain));
            nameText->setStyleClass("business-item-name");

            // Address
            std::string address = poi.houseNumber + " " + poi.street;
            if (!poi.city.empty()) address += ", " + poi.city;
            if (!poi.state.empty()) address += ", " + poi.state;
            if (!poi.postcode.empty()) address += " " + poi.postcode;
            auto addressText = itemCard->addWidget(std::make_unique<Wt::WText>(address, Wt::TextFormat::Plain));
            addressText->setStyleClass("business-item-address");

            // Contact info row
            auto contactRow = itemCard->addWidget(std::make_unique<Wt::WContainerWidget>());
            contactRow->setStyleClass("business-item-contact");

            if (!poi.phone.empty()) {
                auto phoneText = contactRow->addWidget(std::make_unique<Wt::WText>("📞 " + poi.phone, Wt::TextFormat::Plain));
                phoneText->setStyleClass("contact-info");
            }
            if (!poi.website.empty()) {
                auto webText = contactRow->addWidget(std::make_unique<Wt::WText>("🌐 " + poi.website, Wt::TextFormat::Plain));
                webText->setStyleClass("contact-info");
            }
            if (!poi.email.empty()) {
                auto emailText = contactRow->addWidget(std::make_unique<Wt::WText>("✉️ " + poi.email, Wt::TextFormat::Plain));
                emailText->setStyleClass("contact-info");
            }
        }

        // Attribution
        auto attribution = drillDownContainer->addWidget(std::make_unique<Wt::WText>(
            "Data source: OpenStreetMap contributors"
        ));
        attribution->setStyleClass("section-description");
    };

    // Function to display statistics
    auto displayStatsFunc = [currentSearchArea, showDrillDownFunc](
        Wt::WContainerWidget* resultsContainer,
        const Services::OSMAreaStats& stats,
        const Models::SearchArea& searchArea
    ) {
        resultsContainer->clear();
        *currentSearchArea = searchArea;  // Store for drill-down

        // Market Score Card Section
        auto scoreSection = resultsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
        scoreSection->setStyleClass("settings-section");

        auto scoreHeader = scoreSection->addWidget(std::make_unique<Wt::WText>("Market Potential Score"));
        scoreHeader->setStyleClass("section-title");

        auto scoreValue = scoreSection->addWidget(std::make_unique<Wt::WText>(
            std::to_string(stats.marketPotentialScore) + "/100 - " + stats.getMarketQualityDescription()
        ));
        scoreValue->setStyleClass("page-subtitle");

        // Stats Grid Section
        auto statsSection = resultsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
        statsSection->setStyleClass("settings-section");

        auto statsHeader = statsSection->addWidget(std::make_unique<Wt::WText>("Business Categories in Area"));
        statsHeader->setStyleClass("section-title");

        auto statsSubHeader = statsSection->addWidget(std::make_unique<Wt::WText>("Click on a count to view businesses in that category"));
        statsSubHeader->setStyleClass("section-description");

        auto statsGrid = statsSection->addWidget(std::make_unique<Wt::WContainerWidget>());
        statsGrid->setStyleClass("stats-grid");

        std::vector<std::tuple<std::string, int, std::string>> statItems = {
            {"Offices", stats.offices, "Corporate offices & business centers"},
            {"Hotels", stats.hotels, "Hotels & lodging"},
            {"Conference", stats.conferenceVenues, "Conference & event venues"},
            {"Hospitals", stats.hospitals, "Healthcare facilities"},
            {"Universities", stats.universities, "Higher education"},
            {"Schools", stats.schools, "K-12 education"},
            {"Industrial", stats.industrialBuildings, "Manufacturing & industrial"},
            {"Warehouses", stats.warehouses, "Storage & distribution"},
            {"Banks", stats.banks, "Financial institutions"},
            {"Government", stats.governmentBuildings, "Government offices"},
            {"Restaurants", stats.restaurants, "Food service"},
            {"Cafes", stats.cafes, "Coffee shops & cafes"}
        };

        for (const auto& [label, count, desc] : statItems) {
            auto card = statsGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
            card->setStyleClass("stat-card clickable");

            // Make the value a clickable button
            auto valueBtn = card->addWidget(std::make_unique<Wt::WPushButton>(std::to_string(count)));
            valueBtn->setStyleClass("stat-value-btn");

            // Copy to local variables for lambda capture (structured bindings can't be captured directly)
            std::string categoryLabel = label;
            int categoryCount = count;

            // Connect click handler for drill-down
            valueBtn->clicked().connect([showDrillDownFunc, categoryLabel, categoryCount]() {
                showDrillDownFunc(categoryLabel, categoryCount);
            });

            auto labelText = card->addWidget(std::make_unique<Wt::WText>(label));
            labelText->setStyleClass("stat-label");
        }

        // Summary section
        auto summarySection = resultsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
        summarySection->setStyleClass("settings-section");

        auto summaryTitle = summarySection->addWidget(std::make_unique<Wt::WText>("Area Summary"));
        summaryTitle->setStyleClass("section-title");

        std::ostringstream summary;
        summary << "Total Points of Interest: " << stats.totalPois;
        auto totalText = summarySection->addWidget(std::make_unique<Wt::WText>(summary.str()));
        totalText->setStyleClass("section-description");

        std::ostringstream density;
        density << std::fixed << std::setprecision(1);
        density << "Business Density: " << stats.businessDensityPerSqKm << " POIs per square km";
        auto densityText = summarySection->addWidget(std::make_unique<Wt::WText>(density.str()));
        densityText->setStyleClass("section-description");

        // Insights section
        auto insightsSection = resultsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
        insightsSection->setStyleClass("settings-section");

        auto insightsTitle = insightsSection->addWidget(std::make_unique<Wt::WText>("Catering Opportunity Insights"));
        insightsTitle->setStyleClass("section-title");

        std::vector<std::string> insights;
        if (stats.offices > 20) {
            insights.push_back("Strong corporate presence - target office buildings for regular catering");
        }
        if (stats.conferenceVenues > 2) {
            insights.push_back("Conference venues available - opportunity for event catering");
        }
        if (stats.hotels > 5) {
            insights.push_back("Multiple hotels in area - potential for hotel catering partnerships");
        }
        if (stats.hospitals > 1) {
            insights.push_back("Healthcare facilities present - consider hospital cafeteria services");
        }
        if (stats.industrialBuildings + stats.warehouses > 10) {
            insights.push_back("Industrial area - employee meal programs may be valuable");
        }
        if (stats.universities > 0) {
            insights.push_back("Educational institutions nearby - campus catering opportunities");
        }
        if (insights.empty()) {
            insights.push_back("Moderate catering potential - consider expanding search radius");
        }

        for (const auto& insight : insights) {
            auto insightText = insightsSection->addWidget(std::make_unique<Wt::WText>("- " + insight));
            insightText->setStyleClass("section-description");
        }

        // Attribution
        auto attribution = resultsContainer->addWidget(std::make_unique<Wt::WText>(
            "Data source: OpenStreetMap contributors (openstreetmap.org)"
        ));
        attribution->setStyleClass("section-description");
    };

    // Display default Denver stats using SearchArea
    Models::GeoLocation defaultLocation(39.7392, -104.9903, "Denver", "CO");
    Models::SearchArea defaultSearchArea(defaultLocation, 10.0);  // 10km radius
    auto& osmAPI = searchService_->getOSMAPI();
    auto defaultStats = osmAPI.getAreaStatisticsSync(defaultSearchArea);
    displayStatsFunc(resultsContainer, defaultStats, defaultSearchArea);

    // Connect analyze button
    analyzeBtn->clicked().connect([this, locationInput, radiusInput, resultsContainer, drillDownContainer, displayStatsFunc]() {
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

        // Hide drill-down when re-analyzing
        drillDownContainer->setStyleClass("settings-section drilldown-container hidden");

        // Use geocodeAddress from AISearchService to get GeoLocation
        Models::GeoLocation geoLocation = searchService_->geocodeAddress(location);

        // Create SearchArea with the geocoded location
        Models::SearchArea searchArea(geoLocation, radiusKm);

        // Get area statistics using SearchArea-based API
        auto& osmAPI = searchService_->getOSMAPI();
        auto stats = osmAPI.getAreaStatisticsSync(searchArea);
        displayStatsFunc(resultsContainer, stats, searchArea);
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
