#ifndef BBB_API_H
#define BBB_API_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "models/BusinessInfo.h"
#include "models/SearchResult.h"

namespace FranchiseAI {
namespace Services {

/**
 * @brief Configuration for Better Business Bureau API
 */
struct BBBAPIConfig {
    std::string apiKey;
    std::string apiEndpoint = "https://api.bbb.org/api";
    int maxResultsPerQuery = 50;
    int requestTimeoutMs = 30000;
    bool enableCaching = true;
};

/**
 * @brief BBB complaint information
 */
struct BBBComplaint {
    std::string id;
    std::string type;
    std::string status;
    std::string dateOpened;
    std::string dateClosed;
    std::string resolution;
};

/**
 * @brief Better Business Bureau API service
 *
 * Provides methods to search BBB listings and retrieve
 * business ratings, accreditation status, and complaint history.
 */
class BBBAPI {
public:
    using SearchCallback = std::function<void(std::vector<Models::BusinessInfo>, std::string)>;
    using DetailsCallback = std::function<void(Models::BusinessInfo, std::string)>;
    using ComplaintsCallback = std::function<void(std::vector<BBBComplaint>, std::string)>;

    BBBAPI();
    explicit BBBAPI(const BBBAPIConfig& config);
    ~BBBAPI();

    // Configuration
    void setConfig(const BBBAPIConfig& config);
    BBBAPIConfig getConfig() const { return config_; }
    void setApiKey(const std::string& apiKey);
    bool isConfigured() const { return !config_.apiKey.empty(); }

    /**
     * @brief Search BBB listings by location and category
     * @param query Search parameters
     * @param callback Callback with results
     */
    void searchBusinesses(const Models::SearchQuery& query, SearchCallback callback);

    /**
     * @brief Search BBB listings by name and location
     * @param businessName Business name to search
     * @param city City name
     * @param state State code
     * @param callback Callback with results
     */
    void searchByName(
        const std::string& businessName,
        const std::string& city,
        const std::string& state,
        SearchCallback callback
    );

    /**
     * @brief Search BBB accredited businesses in an area
     * @param zipCode ZIP code
     * @param radiusMiles Search radius
     * @param callback Callback with results
     */
    void searchAccreditedBusinesses(
        const std::string& zipCode,
        double radiusMiles,
        SearchCallback callback
    );

    /**
     * @brief Get detailed BBB profile for a business
     * @param bbbId BBB Business ID
     * @param callback Callback with business details
     */
    void getBusinessProfile(const std::string& bbbId, DetailsCallback callback);

    /**
     * @brief Get complaint history for a business
     * @param bbbId BBB Business ID
     * @param callback Callback with complaints
     */
    void getComplaintHistory(const std::string& bbbId, ComplaintsCallback callback);

    /**
     * @brief Check if a business is BBB accredited
     * @param businessName Business name
     * @param city City
     * @param state State
     * @return True if accredited
     */
    bool checkAccreditation(
        const std::string& businessName,
        const std::string& city,
        const std::string& state
    );

    // Synchronous versions
    std::vector<Models::BusinessInfo> searchBusinessesSync(const Models::SearchQuery& query);
    Models::BusinessInfo getBusinessProfileSync(const std::string& bbbId);

    // Cache management
    void clearCache();

    // Statistics
    int getTotalApiCalls() const { return totalApiCalls_; }

private:
    BBBAPIConfig config_;
    int totalApiCalls_ = 0;

    // Helper methods
    Models::BBBRating parseRating(const std::string& ratingStr);
    std::vector<Models::BusinessInfo> generateDemoResults(const Models::SearchQuery& query);
    std::vector<BBBComplaint> generateDemoComplaints(const std::string& bbbId);
};

} // namespace Services
} // namespace FranchiseAI

#endif // BBB_API_H
