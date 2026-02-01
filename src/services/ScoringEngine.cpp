#include "ScoringEngine.h"
#include <algorithm>
#include <sstream>

namespace FranchiseAI {
namespace Services {

ScoringEngine::ScoringEngine() {
    initializeDefaultRules();
}

void ScoringEngine::initializeDefaultRules() {
    rules_.clear();
    ruleIndex_.clear();

    // === Penalty Rules (negative adjustments) ===

    // Missing address penalty
    ScoreRule noAddress;
    noAddress.id = "no_address";
    noAddress.name = "Missing Address";
    noAddress.description = "Prospects without addresses are harder to contact and verify";
    noAddress.defaultPoints = -10;
    noAddress.currentPoints = -10;
    noAddress.minPoints = -25;
    noAddress.maxPoints = 0;
    noAddress.enabled = true;
    noAddress.isPenalty = true;
    noAddress.condition = [](const Models::BusinessInfo& biz) {
        return biz.address.getFullAddress().empty() ||
               (biz.address.street1.empty() && biz.address.city.empty());
    };
    rules_.push_back(noAddress);

    // Missing employee count penalty
    ScoreRule noEmployees;
    noEmployees.id = "no_employees";
    noEmployees.name = "Missing Employee Count";
    noEmployees.description = "Unknown employee count makes catering potential harder to estimate";
    noEmployees.defaultPoints = -3;
    noEmployees.currentPoints = -3;
    noEmployees.minPoints = -15;
    noEmployees.maxPoints = 0;
    noEmployees.enabled = true;
    noEmployees.isPenalty = true;
    noEmployees.condition = [](const Models::BusinessInfo& biz) {
        return biz.employeeCount <= 0 && biz.estimatedEmployeesOnSite <= 0;
    };
    rules_.push_back(noEmployees);

    // Missing contact info penalty
    ScoreRule noContact;
    noContact.id = "no_contact";
    noContact.name = "Missing Contact Info";
    noContact.description = "No phone or email makes outreach difficult";
    noContact.defaultPoints = -5;
    noContact.currentPoints = -5;
    noContact.minPoints = -20;
    noContact.maxPoints = 0;
    noContact.enabled = true;
    noContact.isPenalty = true;
    noContact.condition = [](const Models::BusinessInfo& biz) {
        return biz.contact.primaryPhone.empty() && biz.contact.email.empty();
    };
    rules_.push_back(noContact);

    // === Bonus Rules (positive adjustments) ===

    // Verified business bonus
    ScoreRule verified;
    verified.id = "verified";
    verified.name = "Verified Business";
    verified.description = "Business has been verified through data sources";
    verified.defaultPoints = 5;
    verified.currentPoints = 5;
    verified.minPoints = 0;
    verified.maxPoints = 15;
    verified.enabled = true;
    verified.isPenalty = false;
    verified.condition = [](const Models::BusinessInfo& biz) {
        return biz.isVerified;
    };
    rules_.push_back(verified);

    // BBB accredited bonus
    ScoreRule bbbAccredited;
    bbbAccredited.id = "bbb_accredited";
    bbbAccredited.name = "BBB Accredited";
    bbbAccredited.description = "Business is accredited by the Better Business Bureau";
    bbbAccredited.defaultPoints = 10;
    bbbAccredited.currentPoints = 10;
    bbbAccredited.minPoints = 0;
    bbbAccredited.maxPoints = 20;
    bbbAccredited.enabled = true;
    bbbAccredited.isPenalty = false;
    bbbAccredited.condition = [](const Models::BusinessInfo& biz) {
        return biz.bbbAccredited;
    };
    rules_.push_back(bbbAccredited);

    // High Google rating bonus
    ScoreRule highRating;
    highRating.id = "high_rating";
    highRating.name = "High Google Rating";
    highRating.description = "Business has 4.5+ star Google rating";
    highRating.defaultPoints = 5;
    highRating.currentPoints = 5;
    highRating.minPoints = 0;
    highRating.maxPoints = 15;
    highRating.enabled = true;
    highRating.isPenalty = false;
    highRating.condition = [](const Models::BusinessInfo& biz) {
        return biz.googleRating >= 4.5;
    };
    rules_.push_back(highRating);

    // Conference room bonus
    ScoreRule conferenceRoom;
    conferenceRoom.id = "conference_room";
    conferenceRoom.name = "Has Conference Room";
    conferenceRoom.description = "Business has conference facilities - good for catering";
    conferenceRoom.defaultPoints = 5;
    conferenceRoom.currentPoints = 5;
    conferenceRoom.minPoints = 0;
    conferenceRoom.maxPoints = 15;
    conferenceRoom.enabled = true;
    conferenceRoom.isPenalty = false;
    conferenceRoom.condition = [](const Models::BusinessInfo& biz) {
        return biz.hasConferenceRoom;
    };
    rules_.push_back(conferenceRoom);

    // Event space bonus
    ScoreRule eventSpace;
    eventSpace.id = "event_space";
    eventSpace.name = "Has Event Space";
    eventSpace.description = "Business has dedicated event space";
    eventSpace.defaultPoints = 7;
    eventSpace.currentPoints = 7;
    eventSpace.minPoints = 0;
    eventSpace.maxPoints = 20;
    eventSpace.enabled = true;
    eventSpace.isPenalty = false;
    eventSpace.condition = [](const Models::BusinessInfo& biz) {
        return biz.hasEventSpace;
    };
    rules_.push_back(eventSpace);

    // Large company bonus
    ScoreRule largeCompany;
    largeCompany.id = "large_company";
    largeCompany.name = "Large Company (100+ employees)";
    largeCompany.description = "Larger companies have more catering opportunities";
    largeCompany.defaultPoints = 8;
    largeCompany.currentPoints = 8;
    largeCompany.minPoints = 0;
    largeCompany.maxPoints = 20;
    largeCompany.enabled = true;
    largeCompany.isPenalty = false;
    largeCompany.condition = [](const Models::BusinessInfo& biz) {
        return biz.employeeCount >= 100 || biz.estimatedEmployeesOnSite >= 100;
    };
    rules_.push_back(largeCompany);

    updateIndex();
}

void ScoringEngine::addRule(const ScoreRule& rule) {
    // Check if rule already exists
    auto it = ruleIndex_.find(rule.id);
    if (it != ruleIndex_.end()) {
        rules_[it->second] = rule;
    } else {
        rules_.push_back(rule);
        updateIndex();
    }
}

void ScoringEngine::removeRule(const std::string& id) {
    auto it = ruleIndex_.find(id);
    if (it != ruleIndex_.end()) {
        rules_.erase(rules_.begin() + it->second);
        updateIndex();
    }
}

void ScoringEngine::setRuleEnabled(const std::string& id, bool enabled) {
    auto it = ruleIndex_.find(id);
    if (it != ruleIndex_.end()) {
        rules_[it->second].enabled = enabled;
    }
}

void ScoringEngine::setRulePoints(const std::string& id, int points) {
    auto it = ruleIndex_.find(id);
    if (it != ruleIndex_.end()) {
        auto& rule = rules_[it->second];
        rule.currentPoints = std::max(rule.minPoints, std::min(points, rule.maxPoints));
    }
}

void ScoringEngine::resetRuleToDefault(const std::string& id) {
    auto it = ruleIndex_.find(id);
    if (it != ruleIndex_.end()) {
        rules_[it->second].currentPoints = rules_[it->second].defaultPoints;
    }
}

void ScoringEngine::resetAllToDefaults() {
    for (auto& rule : rules_) {
        rule.currentPoints = rule.defaultPoints;
        rule.enabled = true;
    }
}

const ScoreRule* ScoringEngine::getRule(const std::string& id) const {
    auto it = ruleIndex_.find(id);
    if (it != ruleIndex_.end()) {
        return &rules_[it->second];
    }
    return nullptr;
}

std::vector<const ScoreRule*> ScoringEngine::getPenaltyRules() const {
    std::vector<const ScoreRule*> penalties;
    for (const auto& rule : rules_) {
        if (rule.isPenalty) {
            penalties.push_back(&rule);
        }
    }
    return penalties;
}

std::vector<const ScoreRule*> ScoringEngine::getBonusRules() const {
    std::vector<const ScoreRule*> bonuses;
    for (const auto& rule : rules_) {
        if (!rule.isPenalty) {
            bonuses.push_back(&rule);
        }
    }
    return bonuses;
}

ScoreResult ScoringEngine::calculateScore(const Models::BusinessInfo& business, int baseScore) const {
    ScoreResult result;
    result.baseScore = baseScore;
    result.finalScore = baseScore;

    for (const auto& rule : rules_) {
        ScoreAdjustment adj;
        adj.ruleId = rule.id;
        adj.description = rule.name;
        adj.points = rule.currentPoints;
        adj.applied = false;

        if (rule.enabled && rule.condition && rule.condition(business)) {
            adj.applied = true;
            result.finalScore += rule.currentPoints;
        }

        result.adjustments.push_back(adj);
    }

    // Clamp final score to valid range
    result.finalScore = std::max(0, std::min(result.finalScore, 100));

    return result;
}

int ScoringEngine::calculateFinalScore(const Models::BusinessInfo& business, int baseScore) const {
    int score = baseScore;

    for (const auto& rule : rules_) {
        if (rule.enabled && rule.condition && rule.condition(business)) {
            score += rule.currentPoints;
        }
    }

    return std::max(0, std::min(score, 100));
}

bool ScoringEngine::hasEnabledRules() const {
    for (const auto& rule : rules_) {
        if (rule.enabled) return true;
    }
    return false;
}

int ScoringEngine::getEnabledRuleCount() const {
    int count = 0;
    for (const auto& rule : rules_) {
        if (rule.enabled) ++count;
    }
    return count;
}

void ScoringEngine::updateIndex() {
    ruleIndex_.clear();
    for (size_t i = 0; i < rules_.size(); ++i) {
        ruleIndex_[rules_[i].id] = i;
    }
}

std::string ScoringEngine::serializeSettings() const {
    std::ostringstream json;
    json << "{\"rules\":[";

    bool first = true;
    for (const auto& rule : rules_) {
        if (!first) json << ",";
        first = false;

        json << "{\"id\":\"" << rule.id << "\""
             << ",\"enabled\":" << (rule.enabled ? "true" : "false")
             << ",\"points\":" << rule.currentPoints
             << "}";
    }

    json << "]}";
    return json.str();
}

void ScoringEngine::deserializeSettings(const std::string& json) {
    // Simple JSON parsing for rule settings
    // Format: {"rules":[{"id":"no_address","enabled":true,"points":-10},...]

    size_t pos = 0;
    while ((pos = json.find("\"id\":\"", pos)) != std::string::npos) {
        pos += 6;
        size_t endPos = json.find("\"", pos);
        if (endPos == std::string::npos) break;

        std::string id = json.substr(pos, endPos - pos);
        pos = endPos;

        // Find enabled
        size_t enabledPos = json.find("\"enabled\":", pos);
        if (enabledPos != std::string::npos && enabledPos < json.find("\"id\":", pos + 1)) {
            bool enabled = json.find("true", enabledPos) < json.find("false", enabledPos);
            setRuleEnabled(id, enabled);
        }

        // Find points
        size_t pointsPos = json.find("\"points\":", pos);
        if (pointsPos != std::string::npos && pointsPos < json.find("\"id\":", pos + 1)) {
            pointsPos += 9;
            int points = std::stoi(json.substr(pointsPos));
            setRulePoints(id, points);
        }
    }
}

} // namespace Services
} // namespace FranchiseAI
