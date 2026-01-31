#include "ApiLogicServerClient.h"
#include <curl/curl.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <random>

namespace FranchiseAI {
namespace Services {

// ============================================================================
// UUID Generation Helper
// ============================================================================

/**
 * @brief Generate a UUID v4 string
 * Format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
 * where x is any hex digit and y is one of 8, 9, a, or b
 */
static std::string generateUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;

    for (int i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4";  // UUID version 4
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << dis2(gen);  // Variant bits (8, 9, a, or b)
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; i++) ss << dis(gen);

    return ss.str();
}

// ============================================================================
// StoreLocationDTO implementation
// ============================================================================

std::string StoreLocationDTO::toJson() const {
    std::ostringstream json;
    // Wrap in JSON:API format for ApiLogicServer
    // Order: attributes first, then type, then id (for PATCH)
    json << "{\"data\": {\"attributes\": {";
    json << "\"store_name\": \"" << storeName << "\"";

    if (!storeCode.empty()) {
        json << ", \"store_code\": \"" << storeCode << "\"";
    }
    if (!addressLine1.empty()) {
        json << ", \"address_line1\": \"" << addressLine1 << "\"";
    }
    if (!addressLine2.empty()) {
        json << ", \"address_line2\": \"" << addressLine2 << "\"";
    }
    // Always include city/state/postal - database may require them
    json << ", \"city\": \"" << city << "\"";
    json << ", \"state_province\": \"" << stateProvince << "\"";
    json << ", \"postal_code\": \"" << postalCode << "\"";
    json << ", \"country_code\": \"" << countryCode << "\"";

    if (latitude != 0.0 || longitude != 0.0) {
        json << ", \"latitude\": " << latitude;
        json << ", \"longitude\": " << longitude;
    }
    if (!geocodeSource.empty()) {
        json << ", \"geocode_source\": \"" << geocodeSource << "\"";
    }

    json << ", \"default_search_radius_miles\": " << defaultSearchRadiusMiles;

    if (!phone.empty()) {
        json << ", \"phone\": \"" << phone << "\"";
    }
    if (!email.empty()) {
        json << ", \"email\": \"" << email << "\"";
    }

    json << ", \"is_active\": " << (isActive ? "true" : "false");
    json << ", \"is_primary\": " << (isPrimary ? "true" : "false");

    json << "}, \"type\": \"StoreLocation\"";  // Close attributes, add type
    if (!id.empty()) {
        json << ", \"id\": \"" << id << "\"";
    }
    json << "}}";  // Close data and root
    return json.str();
}

// Simple JSON string extraction helper
static std::string extractJsonString(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return "";

    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) return "";

    size_t valueStart = json.find_first_not_of(" \t\n\r", colonPos + 1);
    if (valueStart == std::string::npos) return "";

    if (json[valueStart] == '"') {
        size_t valueEnd = json.find('"', valueStart + 1);
        if (valueEnd != std::string::npos) {
            return json.substr(valueStart + 1, valueEnd - valueStart - 1);
        }
    } else if (json[valueStart] == 'n' && json.substr(valueStart, 4) == "null") {
        return "";
    } else {
        // Numeric or boolean value
        size_t valueEnd = json.find_first_of(",}]", valueStart);
        if (valueEnd != std::string::npos) {
            std::string value = json.substr(valueStart, valueEnd - valueStart);
            // Trim whitespace
            size_t end = value.find_last_not_of(" \t\n\r");
            if (end != std::string::npos) {
                value = value.substr(0, end + 1);
            }
            return value;
        }
    }
    return "";
}

StoreLocationDTO StoreLocationDTO::fromJson(const std::string& json) {
    StoreLocationDTO dto;

    dto.id = extractJsonString(json, "id");
    dto.franchiseeId = extractJsonString(json, "franchisee_id");
    dto.storeName = extractJsonString(json, "store_name");
    dto.storeCode = extractJsonString(json, "store_code");
    dto.addressLine1 = extractJsonString(json, "address_line1");
    dto.addressLine2 = extractJsonString(json, "address_line2");
    dto.city = extractJsonString(json, "city");
    dto.stateProvince = extractJsonString(json, "state_province");
    dto.postalCode = extractJsonString(json, "postal_code");
    dto.countryCode = extractJsonString(json, "country_code");
    if (dto.countryCode.empty()) dto.countryCode = "US";

    std::string latStr = extractJsonString(json, "latitude");
    std::string lngStr = extractJsonString(json, "longitude");
    if (!latStr.empty()) {
        try { dto.latitude = std::stod(latStr); } catch (...) {}
    }
    if (!lngStr.empty()) {
        try { dto.longitude = std::stod(lngStr); } catch (...) {}
    }

    dto.geocodeSource = extractJsonString(json, "geocode_source");

    std::string radiusStr = extractJsonString(json, "default_search_radius_miles");
    if (!radiusStr.empty()) {
        try { dto.defaultSearchRadiusMiles = std::stod(radiusStr); } catch (...) {}
    }

    dto.phone = extractJsonString(json, "phone");
    dto.email = extractJsonString(json, "email");

    std::string activeStr = extractJsonString(json, "is_active");
    dto.isActive = (activeStr == "true" || activeStr == "1");

    std::string primaryStr = extractJsonString(json, "is_primary");
    dto.isPrimary = (primaryStr == "true" || primaryStr == "1");

    return dto;
}

// ============================================================================
// ApiLogicServerClient implementation
// ============================================================================

ApiLogicServerClient::ApiLogicServerClient() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

ApiLogicServerClient::~ApiLogicServerClient() {
    // Note: Don't call curl_global_cleanup here as other services may use curl
}

size_t ApiLogicServerClient::writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string ApiLogicServerClient::getEndpoint() const {
    return AppConfig::instance().getApiLogicServerEndpoint();
}

ApiResponse ApiLogicServerClient::httpGet(const std::string& path) {
    ApiResponse response;
    CURL* curl = curl_easy_init();

    if (!curl) {
        response.errorMessage = "Failed to initialize CURL";
        return response;
    }

    std::string url = getEndpoint() + path;
    std::string responseBody;

    std::cout << "  [ALS] GET " << url << std::endl;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        response.errorMessage = curl_easy_strerror(res);
        std::cerr << "  [ALS] GET failed: " << response.errorMessage << std::endl;
    } else {
        long httpCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        response.statusCode = static_cast<int>(httpCode);
        response.body = responseBody;
        response.success = (httpCode >= 200 && httpCode < 300);
        std::cout << "  [ALS] Response: " << httpCode << " (" << responseBody.length() << " bytes)" << std::endl;
        std::cout << "  [ALS] Body: " << responseBody << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

ApiResponse ApiLogicServerClient::httpPost(const std::string& path, const std::string& body) {
    ApiResponse response;
    CURL* curl = curl_easy_init();

    if (!curl) {
        response.errorMessage = "Failed to initialize CURL";
        return response;
    }

    std::string url = getEndpoint() + path;
    std::string responseBody;

    std::cout << "  [ALS] POST " << url << std::endl;
    std::cout << "  [ALS] Body: " << body << std::endl;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/vnd.api+json");
    headers = curl_slist_append(headers, "Accept: application/vnd.api+json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        response.errorMessage = curl_easy_strerror(res);
        std::cerr << "  [ALS] POST failed: " << response.errorMessage << std::endl;
    } else {
        long httpCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        response.statusCode = static_cast<int>(httpCode);
        response.body = responseBody;
        response.success = (httpCode >= 200 && httpCode < 300);
        std::cout << "  [ALS] Response: " << httpCode << std::endl;
        if (!response.success) {
            std::cerr << "  [ALS] Error body: " << responseBody << std::endl;
        } else {
            std::cout << "  [ALS] Success body: " << responseBody << std::endl;
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

ApiResponse ApiLogicServerClient::httpPatch(const std::string& path, const std::string& body) {
    ApiResponse response;
    CURL* curl = curl_easy_init();

    if (!curl) {
        response.errorMessage = "Failed to initialize CURL";
        return response;
    }

    std::string url = getEndpoint() + path;
    std::string responseBody;

    std::cout << "  [ALS] PATCH " << url << std::endl;
    std::cout << "  [ALS] Body: " << body << std::endl;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/vnd.api+json");
    headers = curl_slist_append(headers, "Accept: application/vnd.api+json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        response.errorMessage = curl_easy_strerror(res);
        std::cerr << "  [ALS] PATCH failed: " << response.errorMessage << std::endl;
    } else {
        long httpCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        response.statusCode = static_cast<int>(httpCode);
        response.body = responseBody;
        response.success = (httpCode >= 200 && httpCode < 300);
        std::cout << "  [ALS] Response: " << httpCode << std::endl;
        if (!response.success) {
            std::cerr << "  [ALS] Error body: " << responseBody << std::endl;
        } else {
            std::cout << "  [ALS] Success body: " << responseBody << std::endl;
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

ApiResponse ApiLogicServerClient::httpDelete(const std::string& path) {
    ApiResponse response;
    CURL* curl = curl_easy_init();

    if (!curl) {
        response.errorMessage = "Failed to initialize CURL";
        return response;
    }

    std::string url = getEndpoint() + path;
    std::string responseBody;

    std::cout << "  [ALS] DELETE " << url << std::endl;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        response.errorMessage = curl_easy_strerror(res);
    } else {
        long httpCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        response.statusCode = static_cast<int>(httpCode);
        response.body = responseBody;
        response.success = (httpCode >= 200 && httpCode < 300);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

ApiResponse ApiLogicServerClient::saveStoreLocation(const StoreLocationDTO& location) {
    if (location.id.empty()) {
        // Create new record - generate UUID client-side
        StoreLocationDTO newLocation = location;
        newLocation.id = generateUUID();
        std::string json = newLocation.toJson();

        std::cout << "  [ALS] Creating new StoreLocation with generated UUID: " << newLocation.id << std::endl;
        return httpPost("/StoreLocation", json);
    } else {
        // Update existing record (from dropdown selection)
        std::string json = location.toJson();
        std::cout << "  [ALS] Updating existing StoreLocation: " << location.id << std::endl;
        return httpPatch("/StoreLocation/" + location.id, json);
    }
}

ApiResponse ApiLogicServerClient::getStoreLocations() {
    return httpGet("/StoreLocation");
}

ApiResponse ApiLogicServerClient::getStoreLocation(const std::string& id) {
    if (id.empty()) {
        ApiResponse response;
        response.success = false;
        response.statusCode = 400;
        response.errorMessage = "Store location ID cannot be empty";
        return response;
    }
    return httpGet("/StoreLocation/" + id);
}

ApiResponse ApiLogicServerClient::deleteStoreLocation(const std::string& id) {
    if (id.empty()) {
        ApiResponse response;
        response.success = false;
        response.statusCode = 400;
        response.errorMessage = "Store location ID cannot be empty";
        return response;
    }
    return httpDelete("/StoreLocation/" + id);
}

std::vector<StoreLocationDTO> ApiLogicServerClient::parseStoreLocations(const ApiResponse& response) {
    std::vector<StoreLocationDTO> locations;

    if (!response.success || response.body.empty()) {
        return locations;
    }

    // Simple parsing - look for objects in the response
    // ApiLogicServer typically returns { "data": [...] } or just [...]
    const std::string& json = response.body;

    // Find the array start (could be after "data": or direct)
    size_t arrayStart = json.find('[');
    if (arrayStart == std::string::npos) {
        // Single object?
        if (json.find('{') != std::string::npos) {
            locations.push_back(StoreLocationDTO::fromJson(json));
        }
        return locations;
    }

    // Parse array of objects
    size_t pos = arrayStart + 1;
    int braceCount = 0;
    size_t objStart = std::string::npos;

    while (pos < json.length()) {
        char c = json[pos];

        if (c == '{') {
            if (braceCount == 0) {
                objStart = pos;
            }
            braceCount++;
        } else if (c == '}') {
            braceCount--;
            if (braceCount == 0 && objStart != std::string::npos) {
                std::string objJson = json.substr(objStart, pos - objStart + 1);
                locations.push_back(StoreLocationDTO::fromJson(objJson));
                objStart = std::string::npos;
            }
        } else if (c == ']' && braceCount == 0) {
            break;
        }
        pos++;
    }

    return locations;
}

bool ApiLogicServerClient::isAvailable() {
    auto response = httpGet("/");
    return response.success || response.statusCode == 404;  // 404 is ok, means server is up
}

void ApiLogicServerClient::loadAppConfigs() {
    std::cout << "  [ALS] Loading all AppConfig entries..." << std::endl;
    appConfigCache_.clear();

    auto response = httpGet("/AppConfig");
    if (!response.success || response.body.empty()) {
        std::cerr << "  [ALS] Failed to load AppConfig" << std::endl;
        return;
    }

    // Parse the JSON array of config entries
    // JSON:API format has: {"data": [{"attributes": {..., "config_key": "...", ...}, "id": "...", "type": "AppConfig"}, ...]}
    // Important: In JSON:API, "attributes" comes BEFORE "id" in the JSON string (alphabetical order)
    // So we need to find config_key first, then find the id that comes AFTER it in the same record
    const std::string& json = response.body;
    size_t pos = 0;

    while (pos < json.length()) {
        // Find next "config_key" field (indicates we're in an attributes block)
        size_t keyPos = json.find("\"config_key\"", pos);
        if (keyPos == std::string::npos) break;

        AppConfigEntry entry;

        // Extract config_key
        size_t ckStart = json.find("\"", keyPos + 12);
        size_t ckEnd = json.find("\"", ckStart + 1);
        if (ckStart != std::string::npos && ckEnd != std::string::npos) {
            entry.configKey = json.substr(ckStart + 1, ckEnd - ckStart - 1);
        }

        // Extract config_value (comes after config_key in the same attributes block)
        size_t cvPos = json.find("\"config_value\"", keyPos);
        if (cvPos != std::string::npos && cvPos < keyPos + 500) {
            size_t cvStart = json.find("\"", cvPos + 14);
            size_t cvEnd = json.find("\"", cvStart + 1);
            if (cvStart != std::string::npos && cvEnd != std::string::npos) {
                entry.configValue = json.substr(cvStart + 1, cvEnd - cvStart - 1);
            }
        }

        // Find the "id" field that comes AFTER the attributes block (same record)
        // The "id" is at the same level as "attributes", so it comes after the attributes block closes
        size_t idPos = json.find("\"id\"", keyPos);
        if (idPos != std::string::npos) {
            // Make sure we don't go past the next record's config_key
            size_t nextKeyPos = json.find("\"config_key\"", keyPos + 1);
            if (nextKeyPos == std::string::npos || idPos < nextKeyPos) {
                size_t idStart = json.find("\"", idPos + 4);
                size_t idEnd = json.find("\"", idStart + 1);
                if (idStart != std::string::npos && idEnd != std::string::npos) {
                    entry.id = json.substr(idStart + 1, idEnd - idStart - 1);
                }
            }
        }

        // Add to cache if we got a valid key and id
        if (!entry.configKey.empty() && !entry.id.empty()) {
            appConfigCache_[entry.configKey] = entry;
            std::cout << "  [ALS] Cached: " << std::left << std::setw(25) << entry.configKey
                      << " = '" << entry.configValue << "'\t(id: " << entry.id << ")" << std::endl;
        }

        pos = keyPos + 1;
    }

    std::cout << "  [ALS] Loaded " << appConfigCache_.size() << " config entries" << std::endl;
}

std::string ApiLogicServerClient::getAppConfigValue(const std::string& key) {
    // Look up from cache
    auto it = appConfigCache_.find(key);
    if (it != appConfigCache_.end()) {
        return it->second.configValue;
    }
    return "";
}

bool ApiLogicServerClient::setAppConfigValue(const std::string& key, const std::string& value) {
    std::cout << "  [ALS] setAppConfigValue: key=" << key << ", value=" << value << std::endl;

    // Check if we have this key in cache (meaning it exists in DB)
    auto it = appConfigCache_.find(key);
    if (it != appConfigCache_.end()) {
        // Update existing record using cached ID
        std::string id = it->second.id;
        std::cout << "  [ALS] Found in cache, PATCH using id: " << id << std::endl;

        std::string json = "{\"data\": {\"attributes\": {\"config_value\": \"" + value +
                           "\"}, \"type\": \"AppConfig\", \"id\": \"" + id + "\"}}";
        auto response = httpPatch("/AppConfig/" + id, json);

        if (response.success) {
            // Update cache
            it->second.configValue = value;
        }
        return response.success;
    }

    // Create new config entry
    std::cout << "  [ALS] Not in cache, doing POST" << std::endl;
    std::string json = "{\"data\": {\"attributes\": {"
                       "\"config_key\": \"" + key + "\", "
                       "\"config_value\": \"" + value + "\", "
                       "\"config_type\": \"string\", "
                       "\"category\": \"system\", "
                       "\"description\": \"\", "
                       "\"is_sensitive\": false, "
                       "\"is_required\": false, "
                       "\"default_value\": \"\"}, "
                       "\"type\": \"AppConfig\"}}";
    auto response = httpPost("/AppConfig", json);

    if (response.success) {
        // Add to cache - extract ID from response
        std::string newId = extractJsonString(response.body, "id");
        if (!newId.empty()) {
            AppConfigEntry entry;
            entry.id = newId;
            entry.configKey = key;
            entry.configValue = value;
            appConfigCache_[key] = entry;
            std::cout << "  [ALS] Added to cache with id: " << newId << std::endl;
        }
    }
    return response.success;
}

} // namespace Services
} // namespace FranchiseAI
