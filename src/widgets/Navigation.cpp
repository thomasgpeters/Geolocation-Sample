#include "Navigation.h"

namespace FranchiseAI {
namespace Widgets {

Navigation::Navigation() {
    setStyleClass("top-navigation");
    setupUI();
}

void Navigation::setupUI() {
    createLeftSection();
    createCenterSection();
    createRightSection();
}

void Navigation::createLeftSection() {
    auto leftSection = addWidget(std::make_unique<Wt::WContainerWidget>());
    leftSection->setStyleClass("nav-left");

    // Page title
    pageTitleText_ = leftSection->addWidget(std::make_unique<Wt::WText>("AI Search"));
    pageTitleText_->setStyleClass("page-title");

    // Breadcrumbs
    breadcrumbContainer_ = leftSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    breadcrumbContainer_->setStyleClass("breadcrumbs");

    auto homeLink = breadcrumbContainer_->addWidget(std::make_unique<Wt::WText>("Home"));
    homeLink->setStyleClass("breadcrumb-item");

    auto separator = breadcrumbContainer_->addWidget(std::make_unique<Wt::WText>(" / "));
    separator->setStyleClass("breadcrumb-separator");

    auto currentPage = breadcrumbContainer_->addWidget(std::make_unique<Wt::WText>("AI Search"));
    currentPage->setStyleClass("breadcrumb-item current");
}

void Navigation::createCenterSection() {
    auto centerSection = addWidget(std::make_unique<Wt::WContainerWidget>());
    centerSection->setStyleClass("nav-center");

    // Quick search container
    auto searchContainer = centerSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    searchContainer->setStyleClass("quick-search-container");

    auto searchIcon = searchContainer->addWidget(std::make_unique<Wt::WText>("🔍"));
    searchIcon->setStyleClass("search-icon");

    quickSearchInput_ = searchContainer->addWidget(std::make_unique<Wt::WLineEdit>());
    quickSearchInput_->setStyleClass("quick-search-input");
    quickSearchInput_->setPlaceholderText("Quick search prospects...");

    quickSearchInput_->enterPressed().connect([this] {
        onQuickSearch();
    });

    auto searchBtn = searchContainer->addWidget(std::make_unique<Wt::WPushButton>("Search"));
    searchBtn->setStyleClass("quick-search-btn");
    searchBtn->clicked().connect([this] {
        onQuickSearch();
    });
}

void Navigation::createRightSection() {
    auto rightSection = addWidget(std::make_unique<Wt::WContainerWidget>());
    rightSection->setStyleClass("nav-right");

    // Market Score badge (hidden by default)
    marketScoreContainer_ = rightSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    marketScoreContainer_->setStyleClass("market-score-container hidden");

    auto scoreLabel = marketScoreContainer_->addWidget(std::make_unique<Wt::WText>("Market Score"));
    scoreLabel->setStyleClass("market-score-label");

    marketScoreText_ = marketScoreContainer_->addWidget(std::make_unique<Wt::WText>("--"));
    marketScoreText_->setStyleClass("market-score-value");

    // Help button
    auto helpBtn = rightSection->addWidget(std::make_unique<Wt::WPushButton>("❓"));
    helpBtn->setStyleClass("nav-icon-btn");
    helpBtn->setToolTip("Help & Documentation");
    helpBtn->clicked().connect([this] {
        helpClicked_.emit();
    });

    // Notifications button with badge
    auto notifContainer = rightSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    notifContainer->setStyleClass("notification-container");

    auto notifBtn = notifContainer->addWidget(std::make_unique<Wt::WPushButton>("🔔"));
    notifBtn->setStyleClass("nav-icon-btn");
    notifBtn->setToolTip("Notifications");
    notifBtn->clicked().connect([this] {
        notificationsClicked_.emit();
    });

    notificationBadge_ = notifContainer->addWidget(std::make_unique<Wt::WText>("0"));
    notificationBadge_->setStyleClass("notification-badge hidden");

    // User menu container (button + dropdown)
    userMenuContainer_ = rightSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    userMenuContainer_->setStyleClass("user-menu-container");

    // User menu button
    auto userBtn = userMenuContainer_->addWidget(std::make_unique<Wt::WPushButton>("👤"));
    userBtn->setStyleClass("nav-icon-btn user-btn");
    userBtn->setToolTip("User Menu");
    userBtn->clicked().connect([this] {
        toggleUserMenu();
    });

    // User dropdown menu (hidden by default)
    userDropdown_ = userMenuContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    userDropdown_->setStyleClass("user-dropdown hidden");

    // User info header in dropdown
    auto userInfoSection = userDropdown_->addWidget(std::make_unique<Wt::WContainerWidget>());
    userInfoSection->setStyleClass("user-dropdown-header");

    auto userAvatar = userInfoSection->addWidget(std::make_unique<Wt::WText>("👤"));
    userAvatar->setStyleClass("user-dropdown-avatar");

    auto userDetails = userInfoSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    userDetails->setStyleClass("user-dropdown-details");

    userNameText_ = userDetails->addWidget(std::make_unique<Wt::WText>(userName_));
    userNameText_->setStyleClass("user-dropdown-name");

    userEmailText_ = userDetails->addWidget(std::make_unique<Wt::WText>(userEmail_));
    userEmailText_->setStyleClass("user-dropdown-email");

    // Divider
    auto divider1 = userDropdown_->addWidget(std::make_unique<Wt::WContainerWidget>());
    divider1->setStyleClass("user-dropdown-divider");

    // Menu items
    auto menuItems = userDropdown_->addWidget(std::make_unique<Wt::WContainerWidget>());
    menuItems->setStyleClass("user-dropdown-menu");

    // User Profile option
    auto profileItem = menuItems->addWidget(std::make_unique<Wt::WContainerWidget>());
    profileItem->setStyleClass("user-dropdown-item");
    auto profileIcon = profileItem->addWidget(std::make_unique<Wt::WText>("👤"));
    profileIcon->setStyleClass("dropdown-item-icon");
    auto profileText = profileItem->addWidget(std::make_unique<Wt::WText>("User Profile"));
    profileText->setStyleClass("dropdown-item-text");
    profileItem->clicked().connect([this] {
        toggleUserMenu();  // Close menu
        userProfileClicked_.emit();
    });

    // Settings option
    auto settingsItem = menuItems->addWidget(std::make_unique<Wt::WContainerWidget>());
    settingsItem->setStyleClass("user-dropdown-item");
    auto settingsIcon = settingsItem->addWidget(std::make_unique<Wt::WText>("⚙️"));
    settingsIcon->setStyleClass("dropdown-item-icon");
    auto settingsText = settingsItem->addWidget(std::make_unique<Wt::WText>("Settings"));
    settingsText->setStyleClass("dropdown-item-text");

    // Divider before logout
    auto divider2 = userDropdown_->addWidget(std::make_unique<Wt::WContainerWidget>());
    divider2->setStyleClass("user-dropdown-divider");

    // Logout option
    auto logoutItem = userDropdown_->addWidget(std::make_unique<Wt::WContainerWidget>());
    logoutItem->setStyleClass("user-dropdown-item logout-item");
    auto logoutIcon = logoutItem->addWidget(std::make_unique<Wt::WText>("🚪"));
    logoutIcon->setStyleClass("dropdown-item-icon");
    auto logoutText = logoutItem->addWidget(std::make_unique<Wt::WText>("Logout"));
    logoutText->setStyleClass("dropdown-item-text");
    logoutItem->clicked().connect([this] {
        toggleUserMenu();  // Close menu
        logoutClicked_.emit();
    });
}

void Navigation::setPageTitle(const std::string& title) {
    if (pageTitleText_) {
        pageTitleText_->setText(title);
    }
}

void Navigation::setBreadcrumbs(const std::vector<std::string>& breadcrumbs) {
    if (!breadcrumbContainer_) return;

    breadcrumbContainer_->clear();

    for (size_t i = 0; i < breadcrumbs.size(); ++i) {
        auto item = breadcrumbContainer_->addWidget(std::make_unique<Wt::WText>(breadcrumbs[i]));

        if (i == breadcrumbs.size() - 1) {
            item->setStyleClass("breadcrumb-item current");
        } else {
            item->setStyleClass("breadcrumb-item");

            auto separator = breadcrumbContainer_->addWidget(std::make_unique<Wt::WText>(" / "));
            separator->setStyleClass("breadcrumb-separator");
        }
    }
}

void Navigation::setNotificationCount(int count) {
    notificationCount_ = count;

    if (notificationBadge_) {
        if (count > 0) {
            notificationBadge_->setText(count > 99 ? "99+" : std::to_string(count));
            notificationBadge_->setStyleClass("notification-badge");
        } else {
            notificationBadge_->setStyleClass("notification-badge hidden");
        }
    }
}

void Navigation::onQuickSearch() {
    if (quickSearchInput_ && !quickSearchInput_->text().empty()) {
        quickSearchSubmitted_.emit(quickSearchInput_->text().toUTF8());
    }
}

void Navigation::setMarketScore(int score) {
    if (!marketScoreContainer_ || !marketScoreText_) return;

    if (score < 0) {
        marketScoreContainer_->setStyleClass("market-score-container hidden");
    } else {
        marketScoreText_->setText(std::to_string(score) + "/100");

        // Color code based on score
        std::string colorClass = "market-score-container";
        if (score >= 70) {
            colorClass += " score-high";
        } else if (score >= 40) {
            colorClass += " score-medium";
        } else {
            colorClass += " score-low";
        }
        marketScoreContainer_->setStyleClass(colorClass);
    }
}

void Navigation::toggleUserMenu() {
    userMenuOpen_ = !userMenuOpen_;

    if (userDropdown_) {
        if (userMenuOpen_) {
            userDropdown_->setStyleClass("user-dropdown");
        } else {
            userDropdown_->setStyleClass("user-dropdown hidden");
        }
    }
}

void Navigation::setUserName(const std::string& name) {
    userName_ = name;
    if (userNameText_) {
        userNameText_->setText(name);
    }
}

void Navigation::setUserEmail(const std::string& email) {
    userEmail_ = email;
    if (userEmailText_) {
        userEmailText_->setText(email);
    }
}

} // namespace Widgets
} // namespace FranchiseAI
