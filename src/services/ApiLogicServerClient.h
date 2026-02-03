#ifndef API_LOGIC_SERVER_CLIENT_H
#define API_LOGIC_SERVER_CLIENT_H

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include "models/Franchisee.h"
#include "AppConfig.h"

namespace FranchiseAI {
namespace Services {

/**
 * @brief App config entry from database
 */
struct AppConfigEntry {
    std::string id;           // UUID for PATCH updates
    std::string configKey;
    std::string configValue;
    std::string configType;
    std::string category;
};

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

    // Search criteria (persisted to database)
    std::string targetBusinessTypes;  // Comma-separated list of business type IDs
    int minEmployees = 0;
    int maxEmployees = 100000;
    bool includeOpenStreetMap = true;
    bool includeGooglePlaces = false;
    bool includeBBB = false;

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

        // Search criteria
        dto.minEmployees = f.searchCriteria.minEmployees;
        dto.maxEmployees = f.searchCriteria.maxEmployees;
        dto.includeOpenStreetMap = f.searchCriteria.includeOpenStreetMap;
        dto.includeGooglePlaces = f.searchCriteria.includeGooglePlaces;
        dto.includeBBB = f.searchCriteria.includeBBB;

        // Convert business types to comma-separated string
        std::string types;
        for (const auto& bt : f.searchCriteria.businessTypes) {
            if (!types.empty()) types += ",";
            types += std::to_string(static_cast<int>(bt));
        }
        dto.targetBusinessTypes = types;

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
        f.location.isValid = true;
        f.defaultSearchRadiusMiles = defaultSearchRadiusMiles;
        f.phone = phone;
        f.email = email;
        f.isConfigured = true;

        // Search criteria
        f.searchCriteria.radiusMiles = defaultSearchRadiusMiles;
        f.searchCriteria.minEmployees = minEmployees;
        f.searchCriteria.maxEmployees = maxEmployees;
        f.searchCriteria.includeOpenStreetMap = includeOpenStreetMap;
        f.searchCriteria.includeGooglePlaces = includeGooglePlaces;
        f.searchCriteria.includeBBB = includeBBB;

        // Parse business types from comma-separated string
        if (!targetBusinessTypes.empty()) {
            f.searchCriteria.clearBusinessTypes();
            std::string types = targetBusinessTypes;
            size_t pos = 0;
            while ((pos = types.find(',')) != std::string::npos || !types.empty()) {
                std::string token;
                if (pos != std::string::npos) {
                    token = types.substr(0, pos);
                    types.erase(0, pos + 1);
                } else {
                    token = types;
                    types.clear();
                }
                try {
                    int typeInt = std::stoi(token);
                    f.searchCriteria.addBusinessType(static_cast<Models::BusinessType>(typeInt));
                } catch (...) {}
            }
        }

        return f;
    }

    // Convert to JSON string for API
    std::string toJson() const;

    // Parse from JSON response
    static StoreLocationDTO fromJson(const std::string& json);
};

/**
 * @brief Franchisee data for API communication
 */
struct FranchiseeDTO {
    std::string id;              // UUID from database
    std::string businessName;
    std::string dbaName;
    std::string franchiseNumber;
    std::string ownerFirstName;
    std::string ownerLastName;
    std::string email;
    std::string phone;
    std::string addressLine1;
    std::string addressLine2;
    std::string city;
    std::string stateProvince;
    std::string postalCode;
    std::string countryCode = "US";
    double latitude = 0.0;
    double longitude = 0.0;
    bool isActive = true;

    // Convert to JSON string for API
    std::string toJson() const;

    // Parse from JSON response
    static FranchiseeDTO fromJson(const std::string& json);
};

/**
 * @brief Scoring rule data for API communication
 */
struct ScoringRuleDTO {
    std::string id;              // UUID from database
    std::string ruleId;          // Rule identifier (e.g., 'no_address')
    std::string name;            // Display name
    std::string description;     // Rule description
    bool isPenalty = false;      // True for penalties, false for bonuses
    bool enabled = true;         // Whether rule is active
    int defaultPoints = 0;       // Default point adjustment
    int currentPoints = 0;       // Current configured adjustment
    int minPoints = -50;         // Minimum allowed value
    int maxPoints = 50;          // Maximum allowed value
    std::string franchiseeId;    // Optional: rule belongs to specific franchisee

    // Convert to JSON string for API
    std::string toJson() const;

    // Parse from JSON response
    static ScoringRuleDTO fromJson(const std::string& json);
};

/**
 * @brief Saved prospect data for API communication
 * Links a prospect (business) to a franchisee
 */
struct SavedProspectDTO {
    std::string id;                  // UUID from database
    std::string storeLocationId;     // FK to store_locations (current franchisee's store)
    std::string businessName;
    std::string businessCategory;
    std::string addressLine1;
    std::string addressLine2;
    std::string city;
    std::string stateProvince;
    std::string postalCode;
    std::string countryCode = "US";
    double latitude = 0.0;
    double longitude = 0.0;
    std::string phone;
    std::string email;
    std::string website;
    int employeeCount = 0;
    int cateringPotentialScore = 0;
    double relevanceScore = 0.0;
    double distanceMiles = 0.0;
    std::string aiSummary;
    std::string matchReason;
    std::string keyHighlights;       // Comma-separated list
    std::string recommendedActions;  // Comma-separated list
    std::string dataSource;          // e.g., "OpenStreetMap", "GooglePlaces"
    std::string savedAt;             // ISO timestamp
    bool isContacted = false;
    bool isConverted = false;
    std::string notes;

    // Convert to JSON string for API
    std::string toJson() const;

    // Parse from JSON response
    static SavedProspectDTO fromJson(const std::string& json);
};

/**
 * @brief Data transfer object for Prospect entity (Franchisee's prospect list)
 */
struct ProspectDTO {
    std::string id;                    // UUID (client-generated for new records)
    std::string territoryId;           // FK to territory
    std::string franchiseeId;          // FK to franchisee
    std::string assignedToUserId;      // FK to user
    std::string businessName;
    std::string dbaName;               // "Doing Business As" name
    std::string legalName;
    std::string industryId;            // FK to industry
    std::string industryNaics;         // NAICS code
    std::string businessType;
    int employeeCount = 0;
    std::string employeeCountRange;    // e.g., "1-50", "51-200"
    double annualRevenue = 0.0;
    int yearEstablished = 0;
    std::string addressLine1;
    std::string addressLine2;
    std::string city;
    std::string stateProvince;
    std::string postalCode;
    std::string countryCode = "US";
    double latitude = 0.0;
    double longitude = 0.0;
    std::string geocodeAccuracy;
    std::string primaryPhone;
    std::string secondaryPhone;
    std::string email;
    std::string website;
    std::string linkedinUrl;
    std::string facebookUrl;
    std::string status = "new";        // new, contacted, qualified, converted, etc.
    std::string statusChangedAt;
    std::string dataSource;            // e.g., "OpenStreetMap", "GooglePlaces"
    std::string sourceRecordId;        // Original ID from data source
    bool isVerified = false;
    bool isDuplicate = false;
    std::string duplicateOfId;
    bool doNotContact = false;
    std::string createdAt;
    std::string updatedAt;

    // AI and scoring fields (for My Prospects display)
    int aiScore = 0;                   // Original AI-derived score (0-100)
    int optimizedScore = 0;            // Score after scoring rules applied (0-100)
    std::string aiSummary;             // AI-generated summary text
    std::string keyHighlights;         // Comma-separated list of highlights
    std::string recommendedActions;    // Comma-separated list of actions
    double relevanceScore = 0.0;       // AI relevance score (0.0-1.0)
    std::string dataSources;           // Comma-separated list of data sources

    // Convert to JSON string for API (JSON:API format)
    std::string toJson() const;

    // Parse from JSON response
    static ProspectDTO fromJson(const std::string& json);
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

    // ========================================================================
    // Franchisee Operations
    // ========================================================================

    /**
     * @brief Save a franchisee (create or update)
     * @param franchisee Franchisee data
     * @return API response with created/updated record
     */
    ApiResponse saveFranchisee(const FranchiseeDTO& franchisee);

    /**
     * @brief Get all franchisees
     * @return API response with list of franchisees
     */
    ApiResponse getFranchisees();

    /**
     * @brief Get a specific franchisee by ID
     * @param id UUID of the franchisee
     * @return API response with franchisee data
     */
    ApiResponse getFranchisee(const std::string& id);

    /**
     * @brief Delete a franchisee
     * @param id UUID of the franchisee
     * @return API response
     */
    ApiResponse deleteFranchisee(const std::string& id);

    /**
     * @brief Parse franchisees from API response
     * @param response API response containing JSON array
     * @return Vector of franchisee DTOs
     */
    static std::vector<FranchiseeDTO> parseFranchisees(const ApiResponse& response);

    // ========================================================================
    // Scoring Rule Operations
    // ========================================================================

    /**
     * @brief Save a scoring rule (create or update)
     * @param rule Scoring rule data
     * @return API response with created/updated record
     */
    ApiResponse saveScoringRule(const ScoringRuleDTO& rule);

    /**
     * @brief Get all scoring rules
     * @return API response with list of scoring rules
     */
    ApiResponse getScoringRules();

    /**
     * @brief Get scoring rules for a specific franchisee
     * @param franchiseeId UUID of the franchisee (empty for global rules)
     * @return API response with list of scoring rules
     */
    ApiResponse getScoringRulesForFranchisee(const std::string& franchiseeId = "");

    /**
     * @brief Get a specific scoring rule by ID
     * @param id UUID of the scoring rule
     * @return API response with scoring rule data
     */
    ApiResponse getScoringRule(const std::string& id);

    /**
     * @brief Delete a scoring rule
     * @param id UUID of the scoring rule
     * @return API response
     */
    ApiResponse deleteScoringRule(const std::string& id);

    /**
     * @brief Parse scoring rules from API response
     * @param response API response containing JSON array
     * @return Vector of scoring rule DTOs
     */
    static std::vector<ScoringRuleDTO> parseScoringRules(const ApiResponse& response);

    // ========================================================================
    // Saved Prospect Operations
    // ========================================================================

    /**
     * @brief Save a prospect (create or update)
     * @param prospect Saved prospect data
     * @return API response with created/updated record
     */
    ApiResponse saveProspect(const SavedProspectDTO& prospect);

    /**
     * @brief Get all saved prospects for a store location
     * @param storeLocationId UUID of the store location (franchisee)
     * @return API response with list of saved prospects
     */
    ApiResponse getProspectsForStore(const std::string& storeLocationId);

    /**
     * @brief Get all prospects for a franchisee using the ProspectList relationship
     * @param franchiseeId UUID of the franchisee
     * @param offset Pagination offset (default 0)
     * @param limit Pagination limit (default 50)
     * @return API response with list of prospects
     */
    ApiResponse getProspectsForFranchisee(const std::string& franchiseeId, int offset = 0, int limit = 50);

    /**
     * @brief Save a prospect (create new or update existing)
     * @param prospect Prospect data
     * @return API response
     */
    ApiResponse saveProspect(const ProspectDTO& prospect);

    /**
     * @brief Get a specific prospect by ID
     * @param id UUID of the prospect
     * @return API response with prospect data
     */
    ApiResponse getProspect(const std::string& id);

    /**
     * @brief Delete a prospect
     * @param id UUID of the prospect
     * @return API response
     */
    ApiResponse deleteProspect(const std::string& id);

    /**
     * @brief Parse prospects from API response
     * @param response API response containing JSON array
     * @return Vector of prospect DTOs
     */
    static std::vector<ProspectDTO> parseProspects(const ApiResponse& response);

    /**
     * @brief Get a specific saved prospect by ID
     * @param id UUID of the saved prospect
     * @return API response with prospect data
     */
    ApiResponse getSavedProspect(const std::string& id);

    /**
     * @brief Delete a saved prospect
     * @param id UUID of the saved prospect
     * @return API response
     */
    ApiResponse deleteSavedProspect(const std::string& id);

    /**
     * @brief Parse saved prospects from API response
     * @param response API response containing JSON array
     * @return Vector of saved prospect DTOs
     */
    static std::vector<SavedProspectDTO> parseSavedProspects(const ApiResponse& response);

    // ========================================================================
    // App Config Operations
    // ========================================================================

    /**
     * @brief Load all app configs into memory cache
     * Must be called before using getAppConfigValue/setAppConfigValue
     */
    void loadAppConfigs();

    /**
     * @brief Get app config value by key (from cache)
     * @param key Config key (e.g., "current_store_id")
     * @return Config value or empty string if not found
     */
    std::string getAppConfigValue(const std::string& key);

    /**
     * @brief Set app config value (updates cache and server)
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

    /**
     * @brief Set app config value in cache (shorthand for setAppConfigValue)
     */
    void setAppConfig(const std::string& key, const std::string& value) {
        setAppConfigValue(key, value);
    }

    // ========================================================================
    // Generic Resource Operations (for Auth and other services)
    // ========================================================================

    /**
     * @brief Get a resource by type and optional ID
     * @param resourceType The API resource type (e.g., "User", "UserSession")
     * @param id Optional resource ID for specific record
     * @param filter Optional filter query (e.g., "email=test@example.com")
     * @return JSON response body
     */
    std::string getResource(const std::string& resourceType, const std::string& id = "", const std::string& filter = "");

    /**
     * @brief Create a new resource
     * @param resourceType The API resource type
     * @param jsonBody JSON body for the new resource
     * @return JSON response body
     */
    std::string createResource(const std::string& resourceType, const std::string& jsonBody);

    /**
     * @brief Update an existing resource
     * @param resourceType The API resource type
     * @param id Resource ID to update
     * @param jsonBody JSON body with updates
     * @return JSON response body
     */
    std::string updateResource(const std::string& resourceType, const std::string& id, const std::string& jsonBody);

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

    /**
     * @brief In-memory cache of app config entries
     * Key: config_key, Value: AppConfigEntry (includes ID for updates)
     */
    std::unordered_map<std::string, AppConfigEntry> appConfigCache_;
};

} // namespace Services
} // namespace FranchiseAI

#endif // API_LOGIC_SERVER_CLIENT_H
