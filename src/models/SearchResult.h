#ifndef SEARCH_RESULT_H
#define SEARCH_RESULT_H

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include "BusinessInfo.h"
#include "DemographicData.h"

namespace FranchiseAI {
namespace Models {

/**
 * @brief Search result type enumeration
 */
enum class SearchResultType {
    BUSINESS,
    DEMOGRAPHIC_AREA,
    COMBINED
};

/**
 * @brief Individual search result item
 *
 * Represents a single result from the AI search,
 * which could be a business or demographic area.
 */
class SearchResultItem {
public:
    SearchResultItem() = default;
    ~SearchResultItem() = default;

    // Result identification
    std::string id;
    SearchResultType resultType = SearchResultType::BUSINESS;

    // Relevance scoring
    double relevanceScore = 0.0;      // 0.0 - 1.0
    double aiConfidenceScore = 0.0;   // 0.0 - 1.0
    int overallScore = 0;             // Combined score 0-100

    // Associated data
    std::shared_ptr<BusinessInfo> business;
    std::shared_ptr<DemographicData> demographic;

    // AI-generated content
    std::string matchReason;
    std::string aiSummary;
    std::vector<std::string> keyHighlights;
    std::vector<std::string> recommendedActions;

    // Source tracking
    std::vector<DataSource> sources;

    // Distance from search location (if applicable)
    double distanceMiles = 0.0;

    // Helper methods
    std::string getTitle() const;
    std::string getSubtitle() const;
    std::string getResultTypeString() const;
    bool hasBusinessData() const { return business != nullptr; }
    bool hasDemographicData() const { return demographic != nullptr; }
};

/**
 * @brief Search query parameters
 */
struct SearchQuery {
    // Location parameters
    std::string location;
    std::string zipCode;
    std::string city;
    std::string state;
    double latitude = 0.0;
    double longitude = 0.0;
    double radiusMiles = 25.0;

    // Search filters
    std::string keywords;
    std::vector<BusinessType> businessTypes;
    int minEmployees = 0;
    int maxEmployees = 0;
    double minCateringScore = 0.0;

    // Data source preferences
    bool includeGoogleMyBusiness = true;
    bool includeBBB = true;
    bool includeDemographics = true;
    bool includeOpenStreetMap = true;

    // Sorting
    enum class SortBy {
        RELEVANCE,
        DISTANCE,
        CATERING_POTENTIAL,
        EMPLOYEE_COUNT,
        RATING
    };
    SortBy sortBy = SortBy::RELEVANCE;
    bool sortAscending = false;

    // Pagination
    int pageSize = 20;
    int pageNumber = 1;
};

/**
 * @brief Complete search results container
 */
class SearchResults {
public:
    SearchResults() = default;
    ~SearchResults() = default;

    // Query that produced these results
    SearchQuery query;

    // Result items
    std::vector<SearchResultItem> items;

    // Result statistics
    int totalResults = 0;
    int googleResults = 0;
    int bbbResults = 0;
    int demographicResults = 0;
    int osmResults = 0;

    // Search metadata
    std::chrono::milliseconds searchDuration{0};
    std::string searchTimestamp;
    bool isComplete = false;
    std::string errorMessage;

    // AI analysis summary
    std::string aiOverallAnalysis;
    std::vector<std::string> topRecommendations;
    std::string marketSummary;

    // Pagination info
    int currentPage = 1;
    int totalPages = 1;
    bool hasMoreResults = false;

    // Helper methods
    void sortResults(SearchQuery::SortBy sortBy, bool ascending = false);
    void filterByScore(int minScore);
    std::vector<SearchResultItem> getTopResults(int count) const;

    // Statistics methods
    double getAverageRelevanceScore() const;
    double getAverageCateringPotential() const;
    std::map<BusinessType, int> getResultsByType() const;
};

} // namespace Models
} // namespace FranchiseAI

#endif // SEARCH_RESULT_H
