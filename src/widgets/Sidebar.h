#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <Wt/WContainerWidget.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>
#include <Wt/WImage.h>
#include <Wt/WSignal.h>
#include <string>
#include <vector>
#include <functional>

namespace FranchiseAI {
namespace Widgets {

/**
 * @brief Navigation menu item
 */
struct MenuItem {
    std::string id;
    std::string label;
    std::string icon;
    bool isActive = false;
    bool isDivider = false;  // If true, renders as a dividing line
};

/**
 * @brief Sidebar widget for main navigation
 *
 * Provides the main navigation sidebar with menu items
 * for different sections of the application.
 */
class Sidebar : public Wt::WContainerWidget {
public:
    Sidebar();
    ~Sidebar() override = default;

    /**
     * @brief Set the active menu item
     * @param itemId Menu item ID to activate
     */
    void setActiveItem(const std::string& itemId);

    /**
     * @brief Get the currently active menu item ID
     */
    std::string getActiveItem() const { return activeItemId_; }

    /**
     * @brief Signal emitted when a menu item is selected
     */
    Wt::Signal<std::string>& itemSelected() { return itemSelected_; }

    /**
     * @brief Set user information displayed in sidebar
     * @param userName User's display name
     * @param franchiseName Franchise name
     */
    void setUserInfo(const std::string& userName, const std::string& franchiseName);

    /**
     * @brief Toggle sidebar collapsed state
     */
    void toggleCollapse();

    /**
     * @brief Check if sidebar is collapsed
     */
    bool isCollapsed() const { return isCollapsed_; }

private:
    void setupUI();
    void createHeader();
    void createMenu();
    void createFooter();
    void onMenuItemClicked(const std::string& itemId);

    std::vector<MenuItem> menuItems_;
    std::string activeItemId_;
    bool isCollapsed_ = false;

    Wt::Signal<std::string> itemSelected_;

    // UI components
    Wt::WContainerWidget* headerContainer_ = nullptr;
    Wt::WContainerWidget* menuContainer_ = nullptr;
    Wt::WContainerWidget* footerContainer_ = nullptr;
    Wt::WText* userNameText_ = nullptr;
    Wt::WText* franchiseNameText_ = nullptr;
};

} // namespace Widgets
} // namespace FranchiseAI

#endif // SIDEBAR_H
