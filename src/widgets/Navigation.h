#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WSignal.h>
#include <string>

namespace FranchiseAI {
namespace Widgets {

/**
 * @brief Top navigation bar widget
 *
 * Provides the top navigation bar with search input,
 * notifications, and user actions.
 */
class Navigation : public Wt::WContainerWidget {
public:
    Navigation();
    ~Navigation() override = default;

    /**
     * @brief Set the current page title
     * @param title Page title to display
     */
    void setPageTitle(const std::string& title);

    /**
     * @brief Set the breadcrumb path
     * @param breadcrumbs List of breadcrumb items
     */
    void setBreadcrumbs(const std::vector<std::string>& breadcrumbs);

    /**
     * @brief Set notification count
     * @param count Number of notifications
     */
    void setNotificationCount(int count);

    /**
     * @brief Set the market potential score (shown as badge in header)
     * @param score Score from 0-100, or -1 to hide
     */
    void setMarketScore(int score);

    /**
     * @brief Signal emitted when quick search is submitted
     */
    Wt::Signal<std::string>& quickSearchSubmitted() { return quickSearchSubmitted_; }

    /**
     * @brief Signal emitted when help is clicked
     */
    Wt::Signal<>& helpClicked() { return helpClicked_; }

    /**
     * @brief Signal emitted when notifications are clicked
     */
    Wt::Signal<>& notificationsClicked() { return notificationsClicked_; }

private:
    void setupUI();
    void createLeftSection();
    void createCenterSection();
    void createRightSection();
    void onQuickSearch();

    Wt::Signal<std::string> quickSearchSubmitted_;
    Wt::Signal<> helpClicked_;
    Wt::Signal<> notificationsClicked_;

    // UI components
    Wt::WText* pageTitleText_ = nullptr;
    Wt::WContainerWidget* breadcrumbContainer_ = nullptr;
    Wt::WLineEdit* quickSearchInput_ = nullptr;
    Wt::WText* notificationBadge_ = nullptr;
    Wt::WContainerWidget* marketScoreContainer_ = nullptr;
    Wt::WText* marketScoreText_ = nullptr;
    int notificationCount_ = 0;
};

} // namespace Widgets
} // namespace FranchiseAI

#endif // NAVIGATION_H
