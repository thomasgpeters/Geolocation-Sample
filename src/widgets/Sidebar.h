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
    bool isAdminOnly = false;  // If true, only visible to admin users
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

    /**
     * @brief Set user role to show/hide admin menu items
     * @param role User role ("admin", "franchisee", "staff")
     */
    void setUserRole(const std::string& role);

    /**
     * @brief Signal emitted when logout is requested
     */
    Wt::Signal<>& logoutRequested() { return logoutRequested_; }

    /**
     * @brief Signal emitted when view profile is requested
     */
    Wt::Signal<>& viewProfileRequested() { return viewProfileRequested_; }

    /**
     * @brief Set the logo URL
     * @param url URL or path to the logo image
     */
    void setLogoUrl(const std::string& url);

private:
    void setupUI();
    void createHeader();
    void createMenu();
    void createFooter();
    void onMenuItemClicked(const std::string& itemId);
    void toggleUserDropdown();

    std::vector<MenuItem> menuItems_;
    std::string activeItemId_;
    std::string userRole_ = "franchisee";
    bool isCollapsed_ = false;
    bool isAdmin_ = false;
    bool isDropdownOpen_ = false;

    Wt::Signal<std::string> itemSelected_;
    Wt::Signal<> logoutRequested_;
    Wt::Signal<> viewProfileRequested_;

    // UI components
    Wt::WContainerWidget* headerContainer_ = nullptr;
    Wt::WContainerWidget* menuContainer_ = nullptr;
    Wt::WContainerWidget* footerContainer_ = nullptr;
    Wt::WText* userNameText_ = nullptr;
    Wt::WText* franchiseNameText_ = nullptr;
    Wt::WContainerWidget* userDropdown_ = nullptr;
    Wt::WImage* brandLogo_ = nullptr;
};

} // namespace Widgets
} // namespace FranchiseAI

#endif // SIDEBAR_H
