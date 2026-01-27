#ifndef DEMOGRAPHICS_API_H
#define DEMOGRAPHICS_API_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "models/DemographicData.h"
#include "models/SearchResult.h"

namespace FranchiseAI {
namespace Services {

/**
 * @brief Configuration for Demographics API
 */
struct DemographicsAPIConfig {
    std::string apiKey;
    std::string censusApiEndpoint = "https://api.census.gov/data";
    std::string economicDataEndpoint = "https://api.bls.gov";
    int requestTimeoutMs = 30000;
    bool enableCaching = true;
    int cacheDurationMinutes = 1440;  // 24 hours
};

/**
 * @brief Demographics API service
 *
 * Provides access to demographic and economic data for geographic areas,
 * useful for identifying high-potential catering markets.
 */
class DemographicsAPI {
public:
    using DemographicCallback = std::function<void(Models::DemographicData, std::string)>;
    using MultiDemographicCallback = std::function<void(std::vector<Models::DemographicData>, std::string)>;

    DemographicsAPI();
    explicit DemographicsAPI(const DemographicsAPIConfig& config);
    ~DemographicsAPI();

    // Configuration
    void setConfig(const DemographicsAPIConfig& config);
    DemographicsAPIConfig getConfig() const { return config_; }
    void setApiKey(const std::string& apiKey);
    bool isConfigured() const { return !config_.apiKey.empty(); }

    /**
     * @brief Get demographic data for a ZIP code
     * @param zipCode ZIP code to query
     * @param callback Callback with demographic data
     */
    void getByZipCode(const std::string& zipCode, DemographicCallback callback);

    /**
     * @brief Get demographic data for a city
     * @param city City name
     * @param state State code
     * @param callback Callback with demographic data
     */
    void getByCity(const std::string& city, const std::string& state, DemographicCallback callback);

    /**
     * @brief Get demographic data for multiple ZIP codes
     * @param zipCodes List of ZIP codes
     * @param callback Callback with demographic data list
     */
    void getMultipleZipCodes(
        const std::vector<std::string>& zipCodes,
        MultiDemographicCallback callback
    );

    /**
     * @brief Get ZIP codes within radius of a location
     * @param centerZip Center ZIP code
     * @param radiusMiles Radius in miles
     * @param callback Callback with demographic data list
     */
    void getZipCodesInRadius(
        const std::string& centerZip,
        double radiusMiles,
        MultiDemographicCallback callback
    );

    /**
     * @brief Find high-potential areas for catering
     * @param centerLocation Center ZIP code or city
     * @param radiusMiles Search radius
     * @param minScore Minimum market potential score
     * @param callback Callback with results
     */
    void findHighPotentialAreas(
        const std::string& centerLocation,
        double radiusMiles,
        int minScore,
        MultiDemographicCallback callback
    );

    /**
     * @brief Get business density data for an area
     * @param zipCode ZIP code
     * @param callback Callback with demographic data including business stats
     */
    void getBusinessDensity(const std::string& zipCode, DemographicCallback callback);

    /**
     * @brief Get employment statistics by sector for an area
     * @param zipCode ZIP code
     * @param callback Callback with demographic data
     */
    void getEmploymentBySector(const std::string& zipCode, DemographicCallback callback);

    // Synchronous versions
    Models::DemographicData getByZipCodeSync(const std::string& zipCode);
    std::vector<Models::DemographicData> getZipCodesInRadiusSync(
        const std::string& centerZip,
        double radiusMiles
    );

    // Cache management
    void clearCache();
    int getCacheSize() const;

    // Statistics
    int getTotalApiCalls() const { return totalApiCalls_; }

private:
    DemographicsAPIConfig config_;
    int totalApiCalls_ = 0;

    // Helper methods
    Models::DemographicData generateDemoData(const std::string& zipCode);
    std::vector<Models::DemographicData> generateDemoAreaData(
        const std::string& centerZip,
        double radiusMiles
    );
    void populateEmploymentData(Models::DemographicData& data);
};

} // namespace Services
} // namespace FranchiseAI

#endif // DEMOGRAPHICS_API_H
