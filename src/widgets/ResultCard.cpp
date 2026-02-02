#include "ResultCard.h"
#include <sstream>
#include <iomanip>

namespace FranchiseAI {
namespace Widgets {

ResultCard::ResultCard(const Models::SearchResultItem& item) : item_(item) {
    setStyleClass("result-card");
    setupUI();
}

void ResultCard::setupUI() {
    createHeader();
    createBody();
    createExpandedDetails();
}

void ResultCard::createHeader() {
    headerContainer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    headerContainer_->setStyleClass("card-header");

    // Selection checkbox (leftmost)
    selectCheckbox_ = headerContainer_->addWidget(std::make_unique<Wt::WCheckBox>());
    selectCheckbox_->setStyleClass("result-select-checkbox");
    selectCheckbox_->changed().connect([this] {
        selectionChanged_.emit(item_.id, selectCheckbox_->isChecked());
    });

    // Left side: Score badge and title
    auto leftSection = headerContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    leftSection->setStyleClass("header-left");

    // Score badge
    auto scoreBadge = leftSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    scoreBadge->setStyleClass("score-badge " + getScoreClass(item_.overallScore));

    scoreText_ = scoreBadge->addWidget(std::make_unique<Wt::WText>(formatScore(item_.overallScore)));
    scoreText_->setStyleClass("score-value");

    auto scoreLabel = scoreBadge->addWidget(std::make_unique<Wt::WText>("Score"));
    scoreLabel->setStyleClass("score-label");

    // Title section
    auto titleSection = leftSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    titleSection->setStyleClass("title-section");

    auto title = titleSection->addWidget(std::make_unique<Wt::WText>(item_.getTitle(), Wt::TextFormat::Plain));
    title->setStyleClass("card-title");

    auto subtitle = titleSection->addWidget(std::make_unique<Wt::WText>(item_.getSubtitle(), Wt::TextFormat::Plain));
    subtitle->setStyleClass("card-subtitle");

    // Type badge
    auto typeBadge = titleSection->addWidget(std::make_unique<Wt::WContainerWidget>());
    typeBadge->setStyleClass("type-badge");

    std::string typeIcon = "🏢";
    std::string typeName = item_.getResultTypeString();

    if (item_.business) {
        switch (item_.business->type) {
            case Models::BusinessType::CORPORATE_OFFICE: typeIcon = "🏢"; break;
            case Models::BusinessType::WAREHOUSE: typeIcon = "🏭"; break;
            case Models::BusinessType::CONFERENCE_CENTER: typeIcon = "🎪"; break;
            case Models::BusinessType::TECH_COMPANY: typeIcon = "💻"; break;
            case Models::BusinessType::HOTEL: typeIcon = "🏨"; break;
            case Models::BusinessType::COWORKING_SPACE: typeIcon = "🪑"; break;
            case Models::BusinessType::MEDICAL_FACILITY: typeIcon = "🏥"; break;
            case Models::BusinessType::MANUFACTURING: typeIcon = "⚙️"; break;
            default: typeIcon = "🏛️"; break;
        }
        typeName = item_.business->getBusinessTypeString();
    } else if (item_.demographic) {
        typeIcon = "📊";
        typeName = "Area Analysis";
    }

    auto typeIconText = typeBadge->addWidget(std::make_unique<Wt::WText>(typeIcon));
    auto typeNameText = typeBadge->addWidget(std::make_unique<Wt::WText>(" " + typeName));

    // Right side: Action buttons and expand button
    auto rightSection = headerContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    rightSection->setStyleClass("header-right");

    auto viewBtn = rightSection->addWidget(std::make_unique<Wt::WPushButton>("View Details"));
    viewBtn->setStyleClass("btn btn-outline btn-xs");
    viewBtn->clicked().connect([this] {
        viewDetailsRequested_.emit(item_.id);
    });

    auto addBtn = rightSection->addWidget(std::make_unique<Wt::WPushButton>("+ Add to Prospects"));
    addBtn->setStyleClass("btn btn-primary btn-xs");
    addBtn->clicked().connect([this] {
        addToProspectsRequested_.emit(item_.id);
    });

    expandBtn_ = rightSection->addWidget(std::make_unique<Wt::WPushButton>("▼"));
    expandBtn_->setStyleClass("expand-btn");
    expandBtn_->clicked().connect([this] { toggleExpanded(); });
}

void ResultCard::createBody() {
    bodyContainer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    bodyContainer_->setStyleClass("card-body");

    createMetrics();
    createInsights();
}

void ResultCard::createMetrics() {
    auto metricsContainer = bodyContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    metricsContainer->setStyleClass("metrics-container");

    if (item_.business) {
        auto& biz = *item_.business;

        // Employee count
        auto empMetric = metricsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
        empMetric->setStyleClass("metric");
        auto empIcon = empMetric->addWidget(std::make_unique<Wt::WText>("👥"));
        auto empValue = empMetric->addWidget(std::make_unique<Wt::WText>(std::to_string(biz.employeeCount)));
        empValue->setStyleClass("metric-value");
        auto empLabel = empMetric->addWidget(std::make_unique<Wt::WText>("Employees"));
        empLabel->setStyleClass("metric-label");

        // Google rating
        if (biz.googleRating > 0) {
            auto ratingMetric = metricsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
            ratingMetric->setStyleClass("metric");
            auto ratingIcon = ratingMetric->addWidget(std::make_unique<Wt::WText>("⭐"));
            auto ratingValue = ratingMetric->addWidget(std::make_unique<Wt::WText>(formatRating(biz.googleRating)));
            ratingValue->setStyleClass("metric-value");
            auto ratingLabel = ratingMetric->addWidget(std::make_unique<Wt::WText>("Google Rating"));
            ratingLabel->setStyleClass("metric-label");
        }

        // BBB info
        if (biz.bbbAccredited) {
            auto bbbMetric = metricsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
            bbbMetric->setStyleClass("metric");
            auto bbbIcon = bbbMetric->addWidget(std::make_unique<Wt::WText>("✓"));
            bbbIcon->setStyleClass("bbb-accredited");
            auto bbbValue = bbbMetric->addWidget(std::make_unique<Wt::WText>(biz.getBBBRatingString()));
            bbbValue->setStyleClass("metric-value");
            auto bbbLabel = bbbMetric->addWidget(std::make_unique<Wt::WText>("BBB Rating"));
            bbbLabel->setStyleClass("metric-label");
        }

        // Conference room indicator
        if (biz.hasConferenceRoom) {
            auto confMetric = metricsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
            confMetric->setStyleClass("metric feature-badge");
            auto confIcon = confMetric->addWidget(std::make_unique<Wt::WText>("🎤"));
            auto confLabel = confMetric->addWidget(std::make_unique<Wt::WText>("Conference Room"));
            confLabel->setStyleClass("metric-label");
        }

        // Event space indicator
        if (biz.hasEventSpace) {
            auto eventMetric = metricsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
            eventMetric->setStyleClass("metric feature-badge");
            auto eventIcon = eventMetric->addWidget(std::make_unique<Wt::WText>("🎉"));
            auto eventLabel = eventMetric->addWidget(std::make_unique<Wt::WText>("Event Space"));
            eventLabel->setStyleClass("metric-label");
        }

    } else if (item_.demographic) {
        auto& demo = *item_.demographic;

        // Total businesses
        auto bizMetric = metricsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
        bizMetric->setStyleClass("metric");
        auto bizIcon = bizMetric->addWidget(std::make_unique<Wt::WText>("🏢"));
        auto bizValue = bizMetric->addWidget(std::make_unique<Wt::WText>(std::to_string(demo.totalBusinesses)));
        bizValue->setStyleClass("metric-value");
        auto bizLabel = bizMetric->addWidget(std::make_unique<Wt::WText>("Businesses"));
        bizLabel->setStyleClass("metric-label");

        // Working population
        auto popMetric = metricsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
        popMetric->setStyleClass("metric");
        auto popIcon = popMetric->addWidget(std::make_unique<Wt::WText>("👥"));
        auto popValue = popMetric->addWidget(std::make_unique<Wt::WText>(std::to_string(demo.workingAgePopulation)));
        popValue->setStyleClass("metric-value");
        auto popLabel = popMetric->addWidget(std::make_unique<Wt::WText>("Working Pop."));
        popLabel->setStyleClass("metric-label");

        // Market potential
        auto marketMetric = metricsContainer->addWidget(std::make_unique<Wt::WContainerWidget>());
        marketMetric->setStyleClass("metric");
        auto marketIcon = marketMetric->addWidget(std::make_unique<Wt::WText>("📈"));
        auto marketValue = marketMetric->addWidget(std::make_unique<Wt::WText>(demo.getMarketPotentialDescription()));
        marketValue->setStyleClass("metric-value");
        auto marketLabel = marketMetric->addWidget(std::make_unique<Wt::WText>("Market Potential"));
        marketLabel->setStyleClass("metric-label");
    }
}

void ResultCard::createInsights() {
    if (item_.aiSummary.empty()) return;

    auto insightsContainer = bodyContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
    insightsContainer->setStyleClass("insights-container");

    auto insightsIcon = insightsContainer->addWidget(std::make_unique<Wt::WText>("🤖 "));
    insightsIcon->setStyleClass("insights-icon");

    auto insightsText = insightsContainer->addWidget(std::make_unique<Wt::WText>(item_.aiSummary, Wt::TextFormat::Plain));
    insightsText->setStyleClass("insights-text");
}

void ResultCard::createExpandedDetails() {
    expandedContainer_ = addWidget(std::make_unique<Wt::WContainerWidget>());
    expandedContainer_->setStyleClass("expanded-details hidden");

    // Key highlights
    if (!item_.keyHighlights.empty()) {
        auto highlightsSection = expandedContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
        highlightsSection->setStyleClass("expanded-section");

        auto highlightsTitle = highlightsSection->addWidget(std::make_unique<Wt::WText>("Key Highlights"));
        highlightsTitle->setStyleClass("section-title");

        auto highlightsList = highlightsSection->addWidget(std::make_unique<Wt::WContainerWidget>());
        highlightsList->setStyleClass("highlights-list");

        for (const auto& highlight : item_.keyHighlights) {
            auto item = highlightsList->addWidget(std::make_unique<Wt::WContainerWidget>());
            item->setStyleClass("highlight-item");

            auto bullet = item->addWidget(std::make_unique<Wt::WText>("- ", Wt::TextFormat::Plain));
            auto text = item->addWidget(std::make_unique<Wt::WText>(highlight, Wt::TextFormat::Plain));
        }
    }

    // Recommended actions
    if (!item_.recommendedActions.empty()) {
        auto actionsSection = expandedContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
        actionsSection->setStyleClass("expanded-section");

        auto actionsTitle = actionsSection->addWidget(std::make_unique<Wt::WText>("Recommended Actions"));
        actionsTitle->setStyleClass("section-title");

        auto actionsList = actionsSection->addWidget(std::make_unique<Wt::WContainerWidget>());
        actionsList->setStyleClass("actions-list");

        int actionNum = 1;
        for (const auto& action : item_.recommendedActions) {
            auto item = actionsList->addWidget(std::make_unique<Wt::WContainerWidget>());
            item->setStyleClass("action-item");

            auto number = item->addWidget(std::make_unique<Wt::WText>(std::to_string(actionNum++) + ". ", Wt::TextFormat::Plain));
            number->setStyleClass("action-number");
            auto text = item->addWidget(std::make_unique<Wt::WText>(action, Wt::TextFormat::Plain));
        }
    }

    // Contact info (if business)
    if (item_.business && !item_.business->contact.primaryPhone.empty()) {
        auto contactSection = expandedContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
        contactSection->setStyleClass("expanded-section contact-section");

        auto contactTitle = contactSection->addWidget(std::make_unique<Wt::WText>("Contact Information"));
        contactTitle->setStyleClass("section-title");

        auto contactGrid = contactSection->addWidget(std::make_unique<Wt::WContainerWidget>());
        contactGrid->setStyleClass("contact-grid");

        auto& contact = item_.business->contact;

        if (!contact.primaryPhone.empty()) {
            auto phoneItem = contactGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
            phoneItem->setStyleClass("contact-item");
            auto phoneIcon = phoneItem->addWidget(std::make_unique<Wt::WText>("📞 "));
            auto phoneText = phoneItem->addWidget(std::make_unique<Wt::WText>(contact.primaryPhone, Wt::TextFormat::Plain));
        }

        if (!contact.email.empty()) {
            auto emailItem = contactGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
            emailItem->setStyleClass("contact-item");
            auto emailIcon = emailItem->addWidget(std::make_unique<Wt::WText>("✉️ "));
            auto emailText = emailItem->addWidget(std::make_unique<Wt::WText>(contact.email, Wt::TextFormat::Plain));
        }

        if (!contact.website.empty()) {
            auto webItem = contactGrid->addWidget(std::make_unique<Wt::WContainerWidget>());
            webItem->setStyleClass("contact-item");
            auto webIcon = webItem->addWidget(std::make_unique<Wt::WText>("🌐 "));
            auto webText = webItem->addWidget(std::make_unique<Wt::WText>(contact.website, Wt::TextFormat::Plain));
        }
    }

    // Data sources
    if (!item_.sources.empty()) {
        auto sourcesSection = expandedContainer_->addWidget(std::make_unique<Wt::WContainerWidget>());
        sourcesSection->setStyleClass("expanded-section sources-section");

        auto sourcesLabel = sourcesSection->addWidget(std::make_unique<Wt::WText>("Data Sources: "));
        sourcesLabel->setStyleClass("sources-label");

        for (const auto& source : item_.sources) {
            auto sourceBadge = sourcesSection->addWidget(std::make_unique<Wt::WText>(
                Models::dataSourceToString(source), Wt::TextFormat::Plain
            ));
            sourceBadge->setStyleClass("source-badge");
        }
    }
}

void ResultCard::toggleExpanded() {
    isExpanded_ = !isExpanded_;

    if (isExpanded_) {
        expandedContainer_->setStyleClass("expanded-details");
        expandBtn_->setText("▲");
        setStyleClass("result-card expanded");
    } else {
        expandedContainer_->setStyleClass("expanded-details hidden");
        expandBtn_->setText("▼");
        setStyleClass("result-card");
    }
}

void ResultCard::updateData(const Models::SearchResultItem& item) {
    item_ = item;
    // Would need to rebuild UI - simplified for now
}

bool ResultCard::isSelected() const {
    return selectCheckbox_ && selectCheckbox_->isChecked();
}

void ResultCard::setSelected(bool selected) {
    if (selectCheckbox_) {
        selectCheckbox_->setChecked(selected);
    }
}

std::string ResultCard::formatScore(int score) const {
    return std::to_string(score);
}

std::string ResultCard::getScoreClass(int score) const {
    if (score >= 80) return "score-excellent";
    if (score >= 60) return "score-high";
    if (score >= 40) return "score-moderate";
    if (score >= 20) return "score-low";
    return "score-minimal";
}

std::string ResultCard::formatRating(double rating) const {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << rating;
    return ss.str();
}

} // namespace Widgets
} // namespace FranchiseAI
