#include "GeocodingService.h"
#include <curl/curl.h>
#include <algorithm>
#include <cctype>
#include <ctime>

namespace FranchiseAI {
namespace Services {

// Known locations for demo/fallback (when API is unavailable)
const std::unordered_map<std::string, Models::GeoLocation> NominatimGeocodingService::knownLocations_ = {
    {"new york", Models::GeoLocation(40.7128, -74.0060, "New York", "NY")},
    {"new york ny", Models::GeoLocation(40.7128, -74.0060, "New York", "NY")},
    {"new york city", Models::GeoLocation(40.7128, -74.0060, "New York", "NY")},
    {"nyc", Models::GeoLocation(40.7128, -74.0060, "New York", "NY")},
    {"los angeles", Models::GeoLocation(34.0522, -118.2437, "Los Angeles", "CA")},
    {"los angeles ca", Models::GeoLocation(34.0522, -118.2437, "Los Angeles", "CA")},
    {"la", Models::GeoLocation(34.0522, -118.2437, "Los Angeles", "CA")},
    {"chicago", Models::GeoLocation(41.8781, -87.6298, "Chicago", "IL")},
    {"chicago il", Models::GeoLocation(41.8781, -87.6298, "Chicago", "IL")},
    {"houston", Models::GeoLocation(29.7604, -95.3698, "Houston", "TX")},
    {"houston tx", Models::GeoLocation(29.7604, -95.3698, "Houston", "TX")},
    {"phoenix", Models::GeoLocation(33.4484, -112.0740, "Phoenix", "AZ")},
    {"phoenix az", Models::GeoLocation(33.4484, -112.0740, "Phoenix", "AZ")},
    {"philadelphia", Models::GeoLocation(39.9526, -75.1652, "Philadelphia", "PA")},
    {"philadelphia pa", Models::GeoLocation(39.9526, -75.1652, "Philadelphia", "PA")},
    {"san antonio", Models::GeoLocation(29.4241, -98.4936, "San Antonio", "TX")},
    {"san antonio tx", Models::GeoLocation(29.4241, -98.4936, "San Antonio", "TX")},
    {"san diego", Models::GeoLocation(32.7157, -117.1611, "San Diego", "CA")},
    {"san diego ca", Models::GeoLocation(32.7157, -117.1611, "San Diego", "CA")},
    {"dallas", Models::GeoLocation(32.7767, -96.7970, "Dallas", "TX")},
    {"dallas tx", Models::GeoLocation(32.7767, -96.7970, "Dallas", "TX")},
    {"san jose", Models::GeoLocation(37.3382, -121.8863, "San Jose", "CA")},
    {"san jose ca", Models::GeoLocation(37.3382, -121.8863, "San Jose", "CA")},
    {"austin", Models::GeoLocation(30.2672, -97.7431, "Austin", "TX")},
    {"austin tx", Models::GeoLocation(30.2672, -97.7431, "Austin", "TX")},
    {"san francisco", Models::GeoLocation(37.7749, -122.4194, "San Francisco", "CA")},
    {"san francisco ca", Models::GeoLocation(37.7749, -122.4194, "San Francisco", "CA")},
    {"sf", Models::GeoLocation(37.7749, -122.4194, "San Francisco", "CA")},
    {"seattle", Models::GeoLocation(47.6062, -122.3321, "Seattle", "WA")},
    {"seattle wa", Models::GeoLocation(47.6062, -122.3321, "Seattle", "WA")},
    {"denver", Models::GeoLocation(39.7392, -104.9903, "Denver", "CO")},
    {"denver co", Models::GeoLocation(39.7392, -104.9903, "Denver", "CO")},
    {"boston", Models::GeoLocation(42.3601, -71.0589, "Boston", "MA")},
    {"boston ma", Models::GeoLocation(42.3601, -71.0589, "Boston", "MA")},
    {"atlanta", Models::GeoLocation(33.7490, -84.3880, "Atlanta", "GA")},
    {"atlanta ga", Models::GeoLocation(33.7490, -84.3880, "Atlanta", "GA")},
    {"miami", Models::GeoLocation(25.7617, -80.1918, "Miami", "FL")},
    {"miami fl", Models::GeoLocation(25.7617, -80.1918, "Miami", "FL")},
    {"portland", Models::GeoLocation(45.5152, -122.6784, "Portland", "OR")},
    {"portland or", Models::GeoLocation(45.5152, -122.6784, "Portland", "OR")},
    {"las vegas", Models::GeoLocation(36.1699, -115.1398, "Las Vegas", "NV")},
    {"las vegas nv", Models::GeoLocation(36.1699, -115.1398, "Las Vegas", "NV")},
    {"minneapolis", Models::GeoLocation(44.9778, -93.2650, "Minneapolis", "MN")},
    {"minneapolis mn", Models::GeoLocation(44.9778, -93.2650, "Minneapolis", "MN")},
    {"detroit", Models::GeoLocation(42.3314, -83.0458, "Detroit", "MI")},
    {"detroit mi", Models::GeoLocation(42.3314, -83.0458, "Detroit", "MI")},
    {"nashville", Models::GeoLocation(36.1627, -86.7816, "Nashville", "TN")},
    {"nashville tn", Models::GeoLocation(36.1627, -86.7816, "Nashville", "TN")},
    {"charlotte", Models::GeoLocation(35.2271, -80.8431, "Charlotte", "NC")},
    {"charlotte nc", Models::GeoLocation(35.2271, -80.8431, "Charlotte", "NC")},
    {"washington", Models::GeoLocation(38.9072, -77.0369, "Washington", "DC")},
    {"washington dc", Models::GeoLocation(38.9072, -77.0369, "Washington", "DC")},
    {"dc", Models::GeoLocation(38.9072, -77.0369, "Washington", "DC")}
};

// CURL write callback
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

NominatimGeocodingService::NominatimGeocodingService() {
    config_.provider = GeocodingProvider::NOMINATIM;
    config_.endpoint = "https://nominatim.openstreetmap.org";
}

NominatimGeocodingService::NominatimGeocodingService(const GeocodingConfig& config)
    : config_(config) {
    if (config_.endpoint.empty()) {
        config_.endpoint = "https://nominatim.openstreetmap.org";
    }
}

void NominatimGeocodingService::setConfig(const GeocodingConfig& config) {
    config_ = config;
    if (config_.endpoint.empty()) {
        config_.endpoint = "https://nominatim.openstreetmap.org";
    }
}

std::string NominatimGeocodingService::normalizeAddress(const std::string& address) {
    std::string normalized = address;
    // Convert to lowercase
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    // Remove extra whitespace
    normalized.erase(std::unique(normalized.begin(), normalized.end(),
                                  [](char a, char b) { return a == ' ' && b == ' '; }),
                      normalized.end());
    // Trim
    size_t start = normalized.find_first_not_of(" \t");
    size_t end = normalized.find_last_not_of(" \t");
    if (start != std::string::npos && end != std::string::npos) {
        normalized = normalized.substr(start, end - start + 1);
    }
    // Remove common punctuation
    normalized.erase(std::remove_if(normalized.begin(), normalized.end(),
                                     [](char c) { return c == ',' || c == '.'; }),
                      normalized.end());
    return normalized;
}

void NominatimGeocodingService::geocode(const std::string& address, GeocodeCallback callback) {
    auto result = geocodeSync(address);
    if (callback) {
        callback(result, result.isValid ? "" : "Geocoding failed");
    }
}

void NominatimGeocodingService::reverseGeocode(double latitude, double longitude, ReverseGeocodeCallback callback) {
    auto result = reverseGeocodeSync(latitude, longitude);
    if (callback) {
        callback(result, result.isValid ? "" : "Reverse geocoding failed");
    }
}

Models::GeoLocation NominatimGeocodingService::geocodeSync(const std::string& address) {
    // Check cache first
    std::string cacheKey = normalizeAddress(address);
    if (config_.enableCaching) {
        auto it = cache_.find(cacheKey);
        if (it != cache_.end()) {
            time_t now = std::time(nullptr);
            if (now - it->second.second < config_.cacheDurationMinutes * 60) {
                return it->second.first;
            }
        }
    }

    // Check known locations (demo data)
    auto known = knownLocations_.find(cacheKey);
    if (known != knownLocations_.end()) {
        Models::GeoLocation result = known->second;
        result.source = "local";
        result.formattedAddress = result.city + ", " + result.state;

        // Cache the result
        if (config_.enableCaching) {
            cache_[cacheKey] = {result, std::time(nullptr)};
        }
        return result;
    }

    // For demo/prototype, return Denver as default for unknown addresses
    // In production, this would call the actual Nominatim API
    Models::GeoLocation defaultLocation(39.7392, -104.9903, "Denver", "CO");
    defaultLocation.source = "local";
    defaultLocation.formattedAddress = address;

    // Cache the result
    if (config_.enableCaching) {
        cache_[cacheKey] = {defaultLocation, std::time(nullptr)};
    }

    return defaultLocation;
}

Models::GeoLocation NominatimGeocodingService::reverseGeocodeSync(double latitude, double longitude) {
    Models::GeoLocation result(latitude, longitude);
    result.source = "nominatim";

    // For demo/prototype, just return the coordinates with generic city info
    // In production, this would call the actual Nominatim reverse API
    result.city = "Unknown City";
    result.state = "XX";
    result.formattedAddress = std::to_string(latitude) + ", " + std::to_string(longitude);

    return result;
}

std::string NominatimGeocodingService::buildGeocodeUrl(const std::string& address) {
    // URL encode the address
    CURL* curl = curl_easy_init();
    std::string url = config_.endpoint + "/search?format=json&q=";

    if (curl) {
        char* encoded = curl_easy_escape(curl, address.c_str(), static_cast<int>(address.length()));
        if (encoded) {
            url += encoded;
            curl_free(encoded);
        }
        curl_easy_cleanup(curl);
    }

    return url;
}

std::string NominatimGeocodingService::buildReverseGeocodeUrl(double lat, double lon) {
    return config_.endpoint + "/reverse?format=json&lat=" +
           std::to_string(lat) + "&lon=" + std::to_string(lon);
}

void NominatimGeocodingService::clearCache() {
    cache_.clear();
}

int NominatimGeocodingService::getCacheSize() const {
    return static_cast<int>(cache_.size());
}

// Factory implementation
std::unique_ptr<IGeocodingService> GeocodingServiceFactory::create(
    GeocodingProvider provider,
    const GeocodingConfig& config
) {
    switch (provider) {
        case GeocodingProvider::NOMINATIM:
        case GeocodingProvider::LOCAL:
        default:
            return std::make_unique<NominatimGeocodingService>(config);
    }
}

std::unique_ptr<IGeocodingService> GeocodingServiceFactory::createDefault() {
    return std::make_unique<NominatimGeocodingService>();
}

} // namespace Services
} // namespace FranchiseAI
