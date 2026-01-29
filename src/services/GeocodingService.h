#ifndef GEOCODING_SERVICE_H
#define GEOCODING_SERVICE_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>
#include "models/GeoLocation.h"

namespace FranchiseAI {
namespace Services {

/**
 * @brief Geocoding provider enumeration
 */
enum class GeocodingProvider {
    NOMINATIM,      // OpenStreetMap Nominatim (free, no API key)
    GOOGLE,         // Google Geocoding API (requires API key)
    MAPBOX,         // Mapbox Geocoding (requires API key)
    HERE,           // HERE Geocoding (requires API key)
    LOCAL           // Local/demo data for testing
};

/**
 * @brief Configuration for geocoding service
 */
struct GeocodingConfig {
    GeocodingProvider provider = GeocodingProvider::NOMINATIM;
    std::string apiKey;
    std::string endpoint;
    int requestTimeoutMs = 10000;
    bool enableCaching = true;
    int cacheDurationMinutes = 1440;  // 24 hours
    std::string userAgent = "FranchiseAI/1.0";

    // Rate limiting
    int maxRequestsPerSecond = 1;  // Nominatim requires max 1 req/sec
};

/**
 * @brief Abstract geocoding service interface
 *
 * Provides address-to-coordinates and coordinates-to-address conversion.
 * Implementations can use different providers (Nominatim, Google, etc.)
 */
class IGeocodingService {
public:
    virtual ~IGeocodingService() = default;

    using GeocodeCallback = std::function<void(Models::GeoLocation, std::string error)>;
    using ReverseGeocodeCallback = std::function<void(Models::GeoLocation, std::string error)>;
    using MultiGeocodeCallback = std::function<void(std::vector<Models::GeoLocation>, std::string error)>;

    /**
     * @brief Convert address string to coordinates (async)
     */
    virtual void geocode(const std::string& address, GeocodeCallback callback) = 0;

    /**
     * @brief Convert coordinates to address (async)
     */
    virtual void reverseGeocode(double latitude, double longitude, ReverseGeocodeCallback callback) = 0;

    /**
     * @brief Synchronous geocode
     */
    virtual Models::GeoLocation geocodeSync(const std::string& address) = 0;

    /**
     * @brief Synchronous reverse geocode
     */
    virtual Models::GeoLocation reverseGeocodeSync(double latitude, double longitude) = 0;

    /**
     * @brief Get provider name
     */
    virtual std::string getProviderName() const = 0;

    /**
     * @brief Check if service is configured and ready
     */
    virtual bool isConfigured() const = 0;
};

/**
 * @brief Nominatim (OpenStreetMap) geocoding implementation
 *
 * Free geocoding service with no API key required.
 * Rate limited to 1 request per second.
 */
class NominatimGeocodingService : public IGeocodingService {
public:
    NominatimGeocodingService();
    explicit NominatimGeocodingService(const GeocodingConfig& config);
    ~NominatimGeocodingService() override = default;

    void setConfig(const GeocodingConfig& config);
    GeocodingConfig getConfig() const { return config_; }

    // IGeocodingService interface
    void geocode(const std::string& address, GeocodeCallback callback) override;
    void reverseGeocode(double latitude, double longitude, ReverseGeocodeCallback callback) override;
    Models::GeoLocation geocodeSync(const std::string& address) override;
    Models::GeoLocation reverseGeocodeSync(double latitude, double longitude) override;
    std::string getProviderName() const override { return "Nominatim (OpenStreetMap)"; }
    bool isConfigured() const override { return true; }  // No API key needed

    // Cache management
    void clearCache();
    int getCacheSize() const;

private:
    GeocodingConfig config_;
    std::unordered_map<std::string, std::pair<Models::GeoLocation, time_t>> cache_;

    // Demo data for common cities (used when API unavailable)
    static const std::unordered_map<std::string, Models::GeoLocation> knownLocations_;

    std::string buildGeocodeUrl(const std::string& address);
    std::string buildReverseGeocodeUrl(double lat, double lon);
    Models::GeoLocation parseNominatimResponse(const std::string& json);
    std::string normalizeAddress(const std::string& address);
};

/**
 * @brief Factory for creating geocoding services
 */
class GeocodingServiceFactory {
public:
    static std::unique_ptr<IGeocodingService> create(GeocodingProvider provider, const GeocodingConfig& config = {});
    static std::unique_ptr<IGeocodingService> createDefault();
};

} // namespace Services
} // namespace FranchiseAI

#endif // GEOCODING_SERVICE_H
