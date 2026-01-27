#include "SearchResult.h"
#include <algorithm>
#include <numeric>

namespace FranchiseAI {
namespace Models {

std::string SearchResultItem::getTitle() const {
    if (resultType == SearchResultType::BUSINESS && business) {
        return business->name;
    } else if (resultType == SearchResultType::DEMOGRAPHIC_AREA && demographic) {
        return demographic->city + ", " + demographic->state + " " + demographic->zipCode;
    }
    return "Unknown Result";
}

std::string SearchResultItem::getSubtitle() const {
    if (resultType == SearchResultType::BUSINESS && business) {
        return business->address.getFullAddress();
    } else if (resultType == SearchResultType::DEMOGRAPHIC_AREA && demographic) {
        return "Population: " + std::to_string(demographic->totalPopulation) +
               " | Businesses: " + std::to_string(demographic->totalBusinesses);
    }
    return "";
}

std::string SearchResultItem::getResultTypeString() const {
    switch (resultType) {
        case SearchResultType::BUSINESS: return "Business";
        case SearchResultType::DEMOGRAPHIC_AREA: return "Area Analysis";
        case SearchResultType::COMBINED: return "Combined";
        default: return "Unknown";
    }
}

void SearchResults::sortResults(SearchQuery::SortBy sortBy, bool ascending) {
    std::sort(items.begin(), items.end(),
        [sortBy, ascending](const SearchResultItem& a, const SearchResultItem& b) {
            double valueA = 0, valueB = 0;

            switch (sortBy) {
                case SearchQuery::SortBy::RELEVANCE:
                    valueA = a.relevanceScore;
                    valueB = b.relevanceScore;
                    break;
                case SearchQuery::SortBy::DISTANCE:
                    valueA = a.distanceMiles;
                    valueB = b.distanceMiles;
                    break;
                case SearchQuery::SortBy::CATERING_POTENTIAL:
                    valueA = a.business ? a.business->cateringPotentialScore : 0;
                    valueB = b.business ? b.business->cateringPotentialScore : 0;
                    break;
                case SearchQuery::SortBy::EMPLOYEE_COUNT:
                    valueA = a.business ? a.business->employeeCount : 0;
                    valueB = b.business ? b.business->employeeCount : 0;
                    break;
                case SearchQuery::SortBy::RATING:
                    valueA = a.business ? a.business->googleRating : 0;
                    valueB = b.business ? b.business->googleRating : 0;
                    break;
            }

            return ascending ? valueA < valueB : valueA > valueB;
        });
}

void SearchResults::filterByScore(int minScore) {
    items.erase(
        std::remove_if(items.begin(), items.end(),
            [minScore](const SearchResultItem& item) {
                return item.overallScore < minScore;
            }),
        items.end()
    );
    totalResults = static_cast<int>(items.size());
}

std::vector<SearchResultItem> SearchResults::getTopResults(int count) const {
    std::vector<SearchResultItem> top;
    int n = std::min(count, static_cast<int>(items.size()));
    for (int i = 0; i < n; ++i) {
        top.push_back(items[i]);
    }
    return top;
}

double SearchResults::getAverageRelevanceScore() const {
    if (items.empty()) return 0.0;

    double sum = std::accumulate(items.begin(), items.end(), 0.0,
        [](double acc, const SearchResultItem& item) {
            return acc + item.relevanceScore;
        });

    return sum / items.size();
}

double SearchResults::getAverageCateringPotential() const {
    if (items.empty()) return 0.0;

    int count = 0;
    double sum = 0.0;

    for (const auto& item : items) {
        if (item.business) {
            sum += item.business->cateringPotentialScore;
            ++count;
        }
    }

    return count > 0 ? sum / count : 0.0;
}

std::map<BusinessType, int> SearchResults::getResultsByType() const {
    std::map<BusinessType, int> typeCount;

    for (const auto& item : items) {
        if (item.business) {
            typeCount[item.business->type]++;
        }
    }

    return typeCount;
}

} // namespace Models
} // namespace FranchiseAI
