#include "Sidebar.h"
#include "../AppConfig.h"
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

    // Load logo from config (uses default if not configured)
    auto& appConfig = AppConfig::instance();
    brandLogo_ = logoContainer->addWidget(std::make_unique<Wt::WImage>(appConfig.getBrandLogoPath()));
    brandLogo_->setStyleClass("brand-logo");
    brandLogo_->setAlternateText("FranchiseAI Logo");

    auto brandText = logoContainer->addWidget(std::make_unique<Wt::WText>("FranchiseAI"));
    brandText->setStyleClass("brand-text");

    // Franchise info section with popup
    auto franchiseSectionWrapper = headerContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    franchiseSectionWrapper->setStyleClass("franchise-section-wrapper");

    auto franchiseSection = franchiseSectionWrapper->addWidget(std::make_unique<Wt::WContainerWidget>());
    franchiseSection->setStyleClass("franchise-section clickable");

    // Owner avatar container (supports image or emoji fallback)
    auto avatarContainer = franchiseSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    avatarContainer->setStyleClass("owner-avatar-container");
    ownerAvatar_ = avatarContainer->addWidget(std::make_unique<Wt::WImage>());
    ownerAvatar_->setStyleClass("owner-avatar-image hidden");
    auto avatarFallback = avatarContainer->addWidget(std::make_unique<Wt::WText>("👤"));
    avatarFallback->setStyleClass("owner-avatar-fallback");

    auto franchiseInfo = franchiseSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    franchiseInfo->setStyleClass("franchise-info");

    ownerNameText_ = franchiseInfo->addWidget(std::make_unique<Wt::WText>(ownerName_));
    ownerNameText_->setStyleClass("owner-name");

    franchiseNameText_ = franchiseInfo->addWidget(std::make_unique<Wt::WText>(franchiseName_));
    franchiseNameText_->setStyleClass("franchise-name-text");

    // Info expand icon
    auto expandIcon = franchiseSection->addWidget(std::make_unique<Wt::WText>("ℹ️"));
    expandIcon->setStyleClass("franchise-info-icon");

    // Click handler for franchise section
    franchiseSection->clicked().connect([this] {
        toggleFranchisePopup();
    });

    // Franchise Info Popup (hidden by default)
    franchisePopup_ = franchiseSectionWrapper->addWidget(std::make_unique<Wt::WContainerWidget>());
    franchisePopup_->setStyleClass("franchise-popup hidden");

    // Popup header with avatar and edit button
    auto popupHeader = franchisePopup_->addWidget(std::make_unique<Wt::WContainerWidget>());
    popupHeader->setStyleClass("franchise-popup-header");

    // Large avatar in popup
    auto popupAvatarContainer = popupHeader->addWidget(std::make_unique<Wt::WContainerWidget>());
    popupAvatarContainer->setStyleClass("franchise-popup-avatar");
    popupOwnerAvatar_ = popupAvatarContainer->addWidget(std::make_unique<Wt::WImage>());
    popupOwnerAvatar_->setStyleClass("popup-avatar-image hidden");
    auto popupAvatarFallback = popupAvatarContainer->addWidget(std::make_unique<Wt::WText>("👤"));
    popupAvatarFallback->setStyleClass("popup-avatar-fallback");

    // Header info (name + franchise)
    auto popupHeaderInfo = popupHeader->addWidget(std::make_unique<Wt::WContainerWidget>());
    popupHeaderInfo->setStyleClass("franchise-popup-header-info");

    popupOwnerNameText_ = popupHeaderInfo->addWidget(std::make_unique<Wt::WText>(ownerName_));
    popupOwnerNameText_->setStyleClass("popup-owner-name");

    popupFranchiseNameText_ = popupHeaderInfo->addWidget(std::make_unique<Wt::WText>(franchiseName_));
    popupFranchiseNameText_->setStyleClass("popup-franchise-name");

    popupStoreIdText_ = popupHeaderInfo->addWidget(std::make_unique<Wt::WText>(storeId_));
    popupStoreIdText_->setStyleClass("popup-store-id");

    // Edit button with pencil icon
    auto editBtn = popupHeader->addWidget(std::make_unique<Wt::WPushButton>("✏️"));
    editBtn->setStyleClass("franchise-edit-btn");
    editBtn->setToolTip("Edit Franchise Details");
    editBtn->clicked().connect([this] {
        toggleFranchisePopup();
        editFranchiseRequested_.emit();
    });

    // Popup divider
    auto popupDivider = franchisePopup_->addWidget(std::make_unique<Wt::WContainerWidget>());
    popupDivider->setStyleClass("franchise-popup-divider");

    // Popup details section
    auto popupDetails = franchisePopup_->addWidget(std::make_unique<Wt::WContainerWidget>());
    popupDetails->setStyleClass("franchise-popup-details");

    // Address row
    auto addressRow = popupDetails->addWidget(std::make_unique<Wt::WContainerWidget>());
    addressRow->setStyleClass("popup-detail-row");
    auto addressIcon = addressRow->addWidget(std::make_unique<Wt::WText>("📍"));
    addressIcon->setStyleClass("popup-detail-icon");
    popupAddressText_ = addressRow->addWidget(std::make_unique<Wt::WText>("No address set"));
    popupAddressText_->setStyleClass("popup-detail-text");

    // Phone row
    auto phoneRow = popupDetails->addWidget(std::make_unique<Wt::WContainerWidget>());
    phoneRow->setStyleClass("popup-detail-row");
    auto phoneIcon = phoneRow->addWidget(std::make_unique<Wt::WText>("📞"));
    phoneIcon->setStyleClass("popup-detail-icon");
    popupPhoneText_ = phoneRow->addWidget(std::make_unique<Wt::WText>("No phone set"));
    popupPhoneText_->setStyleClass("popup-detail-text");

    // Email row
    auto emailRow = popupDetails->addWidget(std::make_unique<Wt::WContainerWidget>());
    emailRow->setStyleClass("popup-detail-row");
    auto emailIcon = emailRow->addWidget(std::make_unique<Wt::WText>("✉️"));
    emailIcon->setStyleClass("popup-detail-icon");
    popupEmailText_ = emailRow->addWidget(std::make_unique<Wt::WText>("No email set"));
    popupEmailText_->setStyleClass("popup-detail-text");

    // Quick actions section
    auto popupActions = franchisePopup_->addWidget(std::make_unique<Wt::WContainerWidget>());
    popupActions->setStyleClass("franchise-popup-actions");

    // View Profile action
    auto profileAction = popupActions->addWidget(std::make_unique<Wt::WContainerWidget>());
    profileAction->setStyleClass("popup-action-item");
    auto profileIcon = profileAction->addWidget(std::make_unique<Wt::WText>("👤"));
    profileIcon->setStyleClass("popup-action-icon");
    auto profileLabel = profileAction->addWidget(std::make_unique<Wt::WText>("View My Profile"));
    profileLabel->setStyleClass("popup-action-label");
    profileAction->clicked().connect([this] {
        toggleFranchisePopup();
        viewProfileRequested_.emit();
    });

    // Logout action
    auto logoutAction = popupActions->addWidget(std::make_unique<Wt::WContainerWidget>());
    logoutAction->setStyleClass("popup-action-item logout-action");
    auto logoutIcon = logoutAction->addWidget(std::make_unique<Wt::WText>("🚪"));
    logoutIcon->setStyleClass("popup-action-icon");
    auto logoutLabel = logoutAction->addWidget(std::make_unique<Wt::WText>("Logout"));
    logoutLabel->setStyleClass("popup-action-label");
    logoutAction->clicked().connect([this] {
        toggleFranchisePopup();
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

void Sidebar::toggleFranchisePopup() {
    isFranchisePopupOpen_ = !isFranchisePopupOpen_;
    if (franchisePopup_) {
        if (isFranchisePopupOpen_) {
            franchisePopup_->setStyleClass("franchise-popup");
        } else {
            franchisePopup_->setStyleClass("franchise-popup hidden");
        }
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
    ownerName_ = userName;
    franchiseName_ = franchiseName;

    if (ownerNameText_) {
        ownerNameText_->setText(userName);
    }
    if (franchiseNameText_) {
        franchiseNameText_->setText(franchiseName);
    }
    if (popupOwnerNameText_) {
        popupOwnerNameText_->setText(userName);
    }
    if (popupFranchiseNameText_) {
        popupFranchiseNameText_->setText(franchiseName);
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

void Sidebar::setLogoUrl(const std::string& url) {
    if (brandLogo_) {
        brandLogo_->setImageLink(Wt::WLink(url));
    }
}

void Sidebar::setOwnerAvatarUrl(const std::string& url) {
    ownerAvatarUrl_ = url;

    if (!url.empty()) {
        // Show image avatar, hide fallback
        if (ownerAvatar_) {
            ownerAvatar_->setImageLink(Wt::WLink(url));
            ownerAvatar_->setStyleClass("owner-avatar-image");
        }
        if (popupOwnerAvatar_) {
            popupOwnerAvatar_->setImageLink(Wt::WLink(url));
            popupOwnerAvatar_->setStyleClass("popup-avatar-image");
        }
    } else {
        // Hide image avatar, show fallback
        if (ownerAvatar_) {
            ownerAvatar_->setStyleClass("owner-avatar-image hidden");
        }
        if (popupOwnerAvatar_) {
            popupOwnerAvatar_->setStyleClass("popup-avatar-image hidden");
        }
    }
}

void Sidebar::setFranchiseDetails(
    const std::string& ownerName,
    const std::string& franchiseName,
    const std::string& storeId,
    const std::string& address,
    const std::string& phone,
    const std::string& email
) {
    ownerName_ = ownerName;
    franchiseName_ = franchiseName;
    storeId_ = storeId;
    franchiseAddress_ = address;
    franchisePhone_ = phone;
    franchiseEmail_ = email;

    // Update sidebar display
    if (ownerNameText_) {
        ownerNameText_->setText(ownerName);
    }
    if (franchiseNameText_) {
        franchiseNameText_->setText(franchiseName);
    }

    // Update popup display
    if (popupOwnerNameText_) {
        popupOwnerNameText_->setText(ownerName);
    }
    if (popupFranchiseNameText_) {
        popupFranchiseNameText_->setText(franchiseName);
    }
    if (popupStoreIdText_) {
        popupStoreIdText_->setText(storeId);
    }
    if (popupAddressText_) {
        popupAddressText_->setText(address.empty() ? "No address set" : address);
    }
    if (popupPhoneText_) {
        popupPhoneText_->setText(phone.empty() ? "No phone set" : phone);
    }
    if (popupEmailText_) {
        popupEmailText_->setText(email.empty() ? "No email set" : email);
    }
}

} // namespace Widgets
} // namespace FranchiseAI
