#include "GoogleGeocodingAPI.h"
#include <curl/curl.h>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace FranchiseAI {
namespace Services {

// CURL write callback
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

GoogleGeocodingAPI::GoogleGeocodingAPI() {
    initializeThreadPool();
}

GoogleGeocodingAPI::GoogleGeocodingAPI(const GoogleGeocodingConfig& config)
    : config_(config) {
    initializeThreadPool();
}

GoogleGeocodingAPI::~GoogleGeocodingAPI() {
    if (threadPool_) {
        threadPool_->shutdown(true);
    }
}

void GoogleGeocodingAPI::initializeThreadPool() {
    ThreadPoolConfig poolConfig;
    poolConfig.threadCount = config_.threadPoolSize;
    poolConfig.maxQueueSize = config_.maxQueuedRequests;
    poolConfig.enableMetrics = true;

    threadPool_ = std::make_unique<ThreadPool>(poolConfig);
}

void GoogleGeocodingAPI::setConfig(const GoogleGeocodingConfig& config) {
    config_ = config;

    // Resize thread pool if needed
    if (threadPool_ && threadPool_->getThreadCount() != config_.threadPoolSize) {
        std::lock_guard<std::mutex> lock(threadPoolMutex_);
        threadPool_->resize(config_.threadPoolSize);
    }
}

std::string GoogleGeocodingAPI::normalizeAddress(const std::string& address) {
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

    return normalized;
}

Models::GeoLocation GoogleGeocodingAPI::getCachedResult(const std::string& normalizedAddress) {
    if (!config_.enableCaching) {
        return Models::GeoLocation();
    }

    std::lock_guard<std::mutex> lock(cacheMutex_);
    auto it = cache_.find(normalizedAddress);
    if (it != cache_.end()) {
        time_t now = std::time(nullptr);
        if (now - it->second.second < config_.cacheDurationMinutes * 60) {
            stats_.cacheHits++;
            return it->second.first;
        }
    }

    stats_.cacheMisses++;
    return Models::GeoLocation();
}

void GoogleGeocodingAPI::cacheResult(const std::string& normalizedAddress, const Models::GeoLocation& location) {
    if (!config_.enableCaching) {
        return;
    }

    std::lock_guard<std::mutex> lock(cacheMutex_);
    cache_[normalizedAddress] = {location, std::time(nullptr)};
}

std::string GoogleGeocodingAPI::buildGeocodeUrl(const std::string& address) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    char* encoded = curl_easy_escape(curl, address.c_str(), static_cast<int>(address.length()));
    std::string url = config_.endpoint + "?address=" + std::string(encoded) +
                      "&key=" + config_.apiKey;

    curl_free(encoded);
    curl_easy_cleanup(curl);

    return url;
}

std::string GoogleGeocodingAPI::buildReverseGeocodeUrl(double lat, double lon) {
    std::ostringstream url;
    url << std::fixed << std::setprecision(6);
    url << config_.endpoint << "?latlng=" << lat << "," << lon
        << "&key=" << config_.apiKey;
    return url.str();
}

std::string GoogleGeocodingAPI::executeHttpRequest(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, config_.userAgent.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, config_.requestTimeoutMs);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, config_.connectTimeoutMs);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

    // SSL settings
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "";
    }

    return response;
}

// Helper to extract JSON string value
static std::string extractJsonValue(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return "";

    size_t colonPos = json.find(':', keyPos + searchKey.length());
    if (colonPos == std::string::npos) return "";

    // Skip whitespace
    size_t valueStart = colonPos + 1;
    while (valueStart < json.length() && std::isspace(json[valueStart])) valueStart++;

    if (valueStart >= json.length()) return "";

    if (json[valueStart] == '"') {
        // String value
        size_t valueEnd = json.find('"', valueStart + 1);
        if (valueEnd == std::string::npos) return "";
        return json.substr(valueStart + 1, valueEnd - valueStart - 1);
    } else if (json[valueStart] == '-' || std::isdigit(json[valueStart])) {
        // Numeric value
        size_t valueEnd = valueStart;
        while (valueEnd < json.length() &&
               (std::isdigit(json[valueEnd]) || json[valueEnd] == '.' || json[valueEnd] == '-')) {
            valueEnd++;
        }
        return json.substr(valueStart, valueEnd - valueStart);
    }

    return "";
}

Models::GeoLocation GoogleGeocodingAPI::parseGeocodeResponse(
    const std::string& json,
    const std::string& originalAddress
) {
    Models::GeoLocation result;
    result.isValid = false;

    // Check status
    std::string status = extractJsonValue(json, "status");
    if (status != "OK") {
        if (status == "OVER_QUERY_LIMIT") {
            stats_.rateLimitHits++;
        }
        return result;
    }

    // Find geometry.location
    size_t locationPos = json.find("\"location\"");
    if (locationPos == std::string::npos) {
        return result;
    }

    // Extract lat/lng from location object
    size_t locStart = json.find('{', locationPos);
    size_t locEnd = json.find('}', locStart);
    if (locStart == std::string::npos || locEnd == std::string::npos) {
        return result;
    }

    std::string locationJson = json.substr(locStart, locEnd - locStart + 1);

    std::string latStr = extractJsonValue(locationJson, "lat");
    std::string lngStr = extractJsonValue(locationJson, "lng");

    if (latStr.empty() || lngStr.empty()) {
        return result;
    }

    try {
        result.latitude = std::stod(latStr);
        result.longitude = std::stod(lngStr);
        result.isValid = true;
        result.source = "google";
    } catch (...) {
        return result;
    }

    // Extract formatted address
    result.formattedAddress = extractJsonValue(json, "formatted_address");
    if (result.formattedAddress.empty()) {
        result.formattedAddress = originalAddress;
    }

    // Extract address components
    size_t componentsPos = json.find("\"address_components\"");
    if (componentsPos != std::string::npos) {
        // Parse address components array
        size_t arrayStart = json.find('[', componentsPos);
        size_t arrayEnd = json.find(']', arrayStart);

        if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
            std::string componentsJson = json.substr(arrayStart, arrayEnd - arrayStart + 1);

            // Extract locality (city)
            size_t localityPos = componentsJson.find("\"locality\"");
            if (localityPos != std::string::npos) {
                // Find the long_name for this component
                size_t objStart = componentsJson.rfind('{', localityPos);
                if (objStart != std::string::npos) {
                    std::string objJson = componentsJson.substr(objStart, localityPos - objStart + 20);
                    result.city = extractJsonValue(objJson, "long_name");
                }
            }

            // Extract administrative_area_level_1 (state)
            size_t statePos = componentsJson.find("\"administrative_area_level_1\"");
            if (statePos != std::string::npos) {
                size_t objStart = componentsJson.rfind('{', statePos);
                if (objStart != std::string::npos) {
                    std::string objJson = componentsJson.substr(objStart, statePos - objStart + 50);
                    result.state = extractJsonValue(objJson, "short_name");
                }
            }

            // Extract postal_code
            size_t postalPos = componentsJson.find("\"postal_code\"");
            if (postalPos != std::string::npos) {
                size_t objStart = componentsJson.rfind('{', postalPos);
                if (objStart != std::string::npos) {
                    std::string objJson = componentsJson.substr(objStart, postalPos - objStart + 30);
                    result.postalCode = extractJsonValue(objJson, "long_name");
                }
            }

            // Extract country
            size_t countryPos = componentsJson.find("\"country\"");
            if (countryPos != std::string::npos) {
                size_t objStart = componentsJson.rfind('{', countryPos);
                if (objStart != std::string::npos) {
                    std::string objJson = componentsJson.substr(objStart, countryPos - objStart + 20);
                    result.country = extractJsonValue(objJson, "short_name");
                }
            }

            // Extract street
            size_t routePos = componentsJson.find("\"route\"");
            if (routePos != std::string::npos) {
                size_t objStart = componentsJson.rfind('{', routePos);
                if (objStart != std::string::npos) {
                    std::string objJson = componentsJson.substr(objStart, routePos - objStart + 20);
                    result.street = extractJsonValue(objJson, "long_name");
                }
            }
        }
    }

    return result;
}

Models::GeoLocation GoogleGeocodingAPI::callGoogleAPI(const std::string& address) {
    if (!config_.isConfigured()) {
        Models::GeoLocation invalid;
        invalid.isValid = false;
        return invalid;
    }

    stats_.totalRequests++;

    auto startTime = std::chrono::high_resolution_clock::now();

    std::string url = buildGeocodeUrl(address);
    std::string response;

    // Retry logic
    for (int attempt = 0; attempt <= config_.maxRetries; ++attempt) {
        response = executeHttpRequest(url);

        if (!response.empty()) {
            break;
        }

        if (attempt < config_.maxRetries) {
            std::this_thread::sleep_for(std::chrono::milliseconds(
                config_.retryDelayMs * (attempt + 1)));
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();

    if (response.empty()) {
        stats_.failedRequests++;
        Models::GeoLocation invalid;
        invalid.isValid = false;
        return invalid;
    }

    Models::GeoLocation result = parseGeocodeResponse(response, address);

    if (result.isValid) {
        stats_.successfulRequests++;
        stats_.totalLatencyMs += latency;
    } else {
        stats_.failedRequests++;
    }

    return result;
}

Models::GeoLocation GoogleGeocodingAPI::callGoogleReverseAPI(double lat, double lon) {
    if (!config_.isConfigured()) {
        Models::GeoLocation invalid(lat, lon);
        invalid.isValid = false;
        return invalid;
    }

    stats_.totalRequests++;

    std::string url = buildReverseGeocodeUrl(lat, lon);
    std::string response = executeHttpRequest(url);

    if (response.empty()) {
        stats_.failedRequests++;
        Models::GeoLocation invalid(lat, lon);
        invalid.isValid = false;
        return invalid;
    }

    std::string coordStr = std::to_string(lat) + ", " + std::to_string(lon);
    Models::GeoLocation result = parseGeocodeResponse(response, coordStr);

    if (result.isValid) {
        stats_.successfulRequests++;
    } else {
        stats_.failedRequests++;
        result.latitude = lat;
        result.longitude = lon;
    }

    return result;
}

void GoogleGeocodingAPI::geocode(const std::string& address, GeocodeCallback callback) {
    // Submit to thread pool for async execution
    threadPool_->execute([this, address, callback]() {
        auto result = geocodeSync(address);
        if (callback) {
            callback(result, result.isValid ? "" : "Geocoding failed");
        }
    });
}

void GoogleGeocodingAPI::reverseGeocode(double latitude, double longitude, ReverseGeocodeCallback callback) {
    threadPool_->execute([this, latitude, longitude, callback]() {
        auto result = reverseGeocodeSync(latitude, longitude);
        if (callback) {
            callback(result, result.isValid ? "" : "Reverse geocoding failed");
        }
    });
}

Models::GeoLocation GoogleGeocodingAPI::geocodeSync(const std::string& address) {
    std::string normalizedAddr = normalizeAddress(address);

    // Check cache first
    Models::GeoLocation cached = getCachedResult(normalizedAddr);
    if (cached.isValid) {
        return cached;
    }

    // Call Google API
    Models::GeoLocation result = callGoogleAPI(address);

    // Cache result
    if (result.isValid) {
        cacheResult(normalizedAddr, result);
    }

    return result;
}

Models::GeoLocation GoogleGeocodingAPI::reverseGeocodeSync(double latitude, double longitude) {
    // Build cache key from coordinates
    std::ostringstream cacheKey;
    cacheKey << std::fixed << std::setprecision(6) << latitude << "," << longitude;
    std::string key = cacheKey.str();

    // Check cache
    Models::GeoLocation cached = getCachedResult(key);
    if (cached.isValid) {
        return cached;
    }

    // Call Google API
    Models::GeoLocation result = callGoogleReverseAPI(latitude, longitude);

    // Cache result
    if (result.isValid) {
        cacheResult(key, result);
    }

    return result;
}

void GoogleGeocodingAPI::geocodeBatch(
    const std::vector<std::string>& addresses,
    BatchCallback callback,
    ProgressCallback progressCallback
) {
    auto startTime = std::chrono::high_resolution_clock::now();

    // Use shared state for collecting results
    auto results = std::make_shared<std::vector<Models::GeoLocation>>(addresses.size());
    auto errors = std::make_shared<std::vector<std::string>>(addresses.size());
    auto completed = std::make_shared<std::atomic<int>>(0);
    auto successCount = std::make_shared<std::atomic<int>>(0);

    int total = static_cast<int>(addresses.size());

    // Submit all geocoding tasks
    for (size_t i = 0; i < addresses.size(); ++i) {
        threadPool_->execute([this, i, addresses, results, errors, completed,
                              successCount, total, progressCallback, callback, startTime]() {
            auto result = geocodeSync(addresses[i]);

            (*results)[i] = result;
            if (result.isValid) {
                (*successCount)++;
            } else {
                (*errors)[i] = "Geocoding failed for: " + addresses[i];
            }

            int done = ++(*completed);

            if (progressCallback) {
                progressCallback(done, total);
            }

            // Check if all done
            if (done == total && callback) {
                auto endTime = std::chrono::high_resolution_clock::now();
                auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                    endTime - startTime).count();

                BatchGeocodeResult batchResult;
                batchResult.results = std::move(*results);
                batchResult.successCount = successCount->load();
                batchResult.failureCount = total - batchResult.successCount;
                batchResult.totalTimeMs = totalTime;

                // Collect non-empty errors
                for (const auto& err : *errors) {
                    if (!err.empty()) {
                        batchResult.errors.push_back(err);
                    }
                }

                callback(batchResult);
            }
        });
    }
}

BatchGeocodeResult GoogleGeocodingAPI::geocodeBatchSync(const std::vector<std::string>& addresses) {
    auto startTime = std::chrono::high_resolution_clock::now();

    BatchGeocodeResult result;
    result.results.reserve(addresses.size());

    // Submit all tasks and collect futures
    std::vector<std::future<Models::GeoLocation>> futures;
    futures.reserve(addresses.size());

    for (const auto& address : addresses) {
        futures.push_back(threadPool_->submit([this, address]() {
            return geocodeSync(address);
        }));
    }

    // Collect results
    for (size_t i = 0; i < futures.size(); ++i) {
        try {
            auto location = futures[i].get();
            result.results.push_back(location);

            if (location.isValid) {
                result.successCount++;
            } else {
                result.failureCount++;
                result.errors.push_back("Geocoding failed for: " + addresses[i]);
            }
        } catch (const std::exception& e) {
            result.failureCount++;
            result.errors.push_back(std::string("Exception: ") + e.what());
            result.results.push_back(Models::GeoLocation());
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.totalTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();

    return result;
}

void GoogleGeocodingAPI::prewarmCache(const std::vector<std::string>& addresses) {
    for (const auto& address : addresses) {
        threadPool_->execute([this, address]() {
            geocodeSync(address);
        });
    }
}

void GoogleGeocodingAPI::setThreadPoolSize(int threadCount) {
    std::lock_guard<std::mutex> lock(threadPoolMutex_);
    config_.threadPoolSize = std::max(1, threadCount);

    if (threadPool_) {
        threadPool_->resize(config_.threadPoolSize);
    }
}

int GoogleGeocodingAPI::getThreadPoolSize() const {
    return config_.threadPoolSize;
}

int GoogleGeocodingAPI::getRecommendedMemoryMB() const {
    return ThreadPoolConfig::getRecommendedMemoryMB(config_.threadPoolSize);
}

const ThreadPoolMetrics& GoogleGeocodingAPI::getThreadPoolMetrics() const {
    return threadPool_->getMetrics();
}

void GoogleGeocodingAPI::clearCache() {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    cache_.clear();
}

int GoogleGeocodingAPI::getCacheSize() const {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    return static_cast<int>(cache_.size());
}

} // namespace Services
} // namespace FranchiseAI
