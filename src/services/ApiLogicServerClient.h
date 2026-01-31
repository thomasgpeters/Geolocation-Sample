#ifndef API_LOGIC_SERVER_CLIENT_H
#define API_LOGIC_SERVER_CLIENT_H

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include "models/Franchisee.h"
#include "AppConfig.h"

namespace FranchiseAI {
namespace Services {

/**
 * @brief Store location data for API communication
 */
struct StoreLocationDTO {
    std::string id;              // UUID from database
    std::string franchiseeId;
    std::string storeName;
    std::string storeCode;
    std::string addressLine1;
    std::string addressLine2;
    std::string city;
    std::string stateProvince;
    std::string postalCode;
    std::string countryCode = "US";
    double latitude = 0.0;
    double longitude = 0.0;
    std::string geocodeSource;
    double defaultSearchRadiusMiles = 5.0;
    std::string phone;
    std::string email;
    bool isActive = true;
    bool isPrimary = false;

    // Convert from Franchisee model
    static StoreLocationDTO fromFranchisee(const Models::Franchisee& f) {
        StoreLocationDTO dto;
        dto.storeName = f.storeName;
        dto.addressLine1 = f.address;
        dto.city = f.location.city;
        dto.stateProvince = f.location.state;
        dto.postalCode = f.location.postalCode;
        dto.latitude = f.location.latitude;
        dto.longitude = f.location.longitude;
        dto.defaultSearchRadiusMiles = f.defaultSearchRadiusMiles;
        dto.phone = f.phone;
        dto.email = f.email;
        dto.geocodeSource = "nominatim";
        dto.isPrimary = true;
        return dto;
    }

    // Convert to Franchisee model
    Models::Franchisee toFranchisee() const {
        Models::Franchisee f;
        f.storeId = id;
        f.storeName = storeName;
        f.address = addressLine1;
        f.location.city = city;
        f.location.state = stateProvince;
        f.location.postalCode = postalCode;
        f.location.latitude = latitude;
        f.location.longitude = longitude;
        f.defaultSearchRadiusMiles = defaultSearchRadiusMiles;
        f.phone = phone;
        f.email = email;
        f.isConfigured = true;
        return f;
    }

    // Convert to JSON string for API
    std::string toJson() const;

    // Parse from JSON response
    static StoreLocationDTO fromJson(const std::string& json);
};

/**
 * @brief API response wrapper
 */
struct ApiResponse {
    bool success = false;
    int statusCode = 0;
    std::string body;
    std::string errorMessage;
};

/**
 * @brief Client for communicating with ApiLogicServer
 *
 * Provides CRUD operations for store_locations and other entities
 * through the REST API exposed by ApiLogicServer.
 */
class ApiLogicServerClient {
public:
    ApiLogicServerClient();
    ~ApiLogicServerClient();

    /**
     * @brief Save a store location (create or update)
     * @param location Store location data
     * @return API response with created/updated record
     */
    ApiResponse saveStoreLocation(const StoreLocationDTO& location);

    /**
     * @brief Get all store locations
     * @return API response with list of store locations
     */
    ApiResponse getStoreLocations();

    /**
     * @brief Get a specific store location by ID
     * @param id UUID of the store location
     * @return API response with store location data
     */
    ApiResponse getStoreLocation(const std::string& id);

    /**
     * @brief Delete a store location
     * @param id UUID of the store location
     * @return API response
     */
    ApiResponse deleteStoreLocation(const std::string& id);

    /**
     * @brief Parse store locations from API response
     * @param response API response containing JSON array
     * @return Vector of store location DTOs
     */
    static std::vector<StoreLocationDTO> parseStoreLocations(const ApiResponse& response);

    /**
     * @brief Get app config value by key
     * @param key Config key (e.g., "current_store_id")
     * @return Config value or empty string if not found
     */
    std::string getAppConfigValue(const std::string& key);

    /**
     * @brief Set app config value
     * @param key Config key
     * @param value Config value
     * @return true if successful
     */
    bool setAppConfigValue(const std::string& key, const std::string& value);

    /**
     * @brief Check if API is reachable
     * @return true if API responds successfully
     */
    bool isAvailable();

    /**
     * @brief Get the base endpoint URL
     */
    std::string getEndpoint() const;

private:
    /**
     * @brief Make HTTP GET request
     */
    ApiResponse httpGet(const std::string& path);

    /**
     * @brief Make HTTP POST request
     */
    ApiResponse httpPost(const std::string& path, const std::string& body);

    /**
     * @brief Make HTTP PATCH request
     */
    ApiResponse httpPatch(const std::string& path, const std::string& body);

    /**
     * @brief Make HTTP DELETE request
     */
    ApiResponse httpDelete(const std::string& path);

    /**
     * @brief CURL write callback
     */
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
};

} // namespace Services
} // namespace FranchiseAI

#endif // API_LOGIC_SERVER_CLIENT_H
