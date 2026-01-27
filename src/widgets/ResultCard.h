#ifndef RESULT_CARD_H
#define RESULT_CARD_H

#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>
#include <Wt/WSignal.h>
#include "models/SearchResult.h"

namespace FranchiseAI {
namespace Widgets {

/**
 * @brief Individual result card widget
 *
 * Displays a single search result with business information,
 * ratings, and action buttons.
 */
class ResultCard : public Wt::WContainerWidget {
public:
    explicit ResultCard(const Models::SearchResultItem& item);
    ~ResultCard() override = default;

    /**
     * @brief Get the result item
     */
    const Models::SearchResultItem& getItem() const { return item_; }

    /**
     * @brief Signal emitted when view details is clicked
     */
    Wt::Signal<std::string>& viewDetailsRequested() { return viewDetailsRequested_; }

    /**
     * @brief Signal emitted when add to prospects is clicked
     */
    Wt::Signal<std::string>& addToProspectsRequested() { return addToProspectsRequested_; }

    /**
     * @brief Update the card with new data
     */
    void updateData(const Models::SearchResultItem& item);

    /**
     * @brief Expand/collapse the card details
     */
    void toggleExpanded();

    /**
     * @brief Check if card is expanded
     */
    bool isExpanded() const { return isExpanded_; }

private:
    void setupUI();
    void createHeader();
    void createBody();
    void createMetrics();
    void createInsights();
    void createActions();
    void createExpandedDetails();

    std::string formatScore(int score) const;
    std::string getScoreClass(int score) const;
    std::string formatRating(double rating) const;

    Models::SearchResultItem item_;
    bool isExpanded_ = false;

    Wt::Signal<std::string> viewDetailsRequested_;
    Wt::Signal<std::string> addToProspectsRequested_;

    // UI components
    Wt::WContainerWidget* headerContainer_ = nullptr;
    Wt::WContainerWidget* bodyContainer_ = nullptr;
    Wt::WContainerWidget* expandedContainer_ = nullptr;
    Wt::WText* scoreText_ = nullptr;
    Wt::WPushButton* expandBtn_ = nullptr;
};

} // namespace Widgets
} // namespace FranchiseAI

#endif // RESULT_CARD_H
