#include "GooglePlacesAPI.h"
#include <curl/curl.h>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <thread>

namespace FranchiseAI {
namespace Services {

// CURL write callback
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Helper to extract JSON string value
static std::string extractJsonValue(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return "";

    size_t colonPos = json.find(':', keyPos + searchKey.length());
    if (colonPos == std::string::npos) return "";

    size_t valueStart = colonPos + 1;
    while (valueStart < json.length() && std::isspace(json[valueStart])) valueStart++;

    if (valueStart >= json.length()) return "";

    if (json[valueStart] == '"') {
        size_t valueEnd = json.find('"', valueStart + 1);
        if (valueEnd == std::string::npos) return "";
        return json.substr(valueStart + 1, valueEnd - valueStart - 1);
    } else if (json[valueStart] == '-' || std::isdigit(json[valueStart])) {
        size_t valueEnd = valueStart;
        while (valueEnd < json.length() &&
               (std::isdigit(json[valueEnd]) || json[valueEnd] == '.' || json[valueEnd] == '-')) {
            valueEnd++;
        }
        return json.substr(valueStart, valueEnd - valueStart);
    }

    return "";
}

// Helper to extract JSON number
static double extractJsonNumber(const std::string& json, const std::string& key) {
    std::string value = extractJsonValue(json, key);
    if (value.empty()) return 0.0;
    try {
        return std::stod(value);
    } catch (...) {
        return 0.0;
    }
}

// Helper to extract JSON array of strings
static std::vector<std::string> extractJsonStringArray(const std::string& json, const std::string& key) {
    std::vector<std::string> result;

    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return result;

    size_t arrayStart = json.find('[', keyPos);
    if (arrayStart == std::string::npos) return result;

    size_t arrayEnd = json.find(']', arrayStart);
    if (arrayEnd == std::string::npos) return result;

    std::string arrayJson = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

    size_t pos = 0;
    while (pos < arrayJson.length()) {
        size_t quoteStart = arrayJson.find('"', pos);
        if (quoteStart == std::string::npos) break;

        size_t quoteEnd = arrayJson.find('"', quoteStart + 1);
        if (quoteEnd == std::string::npos) break;

        result.push_back(arrayJson.substr(quoteStart + 1, quoteEnd - quoteStart - 1));
        pos = quoteEnd + 1;
    }

    return result;
}

Models::BusinessType GooglePlace::inferBusinessType() const {
    // Check place types in priority order
    for (const auto& type : types) {
        if (type == "corporate_office" || type == "establishment") {
            // Need more context
        } else if (type == "accounting" || type == "insurance_agency" ||
                   type == "bank" || type == "finance") {
            return Models::BusinessType::FINANCIAL_SERVICES;
        } else if (type == "lawyer") {
            return Models::BusinessType::LAW_FIRM;
        } else if (type == "hospital" || type == "doctor" ||
                   type == "health" || type == "medical_center") {
            return Models::BusinessType::MEDICAL_FACILITY;
        } else if (type == "university" || type == "school" ||
                   type == "secondary_school" || type == "primary_school") {
            return Models::BusinessType::EDUCATIONAL_INSTITUTION;
        } else if (type == "lodging" || type == "hotel") {
            return Models::BusinessType::HOTEL;
        } else if (type == "local_government_office" || type == "city_hall" ||
                   type == "courthouse" || type == "embassy") {
            return Models::BusinessType::GOVERNMENT_OFFICE;
        } else if (type == "stadium" || type == "convention_center") {
            return Models::BusinessType::CONFERENCE_CENTER;
        } else if (type == "storage" || type == "moving_company") {
            return Models::BusinessType::WAREHOUSE;
        } else if (type == "gym" || type == "physiotherapist") {
            return Models::BusinessType::OTHER;
        }
    }

    // Check name for hints
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

    if (lowerName.find("tech") != std::string::npos ||
        lowerName.find("software") != std::string::npos ||
        lowerName.find("digital") != std::string::npos) {
        return Models::BusinessType::TECH_COMPANY;
    }

    if (lowerName.find("cowork") != std::string::npos ||
        lowerName.find("shared office") != std::string::npos) {
        return Models::BusinessType::COWORKING_SPACE;
    }

    if (lowerName.find("conference") != std::string::npos ||
        lowerName.find("convention") != std::string::npos) {
        return Models::BusinessType::CONFERENCE_CENTER;
    }

    return Models::BusinessType::CORPORATE_OFFICE;
}

GooglePlacesAPI::GooglePlacesAPI() {
    initializeThreadPool();
}

GooglePlacesAPI::GooglePlacesAPI(const GooglePlacesConfig& config)
    : config_(config) {
    initializeThreadPool();
}

GooglePlacesAPI::~GooglePlacesAPI() {
    if (threadPool_) {
        threadPool_->shutdown(true);
    }
}

void GooglePlacesAPI::initializeThreadPool() {
    ThreadPoolConfig poolConfig;
    poolConfig.threadCount = config_.threadPoolSize;
    poolConfig.maxQueueSize = config_.maxQueuedRequests;
    poolConfig.enableMetrics = true;

    threadPool_ = std::make_unique<ThreadPool>(poolConfig);
}

void GooglePlacesAPI::setConfig(const GooglePlacesConfig& config) {
    config_ = config;

    if (threadPool_ && threadPool_->getThreadCount() != config_.threadPoolSize) {
        std::lock_guard<std::mutex> lock(threadPoolMutex_);
        threadPool_->resize(config_.threadPoolSize);
    }
}

std::string GooglePlacesAPI::executeHttpRequest(const std::string& url) {
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
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "";
    }

    return response;
}

std::string GooglePlacesAPI::buildNearbySearchUrl(
    double lat,
    double lon,
    int radiusMeters,
    const std::vector<std::string>& types,
    const std::string& pageToken
) {
    std::ostringstream url;
    url << std::fixed << std::setprecision(6);
    url << config_.nearbySearchEndpoint
        << "?location=" << lat << "," << lon
        << "&radius=" << radiusMeters
        << "&key=" << config_.apiKey;

    if (!types.empty()) {
        url << "&type=" << types[0];  // Google only allows one type per request
    }

    if (!pageToken.empty()) {
        url << "&pagetoken=" << pageToken;
    }

    return url.str();
}

std::string GooglePlacesAPI::buildTextSearchUrl(const std::string& query, const std::string& pageToken) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";

    char* encoded = curl_easy_escape(curl, query.c_str(), static_cast<int>(query.length()));
    std::ostringstream url;
    url << config_.textSearchEndpoint
        << "?query=" << encoded
        << "&key=" << config_.apiKey;

    curl_free(encoded);
    curl_easy_cleanup(curl);

    if (!pageToken.empty()) {
        url << "&pagetoken=" << pageToken;
    }

    return url.str();
}

std::string GooglePlacesAPI::buildDetailsUrl(const std::string& placeId) {
    return config_.detailsEndpoint +
           "?place_id=" + placeId +
           "&fields=name,formatted_address,formatted_phone_number,website,opening_hours,rating,user_ratings_total,types,geometry,business_status,price_level" +
           "&key=" + config_.apiKey;
}

std::vector<GooglePlace> GooglePlacesAPI::parseNearbySearchResponse(
    const std::string& json,
    std::string& nextPageToken
) {
    std::vector<GooglePlace> places;
    nextPageToken.clear();

    // Check status
    std::string status = extractJsonValue(json, "status");
    if (status != "OK" && status != "ZERO_RESULTS") {
        return places;
    }

    // Extract next_page_token if present
    nextPageToken = extractJsonValue(json, "next_page_token");

    // Find results array
    size_t resultsPos = json.find("\"results\"");
    if (resultsPos == std::string::npos) {
        return places;
    }

    size_t arrayStart = json.find('[', resultsPos);
    if (arrayStart == std::string::npos) {
        return places;
    }

    // Parse each result object
    size_t pos = arrayStart + 1;
    while (pos < json.length()) {
        size_t objStart = json.find('{', pos);
        if (objStart == std::string::npos || objStart > json.length() - 10) break;

        // Find matching closing brace
        int braceCount = 1;
        size_t objEnd = objStart + 1;
        while (objEnd < json.length() && braceCount > 0) {
            if (json[objEnd] == '{') braceCount++;
            else if (json[objEnd] == '}') braceCount--;
            objEnd++;
        }

        if (braceCount != 0) break;

        std::string objJson = json.substr(objStart, objEnd - objStart);

        GooglePlace place;
        place.placeId = extractJsonValue(objJson, "place_id");
        place.name = extractJsonValue(objJson, "name");
        place.vicinity = extractJsonValue(objJson, "vicinity");
        place.formattedAddress = extractJsonValue(objJson, "formatted_address");
        if (place.formattedAddress.empty()) {
            place.formattedAddress = place.vicinity;
        }

        place.rating = static_cast<float>(extractJsonNumber(objJson, "rating"));
        place.userRatingsTotal = static_cast<int>(extractJsonNumber(objJson, "user_ratings_total"));
        place.businessStatus = extractJsonValue(objJson, "business_status");
        place.permanentlyClosed = (place.businessStatus == "CLOSED_PERMANENTLY");

        // Extract geometry.location
        size_t geomPos = objJson.find("\"geometry\"");
        if (geomPos != std::string::npos) {
            size_t locPos = objJson.find("\"location\"", geomPos);
            if (locPos != std::string::npos) {
                size_t locStart = objJson.find('{', locPos);
                size_t locEnd = objJson.find('}', locStart);
                if (locStart != std::string::npos && locEnd != std::string::npos) {
                    std::string locJson = objJson.substr(locStart, locEnd - locStart + 1);
                    place.latitude = extractJsonNumber(locJson, "lat");
                    place.longitude = extractJsonNumber(locJson, "lng");
                }
            }
        }

        // Extract types array
        place.types = extractJsonStringArray(objJson, "types");

        // Only add valid places
        if (!place.placeId.empty() && !place.name.empty() && !place.permanentlyClosed) {
            places.push_back(place);
        }

        pos = objEnd;
        if (pos < json.length() && json[pos] == ']') break;
    }

    stats_.totalPlacesFound += static_cast<int>(places.size());
    return places;
}

GooglePlace GooglePlacesAPI::parseDetailsResponse(const std::string& json) {
    GooglePlace place;

    std::string status = extractJsonValue(json, "status");
    if (status != "OK") {
        return place;
    }

    // Find result object
    size_t resultPos = json.find("\"result\"");
    if (resultPos == std::string::npos) {
        return place;
    }

    size_t objStart = json.find('{', resultPos);
    size_t objEnd = json.rfind('}');
    if (objStart == std::string::npos) {
        return place;
    }

    std::string objJson = json.substr(objStart, objEnd - objStart + 1);

    place.name = extractJsonValue(objJson, "name");
    place.formattedAddress = extractJsonValue(objJson, "formatted_address");
    place.formattedPhoneNumber = extractJsonValue(objJson, "formatted_phone_number");
    place.phoneNumber = extractJsonValue(objJson, "international_phone_number");
    if (place.phoneNumber.empty()) {
        place.phoneNumber = place.formattedPhoneNumber;
    }
    place.website = extractJsonValue(objJson, "website");
    place.rating = static_cast<float>(extractJsonNumber(objJson, "rating"));
    place.userRatingsTotal = static_cast<int>(extractJsonNumber(objJson, "user_ratings_total"));
    place.priceLevel = static_cast<int>(extractJsonNumber(objJson, "price_level"));
    place.businessStatus = extractJsonValue(objJson, "business_status");

    // Extract geometry
    size_t geomPos = objJson.find("\"geometry\"");
    if (geomPos != std::string::npos) {
        size_t locPos = objJson.find("\"location\"", geomPos);
        if (locPos != std::string::npos) {
            size_t locStart = objJson.find('{', locPos);
            size_t locEnd = objJson.find('}', locStart);
            if (locStart != std::string::npos && locEnd != std::string::npos) {
                std::string locJson = objJson.substr(locStart, locEnd - locStart + 1);
                place.latitude = extractJsonNumber(locJson, "lat");
                place.longitude = extractJsonNumber(locJson, "lng");
            }
        }
    }

    place.types = extractJsonStringArray(objJson, "types");

    // Extract opening hours weekday_text
    size_t hoursPos = objJson.find("\"opening_hours\"");
    if (hoursPos != std::string::npos) {
        place.weekdayText = extractJsonStringArray(objJson, "weekday_text");
    }

    return place;
}

std::string GooglePlacesAPI::buildCacheKey(
    double lat,
    double lon,
    int radius,
    const std::vector<std::string>& types
) {
    std::ostringstream key;
    key << std::fixed << std::setprecision(4);
    key << lat << "," << lon << "," << radius;
    for (const auto& type : types) {
        key << "," << type;
    }
    return key.str();
}

void GooglePlacesAPI::searchNearby(
    double latitude,
    double longitude,
    int radiusMeters,
    const std::vector<std::string>& types,
    PlacesCallback callback
) {
    threadPool_->execute([this, latitude, longitude, radiusMeters, types, callback]() {
        auto results = searchNearbySync(latitude, longitude, radiusMeters, types);
        if (callback) {
            callback(results, results.empty() ? "No places found" : "");
        }
    });
}

std::vector<GooglePlace> GooglePlacesAPI::searchNearbySync(
    double latitude,
    double longitude,
    int radiusMeters,
    const std::vector<std::string>& types
) {
    if (!config_.isConfigured()) {
        return {};
    }

    // Check cache
    std::string cacheKey = buildCacheKey(latitude, longitude, radiusMeters, types);
    {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        auto it = searchCache_.find(cacheKey);
        if (it != searchCache_.end()) {
            time_t now = std::time(nullptr);
            if (now - it->second.second < config_.cacheDurationMinutes * 60) {
                stats_.cacheHits++;
                return it->second.first;
            }
        }
        stats_.cacheMisses++;
    }

    stats_.totalRequests++;
    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<GooglePlace> allPlaces;
    std::string pageToken;
    int page = 0;

    do {
        std::string url = buildNearbySearchUrl(latitude, longitude, radiusMeters, types, pageToken);
        std::string response = executeHttpRequest(url);

        if (response.empty()) {
            stats_.failedRequests++;
            break;
        }

        auto places = parseNearbySearchResponse(response, pageToken);
        allPlaces.insert(allPlaces.end(), places.begin(), places.end());

        page++;

        // Google requires a short delay before requesting next page
        if (!pageToken.empty() && page < config_.maxPages) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

    } while (!pageToken.empty() && page < config_.maxPages);

    auto endTime = std::chrono::high_resolution_clock::now();
    auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();

    if (!allPlaces.empty()) {
        stats_.successfulRequests++;
        stats_.totalLatencyMs += latency;

        // Cache results
        std::lock_guard<std::mutex> lock(cacheMutex_);
        searchCache_[cacheKey] = {allPlaces, std::time(nullptr)};
    }

    return allPlaces;
}

std::vector<std::string> GooglePlacesAPI::getCateringProspectTypes() {
    return {
        "corporate_office",
        "accounting",
        "bank",
        "insurance_agency",
        "lawyer",
        "hospital",
        "doctor",
        "university",
        "school",
        "lodging",
        "local_government_office",
        "city_hall",
        "courthouse",
        "stadium"
    };
}

void GooglePlacesAPI::searchCateringProspects(
    double latitude,
    double longitude,
    double radiusMiles,
    BusinessCallback callback
) {
    threadPool_->execute([this, latitude, longitude, radiusMiles, callback]() {
        auto results = searchCateringProspectsSync(latitude, longitude, radiusMiles);
        if (callback) {
            callback(results, results.empty() ? "No prospects found" : "");
        }
    });
}

std::vector<Models::BusinessInfo> GooglePlacesAPI::searchCateringProspectsSync(
    double latitude,
    double longitude,
    double radiusMiles
) {
    int radiusMeters = static_cast<int>(radiusMiles * 1609.34);

    // Search for multiple place types relevant to catering
    auto types = getCateringProspectTypes();

    std::vector<Models::BusinessInfo> allBusinesses;
    std::mutex resultsMutex;

    // Submit search tasks for each type in parallel
    std::vector<std::future<std::vector<GooglePlace>>> futures;

    for (const auto& type : types) {
        futures.push_back(threadPool_->submit([this, latitude, longitude, radiusMeters, type]() {
            return searchNearbySync(latitude, longitude, radiusMeters, {type});
        }));
    }

    // Collect results
    for (auto& future : futures) {
        try {
            auto places = future.get();
            for (const auto& place : places) {
                auto business = placeToBusinessInfo(place);
                std::lock_guard<std::mutex> lock(resultsMutex);
                allBusinesses.push_back(business);
            }
        } catch (...) {
            // Continue with other types
        }
    }

    // Remove duplicates based on place ID
    std::sort(allBusinesses.begin(), allBusinesses.end(),
        [](const Models::BusinessInfo& a, const Models::BusinessInfo& b) {
            return a.id < b.id;
        });

    allBusinesses.erase(
        std::unique(allBusinesses.begin(), allBusinesses.end(),
            [](const Models::BusinessInfo& a, const Models::BusinessInfo& b) {
                return a.id == b.id;
            }),
        allBusinesses.end()
    );

    // Sort by catering potential
    std::sort(allBusinesses.begin(), allBusinesses.end(),
        [](const Models::BusinessInfo& a, const Models::BusinessInfo& b) {
            return a.cateringPotentialScore > b.cateringPotentialScore;
        });

    return allBusinesses;
}

void GooglePlacesAPI::searchBusinesses(
    const Models::SearchArea& searchArea,
    BusinessCallback callback
) {
    searchCateringProspects(
        searchArea.center.latitude,
        searchArea.center.longitude,
        searchArea.radiusMiles,
        callback
    );
}

std::vector<Models::BusinessInfo> GooglePlacesAPI::searchBusinessesSync(
    const Models::SearchArea& searchArea
) {
    return searchCateringProspectsSync(
        searchArea.center.latitude,
        searchArea.center.longitude,
        searchArea.radiusMiles
    );
}

void GooglePlacesAPI::textSearch(const std::string& query, PlacesCallback callback) {
    threadPool_->execute([this, query, callback]() {
        auto results = textSearchSync(query);
        if (callback) {
            callback(results, results.empty() ? "No places found" : "");
        }
    });
}

std::vector<GooglePlace> GooglePlacesAPI::textSearchSync(const std::string& query) {
    if (!config_.isConfigured()) {
        return {};
    }

    stats_.totalRequests++;

    std::string url = buildTextSearchUrl(query);
    std::string response = executeHttpRequest(url);

    if (response.empty()) {
        stats_.failedRequests++;
        return {};
    }

    std::string nextPageToken;
    auto places = parseNearbySearchResponse(response, nextPageToken);

    if (!places.empty()) {
        stats_.successfulRequests++;
    }

    return places;
}

void GooglePlacesAPI::getPlaceDetails(const std::string& placeId, PlaceDetailsCallback callback) {
    threadPool_->execute([this, placeId, callback]() {
        auto result = getPlaceDetailsSync(placeId);
        if (callback) {
            callback(result, result.name.empty() ? "Place not found" : "");
        }
    });
}

GooglePlace GooglePlacesAPI::getPlaceDetailsSync(const std::string& placeId) {
    if (!config_.isConfigured()) {
        return GooglePlace();
    }

    // Check cache
    {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        auto it = detailsCache_.find(placeId);
        if (it != detailsCache_.end()) {
            time_t now = std::time(nullptr);
            if (now - it->second.second < config_.cacheDurationMinutes * 60) {
                stats_.cacheHits++;
                return it->second.first;
            }
        }
        stats_.cacheMisses++;
    }

    stats_.totalRequests++;

    std::string url = buildDetailsUrl(placeId);
    std::string response = executeHttpRequest(url);

    if (response.empty()) {
        stats_.failedRequests++;
        return GooglePlace();
    }

    auto place = parseDetailsResponse(response);
    place.placeId = placeId;

    if (!place.name.empty()) {
        stats_.successfulRequests++;

        // Cache result
        std::lock_guard<std::mutex> lock(cacheMutex_);
        detailsCache_[placeId] = {place, std::time(nullptr)};
    }

    return place;
}

Models::BusinessInfo GooglePlacesAPI::placeToBusinessInfo(const GooglePlace& place) {
    Models::BusinessInfo business;

    business.id = "gp_" + place.placeId;
    business.name = place.name;
    business.source = Models::DataSource::GOOGLE_MY_BUSINESS;
    business.type = place.inferBusinessType();

    // Address
    business.address.street1 = place.formattedAddress;
    business.address.latitude = place.latitude;
    business.address.longitude = place.longitude;

    // Contact
    business.contact.primaryPhone = place.phoneNumber;
    business.contact.website = place.website;

    // Rating
    business.googleRating = place.rating;
    business.googleReviewCount = place.userRatingsTotal;

    // Set catering-relevant flags based on type
    switch (business.type) {
        case Models::BusinessType::CORPORATE_OFFICE:
        case Models::BusinessType::TECH_COMPANY:
        case Models::BusinessType::FINANCIAL_SERVICES:
        case Models::BusinessType::COWORKING_SPACE:
            business.hasConferenceRoom = true;
            business.regularMeetings = true;
            break;
        case Models::BusinessType::CONFERENCE_CENTER:
        case Models::BusinessType::HOTEL:
            business.hasConferenceRoom = true;
            business.hasEventSpace = true;
            business.regularMeetings = true;
            break;
        case Models::BusinessType::MEDICAL_FACILITY:
        case Models::BusinessType::EDUCATIONAL_INSTITUTION:
        case Models::BusinessType::GOVERNMENT_OFFICE:
            business.regularMeetings = true;
            break;
        default:
            break;
    }

    // Calculate catering potential
    business.calculateCateringPotential();

    // Metadata
    business.dateAdded = std::time(nullptr);
    business.lastUpdated = std::time(nullptr);
    business.isVerified = true;

    return business;
}

void GooglePlacesAPI::setThreadPoolSize(int threadCount) {
    std::lock_guard<std::mutex> lock(threadPoolMutex_);
    config_.threadPoolSize = std::max(1, threadCount);

    if (threadPool_) {
        threadPool_->resize(config_.threadPoolSize);
    }
}

int GooglePlacesAPI::getThreadPoolSize() const {
    return config_.threadPoolSize;
}

int GooglePlacesAPI::getRecommendedMemoryMB() const {
    return ThreadPoolConfig::getRecommendedMemoryMB(config_.threadPoolSize);
}

const ThreadPoolMetrics& GooglePlacesAPI::getThreadPoolMetrics() const {
    return threadPool_->getMetrics();
}

void GooglePlacesAPI::clearCache() {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    searchCache_.clear();
    detailsCache_.clear();
}

int GooglePlacesAPI::getCacheSize() const {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    return static_cast<int>(searchCache_.size() + detailsCache_.size());
}

} // namespace Services
} // namespace FranchiseAI
