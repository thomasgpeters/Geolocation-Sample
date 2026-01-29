#include "FranchiseApp.h"
#include "models/GeoLocation.h"
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WCssStyleSheet.h>
#include <Wt/WText.h>
#include <Wt/WMessageBox.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WBreak.h>
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

    // Show default page
    showAISearchPage();
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

        if (path == "/dashboard") {
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
            showAISearchPage();  // Default
        }
    });
}

void FranchiseApp::onMenuItemSelected(const std::string& itemId) {
    currentPage_ = itemId;

    if (itemId == "dashboard") {
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

    // Two-column layout
    auto columnsContainer = container->addWidget(std::make_unique<Wt::WContainerWidget>());
    columnsContainer->setStyleClass("search-columns");

    // Left column - Search panel
    auto leftColumn = columnsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    leftColumn->setStyleClass("search-column left");

    searchPanel_ = leftColumn->addWidget(std::make_unique<Widgets::SearchPanel>());
    searchPanel_->searchRequested().connect(this, &FranchiseApp::onSearchRequested);
    searchPanel_->searchCancelled().connect(this, &FranchiseApp::onSearchCancelled);

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

    // Function to display statistics
    auto displayStatsFunc = [](Wt::WContainerWidget* resultsContainer, const Services::OSMAreaStats& stats) {
        resultsContainer->clear();

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
            card->setStyleClass("stat-card");

            auto valueText = card->addWidget(std::make_unique<Wt::WText>(std::to_string(count)));
            valueText->setStyleClass("stat-value");

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
            auto insightText = insightsSection->addWidget(std::make_unique<Wt::WText>("• " + insight));
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
    displayStatsFunc(resultsContainer, defaultStats);

    // Connect analyze button
    analyzeBtn->clicked().connect([this, locationInput, radiusInput, resultsContainer, displayStatsFunc]() {
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

        // Use geocodeAddress from AISearchService to get GeoLocation
        Models::GeoLocation geoLocation = searchService_->geocodeAddress(location);

        // Create SearchArea with the geocoded location
        Models::SearchArea searchArea(geoLocation, radiusKm);

        // Get area statistics using SearchArea-based API
        auto& osmAPI = searchService_->getOSMAPI();
        auto stats = osmAPI.getAreaStatisticsSync(searchArea);
        displayStatsFunc(resultsContainer, stats);
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
