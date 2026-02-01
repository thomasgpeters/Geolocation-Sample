#include "SearchPanel.h"
#include <Wt/WGroupBox.h>

namespace FranchiseAI {
namespace Widgets {

SearchPanel::SearchPanel() {
    setStyleClass("search-panel");
    setupUI();
}

void SearchPanel::setupUI() {
    // Scrollable content container for form fields
    scrollableContent_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    scrollableContent_->setStyleClass("search-content");

    createSearchHeader();
    createLocationSection();
    createFiltersSection();

    // Actions stay outside scrollable area (sticky at bottom)
    createSearchActions();
}

void SearchPanel::createSearchHeader() {
    auto header = scrollableContent_->addWidget(std::make_unique<Wt::WContainerWidget>());
    header->setStyleClass("search-header");

    auto title = header->addWidget(std::make_unique<Wt::WText>("🔍 AI-Powered Prospect Search"));
    title->setStyleClass("search-title");

    auto subtitle = header->addWidget(std::make_unique<Wt::WText>(
        "Find potential catering clients in your area using intelligent search across multiple data sources."
    ));
    subtitle->setStyleClass("search-subtitle");
}

void SearchPanel::createLocationSection() {
    auto section = scrollableContent_->addWidget(std::make_unique<Wt::WContainerWidget>());
    section->setStyleClass("search-section");

    auto sectionTitle = section->addWidget(std::make_unique<Wt::WText>("📍 Location"));
    sectionTitle->setStyleClass("section-title");

    // Location input - full width row
    auto locationGroup = section->addWidget(std::make_unique<Wt::WContainerWidget>());
    locationGroup->setStyleClass("form-group location-input-group");

    auto locationLabel = locationGroup->addWidget(std::make_unique<Wt::WText>("Search Location"));
    locationLabel->setStyleClass("form-label");

    locationInput_ = locationGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    locationInput_->setStyleClass("form-input location-input");
    locationInput_->setPlaceholderText("Enter city, state or address...");

    // Radius slider - separate row
    auto radiusRow = section->addWidget(std::make_unique<Wt::WContainerWidget>());
    radiusRow->setStyleClass("form-row");

    auto radiusGroup = radiusRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    radiusGroup->setStyleClass("form-group");

    auto radiusLabelContainer = radiusGroup->addWidget(std::make_unique<Wt::WContainerWidget>());
    radiusLabelContainer->setStyleClass("label-with-value");

    auto radiusLabelText = radiusLabelContainer->addWidget(std::make_unique<Wt::WText>("Search Radius"));
    radiusLabelText->setStyleClass("form-label");

    radiusLabel_ = radiusLabelContainer->addWidget(std::make_unique<Wt::WText>("25 miles"));
    radiusLabel_->setStyleClass("form-value");

    radiusSlider_ = radiusGroup->addWidget(std::make_unique<Wt::WSlider>());
    radiusSlider_->setStyleClass("form-slider");
    radiusSlider_->setRange(5, 100);
    radiusSlider_->setValue(25);
    radiusSlider_->valueChanged().connect([this] {
        radiusLabel_->setText(std::to_string(radiusSlider_->value()) + " miles");
    });

    // ZIP code and city/state row
    auto addressRow = section->addWidget(std::make_unique<Wt::WContainerWidget>());
    addressRow->setStyleClass("form-row");

    auto zipGroup = addressRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    zipGroup->setStyleClass("form-group");

    auto zipLabel = zipGroup->addWidget(std::make_unique<Wt::WText>("ZIP Code"));
    zipLabel->setStyleClass("form-label");

    zipCodeInput_ = zipGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    zipCodeInput_->setStyleClass("form-input");
    zipCodeInput_->setPlaceholderText("e.g., 62701");

    auto cityGroup = addressRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    cityGroup->setStyleClass("form-group");

    auto cityLabel = cityGroup->addWidget(std::make_unique<Wt::WText>("City"));
    cityLabel->setStyleClass("form-label");

    cityInput_ = cityGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    cityInput_->setStyleClass("form-input");
    cityInput_->setPlaceholderText("City name");

    auto stateGroup = addressRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    stateGroup->setStyleClass("form-group");

    auto stateLabel = stateGroup->addWidget(std::make_unique<Wt::WText>("State"));
    stateLabel->setStyleClass("form-label");

    stateCombo_ = stateGroup->addWidget(std::make_unique<Wt::WComboBox>());
    stateCombo_->setStyleClass("form-select");
    stateCombo_->addItem("Select State");
    stateCombo_->addItem("AL"); stateCombo_->addItem("AK"); stateCombo_->addItem("AZ");
    stateCombo_->addItem("AR"); stateCombo_->addItem("CA"); stateCombo_->addItem("CO");
    stateCombo_->addItem("CT"); stateCombo_->addItem("DE"); stateCombo_->addItem("FL");
    stateCombo_->addItem("GA"); stateCombo_->addItem("HI"); stateCombo_->addItem("ID");
    stateCombo_->addItem("IL"); stateCombo_->addItem("IN"); stateCombo_->addItem("IA");
    stateCombo_->addItem("KS"); stateCombo_->addItem("KY"); stateCombo_->addItem("LA");
    stateCombo_->addItem("ME"); stateCombo_->addItem("MD"); stateCombo_->addItem("MA");
    stateCombo_->addItem("MI"); stateCombo_->addItem("MN"); stateCombo_->addItem("MS");
    stateCombo_->addItem("MO"); stateCombo_->addItem("MT"); stateCombo_->addItem("NE");
    stateCombo_->addItem("NV"); stateCombo_->addItem("NH"); stateCombo_->addItem("NJ");
    stateCombo_->addItem("NM"); stateCombo_->addItem("NY"); stateCombo_->addItem("NC");
    stateCombo_->addItem("ND"); stateCombo_->addItem("OH"); stateCombo_->addItem("OK");
    stateCombo_->addItem("OR"); stateCombo_->addItem("PA"); stateCombo_->addItem("RI");
    stateCombo_->addItem("SC"); stateCombo_->addItem("SD"); stateCombo_->addItem("TN");
    stateCombo_->addItem("TX"); stateCombo_->addItem("UT"); stateCombo_->addItem("VT");
    stateCombo_->addItem("VA"); stateCombo_->addItem("WA"); stateCombo_->addItem("WV");
    stateCombo_->addItem("WI"); stateCombo_->addItem("WY");
}

void SearchPanel::createFiltersSection() {
    auto section = scrollableContent_->addWidget(std::make_unique<Wt::WContainerWidget>());
    section->setStyleClass("search-section");

    auto sectionHeader = section->addWidget(std::make_unique<Wt::WContainerWidget>());
    sectionHeader->setStyleClass("section-header-row");

    auto sectionTitle = sectionHeader->addWidget(std::make_unique<Wt::WText>("🎯 Search Filters"));
    sectionTitle->setStyleClass("section-title");

    auto advancedBtn = sectionHeader->addWidget(std::make_unique<Wt::WPushButton>("Show Advanced ▼"));
    advancedBtn->setStyleClass("toggle-advanced-btn");
    advancedBtn->clicked().connect([this, advancedBtn] {
        toggleAdvancedFilters();
        advancedBtn->setText(advancedFiltersVisible_ ? "Hide Advanced ▲" : "Show Advanced ▼");
    });

    // Keywords input
    auto keywordsRow = section->addWidget(std::make_unique<Wt::WContainerWidget>());
    keywordsRow->setStyleClass("form-row");

    auto keywordsGroup = keywordsRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    keywordsGroup->setStyleClass("form-group flex-2");

    auto keywordsLabel = keywordsGroup->addWidget(std::make_unique<Wt::WText>("Keywords"));
    keywordsLabel->setStyleClass("form-label");

    keywordsInput_ = keywordsGroup->addWidget(std::make_unique<Wt::WLineEdit>());
    keywordsInput_->setStyleClass("form-input");
    keywordsInput_->setPlaceholderText("e.g., technology, manufacturing, corporate...");

    // Min score slider
    auto scoreGroup = keywordsRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    scoreGroup->setStyleClass("form-group flex-1");

    auto scoreLabelContainer = scoreGroup->addWidget(std::make_unique<Wt::WContainerWidget>());
    scoreLabelContainer->setStyleClass("label-with-value");

    auto scoreLabelText = scoreLabelContainer->addWidget(std::make_unique<Wt::WText>("Min. Potential Score"));
    scoreLabelText->setStyleClass("form-label");

    minScoreLabel_ = scoreLabelContainer->addWidget(std::make_unique<Wt::WText>("0"));
    minScoreLabel_->setStyleClass("form-value");

    minScoreSlider_ = scoreGroup->addWidget(std::make_unique<Wt::WSlider>());
    minScoreSlider_->setStyleClass("form-slider");
    minScoreSlider_->setRange(0, 80);
    minScoreSlider_->setValue(0);
    minScoreSlider_->valueChanged().connect([this] {
        minScoreLabel_->setText(std::to_string(minScoreSlider_->value()));
    });

    // Note: Business Types and Data Sources are configured in Settings > Marketing tab
    // and are automatically applied to searches

    // Advanced filters (hidden by default)
    advancedFilters_ = section->addWidget(std::make_unique<Wt::WContainerWidget>());
    advancedFilters_->setStyleClass("advanced-filters hidden");

    auto sortRow = advancedFilters_->addWidget(std::make_unique<Wt::WContainerWidget>());
    sortRow->setStyleClass("form-row");

    auto sortGroup = sortRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    sortGroup->setStyleClass("form-group");

    auto sortLabel = sortGroup->addWidget(std::make_unique<Wt::WText>("Sort Results By"));
    sortLabel->setStyleClass("form-label");

    sortByCombo_ = sortGroup->addWidget(std::make_unique<Wt::WComboBox>());
    sortByCombo_->setStyleClass("form-select");
    sortByCombo_->addItem("Relevance");
    sortByCombo_->addItem("Catering Potential");
    sortByCombo_->addItem("Distance");
    sortByCombo_->addItem("Employee Count");
    sortByCombo_->addItem("Rating");
}

void SearchPanel::createSearchActions() {
    auto actionsContainer = addWidget(std::make_unique<Wt::WContainerWidget>());
    actionsContainer->setStyleClass("search-actions");

    // Progress indicator
    progressContainer_ = actionsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    progressContainer_->setStyleClass("progress-container hidden");

    auto spinner = progressContainer_->addWidget(std::make_unique<Wt::WText>("⟳"));
    spinner->setStyleClass("spinner");

    progressText_ = progressContainer_->addWidget(std::make_unique<Wt::WText>("Searching..."));
    progressText_->setStyleClass("progress-text");

    // Action buttons
    auto buttonsContainer = actionsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
    buttonsContainer->setStyleClass("buttons-container");

    clearBtn_ = buttonsContainer->addWidget(std::make_unique<Wt::WPushButton>("Clear"));
    clearBtn_->setStyleClass("btn btn-secondary");
    clearBtn_->clicked().connect([this] { clearForm(); });

    cancelBtn_ = buttonsContainer->addWidget(std::make_unique<Wt::WPushButton>("Cancel"));
    cancelBtn_->setStyleClass("btn btn-danger hidden");
    cancelBtn_->clicked().connect([this] { onCancel(); });

    searchBtn_ = buttonsContainer->addWidget(std::make_unique<Wt::WPushButton>("🔍 Search Prospects"));
    searchBtn_->setStyleClass("btn btn-primary");
    searchBtn_->clicked().connect([this] { onSearch(); });
}

void SearchPanel::toggleAdvancedFilters() {
    advancedFiltersVisible_ = !advancedFiltersVisible_;

    if (advancedFiltersVisible_) {
        advancedFilters_->setStyleClass("advanced-filters");
    } else {
        advancedFilters_->setStyleClass("advanced-filters hidden");
    }
}

Models::SearchQuery SearchPanel::getSearchQuery() const {
    Models::SearchQuery query;

    if (locationInput_) query.location = locationInput_->text().toUTF8();
    if (zipCodeInput_) query.zipCode = zipCodeInput_->text().toUTF8();
    if (cityInput_) query.city = cityInput_->text().toUTF8();
    if (stateCombo_ && stateCombo_->currentIndex() > 0) {
        query.state = stateCombo_->currentText().toUTF8();
    }
    if (keywordsInput_) query.keywords = keywordsInput_->text().toUTF8();
    if (radiusSlider_) query.radiusMiles = radiusSlider_->value();
    if (minScoreSlider_) query.minCateringScore = minScoreSlider_->value();

    // Note: Business types and data sources are set from Settings > Marketing tab
    // and will be populated by FranchiseApp before search is executed

    // Sort option
    if (sortByCombo_) {
        switch (sortByCombo_->currentIndex()) {
            case 0: query.sortBy = Models::SearchQuery::SortBy::RELEVANCE; break;
            case 1: query.sortBy = Models::SearchQuery::SortBy::CATERING_POTENTIAL; break;
            case 2: query.sortBy = Models::SearchQuery::SortBy::DISTANCE; break;
            case 3: query.sortBy = Models::SearchQuery::SortBy::EMPLOYEE_COUNT; break;
            case 4: query.sortBy = Models::SearchQuery::SortBy::RATING; break;
        }
    }

    return query;
}

void SearchPanel::setSearchQuery(const Models::SearchQuery& query) {
    if (locationInput_) locationInput_->setText(query.location);
    if (zipCodeInput_) zipCodeInput_->setText(query.zipCode);
    if (cityInput_) cityInput_->setText(query.city);
    if (keywordsInput_) keywordsInput_->setText(query.keywords);
    if (radiusSlider_) {
        radiusSlider_->setValue(static_cast<int>(query.radiusMiles));
        radiusLabel_->setText(std::to_string(static_cast<int>(query.radiusMiles)) + " miles");
    }
    if (minScoreSlider_) {
        minScoreSlider_->setValue(static_cast<int>(query.minCateringScore));
        minScoreLabel_->setText(std::to_string(static_cast<int>(query.minCateringScore)));
    }
    // Business types and data sources are configured in Settings > Marketing tab
}

void SearchPanel::clearForm() {
    if (locationInput_) locationInput_->setText("");
    if (zipCodeInput_) zipCodeInput_->setText("");
    if (cityInput_) cityInput_->setText("");
    if (stateCombo_) stateCombo_->setCurrentIndex(0);
    if (keywordsInput_) keywordsInput_->setText("");
    if (radiusSlider_) {
        radiusSlider_->setValue(25);
        radiusLabel_->setText("25 miles");
    }
    if (minScoreSlider_) {
        minScoreSlider_->setValue(0);
        minScoreLabel_->setText("0");
    }

    if (sortByCombo_) sortByCombo_->setCurrentIndex(0);
}

void SearchPanel::setSearchEnabled(bool enabled) {
    if (searchBtn_) {
        searchBtn_->setEnabled(enabled);
        if (enabled) {
            searchBtn_->setStyleClass("btn btn-primary");
            cancelBtn_->setStyleClass("btn btn-danger hidden");
        } else {
            searchBtn_->setStyleClass("btn btn-primary disabled");
            cancelBtn_->setStyleClass("btn btn-danger");
        }
    }
}

void SearchPanel::setProgressMessage(const std::string& message) {
    if (progressText_) {
        progressText_->setText(message);
    }
}

void SearchPanel::showProgress(bool show) {
    if (progressContainer_) {
        progressContainer_->setStyleClass(show ? "progress-container" : "progress-container hidden");
    }
}

void SearchPanel::onSearch() {
    auto query = getSearchQuery();
    setSearchEnabled(false);
    showProgress(true);
    searchRequested_.emit(query);
}

void SearchPanel::onCancel() {
    searchCancelled_.emit();
    setSearchEnabled(true);
    showProgress(false);
}

} // namespace Widgets
} // namespace FranchiseAI
