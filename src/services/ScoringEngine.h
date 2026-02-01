#ifndef SCORING_ENGINE_H
#define SCORING_ENGINE_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>
#include "models/BusinessInfo.h"

namespace FranchiseAI {
namespace Services {

/**
 * @brief Individual score adjustment applied to a prospect
 */
struct ScoreAdjustment {
    std::string ruleId;
    std::string description;
    int points;
    bool applied;
};

/**
 * @brief Configurable scoring rule
 */
struct ScoreRule {
    std::string id;
    std::string name;           // Display name for UI
    std::string description;    // Explanation of what this rule does
    int defaultPoints;          // Default point adjustment
    int currentPoints;          // Current configured point adjustment
    int minPoints;              // Minimum allowed adjustment
    int maxPoints;              // Maximum allowed adjustment
    bool enabled;               // Whether this rule is active
    bool isPenalty;             // True if negative adjustment, false if bonus

    // Condition function - returns true if rule applies to this business
    std::function<bool(const Models::BusinessInfo&)> condition;

    ScoreRule() : defaultPoints(0), currentPoints(0), minPoints(-50), maxPoints(50),
                  enabled(true), isPenalty(false) {}
};

/**
 * @brief Score calculation result with breakdown
 */
struct ScoreResult {
    int baseScore;
    int finalScore;
    std::vector<ScoreAdjustment> adjustments;

    int getTotalAdjustment() const {
        int total = 0;
        for (const auto& adj : adjustments) {
            if (adj.applied) total += adj.points;
        }
        return total;
    }
};

/**
 * @brief Configurable scoring engine for prospect evaluation
 *
 * Allows franchisees to customize how prospects are scored based on
 * various factors like missing data, verified status, ratings, etc.
 */
class ScoringEngine {
public:
    ScoringEngine();
    ~ScoringEngine() = default;

    /**
     * @brief Initialize with default scoring rules
     */
    void initializeDefaultRules();

    /**
     * @brief Add a custom scoring rule
     */
    void addRule(const ScoreRule& rule);

    /**
     * @brief Remove a rule by ID
     */
    void removeRule(const std::string& id);

    /**
     * @brief Enable or disable a rule
     */
    void setRuleEnabled(const std::string& id, bool enabled);

    /**
     * @brief Set the point value for a rule
     */
    void setRulePoints(const std::string& id, int points);

    /**
     * @brief Reset a rule to its default point value
     */
    void resetRuleToDefault(const std::string& id);

    /**
     * @brief Reset all rules to defaults
     */
    void resetAllToDefaults();

    /**
     * @brief Get a rule by ID
     */
    const ScoreRule* getRule(const std::string& id) const;

    /**
     * @brief Get all rules
     */
    const std::vector<ScoreRule>& getRules() const { return rules_; }

    /**
     * @brief Get only penalty rules
     */
    std::vector<const ScoreRule*> getPenaltyRules() const;

    /**
     * @brief Get only bonus rules
     */
    std::vector<const ScoreRule*> getBonusRules() const;

    /**
     * @brief Calculate score for a business with full breakdown
     */
    ScoreResult calculateScore(const Models::BusinessInfo& business, int baseScore = 50) const;

    /**
     * @brief Calculate final score only (no breakdown)
     */
    int calculateFinalScore(const Models::BusinessInfo& business, int baseScore = 50) const;

    /**
     * @brief Check if engine has any enabled rules
     */
    bool hasEnabledRules() const;

    /**
     * @brief Get count of enabled rules
     */
    int getEnabledRuleCount() const;

    // Serialization for settings persistence
    std::string serializeSettings() const;
    void deserializeSettings(const std::string& json);

private:
    std::vector<ScoreRule> rules_;
    std::unordered_map<std::string, size_t> ruleIndex_;

    void updateIndex();
};

} // namespace Services
} // namespace FranchiseAI

#endif // SCORING_ENGINE_H
