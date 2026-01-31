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
     * @brief Signal emitted when franchise edit is requested
     */
    Wt::Signal<>& editFranchiseRequested() { return editFranchiseRequested_; }

    /**
     * @brief Set the logo URL
     * @param url URL or path to the logo image
     */
    void setLogoUrl(const std::string& url);

    /**
     * @brief Set the franchise owner's avatar URL
     * @param url URL or path to the avatar image
     */
    void setOwnerAvatarUrl(const std::string& url);

    /**
     * @brief Set franchise details for the info popup
     * @param ownerName Owner's name
     * @param franchiseName Franchise name
     * @param storeId Store ID or number
     * @param address Franchise address
     * @param phone Contact phone
     * @param email Contact email
     */
    void setFranchiseDetails(
        const std::string& ownerName,
        const std::string& franchiseName,
        const std::string& storeId,
        const std::string& address,
        const std::string& phone,
        const std::string& email
    );

private:
    void setupUI();
    void createHeader();
    void createMenu();
    void createFooter();
    void onMenuItemClicked(const std::string& itemId);
    void toggleFranchisePopup();

    std::vector<MenuItem> menuItems_;
    std::string activeItemId_;
    std::string userRole_ = "franchisee";
    bool isCollapsed_ = false;
    bool isAdmin_ = false;
    bool isFranchisePopupOpen_ = false;

    Wt::Signal<std::string> itemSelected_;
    Wt::Signal<> logoutRequested_;
    Wt::Signal<> viewProfileRequested_;
    Wt::Signal<> editFranchiseRequested_;

    // Franchise details
    std::string ownerName_ = "Franchise Owner";
    std::string franchiseName_ = "Catering Solutions";
    std::string storeId_ = "Store #001";
    std::string franchiseAddress_ = "";
    std::string franchisePhone_ = "";
    std::string franchiseEmail_ = "";
    std::string ownerAvatarUrl_ = "";

    // UI components
    Wt::WContainerWidget* headerContainer_ = nullptr;
    Wt::WContainerWidget* menuContainer_ = nullptr;
    Wt::WContainerWidget* footerContainer_ = nullptr;
    Wt::WText* ownerNameText_ = nullptr;
    Wt::WText* franchiseNameText_ = nullptr;
    Wt::WContainerWidget* franchisePopup_ = nullptr;
    Wt::WImage* brandLogo_ = nullptr;
    Wt::WImage* ownerAvatar_ = nullptr;

    // Franchise popup detail texts
    Wt::WText* popupOwnerNameText_ = nullptr;
    Wt::WText* popupFranchiseNameText_ = nullptr;
    Wt::WText* popupStoreIdText_ = nullptr;
    Wt::WText* popupAddressText_ = nullptr;
    Wt::WText* popupPhoneText_ = nullptr;
    Wt::WText* popupEmailText_ = nullptr;
    Wt::WImage* popupOwnerAvatar_ = nullptr;
};

} // namespace Widgets
} // namespace FranchiseAI

#endif // SIDEBAR_H
