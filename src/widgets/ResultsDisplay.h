#ifndef RESULTS_DISPLAY_H
#define RESULTS_DISPLAY_H

#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>
#include <Wt/WSignal.h>
#include <set>
#include "models/SearchResult.h"
#include "ResultCard.h"

namespace FranchiseAI {
namespace Widgets {

/**
 * @brief Results display widget
 *
 * Displays search results with summary statistics,
 * filtering options, and result cards.
 */
class ResultsDisplay : public Wt::WContainerWidget {
public:
    ResultsDisplay();
    ~ResultsDisplay() override = default;

    /**
     * @brief Display search results
     * @param results Search results to display
     */
    void showResults(const Models::SearchResults& results);

    /**
     * @brief Update displayed results with new data (preserves UI state)
     * @param results Updated search results
     */
    void updateResults(const Models::SearchResults& results);

    /**
     * @brief Show optimizing indicator (spinner in toolbar)
     */
    void showOptimizing();

    /**
     * @brief Hide optimizing indicator
     */
    void hideOptimizing();

    /**
     * @brief Clear all results
     */
    void clearResults();

    /**
     * @brief Show loading state
     */
    void showLoading();

    /**
     * @brief Show empty state (no results)
     */
    void showEmpty(const std::string& message = "");

    /**
     * @brief Show error state
     */
    void showError(const std::string& message);

    /**
     * @brief Signal emitted when view details is clicked
     */
    Wt::Signal<std::string>& viewDetailsRequested() { return viewDetailsRequested_; }

    /**
     * @brief Signal emitted when add to prospects is clicked
     */
    Wt::Signal<std::string>& addToProspectsRequested() { return addToProspectsRequested_; }

    /**
     * @brief Signal emitted when export is clicked
     */
    Wt::Signal<>& exportRequested() { return exportRequested_; }

    /**
     * @brief Signal emitted when load more is clicked
     */
    Wt::Signal<>& loadMoreRequested() { return loadMoreRequested_; }

    /**
     * @brief Signal emitted when add selected prospects is clicked
     * @param ids Vector of selected prospect IDs
     */
    Wt::Signal<std::vector<std::string>>& addSelectedRequested() { return addSelectedRequested_; }

    /**
     * @brief Get the currently selected prospect IDs
     */
    std::vector<std::string> getSelectedIds() const;

    /**
     * @brief Clear all selections
     */
    void clearSelections();

private:
    void setupUI();
    void createSummarySection();
    void createFiltersBar();
    void createResultsContainer();
    void createPagination();
    void updateSummary(const Models::SearchResults& results);
    void populateResults(const Models::SearchResults& results);
    void onSelectionChanged(const std::string& id, bool selected);
    void updateActionButtons();

    Wt::Signal<std::string> viewDetailsRequested_;
    Wt::Signal<std::string> addToProspectsRequested_;
    Wt::Signal<> exportRequested_;
    Wt::Signal<> loadMoreRequested_;
    Wt::Signal<std::vector<std::string>> addSelectedRequested_;

    // Current results
    Models::SearchResults currentResults_;

    // UI components
    Wt::WContainerWidget* summaryContainer_ = nullptr;
    Wt::WContainerWidget* filtersBar_ = nullptr;
    Wt::WContainerWidget* resultsContainer_ = nullptr;
    Wt::WContainerWidget* paginationContainer_ = nullptr;
    Wt::WContainerWidget* loadingContainer_ = nullptr;
    Wt::WContainerWidget* emptyContainer_ = nullptr;
    Wt::WContainerWidget* errorContainer_ = nullptr;

    // Summary texts
    Wt::WText* totalResultsText_ = nullptr;
    Wt::WText* analysisText_ = nullptr;
    Wt::WText* searchTimeText_ = nullptr;

    // Optimizing indicator
    Wt::WContainerWidget* optimizingIndicator_ = nullptr;

    // Action buttons
    Wt::WPushButton* addAllBtn_ = nullptr;
    Wt::WPushButton* addSelectedBtn_ = nullptr;

    // Result cards
    std::vector<ResultCard*> resultCards_;

    // Selected item IDs
    std::set<std::string> selectedIds_;
};

} // namespace Widgets
} // namespace FranchiseAI

#endif // RESULTS_DISPLAY_H
