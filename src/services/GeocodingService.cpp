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

    // Check known locations first (fast path for common cities)
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

    // Call Nominatim API for any address not in known locations
    Models::GeoLocation result = callNominatimAPI(address);

    // Cache the result (even if invalid, to avoid repeated failed calls)
    if (config_.enableCaching) {
        cache_[cacheKey] = {result, std::time(nullptr)};
    }

    return result;
}

Models::GeoLocation NominatimGeocodingService::callNominatimAPI(const std::string& address) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        // Return invalid location on CURL init failure
        Models::GeoLocation invalid;
        invalid.isValid = false;
        return invalid;
    }

    // Build URL with URL-encoded address
    std::string url = config_.endpoint + "/search?format=json&limit=1&q=";
    char* encoded = curl_easy_escape(curl, address.c_str(), static_cast<int>(address.length()));
    if (encoded) {
        url += encoded;
        curl_free(encoded);
    } else {
        curl_easy_cleanup(curl);
        Models::GeoLocation invalid;
        invalid.isValid = false;
        return invalid;
    }

    // Set up CURL request
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, config_.userAgent.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        // Network error - return invalid location
        Models::GeoLocation invalid;
        invalid.isValid = false;
        return invalid;
    }

    // Parse JSON response
    // Nominatim returns: [{"lat":"40.7127281","lon":"-74.0060152","display_name":"...","address":{"city":"...",...}}]
    return parseNominatimResponse(response, address);
}

Models::GeoLocation NominatimGeocodingService::parseNominatimResponse(const std::string& json, const std::string& originalAddress) {
    Models::GeoLocation result;
    result.isValid = false;

    // Check if response is empty or not a valid array
    if (json.empty() || json[0] != '[') {
        return result;
    }

    // Check for empty array
    if (json == "[]") {
        return result;
    }

    // Extract latitude
    size_t latPos = json.find("\"lat\"");
    if (latPos == std::string::npos) {
        return result;
    }
    size_t latStart = json.find("\"", latPos + 5);
    size_t latEnd = json.find("\"", latStart + 1);
    if (latStart == std::string::npos || latEnd == std::string::npos) {
        return result;
    }
    std::string latStr = json.substr(latStart + 1, latEnd - latStart - 1);

    // Extract longitude
    size_t lonPos = json.find("\"lon\"");
    if (lonPos == std::string::npos) {
        return result;
    }
    size_t lonStart = json.find("\"", lonPos + 5);
    size_t lonEnd = json.find("\"", lonStart + 1);
    if (lonStart == std::string::npos || lonEnd == std::string::npos) {
        return result;
    }
    std::string lonStr = json.substr(lonStart + 1, lonEnd - lonStart - 1);

    // Extract display_name for formatted address
    std::string displayName = originalAddress;
    size_t displayPos = json.find("\"display_name\"");
    if (displayPos != std::string::npos) {
        size_t displayStart = json.find("\"", displayPos + 14);
        size_t displayEnd = json.find("\"", displayStart + 1);
        if (displayStart != std::string::npos && displayEnd != std::string::npos) {
            displayName = json.substr(displayStart + 1, displayEnd - displayStart - 1);
        }
    }

    // Parse coordinates
    try {
        result.latitude = std::stod(latStr);
        result.longitude = std::stod(lonStr);
        result.isValid = true;
        result.source = "nominatim";
        result.formattedAddress = displayName;

        // Try to extract city and state from address object
        size_t cityPos = json.find("\"city\"");
        if (cityPos != std::string::npos) {
            size_t cityStart = json.find("\"", cityPos + 6);
            size_t cityEnd = json.find("\"", cityStart + 1);
            if (cityStart != std::string::npos && cityEnd != std::string::npos) {
                result.city = json.substr(cityStart + 1, cityEnd - cityStart - 1);
            }
        }
        // Try town if city not found
        if (result.city.empty()) {
            size_t townPos = json.find("\"town\"");
            if (townPos != std::string::npos) {
                size_t townStart = json.find("\"", townPos + 6);
                size_t townEnd = json.find("\"", townStart + 1);
                if (townStart != std::string::npos && townEnd != std::string::npos) {
                    result.city = json.substr(townStart + 1, townEnd - townStart - 1);
                }
            }
        }
        // Try village if town not found
        if (result.city.empty()) {
            size_t villagePos = json.find("\"village\"");
            if (villagePos != std::string::npos) {
                size_t villageStart = json.find("\"", villagePos + 9);
                size_t villageEnd = json.find("\"", villageStart + 1);
                if (villageStart != std::string::npos && villageEnd != std::string::npos) {
                    result.city = json.substr(villageStart + 1, villageEnd - villageStart - 1);
                }
            }
        }

        size_t statePos = json.find("\"state\"");
        if (statePos != std::string::npos) {
            size_t stateStart = json.find("\"", statePos + 7);
            size_t stateEnd = json.find("\"", stateStart + 1);
            if (stateStart != std::string::npos && stateEnd != std::string::npos) {
                result.state = json.substr(stateStart + 1, stateEnd - stateStart - 1);
            }
        }
    } catch (...) {
        result.isValid = false;
    }

    return result;
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
