#ifndef SEARCH_PANEL_H
#define SEARCH_PANEL_H

#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WComboBox.h>
#include <Wt/WCheckBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WSlider.h>
#include <Wt/WText.h>
#include <Wt/WSignal.h>
#include "models/SearchResult.h"

namespace FranchiseAI {
namespace Widgets {

/**
 * @brief Search panel widget for AI-powered search
 *
 * Provides the main search interface with location input,
 * filters, and search options.
 */
class SearchPanel : public Wt::WContainerWidget {
public:
    SearchPanel();
    ~SearchPanel() override = default;

    /**
     * @brief Get the current search query from the form
     */
    Models::SearchQuery getSearchQuery() const;

    /**
     * @brief Set search query values in the form
     */
    void setSearchQuery(const Models::SearchQuery& query);

    /**
     * @brief Clear all form inputs
     */
    void clearForm();

    /**
     * @brief Enable/disable the search button
     */
    void setSearchEnabled(bool enabled);

    /**
     * @brief Set search progress message
     */
    void setProgressMessage(const std::string& message);

    /**
     * @brief Show/hide progress indicator
     */
    void showProgress(bool show);

    /**
     * @brief Signal emitted when search is requested
     */
    Wt::Signal<Models::SearchQuery>& searchRequested() { return searchRequested_; }

    /**
     * @brief Signal emitted when search is cancelled
     */
    Wt::Signal<>& searchCancelled() { return searchCancelled_; }

private:
    void setupUI();
    void createSearchHeader();
    void createLocationSection();
    void createFiltersSection();
    void createDataSourcesSection();
    void createSearchActions();
    void onSearch();
    void onCancel();
    void toggleAdvancedFilters();

    Wt::Signal<Models::SearchQuery> searchRequested_;
    Wt::Signal<> searchCancelled_;

    // UI components
    Wt::WLineEdit* locationInput_ = nullptr;
    Wt::WLineEdit* zipCodeInput_ = nullptr;
    Wt::WLineEdit* cityInput_ = nullptr;
    Wt::WComboBox* stateCombo_ = nullptr;
    Wt::WLineEdit* keywordsInput_ = nullptr;
    Wt::WSlider* radiusSlider_ = nullptr;
    Wt::WText* radiusLabel_ = nullptr;
    Wt::WSlider* minScoreSlider_ = nullptr;
    Wt::WText* minScoreLabel_ = nullptr;

    // Business type checkboxes
    Wt::WCheckBox* cbCorporateOffice_ = nullptr;
    Wt::WCheckBox* cbWarehouse_ = nullptr;
    Wt::WCheckBox* cbConferenceCenter_ = nullptr;
    Wt::WCheckBox* cbTechCompany_ = nullptr;
    Wt::WCheckBox* cbHotel_ = nullptr;
    Wt::WCheckBox* cbCoworking_ = nullptr;

    // Data source checkboxes
    Wt::WCheckBox* cbGoogleMyBusiness_ = nullptr;
    Wt::WCheckBox* cbBBB_ = nullptr;
    Wt::WCheckBox* cbDemographics_ = nullptr;

    // Sort options
    Wt::WComboBox* sortByCombo_ = nullptr;

    // Action buttons
    Wt::WPushButton* searchBtn_ = nullptr;
    Wt::WPushButton* cancelBtn_ = nullptr;
    Wt::WPushButton* clearBtn_ = nullptr;

    // Progress indicator
    Wt::WContainerWidget* progressContainer_ = nullptr;
    Wt::WText* progressText_ = nullptr;

    // Advanced filters
    Wt::WContainerWidget* advancedFilters_ = nullptr;
    bool advancedFiltersVisible_ = false;
};

} // namespace Widgets
} // namespace FranchiseAI

#endif // SEARCH_PANEL_H
