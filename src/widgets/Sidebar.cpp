#include "Sidebar.h"
#include <Wt/WVBoxLayout.h>

namespace FranchiseAI {
namespace Widgets {

Sidebar::Sidebar() {
    setStyleClass("sidebar");

    // Define menu items
    menuItems_ = {
        {"dashboard", "Dashboard", "📊", false},
        {"ai-search", "AI Search", "🔍", true},
        {"prospects", "My Prospects", "👥", false},
        {"demographics", "Demographics", "📍", false},
        {"reports", "Reports", "📈", false},
        {"settings", "Settings", "⚙️", false}
    };

    activeItemId_ = "ai-search";

    setupUI();
}

void Sidebar::setupUI() {
    createHeader();
    createMenu();
    createFooter();
}

void Sidebar::createHeader() {
    headerContainer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    headerContainer_->setStyleClass("sidebar-header");

    // Logo/Brand
    auto logoContainer = headerContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    logoContainer->setStyleClass("sidebar-logo");

    auto brandIcon = logoContainer->addWidget(std::make_unique<Wt::WText>("🍽️"));
    brandIcon->setStyleClass("brand-icon");

    auto brandText = logoContainer->addWidget(std::make_unique<Wt::WText>("FranchiseAI"));
    brandText->setStyleClass("brand-text");

    // User info section
    auto userSection = headerContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    userSection->setStyleClass("user-section");

    auto avatarContainer = userSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    avatarContainer->setStyleClass("user-avatar");
    avatarContainer->addWidget(std::make_unique<Wt::WText>("👤"));

    auto userInfo = userSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    userInfo->setStyleClass("user-info");

    userNameText_ = userInfo->addWidget(std::make_unique<Wt::WText>("Franchise Owner"));
    userNameText_->setStyleClass("user-name");

    franchiseNameText_ = userInfo->addWidget(std::make_unique<Wt::WText>("Catering Solutions"));
    franchiseNameText_->setStyleClass("franchise-name");
}

void Sidebar::createMenu() {
    menuContainer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    menuContainer_->setStyleClass("sidebar-menu");

    for (const auto& item : menuItems_) {
        auto menuItem = menuContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());

        std::string itemClass = "menu-item";
        if (item.id == activeItemId_) {
            itemClass += " active";
        }
        menuItem->setStyleClass(itemClass);
        menuItem->setId("menu-" + item.id);

        auto icon = menuItem->addWidget(std::make_unique<Wt::WText>(item.icon));
        icon->setStyleClass("menu-icon");

        auto label = menuItem->addWidget(std::make_unique<Wt::WText>(item.label));
        label->setStyleClass("menu-label");

        // Click handler
        menuItem->clicked().connect([this, itemId = item.id] {
            onMenuItemClicked(itemId);
        });
    }
}

void Sidebar::createFooter() {
    footerContainer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    footerContainer_->setStyleClass("sidebar-footer");

    // Collapse toggle button
    auto collapseBtn = footerContainer_->addWidget(std::make_unique<Wt::WPushButton>("◀"));
    collapseBtn->setStyleClass("collapse-btn");
    collapseBtn->clicked().connect([this] {
        toggleCollapse();
    });

    // Version info
    auto versionText = footerContainer_->addWidget(std::make_unique<Wt::WText>("v1.0.0"));
    versionText->setStyleClass("version-text");
}

void Sidebar::setActiveItem(const std::string& itemId) {
    if (activeItemId_ == itemId) return;

    // Remove active class from current item
    auto currentItem = menuContainer_->find("menu-" + activeItemId_);
    if (currentItem) {
        currentItem->setStyleClass("menu-item");
    }

    // Add active class to new item
    auto newItem = menuContainer_->find("menu-" + itemId);
    if (newItem) {
        newItem->setStyleClass("menu-item active");
    }

    activeItemId_ = itemId;
}

void Sidebar::setUserInfo(const std::string& userName, const std::string& franchiseName) {
    if (userNameText_) {
        userNameText_->setText(userName);
    }
    if (franchiseNameText_) {
        franchiseNameText_->setText(franchiseName);
    }
}

void Sidebar::toggleCollapse() {
    isCollapsed_ = !isCollapsed_;

    if (isCollapsed_) {
        setStyleClass("sidebar collapsed");
    } else {
        setStyleClass("sidebar");
    }
}

void Sidebar::onMenuItemClicked(const std::string& itemId) {
    setActiveItem(itemId);
    itemSelected_.emit(itemId);
}

} // namespace Widgets
} // namespace FranchiseAI
