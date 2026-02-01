#ifndef GOOGLE_PLACES_API_H
#define GOOGLE_PLACES_API_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include "ThreadPool.h"
#include "models/BusinessInfo.h"
#include "models/GeoLocation.h"
#include "models/SearchResult.h"

namespace FranchiseAI {
namespace Services {

/**
 * @brief Configuration for Google Places API
 */
struct GooglePlacesConfig {
    std::string apiKey;
    std::string nearbySearchEndpoint = "https://maps.googleapis.com/maps/api/place/nearbysearch/json";
    std::string detailsEndpoint = "https://maps.googleapis.com/maps/api/place/details/json";
    std::string textSearchEndpoint = "https://maps.googleapis.com/maps/api/place/textsearch/json";

    int requestTimeoutMs = 8000;           // 8 seconds (increased slightly for reliability)
    int connectTimeoutMs = 3000;           // 3 seconds connection timeout
    bool enableCaching = true;
    int cacheDurationMinutes = 60;         // 1 hour for place data
    std::string userAgent = "FranchiseAI/1.0";

    // Thread pool settings
    int threadPoolSize = 4;
    int maxQueuedRequests = 100;

    // Rate limiting (Google allows high QPS for paid tier)
    int maxRequestsPerSecond = 100;

    // Search settings
    int maxResultsPerPage = 20;            // Google Places returns max 20 per page
    int maxPages = 2;                      // Reduced to 2 pages (40 results) for faster response

    /**
     * @brief Check if API key is configured
     */
    bool isConfigured() const {
        return !apiKey.empty();
    }

    /**
     * @brief Get recommended memory for thread pool
     */
    int getRecommendedMemoryMB() const {
        return ThreadPoolConfig::getRecommendedMemoryMB(threadPoolSize);
    }
};

/**
 * @brief Google place types mapped to business types
 */
enum class GooglePlaceType {
    ESTABLISHMENT,
    ACCOUNTING,
    AIRPORT,
    BANK,
    CAR_DEALER,
    CITY_HALL,
    COURTHOUSE,
    DOCTOR,
    EMBASSY,
    FIRE_STATION,
    GYM,
    HOSPITAL,
    INSURANCE_AGENCY,
    LAWYER,
    LOCAL_GOVERNMENT_OFFICE,
    LODGING,
    MOVING_COMPANY,
    PHYSIOTHERAPIST,
    POLICE,
    POST_OFFICE,
    REAL_ESTATE_AGENCY,
    SCHOOL,
    SECONDARY_SCHOOL,
    STADIUM,
    STORAGE,
    UNIVERSITY,
    VETERINARY_CARE,
    CORPORATE_OFFICE,
    TECH_COMPANY,
    CONFERENCE_CENTER
};

/**
 * @brief A place result from Google Places API
 */
struct GooglePlace {
    std::string placeId;
    std::string name;
    std::string formattedAddress;
    std::string vicinity;
    double latitude = 0.0;
    double longitude = 0.0;
    float rating = 0.0f;
    int userRatingsTotal = 0;
    std::vector<std::string> types;
    std::string businessStatus;
    bool permanentlyClosed = false;

    // From Place Details (optional)
    std::string phoneNumber;
    std::string website;
    std::string formattedPhoneNumber;
    std::vector<std::string> weekdayText;  // Opening hours
    int priceLevel = -1;

    Models::BusinessType inferBusinessType() const;
};

/**
 * @brief Statistics for Google Places API usage
 */
struct GooglePlacesStats {
    std::atomic<int> totalRequests{0};
    std::atomic<int> successfulRequests{0};
    std::atomic<int> failedRequests{0};
    std::atomic<int> cacheHits{0};
    std::atomic<int> cacheMisses{0};
    std::atomic<int64_t> totalLatencyMs{0};
    std::atomic<int> totalPlacesFound{0};

    double getAverageLatencyMs() const {
        int successful = successfulRequests.load();
        if (successful == 0) return 0.0;
        return static_cast<double>(totalLatencyMs.load()) / successful;
    }

    void reset() {
        totalRequests = 0;
        successfulRequests = 0;
        failedRequests = 0;
        cacheHits = 0;
        cacheMisses = 0;
        totalLatencyMs = 0;
        totalPlacesFound = 0;
    }
};

/**
 * @brief Google Places API service for finding businesses
 *
 * Provides high-performance business search using Google's Places API
 * with multi-threaded processing and caching.
 */
class GooglePlacesAPI {
public:
    using PlacesCallback = std::function<void(std::vector<GooglePlace>, std::string error)>;
    using BusinessCallback = std::function<void(std::vector<Models::BusinessInfo>, std::string error)>;
    using PlaceDetailsCallback = std::function<void(GooglePlace, std::string error)>;

    GooglePlacesAPI();
    explicit GooglePlacesAPI(const GooglePlacesConfig& config);
    ~GooglePlacesAPI();

    // Configuration
    void setConfig(const GooglePlacesConfig& config);
    GooglePlacesConfig getConfig() const { return config_; }
    bool isConfigured() const { return config_.isConfigured(); }

    /**
     * @brief Search for places near a location (async)
     * @param latitude Center latitude
     * @param longitude Center longitude
     * @param radiusMeters Search radius in meters
     * @param types Place types to search for (empty = all types)
     * @param callback Callback with results
     */
    void searchNearby(
        double latitude,
        double longitude,
        int radiusMeters,
        const std::vector<std::string>& types,
        PlacesCallback callback
    );

    /**
     * @brief Search for places near a location (sync)
     */
    std::vector<GooglePlace> searchNearbySync(
        double latitude,
        double longitude,
        int radiusMeters,
        const std::vector<std::string>& types = {}
    );

    /**
     * @brief Search for businesses suitable for catering (async)
     * @param latitude Center latitude
     * @param longitude Center longitude
     * @param radiusMiles Search radius in miles
     * @param callback Callback with BusinessInfo results
     */
    void searchCateringProspects(
        double latitude,
        double longitude,
        double radiusMiles,
        BusinessCallback callback
    );

    /**
     * @brief Search for businesses suitable for catering (sync)
     */
    std::vector<Models::BusinessInfo> searchCateringProspectsSync(
        double latitude,
        double longitude,
        double radiusMiles
    );

    /**
     * @brief Search for businesses in a search area
     */
    void searchBusinesses(
        const Models::SearchArea& searchArea,
        BusinessCallback callback
    );

    /**
     * @brief Search for businesses in a search area (sync)
     */
    std::vector<Models::BusinessInfo> searchBusinessesSync(
        const Models::SearchArea& searchArea
    );

    /**
     * @brief Text search for places (async)
     * @param query Search query (e.g., "corporate offices in Dallas")
     * @param callback Callback with results
     */
    void textSearch(
        const std::string& query,
        PlacesCallback callback
    );

    /**
     * @brief Text search for places (sync)
     */
    std::vector<GooglePlace> textSearchSync(const std::string& query);

    /**
     * @brief Get detailed information about a place (async)
     * @param placeId Google Place ID
     * @param callback Callback with place details
     */
    void getPlaceDetails(
        const std::string& placeId,
        PlaceDetailsCallback callback
    );

    /**
     * @brief Get detailed information about a place (sync)
     */
    GooglePlace getPlaceDetailsSync(const std::string& placeId);

    /**
     * @brief Convert GooglePlace to BusinessInfo
     */
    static Models::BusinessInfo placeToBusinessInfo(const GooglePlace& place);

    /**
     * @brief Get Google Place types for catering prospects
     */
    static std::vector<std::string> getCateringProspectTypes();

    // Thread pool management
    void setThreadPoolSize(int threadCount);
    int getThreadPoolSize() const;
    int getRecommendedMemoryMB() const;
    const ThreadPoolMetrics& getThreadPoolMetrics() const;

    // Cache management
    void clearCache();
    int getCacheSize() const;

    // Statistics
    const GooglePlacesStats& getStats() const { return stats_; }
    void resetStats() { stats_.reset(); }

private:
    GooglePlacesConfig config_;
    GooglePlacesStats stats_;

    std::unique_ptr<ThreadPool> threadPool_;
    std::mutex threadPoolMutex_;

    // Cache: cache key -> (places, timestamp)
    std::unordered_map<std::string, std::pair<std::vector<GooglePlace>, time_t>> searchCache_;
    std::unordered_map<std::string, std::pair<GooglePlace, time_t>> detailsCache_;
    mutable std::mutex cacheMutex_;

    // Internal methods
    std::string buildNearbySearchUrl(double lat, double lon, int radiusMeters,
                                     const std::vector<std::string>& types,
                                     const std::string& pageToken = "");
    std::string buildTextSearchUrl(const std::string& query, const std::string& pageToken = "");
    std::string buildDetailsUrl(const std::string& placeId);
    std::string executeHttpRequest(const std::string& url);
    std::vector<GooglePlace> parseNearbySearchResponse(const std::string& json, std::string& nextPageToken);
    GooglePlace parseDetailsResponse(const std::string& json);

    void initializeThreadPool();
    std::string buildCacheKey(double lat, double lon, int radius, const std::vector<std::string>& types);
};

} // namespace Services
} // namespace FranchiseAI

#endif // GOOGLE_PLACES_API_H
