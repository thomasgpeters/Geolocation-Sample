#include "ResultsDisplay.h"
#include <sstream>

namespace FranchiseAI {
namespace Widgets {

ResultsDisplay::ResultsDisplay() {
    setStyleClass("results-display");
    setupUI();
}

void ResultsDisplay::setupUI() {
    // Loading state
    loadingContainer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    loadingContainer_->setStyleClass("state-container loading-container hidden");

    auto loadingSpinner = loadingContainer_->addWidget(std::make_unique<Wt::WText>("⟳"));
    loadingSpinner->setStyleClass("loading-spinner");

    auto loadingText = loadingContainer_->addWidget(std::make_unique<Wt::WText>("Searching for prospects..."));
    loadingText->setStyleClass("loading-text");

    // Empty state
    emptyContainer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    emptyContainer_->setStyleClass("state-container empty-container");

    auto emptyIcon = emptyContainer_->addWidget(std::make_unique<Wt::WText>("🔍"));
    emptyIcon->setStyleClass("empty-icon");

    auto emptyTitle = emptyContainer_->addWidget(std::make_unique<Wt::WText>("Ready to Search"));
    emptyTitle->setStyleClass("empty-title");

    auto emptyText = emptyContainer_->addWidget(std::make_unique<Wt::WText>(
        "Enter a location and search criteria to find potential catering clients in your area."
    ));
    emptyText->setStyleClass("empty-text");

    // Error state
    errorContainer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    errorContainer_->setStyleClass("state-container error-container hidden");

    auto errorIcon = errorContainer_->addWidget(std::make_unique<Wt::WText>("⚠️"));
    errorIcon->setStyleClass("error-icon");

    auto errorText = errorContainer_->addWidget(std::make_unique<Wt::WText>("An error occurred"));
    errorText->setStyleClass("error-text");

    // Summary section (hidden initially)
    createSummarySection();

    // Filters bar (hidden initially)
    createFiltersBar();

    // Results container (hidden initially)
    createResultsContainer();

    // Pagination (hidden initially)
    createPagination();
}

void ResultsDisplay::createSummarySection() {
    summaryContainer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    summaryContainer_->setStyleClass("results-summary hidden");

    // Single compact row with everything
    auto compactRow = summaryContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    compactRow->setStyleClass("results-toolbar");

    // Left side: stats and filters
    auto leftGroup = compactRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    leftGroup->setStyleClass("toolbar-left");

    // Compact stats inline
    totalResultsText_ = leftGroup->addWidget(std::make_unique<Wt::WText>("0"));
    totalResultsText_->setStyleClass("stat-count");

    auto resultsLabel = leftGroup->addWidget(std::make_unique<Wt::WText>(" results"));
    resultsLabel->setStyleClass("stat-suffix");

    auto separator = leftGroup->addWidget(std::make_unique<Wt::WText>(" · "));
    separator->setStyleClass("stat-separator");

    searchTimeText_ = leftGroup->addWidget(std::make_unique<Wt::WText>("0ms"));
    searchTimeText_->setStyleClass("stat-time");

    // Quick filter chips inline
    auto filterSep = leftGroup->addWidget(std::make_unique<Wt::WText>(" | "));
    filterSep->setStyleClass("filter-separator");

    std::vector<std::pair<std::string, std::string>> filters = {
        {"all", "All"},
        {"high", "High 60+"},
        {"conference", "Conference"},
    };

    for (const auto& [id, label] : filters) {
        auto chip = leftGroup->addWidget(std::make_unique<Wt::WPushButton>(label));
        chip->setStyleClass(id == "all" ? "filter-chip-sm active" : "filter-chip-sm");
    }

    // Right side: action buttons
    auto rightGroup = compactRow->addWidget(std::make_unique<Wt::WContainerWidget>());
    rightGroup->setStyleClass("toolbar-right");

    auto addAllBtn = rightGroup->addWidget(std::make_unique<Wt::WPushButton>("+ Add All"));
    addAllBtn->setStyleClass("btn btn-sm btn-secondary");

    auto exportBtn = rightGroup->addWidget(std::make_unique<Wt::WPushButton>("Export"));
    exportBtn->setStyleClass("btn btn-sm btn-outline");
    exportBtn->clicked().connect([this] {
        exportRequested_.emit();
    });

    // Hidden - not used in compact mode
    analysisText_ = nullptr;
}

void ResultsDisplay::createFiltersBar() {
    // Filters are now integrated into the compact toolbar above
    // This container is kept for backwards compatibility but hidden
    filtersBar_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    filtersBar_->setStyleClass("hidden");
}

void ResultsDisplay::createResultsContainer() {
    resultsContainer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    resultsContainer_->setStyleClass("results-cards hidden");
}

void ResultsDisplay::createPagination() {
    paginationContainer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    paginationContainer_->setStyleClass("pagination-container hidden");

    auto loadMoreBtn = paginationContainer_->addWidget(std::make_unique<Wt::WPushButton>("Load More Results"));
    loadMoreBtn->setStyleClass("btn btn-outline load-more-btn");
    loadMoreBtn->clicked().connect([this] {
        loadMoreRequested_.emit();
    });
}

void ResultsDisplay::showResults(const Models::SearchResults& results) {
    currentResults_ = results;

    // Hide other states
    loadingContainer_->setStyleClass("state-container loading-container hidden");
    emptyContainer_->setStyleClass("state-container empty-container hidden");
    errorContainer_->setStyleClass("state-container error-container hidden");

    if (results.items.empty()) {
        showEmpty("No prospects found matching your criteria. Try expanding your search radius or adjusting filters.");
        return;
    }

    // Show results sections (filters integrated into toolbar)
    summaryContainer_->setStyleClass("results-summary");
    resultsContainer_->setStyleClass("results-cards");

    if (results.hasMoreResults) {
        paginationContainer_->setStyleClass("pagination-container");
    } else {
        paginationContainer_->setStyleClass("pagination-container hidden");
    }

    updateSummary(results);
    populateResults(results);
}

void ResultsDisplay::clearResults() {
    resultsContainer_->clear();
    resultCards_.clear();

    summaryContainer_->setStyleClass("results-summary hidden");
    filtersBar_->setStyleClass("results-filters hidden");
    resultsContainer_->setStyleClass("results-cards hidden");
    paginationContainer_->setStyleClass("pagination-container hidden");
    errorContainer_->setStyleClass("state-container error-container hidden");

    emptyContainer_->setStyleClass("state-container empty-container");
}

void ResultsDisplay::showLoading() {
    emptyContainer_->setStyleClass("state-container empty-container hidden");
    errorContainer_->setStyleClass("state-container error-container hidden");
    summaryContainer_->setStyleClass("results-summary hidden");
    filtersBar_->setStyleClass("results-filters hidden");
    resultsContainer_->setStyleClass("results-cards hidden");
    paginationContainer_->setStyleClass("pagination-container hidden");

    loadingContainer_->setStyleClass("state-container loading-container");
}

void ResultsDisplay::showEmpty(const std::string& message) {
    loadingContainer_->setStyleClass("state-container loading-container hidden");
    errorContainer_->setStyleClass("state-container error-container hidden");
    summaryContainer_->setStyleClass("results-summary hidden");
    filtersBar_->setStyleClass("results-filters hidden");
    resultsContainer_->setStyleClass("results-cards hidden");
    paginationContainer_->setStyleClass("pagination-container hidden");

    emptyContainer_->setStyleClass("state-container empty-container");

    // Update empty message if provided
    if (!message.empty()) {
        // Would update the empty text here
    }
}

void ResultsDisplay::showError(const std::string& message) {
    loadingContainer_->setStyleClass("state-container loading-container hidden");
    emptyContainer_->setStyleClass("state-container empty-container hidden");
    summaryContainer_->setStyleClass("results-summary hidden");
    filtersBar_->setStyleClass("results-filters hidden");
    resultsContainer_->setStyleClass("results-cards hidden");
    paginationContainer_->setStyleClass("pagination-container hidden");

    errorContainer_->setStyleClass("state-container error-container");
}

void ResultsDisplay::updateSummary(const Models::SearchResults& results) {
    if (totalResultsText_) {
        totalResultsText_->setText(std::to_string(results.totalResults));
    }

    if (searchTimeText_) {
        searchTimeText_->setText(std::to_string(results.searchDuration.count()) + "ms");
    }

    if (analysisText_ && !results.aiOverallAnalysis.empty()) {
        analysisText_->setTextFormat(Wt::TextFormat::Plain);
        analysisText_->setText(results.aiOverallAnalysis);
    }
}

void ResultsDisplay::populateResults(const Models::SearchResults& results) {
    resultsContainer_->clear();
    resultCards_.clear();

    for (const auto& item : results.items) {
        auto card = resultsContainer_->addWidget(std::make_unique<ResultCard>(item));

        // Connect card signals
        card->viewDetailsRequested().connect([this](const std::string& id) {
            viewDetailsRequested_.emit(id);
        });

        card->addToProspectsRequested().connect([this](const std::string& id) {
            addToProspectsRequested_.emit(id);
        });

        resultCards_.push_back(card);
    }
}

} // namespace Widgets
} // namespace FranchiseAI
