#ifndef GOOGLE_MY_BUSINESS_API_H
#define GOOGLE_MY_BUSINESS_API_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "models/BusinessInfo.h"
#include "models/SearchResult.h"

namespace FranchiseAI {
namespace Services {

/**
 * @brief Configuration for Google My Business API
 */
struct GoogleAPIConfig {
    std::string apiKey;
    std::string placeApiEndpoint = "https://maps.googleapis.com/maps/api/place";
    int maxResultsPerQuery = 60;
    int requestTimeoutMs = 30000;
    bool enableCaching = true;
    int cacheDurationMinutes = 60;
};

/**
 * @brief Google My Business API service
 *
 * Provides methods to search for businesses using Google Places API
 * and retrieve detailed business information.
 */
class GoogleMyBusinessAPI {
public:
    using SearchCallback = std::function<void(std::vector<Models::BusinessInfo>, std::string)>;
    using DetailsCallback = std::function<void(Models::BusinessInfo, std::string)>;

    GoogleMyBusinessAPI();
    explicit GoogleMyBusinessAPI(const GoogleAPIConfig& config);
    ~GoogleMyBusinessAPI();

    // Configuration
    void setConfig(const GoogleAPIConfig& config);
    GoogleAPIConfig getConfig() const { return config_; }
    void setApiKey(const std::string& apiKey);
    bool isConfigured() const { return !config_.apiKey.empty(); }

    /**
     * @brief Search for businesses near a location
     * @param query Search parameters
     * @param callback Callback with results
     */
    void searchBusinesses(const Models::SearchQuery& query, SearchCallback callback);

    /**
     * @brief Search for businesses by keyword and location
     * @param keyword Search keyword (e.g., "corporate office", "warehouse")
     * @param latitude Latitude coordinate
     * @param longitude Longitude coordinate
     * @param radiusMeters Search radius in meters
     * @param callback Callback with results
     */
    void searchNearby(
        const std::string& keyword,
        double latitude,
        double longitude,
        int radiusMeters,
        SearchCallback callback
    );

    /**
     * @brief Get detailed information for a specific place
     * @param placeId Google Place ID
     * @param callback Callback with business details
     */
    void getPlaceDetails(const std::string& placeId, DetailsCallback callback);

    /**
     * @brief Search for businesses with catering potential
     * @param location Location string or coordinates
     * @param radiusMiles Search radius in miles
     * @param callback Callback with results
     */
    void searchCateringProspects(
        const std::string& location,
        double radiusMiles,
        SearchCallback callback
    );

    /**
     * @brief Get autocomplete suggestions for business search
     * @param input Partial search input
     * @param callback Callback with suggestions
     */
    void getAutocomplete(
        const std::string& input,
        std::function<void(std::vector<std::string>)> callback
    );

    // Synchronous versions for simpler use cases
    std::vector<Models::BusinessInfo> searchBusinessesSync(const Models::SearchQuery& query);
    Models::BusinessInfo getPlaceDetailsSync(const std::string& placeId);

    // Cache management
    void clearCache();
    int getCacheSize() const;

    // Statistics
    int getTotalApiCalls() const { return totalApiCalls_; }
    void resetStatistics();

private:
    GoogleAPIConfig config_;
    int totalApiCalls_ = 0;

    // Helper methods
    std::string buildSearchUrl(const std::string& keyword, double lat, double lng, int radius);
    std::string buildDetailsUrl(const std::string& placeId);
    Models::BusinessInfo parseBusinessFromJson(const std::string& json);
    std::vector<Models::BusinessInfo> parseSearchResults(const std::string& json);
    Models::BusinessType inferBusinessType(const std::vector<std::string>& types);

    // Demo data generation (for testing without API key)
    std::vector<Models::BusinessInfo> generateDemoResults(const Models::SearchQuery& query);
};

} // namespace Services
} // namespace FranchiseAI

#endif // GOOGLE_MY_BUSINESS_API_H
