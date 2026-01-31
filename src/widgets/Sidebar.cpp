#include "Sidebar.h"
#include <Wt/WVBoxLayout.h>

namespace FranchiseAI {
namespace Widgets {

Sidebar::Sidebar() {
    setStyleClass("sidebar");

    // Define menu items (reordered with divider)
    // Format: {id, label, icon, isActive, isDivider, isAdminOnly}
    menuItems_ = {
        {"dashboard", "Dashboard", "📊", false, false, false},
        {"ai-search", "AI Search", "🔍", true, false, false},
        {"openstreetmap", "Open Street Map", "📍", false, false, false},
        {"divider-1", "", "", false, true, false},  // Dividing line
        {"prospects", "My Prospects", "👥", false, false, false},
        {"reports", "Reports", "📈", false, false, false},
        {"audit-trail", "Audit Trail", "📋", false, false, true},  // Admin only, grouped with reports
        {"divider-2", "", "", false, true, false},  // Divider before settings
        {"settings", "Settings", "⚙️", false, false, false}
    };

    activeItemId_ = "dashboard";

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

    // User info section with dropdown
    auto userSectionWrapper = headerContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    userSectionWrapper->setStyleClass("user-section-wrapper");

    auto userSection = userSectionWrapper->addWidget(std::make_unique<Wt::WContainerWidget>());
    userSection->setStyleClass("user-section clickable");

    auto avatarContainer = userSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    avatarContainer->setStyleClass("user-avatar");
    avatarContainer->addWidget(std::make_unique<Wt::WText>("👤"));

    auto userInfo = userSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    userInfo->setStyleClass("user-info");

    userNameText_ = userInfo->addWidget(std::make_unique<Wt::WText>("Franchise Owner"));
    userNameText_->setStyleClass("user-name");

    franchiseNameText_ = userInfo->addWidget(std::make_unique<Wt::WText>("Catering Solutions"));
    franchiseNameText_->setStyleClass("franchise-name");

    // Dropdown arrow
    auto dropdownArrow = userSection->addWidget(std::make_unique<Wt::WText>("▼"));
    dropdownArrow->setStyleClass("dropdown-arrow");

    // Click handler for user section
    userSection->clicked().connect([this] {
        toggleUserDropdown();
    });

    // Dropdown menu (hidden by default)
    userDropdown_ = userSectionWrapper->addWidget(std::make_unique<Wt::WContainerWidget>());
    userDropdown_->setStyleClass("user-dropdown");
    userDropdown_->hide();

    // View Profile option
    auto profileOption = userDropdown_->addWidget(std::make_unique<Wt::WContainerWidget>());
    profileOption->setStyleClass("dropdown-item");
    profileOption->addWidget(std::make_unique<Wt::WText>("👤"))->setStyleClass("dropdown-icon");
    profileOption->addWidget(std::make_unique<Wt::WText>("View My Profile"))->setStyleClass("dropdown-label");
    profileOption->clicked().connect([this] {
        userDropdown_->hide();
        isDropdownOpen_ = false;
        viewProfileRequested_.emit();
    });

    // Logout option
    auto logoutOption = userDropdown_->addWidget(std::make_unique<Wt::WContainerWidget>());
    logoutOption->setStyleClass("dropdown-item logout");
    logoutOption->addWidget(std::make_unique<Wt::WText>("🚪"))->setStyleClass("dropdown-icon");
    logoutOption->addWidget(std::make_unique<Wt::WText>("Logout"))->setStyleClass("dropdown-label");
    logoutOption->clicked().connect([this] {
        userDropdown_->hide();
        isDropdownOpen_ = false;
        logoutRequested_.emit();
    });
}

void Sidebar::createMenu() {
    menuContainer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    menuContainer_->setStyleClass("sidebar-menu");

    for (const auto& item : menuItems_) {
        // Handle dividers
        if (item.isDivider) {
            auto divider = menuContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
            divider->setStyleClass("menu-divider");
            divider->setId("menu-" + item.id);
            divider->setObjectName("menu-" + item.id);
            // Hide admin-only dividers initially
            if (item.isAdminOnly && !isAdmin_) {
                divider->hide();
            }
            continue;
        }

        auto menuItem = menuContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());

        std::string itemClass = "menu-item";
        if (item.id == activeItemId_) {
            itemClass += " active";
        }
        menuItem->setStyleClass(itemClass);
        menuItem->setId("menu-" + item.id);
        menuItem->setObjectName("menu-" + item.id);

        // Hide admin-only items initially
        if (item.isAdminOnly && !isAdmin_) {
            menuItem->hide();
        }

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

void Sidebar::toggleUserDropdown() {
    isDropdownOpen_ = !isDropdownOpen_;
    if (isDropdownOpen_) {
        userDropdown_->show();
    } else {
        userDropdown_->hide();
    }
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

void Sidebar::setUserRole(const std::string& role) {
    userRole_ = role;
    isAdmin_ = (role == "admin");

    // Show/hide admin-only menu items
    for (const auto& item : menuItems_) {
        if (item.isAdminOnly) {
            auto menuItem = menuContainer_->find("menu-" + item.id);
            if (menuItem) {
                if (isAdmin_) {
                    menuItem->show();
                } else {
                    menuItem->hide();
                }
            }
        }
    }
}

} // namespace Widgets
} // namespace FranchiseAI
