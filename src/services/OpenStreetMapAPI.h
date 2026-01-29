#ifndef OPENSTREETMAP_API_H
#define OPENSTREETMAP_API_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <map>
#include <unordered_map>
#include "models/BusinessInfo.h"
#include "models/DemographicData.h"

namespace FranchiseAI {
namespace Services {

/**
 * @brief Configuration for OpenStreetMap API (Overpass)
 */
struct OSMAPIConfig {
    std::string overpassEndpoint = "https://overpass-api.de/api/interpreter";
    std::string nominatimEndpoint = "https://nominatim.openstreetmap.org";
    int requestTimeoutMs = 30000;
    bool enableCaching = true;
    int cacheDurationMinutes = 1440;  // 24 hours - OSM data is relatively static
    int maxResultsPerQuery = 100;
    std::string userAgent = "FranchiseAI/1.0";  // Required by OSM usage policy
};

/**
 * @brief OSM POI (Point of Interest) data
 */
struct OSMPoi {
    int64_t osmId = 0;
    std::string osmType;  // "node", "way", "relation"
    std::string name;
    double latitude = 0.0;
    double longitude = 0.0;
    std::map<std::string, std::string> tags;

    // Parsed from tags
    std::string amenity;
    std::string building;
    std::string office;
    std::string shop;
    std::string tourism;
    std::string healthcare;

    // Address components (if available)
    std::string street;
    std::string houseNumber;
    std::string city;
    std::string postcode;
    std::string state;
    std::string country;

    // Contact info from tags
    std::string phone;
    std::string website;
    std::string email;
    std::string openingHours;
};

/**
 * @brief Area statistics from OSM data
 */
struct OSMAreaStats {
    std::string areaName;
    double centerLat = 0.0;
    double centerLon = 0.0;
    double radiusKm = 0.0;

    // Business counts by category
    int totalPois = 0;
    int offices = 0;
    int restaurants = 0;
    int cafes = 0;
    int hotels = 0;
    int conferenceVenues = 0;
    int hospitals = 0;
    int schools = 0;
    int universities = 0;
    int industrialBuildings = 0;
    int warehouses = 0;
    int shops = 0;
    int banks = 0;
    int governmentBuildings = 0;

    // Infrastructure
    int parkingLots = 0;
    int busStops = 0;
    int railwayStations = 0;

    // Calculated metrics
    double businessDensityPerSqKm = 0.0;
    int marketPotentialScore = 0;  // 0-100

    void calculateMetrics();
    std::string getMarketQualityDescription() const;
};

/**
 * @brief OpenStreetMap API service using Overpass API
 *
 * Provides geolocation search capabilities using free, open-source
 * OpenStreetMap data via the Overpass API.
 */
class OpenStreetMapAPI {
public:
    using POICallback = std::function<void(std::vector<OSMPoi>, std::string)>;
    using BusinessCallback = std::function<void(std::vector<Models::BusinessInfo>, std::string)>;
    using AreaStatsCallback = std::function<void(OSMAreaStats, std::string)>;
    using GeocodeCallback = std::function<void(double lat, double lon, std::string error)>;

    OpenStreetMapAPI();
    explicit OpenStreetMapAPI(const OSMAPIConfig& config);
    ~OpenStreetMapAPI();

    // Configuration
    void setConfig(const OSMAPIConfig& config);
    OSMAPIConfig getConfig() const { return config_; }

    /**
     * @brief Search for POIs near a location
     * @param latitude Center latitude
     * @param longitude Center longitude
     * @param radiusMeters Search radius in meters
     * @param callback Callback with POI results
     */
    void searchNearby(
        double latitude,
        double longitude,
        int radiusMeters,
        POICallback callback
    );

    /**
     * @brief Search for businesses near a location (converted to BusinessInfo)
     * @param latitude Center latitude
     * @param longitude Center longitude
     * @param radiusMiles Search radius in miles
     * @param callback Callback with business results
     */
    void searchBusinesses(
        double latitude,
        double longitude,
        double radiusMiles,
        BusinessCallback callback
    );

    /**
     * @brief Search for specific business types
     * @param latitude Center latitude
     * @param longitude Center longitude
     * @param radiusMiles Search radius in miles
     * @param types Business types to search for
     * @param callback Callback with results
     */
    void searchByBusinessType(
        double latitude,
        double longitude,
        double radiusMiles,
        const std::vector<Models::BusinessType>& types,
        BusinessCallback callback
    );

    /**
     * @brief Search for catering prospects (offices, conference centers, etc.)
     * @param latitude Center latitude
     * @param longitude Center longitude
     * @param radiusMiles Search radius
     * @param callback Callback with results
     */
    void searchCateringProspects(
        double latitude,
        double longitude,
        double radiusMiles,
        BusinessCallback callback
    );

    /**
     * @brief Get area statistics for demographic analysis
     * @param latitude Center latitude
     * @param longitude Center longitude
     * @param radiusKm Search radius in kilometers
     * @param callback Callback with area statistics
     */
    void getAreaStatistics(
        double latitude,
        double longitude,
        double radiusKm,
        AreaStatsCallback callback
    );

    /**
     * @brief Geocode an address to coordinates
     * @param address Address string
     * @param callback Callback with lat/lon
     */
    void geocodeAddress(
        const std::string& address,
        GeocodeCallback callback
    );

    /**
     * @brief Reverse geocode coordinates to address
     * @param latitude Latitude
     * @param longitude Longitude
     * @param callback Callback with address info
     */
    void reverseGeocode(
        double latitude,
        double longitude,
        std::function<void(OSMPoi, std::string)> callback
    );

    // Synchronous versions
    std::vector<Models::BusinessInfo> searchBusinessesSync(
        double latitude,
        double longitude,
        double radiusMiles
    );

    OSMAreaStats getAreaStatisticsSync(
        double latitude,
        double longitude,
        double radiusKm
    );

    // Cache management
    void clearCache();
    int getCacheSize() const;

    // Statistics
    int getTotalApiCalls() const { return totalApiCalls_; }
    void resetStatistics();

    // Utility: Convert OSM POI to BusinessInfo
    static Models::BusinessInfo poiToBusinessInfo(const OSMPoi& poi);

    // Utility: Map OSM tags to BusinessType
    static Models::BusinessType inferBusinessType(const OSMPoi& poi);

private:
    OSMAPIConfig config_;
    int totalApiCalls_ = 0;

    // Simple in-memory cache
    std::unordered_map<std::string, std::pair<std::vector<OSMPoi>, time_t>> poiCache_;

    // Overpass query builders
    std::string buildOverpassQuery(
        double lat,
        double lon,
        int radiusMeters,
        const std::vector<std::string>& osmFilters
    );

    std::string buildCateringProspectQuery(
        double lat,
        double lon,
        int radiusMeters
    );

    std::string buildAreaStatsQuery(
        double lat,
        double lon,
        int radiusMeters
    );

    // HTTP helpers
    std::string executeOverpassQuery(const std::string& query);
    std::string executeNominatimQuery(const std::string& endpoint);

    // JSON parsing
    std::vector<OSMPoi> parseOverpassResponse(const std::string& json);
    OSMPoi parseNominatimResponse(const std::string& json);

    // OSM tag to business type mapping
    static const std::map<std::string, Models::BusinessType> osmTagMapping_;

    // Get OSM filters for business types
    std::vector<std::string> getOSMFiltersForBusinessTypes(
        const std::vector<Models::BusinessType>& types
    );

    // Demo data generation (for testing without API)
    std::vector<OSMPoi> generateDemoPOIs(
        double lat,
        double lon,
        int radiusMeters
    );

    OSMAreaStats generateDemoAreaStats(
        double lat,
        double lon,
        double radiusKm
    );
};

} // namespace Services
} // namespace FranchiseAI

#endif // OPENSTREETMAP_API_H
