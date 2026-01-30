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

    // User menu button
    auto userBtn = rightSection->addWidget(std::make_unique<Wt::WPushButton>("👤"));
    userBtn->setStyleClass("nav-icon-btn user-btn");
    userBtn->setToolTip("User Menu");
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

} // namespace Widgets
} // namespace FranchiseAI
