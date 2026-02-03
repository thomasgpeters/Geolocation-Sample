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

    if (!franchiseeId.empty()) {
        json << ", \"franchisee_id\": \"" << franchiseeId << "\"";
    }
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

    // Search criteria
    if (!targetBusinessTypes.empty()) {
        json << ", \"target_business_types\": \"" << targetBusinessTypes << "\"";
    }
    json << ", \"min_employees\": " << minEmployees;
    json << ", \"max_employees\": " << maxEmployees;
    json << ", \"include_openstreetmap\": " << (includeOpenStreetMap ? "true" : "false");
    json << ", \"include_google_places\": " << (includeGooglePlaces ? "true" : "false");
    json << ", \"include_bbb\": " << (includeBBB ? "true" : "false");

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

    // Search criteria
    dto.targetBusinessTypes = extractJsonString(json, "target_business_types");

    std::string minEmpStr = extractJsonString(json, "min_employees");
    if (!minEmpStr.empty()) {
        try { dto.minEmployees = std::stoi(minEmpStr); } catch (...) {}
    }

    std::string maxEmpStr = extractJsonString(json, "max_employees");
    if (!maxEmpStr.empty()) {
        try { dto.maxEmployees = std::stoi(maxEmpStr); } catch (...) {}
    }

    std::string osmStr = extractJsonString(json, "include_openstreetmap");
    dto.includeOpenStreetMap = (osmStr.empty() || osmStr == "true" || osmStr == "1");

    std::string googleStr = extractJsonString(json, "include_google_places");
    dto.includeGooglePlaces = (googleStr == "true" || googleStr == "1");

    std::string bbbStr = extractJsonString(json, "include_bbb");
    dto.includeBBB = (bbbStr == "true" || bbbStr == "1");

    return dto;
}

// ============================================================================
// FranchiseeDTO implementation
// ============================================================================

std::string FranchiseeDTO::toJson() const {
    std::ostringstream json;
    // Wrap in JSON:API format for ApiLogicServer
    json << "{\"data\": {\"attributes\": {";
    json << "\"business_name\": \"" << businessName << "\"";

    if (!dbaName.empty()) {
        json << ", \"dba_name\": \"" << dbaName << "\"";
    }
    if (!franchiseNumber.empty()) {
        json << ", \"franchise_number\": \"" << franchiseNumber << "\"";
    }
    if (!ownerFirstName.empty()) {
        json << ", \"owner_first_name\": \"" << ownerFirstName << "\"";
    }
    if (!ownerLastName.empty()) {
        json << ", \"owner_last_name\": \"" << ownerLastName << "\"";
    }
    if (!email.empty()) {
        json << ", \"email\": \"" << email << "\"";
    }
    if (!phone.empty()) {
        json << ", \"phone\": \"" << phone << "\"";
    }
    if (!addressLine1.empty()) {
        json << ", \"address_line1\": \"" << addressLine1 << "\"";
    }
    if (!addressLine2.empty()) {
        json << ", \"address_line2\": \"" << addressLine2 << "\"";
    }
    if (!city.empty()) {
        json << ", \"city\": \"" << city << "\"";
    }
    if (!stateProvince.empty()) {
        json << ", \"state_province\": \"" << stateProvince << "\"";
    }
    if (!postalCode.empty()) {
        json << ", \"postal_code\": \"" << postalCode << "\"";
    }
    json << ", \"country_code\": \"" << countryCode << "\"";

    if (latitude != 0.0 || longitude != 0.0) {
        json << ", \"latitude\": " << latitude;
        json << ", \"longitude\": " << longitude;
    }

    json << ", \"is_active\": " << (isActive ? "true" : "false");

    json << "}, \"type\": \"Franchisee\"";
    if (!id.empty()) {
        json << ", \"id\": \"" << id << "\"";
    }
    json << "}}";
    return json.str();
}

FranchiseeDTO FranchiseeDTO::fromJson(const std::string& json) {
    FranchiseeDTO dto;

    dto.id = extractJsonString(json, "id");
    dto.businessName = extractJsonString(json, "business_name");
    dto.dbaName = extractJsonString(json, "dba_name");
    dto.franchiseNumber = extractJsonString(json, "franchise_number");
    dto.ownerFirstName = extractJsonString(json, "owner_first_name");
    dto.ownerLastName = extractJsonString(json, "owner_last_name");
    dto.email = extractJsonString(json, "email");
    dto.phone = extractJsonString(json, "phone");
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

    std::string activeStr = extractJsonString(json, "is_active");
    dto.isActive = (activeStr == "true" || activeStr == "1");

    return dto;
}

// ============================================================================
// ScoringRuleDTO implementation
// ============================================================================

std::string ScoringRuleDTO::toJson() const {
    std::ostringstream json;
    // Wrap in JSON:API format for ApiLogicServer
    json << "{\"data\": {\"attributes\": {";
    json << "\"rule_id\": \"" << ruleId << "\"";
    json << ", \"name\": \"" << name << "\"";

    if (!description.empty()) {
        json << ", \"description\": \"" << description << "\"";
    }

    json << ", \"is_penalty\": " << (isPenalty ? "true" : "false");
    json << ", \"enabled\": " << (enabled ? "true" : "false");
    json << ", \"default_points\": " << defaultPoints;
    json << ", \"current_points\": " << currentPoints;
    json << ", \"min_points\": " << minPoints;
    json << ", \"max_points\": " << maxPoints;

    if (!franchiseeId.empty()) {
        json << ", \"franchisee_id\": \"" << franchiseeId << "\"";
    }

    json << "}, \"type\": \"ScoringRule\"";
    if (!id.empty()) {
        json << ", \"id\": \"" << id << "\"";
    }
    json << "}}";
    return json.str();
}

ScoringRuleDTO ScoringRuleDTO::fromJson(const std::string& json) {
    ScoringRuleDTO dto;

    dto.id = extractJsonString(json, "id");
    dto.ruleId = extractJsonString(json, "rule_id");
    dto.name = extractJsonString(json, "name");
    dto.description = extractJsonString(json, "description");
    dto.franchiseeId = extractJsonString(json, "franchisee_id");

    std::string penaltyStr = extractJsonString(json, "is_penalty");
    dto.isPenalty = (penaltyStr == "true" || penaltyStr == "1");

    std::string enabledStr = extractJsonString(json, "enabled");
    dto.enabled = (enabledStr == "true" || enabledStr == "1");

    std::string defaultPtsStr = extractJsonString(json, "default_points");
    if (!defaultPtsStr.empty()) {
        try { dto.defaultPoints = std::stoi(defaultPtsStr); } catch (...) {}
    }

    std::string currentPtsStr = extractJsonString(json, "current_points");
    if (!currentPtsStr.empty()) {
        try { dto.currentPoints = std::stoi(currentPtsStr); } catch (...) {}
    }

    std::string minPtsStr = extractJsonString(json, "min_points");
    if (!minPtsStr.empty()) {
        try { dto.minPoints = std::stoi(minPtsStr); } catch (...) {}
    }

    std::string maxPtsStr = extractJsonString(json, "max_points");
    if (!maxPtsStr.empty()) {
        try { dto.maxPoints = std::stoi(maxPtsStr); } catch (...) {}
    }

    return dto;
}

// ============================================================================
// SavedProspectDTO implementation
// ============================================================================

std::string SavedProspectDTO::toJson() const {
    std::ostringstream json;
    // Wrap in JSON:API format for ApiLogicServer
    json << "{\"data\": {\"attributes\": {";
    json << "\"business_name\": \"" << businessName << "\"";

    if (!storeLocationId.empty()) {
        json << ", \"store_location_id\": \"" << storeLocationId << "\"";
    }
    if (!businessCategory.empty()) {
        json << ", \"business_category\": \"" << businessCategory << "\"";
    }
    if (!addressLine1.empty()) {
        json << ", \"address_line1\": \"" << addressLine1 << "\"";
    }
    if (!addressLine2.empty()) {
        json << ", \"address_line2\": \"" << addressLine2 << "\"";
    }
    if (!city.empty()) {
        json << ", \"city\": \"" << city << "\"";
    }
    if (!stateProvince.empty()) {
        json << ", \"state_province\": \"" << stateProvince << "\"";
    }
    if (!postalCode.empty()) {
        json << ", \"postal_code\": \"" << postalCode << "\"";
    }
    json << ", \"country_code\": \"" << countryCode << "\"";

    if (latitude != 0.0 || longitude != 0.0) {
        json << ", \"latitude\": " << latitude;
        json << ", \"longitude\": " << longitude;
    }

    if (!phone.empty()) {
        json << ", \"phone\": \"" << phone << "\"";
    }
    if (!email.empty()) {
        json << ", \"email\": \"" << email << "\"";
    }
    if (!website.empty()) {
        json << ", \"website\": \"" << website << "\"";
    }

    json << ", \"employee_count\": " << employeeCount;
    json << ", \"catering_potential_score\": " << cateringPotentialScore;
    json << ", \"relevance_score\": " << relevanceScore;
    json << ", \"distance_miles\": " << distanceMiles;

    if (!aiSummary.empty()) {
        // Escape quotes in AI summary
        std::string escapedSummary;
        for (char c : aiSummary) {
            if (c == '"') escapedSummary += "\\\"";
            else if (c == '\n') escapedSummary += "\\n";
            else if (c == '\r') escapedSummary += "\\r";
            else escapedSummary += c;
        }
        json << ", \"ai_summary\": \"" << escapedSummary << "\"";
    }
    if (!matchReason.empty()) {
        std::string escapedReason;
        for (char c : matchReason) {
            if (c == '"') escapedReason += "\\\"";
            else if (c == '\n') escapedReason += "\\n";
            else escapedReason += c;
        }
        json << ", \"match_reason\": \"" << escapedReason << "\"";
    }
    if (!keyHighlights.empty()) {
        json << ", \"key_highlights\": \"" << keyHighlights << "\"";
    }
    if (!recommendedActions.empty()) {
        json << ", \"recommended_actions\": \"" << recommendedActions << "\"";
    }
    if (!dataSource.empty()) {
        json << ", \"data_source\": \"" << dataSource << "\"";
    }
    if (!savedAt.empty()) {
        json << ", \"saved_at\": \"" << savedAt << "\"";
    }

    json << ", \"is_contacted\": " << (isContacted ? "true" : "false");
    json << ", \"is_converted\": " << (isConverted ? "true" : "false");

    if (!notes.empty()) {
        std::string escapedNotes;
        for (char c : notes) {
            if (c == '"') escapedNotes += "\\\"";
            else if (c == '\n') escapedNotes += "\\n";
            else escapedNotes += c;
        }
        json << ", \"notes\": \"" << escapedNotes << "\"";
    }

    json << "}, \"type\": \"SavedProspect\"";
    if (!id.empty()) {
        json << ", \"id\": \"" << id << "\"";
    }
    json << "}}";
    return json.str();
}

SavedProspectDTO SavedProspectDTO::fromJson(const std::string& json) {
    SavedProspectDTO dto;

    dto.id = extractJsonString(json, "id");
    dto.storeLocationId = extractJsonString(json, "store_location_id");
    dto.businessName = extractJsonString(json, "business_name");
    dto.businessCategory = extractJsonString(json, "business_category");
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

    dto.phone = extractJsonString(json, "phone");
    dto.email = extractJsonString(json, "email");
    dto.website = extractJsonString(json, "website");

    std::string empStr = extractJsonString(json, "employee_count");
    if (!empStr.empty()) {
        try { dto.employeeCount = std::stoi(empStr); } catch (...) {}
    }

    std::string scoreStr = extractJsonString(json, "catering_potential_score");
    if (!scoreStr.empty()) {
        try { dto.cateringPotentialScore = std::stoi(scoreStr); } catch (...) {}
    }

    std::string relStr = extractJsonString(json, "relevance_score");
    if (!relStr.empty()) {
        try { dto.relevanceScore = std::stod(relStr); } catch (...) {}
    }

    std::string distStr = extractJsonString(json, "distance_miles");
    if (!distStr.empty()) {
        try { dto.distanceMiles = std::stod(distStr); } catch (...) {}
    }

    dto.aiSummary = extractJsonString(json, "ai_summary");
    dto.matchReason = extractJsonString(json, "match_reason");
    dto.keyHighlights = extractJsonString(json, "key_highlights");
    dto.recommendedActions = extractJsonString(json, "recommended_actions");
    dto.dataSource = extractJsonString(json, "data_source");
    dto.savedAt = extractJsonString(json, "saved_at");

    std::string contactedStr = extractJsonString(json, "is_contacted");
    dto.isContacted = (contactedStr == "true" || contactedStr == "1");

    std::string convertedStr = extractJsonString(json, "is_converted");
    dto.isConverted = (convertedStr == "true" || convertedStr == "1");

    dto.notes = extractJsonString(json, "notes");

    return dto;
}

// ============================================================================
// ProspectDTO implementation
// ============================================================================

std::string ProspectDTO::toJson() const {
    std::ostringstream json;
    // JSON:API format for ApiLogicServer
    json << "{\"data\": {\"attributes\": {";

    // Required field
    json << "\"business_name\": \"" << businessName << "\"";

    // Foreign keys
    if (!territoryId.empty()) {
        json << ", \"territory_id\": \"" << territoryId << "\"";
    }
    if (!franchiseeId.empty()) {
        json << ", \"franchisee_id\": \"" << franchiseeId << "\"";
    }
    if (!assignedToUserId.empty()) {
        json << ", \"assigned_to_user_id\": \"" << assignedToUserId << "\"";
    }
    if (!industryId.empty()) {
        json << ", \"industry_id\": \"" << industryId << "\"";
    }
    if (!duplicateOfId.empty()) {
        json << ", \"duplicate_of_id\": \"" << duplicateOfId << "\"";
    }

    // Business details
    if (!dbaName.empty()) {
        json << ", \"dba_name\": \"" << dbaName << "\"";
    }
    if (!legalName.empty()) {
        json << ", \"legal_name\": \"" << legalName << "\"";
    }
    if (!industryNaics.empty()) {
        json << ", \"industry_naics\": \"" << industryNaics << "\"";
    }
    if (!businessType.empty()) {
        json << ", \"business_type\": \"" << businessType << "\"";
    }

    json << ", \"employee_count\": " << employeeCount;

    if (!employeeCountRange.empty()) {
        json << ", \"employee_count_range\": \"" << employeeCountRange << "\"";
    }

    json << ", \"annual_revenue\": " << std::fixed << std::setprecision(2) << annualRevenue;
    json << ", \"year_established\": " << yearEstablished;

    // Address
    if (!addressLine1.empty()) {
        json << ", \"address_line1\": \"" << addressLine1 << "\"";
    }
    if (!addressLine2.empty()) {
        json << ", \"address_line2\": \"" << addressLine2 << "\"";
    }
    if (!city.empty()) {
        json << ", \"city\": \"" << city << "\"";
    }
    if (!stateProvince.empty()) {
        json << ", \"state_province\": \"" << stateProvince << "\"";
    }
    if (!postalCode.empty()) {
        json << ", \"postal_code\": \"" << postalCode << "\"";
    }
    json << ", \"country_code\": \"" << countryCode << "\"";

    // Geolocation
    json << ", \"latitude\": " << std::fixed << std::setprecision(8) << latitude;
    json << ", \"longitude\": " << std::fixed << std::setprecision(8) << longitude;
    if (!geocodeAccuracy.empty()) {
        json << ", \"geocode_accuracy\": \"" << geocodeAccuracy << "\"";
    }

    // Contact info
    if (!primaryPhone.empty()) {
        json << ", \"primary_phone\": \"" << primaryPhone << "\"";
    }
    if (!secondaryPhone.empty()) {
        json << ", \"secondary_phone\": \"" << secondaryPhone << "\"";
    }
    if (!email.empty()) {
        json << ", \"email\": \"" << email << "\"";
    }
    if (!website.empty()) {
        json << ", \"website\": \"" << website << "\"";
    }
    if (!linkedinUrl.empty()) {
        json << ", \"linkedin_url\": \"" << linkedinUrl << "\"";
    }
    if (!facebookUrl.empty()) {
        json << ", \"facebook_url\": \"" << facebookUrl << "\"";
    }

    // Status
    json << ", \"status\": \"" << status << "\"";
    if (!statusChangedAt.empty()) {
        json << ", \"status_changed_at\": \"" << statusChangedAt << "\"";
    }

    // Data source
    if (!dataSource.empty()) {
        json << ", \"data_source\": \"" << dataSource << "\"";
    }
    if (!sourceRecordId.empty()) {
        json << ", \"source_record_id\": \"" << sourceRecordId << "\"";
    }

    // Flags
    json << ", \"is_verified\": " << (isVerified ? "true" : "false");
    json << ", \"is_duplicate\": " << (isDuplicate ? "true" : "false");
    json << ", \"do_not_contact\": " << (doNotContact ? "true" : "false");

    // AI and scoring fields
    json << ", \"ai_score\": " << aiScore;
    json << ", \"optimized_score\": " << optimizedScore;
    json << ", \"relevance_score\": " << std::fixed << std::setprecision(4) << relevanceScore;
    if (!aiSummary.empty()) {
        // Escape special characters in JSON string
        std::string escapedSummary = aiSummary;
        for (size_t i = 0; i < escapedSummary.length(); ++i) {
            if (escapedSummary[i] == '"' || escapedSummary[i] == '\\') {
                escapedSummary.insert(i, "\\");
                ++i;
            } else if (escapedSummary[i] == '\n') {
                escapedSummary.replace(i, 1, "\\n");
                ++i;
            } else if (escapedSummary[i] == '\r') {
                escapedSummary.replace(i, 1, "\\r");
                ++i;
            }
        }
        json << ", \"ai_summary\": \"" << escapedSummary << "\"";
    }
    if (!keyHighlights.empty()) {
        json << ", \"key_highlights\": \"" << keyHighlights << "\"";
    }
    if (!recommendedActions.empty()) {
        json << ", \"recommended_actions\": \"" << recommendedActions << "\"";
    }
    if (!dataSources.empty()) {
        json << ", \"data_sources\": \"" << dataSources << "\"";
    }

    json << "}";  // End attributes
    json << ", \"type\": \"Prospect\"";

    if (!id.empty()) {
        json << ", \"id\": \"" << id << "\"";
    }

    json << "}}";  // End data and root

    return json.str();
}

ProspectDTO ProspectDTO::fromJson(const std::string& json) {
    ProspectDTO dto;

    dto.id = extractJsonString(json, "id");
    dto.territoryId = extractJsonString(json, "territory_id");
    dto.franchiseeId = extractJsonString(json, "franchisee_id");
    dto.assignedToUserId = extractJsonString(json, "assigned_to_user_id");
    dto.businessName = extractJsonString(json, "business_name");
    dto.dbaName = extractJsonString(json, "dba_name");
    dto.legalName = extractJsonString(json, "legal_name");
    dto.industryId = extractJsonString(json, "industry_id");
    dto.industryNaics = extractJsonString(json, "industry_naics");
    dto.businessType = extractJsonString(json, "business_type");

    std::string empStr = extractJsonString(json, "employee_count");
    if (!empStr.empty()) {
        try { dto.employeeCount = std::stoi(empStr); } catch (...) {}
    }
    dto.employeeCountRange = extractJsonString(json, "employee_count_range");

    std::string revStr = extractJsonString(json, "annual_revenue");
    if (!revStr.empty()) {
        try { dto.annualRevenue = std::stod(revStr); } catch (...) {}
    }

    std::string yearStr = extractJsonString(json, "year_established");
    if (!yearStr.empty()) {
        try { dto.yearEstablished = std::stoi(yearStr); } catch (...) {}
    }

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
    dto.geocodeAccuracy = extractJsonString(json, "geocode_accuracy");

    dto.primaryPhone = extractJsonString(json, "primary_phone");
    dto.secondaryPhone = extractJsonString(json, "secondary_phone");
    dto.email = extractJsonString(json, "email");
    dto.website = extractJsonString(json, "website");
    dto.linkedinUrl = extractJsonString(json, "linkedin_url");
    dto.facebookUrl = extractJsonString(json, "facebook_url");

    dto.status = extractJsonString(json, "status");
    if (dto.status.empty()) dto.status = "new";
    dto.statusChangedAt = extractJsonString(json, "status_changed_at");

    dto.dataSource = extractJsonString(json, "data_source");
    dto.sourceRecordId = extractJsonString(json, "source_record_id");

    std::string verifiedStr = extractJsonString(json, "is_verified");
    dto.isVerified = (verifiedStr == "true" || verifiedStr == "1");

    std::string duplicateStr = extractJsonString(json, "is_duplicate");
    dto.isDuplicate = (duplicateStr == "true" || duplicateStr == "1");

    dto.duplicateOfId = extractJsonString(json, "duplicate_of_id");

    std::string dncStr = extractJsonString(json, "do_not_contact");
    dto.doNotContact = (dncStr == "true" || dncStr == "1");

    dto.createdAt = extractJsonString(json, "created_at");
    dto.updatedAt = extractJsonString(json, "updated_at");

    // AI and scoring fields
    std::string aiScoreStr = extractJsonString(json, "ai_score");
    if (!aiScoreStr.empty()) {
        try { dto.aiScore = std::stoi(aiScoreStr); } catch (...) {}
    }
    std::string optScoreStr = extractJsonString(json, "optimized_score");
    if (!optScoreStr.empty()) {
        try { dto.optimizedScore = std::stoi(optScoreStr); } catch (...) {}
    }
    std::string relScoreStr = extractJsonString(json, "relevance_score");
    if (!relScoreStr.empty()) {
        try { dto.relevanceScore = std::stod(relScoreStr); } catch (...) {}
    }
    dto.aiSummary = extractJsonString(json, "ai_summary");
    dto.keyHighlights = extractJsonString(json, "key_highlights");
    dto.recommendedActions = extractJsonString(json, "recommended_actions");
    dto.dataSources = extractJsonString(json, "data_sources");

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
    // Always use PATCH with ID in URL (upsert pattern)
    // ApiLogicServer expects client-generated UUIDs
    StoreLocationDTO dto = location;
    if (dto.id.empty()) {
        dto.id = generateUUID();
        std::cout << "  [ALS] Creating new StoreLocation with generated UUID: " << dto.id << std::endl;
    } else {
        std::cout << "  [ALS] Updating existing StoreLocation: " << dto.id << std::endl;
    }
    std::string json = dto.toJson();
    return httpPatch("/StoreLocation/" + dto.id, json);
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

// ============================================================================
// Franchisee API Operations
// ============================================================================

ApiResponse ApiLogicServerClient::saveFranchisee(const FranchiseeDTO& franchisee) {
    // Always use PATCH with ID in URL (upsert pattern)
    // ApiLogicServer expects client-generated UUIDs
    FranchiseeDTO dto = franchisee;
    if (dto.id.empty()) {
        dto.id = generateUUID();
        std::cout << "  [ALS] Creating new Franchisee with generated UUID: " << dto.id << std::endl;
    } else {
        std::cout << "  [ALS] Updating existing Franchisee: " << dto.id << std::endl;
    }
    std::string json = dto.toJson();
    return httpPatch("/Franchisee/" + dto.id, json);
}

ApiResponse ApiLogicServerClient::getFranchisees() {
    return httpGet("/Franchisee");
}

ApiResponse ApiLogicServerClient::getFranchisee(const std::string& id) {
    if (id.empty()) {
        ApiResponse response;
        response.success = false;
        response.statusCode = 400;
        response.errorMessage = "Franchisee ID cannot be empty";
        return response;
    }
    return httpGet("/Franchisee/" + id);
}

ApiResponse ApiLogicServerClient::deleteFranchisee(const std::string& id) {
    if (id.empty()) {
        ApiResponse response;
        response.success = false;
        response.statusCode = 400;
        response.errorMessage = "Franchisee ID cannot be empty";
        return response;
    }
    return httpDelete("/Franchisee/" + id);
}

std::vector<FranchiseeDTO> ApiLogicServerClient::parseFranchisees(const ApiResponse& response) {
    std::vector<FranchiseeDTO> franchisees;

    if (!response.success || response.body.empty()) {
        return franchisees;
    }

    const std::string& json = response.body;

    // Find the array start
    size_t arrayStart = json.find('[');
    if (arrayStart == std::string::npos) {
        // Single object?
        if (json.find('{') != std::string::npos) {
            franchisees.push_back(FranchiseeDTO::fromJson(json));
        }
        return franchisees;
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
                franchisees.push_back(FranchiseeDTO::fromJson(objJson));
                objStart = std::string::npos;
            }
        } else if (c == ']' && braceCount == 0) {
            break;
        }
        pos++;
    }

    return franchisees;
}

// ============================================================================
// Scoring Rule API Operations
// ============================================================================

ApiResponse ApiLogicServerClient::saveScoringRule(const ScoringRuleDTO& rule) {
    ScoringRuleDTO dto = rule;
    if (dto.id.empty()) {
        // No cached ID - try to find existing rule by rule_id first
        std::cout << "  [ALS] Looking up existing ScoringRule by rule_id: " << dto.ruleId << std::endl;
        auto existingResponse = httpGet("/ScoringRule?filter[rule_id]=" + dto.ruleId);
        if (existingResponse.success && !existingResponse.body.empty()) {
            // Try to extract the ID from the response
            std::string existingId = extractJsonString(existingResponse.body, "id");
            if (!existingId.empty()) {
                dto.id = existingId;
                std::cout << "  [ALS] Found existing rule with UUID: " << dto.id << std::endl;
            }
        }
    }

    if (dto.id.empty()) {
        // Create new record - POST to collection endpoint
        dto.id = generateUUID();
        std::string json = dto.toJson();
        std::cout << "  [ALS] Creating new ScoringRule with generated UUID: " << dto.id << std::endl;
        return httpPost("/ScoringRule", json);
    } else {
        // Update existing record - PATCH to resource endpoint
        std::string json = dto.toJson();
        std::cout << "  [ALS] Updating existing ScoringRule: " << dto.id << std::endl;
        return httpPatch("/ScoringRule/" + dto.id, json);
    }
}

ApiResponse ApiLogicServerClient::getScoringRules() {
    return httpGet("/ScoringRule");
}

ApiResponse ApiLogicServerClient::getScoringRulesForFranchisee(const std::string& franchiseeId) {
    if (franchiseeId.empty()) {
        // Get global rules (no franchisee filter)
        return httpGet("/ScoringRule?filter[franchisee_id]=null");
    }
    return httpGet("/ScoringRule?filter[franchisee_id]=" + franchiseeId);
}

ApiResponse ApiLogicServerClient::getScoringRule(const std::string& id) {
    if (id.empty()) {
        ApiResponse response;
        response.success = false;
        response.statusCode = 400;
        response.errorMessage = "Scoring rule ID cannot be empty";
        return response;
    }
    return httpGet("/ScoringRule/" + id);
}

ApiResponse ApiLogicServerClient::deleteScoringRule(const std::string& id) {
    if (id.empty()) {
        ApiResponse response;
        response.success = false;
        response.statusCode = 400;
        response.errorMessage = "Scoring rule ID cannot be empty";
        return response;
    }
    return httpDelete("/ScoringRule/" + id);
}

std::vector<ScoringRuleDTO> ApiLogicServerClient::parseScoringRules(const ApiResponse& response) {
    std::vector<ScoringRuleDTO> rules;

    if (!response.success || response.body.empty()) {
        return rules;
    }

    const std::string& json = response.body;

    // Find the array start
    size_t arrayStart = json.find('[');
    if (arrayStart == std::string::npos) {
        // Single object?
        if (json.find('{') != std::string::npos) {
            rules.push_back(ScoringRuleDTO::fromJson(json));
        }
        return rules;
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
                rules.push_back(ScoringRuleDTO::fromJson(objJson));
                objStart = std::string::npos;
            }
        } else if (c == ']' && braceCount == 0) {
            break;
        }
        pos++;
    }

    return rules;
}

// ============================================================================
// Saved Prospect API Operations
// ============================================================================

ApiResponse ApiLogicServerClient::saveProspect(const SavedProspectDTO& prospect) {
    // Always use PATCH with ID in URL (upsert pattern)
    // ApiLogicServer expects client-generated UUIDs
    SavedProspectDTO dto = prospect;
    if (dto.id.empty()) {
        dto.id = generateUUID();
        std::cout << "  [ALS] Creating new SavedProspect with generated UUID: " << dto.id << std::endl;
    } else {
        std::cout << "  [ALS] Updating existing SavedProspect: " << dto.id << std::endl;
    }
    std::string json = dto.toJson();
    return httpPatch("/SavedProspect/" + dto.id, json);
}

ApiResponse ApiLogicServerClient::getProspectsForStore(const std::string& storeLocationId) {
    if (storeLocationId.empty()) {
        // Get all prospects
        return httpGet("/SavedProspect");
    }
    return httpGet("/SavedProspect?filter[store_location_id]=" + storeLocationId);
}

ApiResponse ApiLogicServerClient::getProspectsForFranchisee(const std::string& franchiseeId, int offset, int limit) {
    if (franchiseeId.empty()) {
        ApiResponse response;
        response.success = false;
        response.statusCode = 400;
        response.errorMessage = "Franchisee ID cannot be empty";
        return response;
    }

    // Build the API URL for the franchisee's prospect list
    // GET /api/Franchisee/{franchisee_id}/ProspectList
    std::string url = "/Franchisee/" + franchiseeId + "/ProspectList";

    // Add fields selection for Prospect entity
    url += "?fields[Prospect]=territory_id,franchisee_id,assigned_to_user_id,business_name,dba_name,";
    url += "legal_name,industry_id,industry_naics,business_type,employee_count,employee_count_range,";
    url += "annual_revenue,year_established,address_line1,address_line2,city,state_province,postal_code,";
    url += "country_code,latitude,longitude,geocode_accuracy,primary_phone,secondary_phone,email,website,";
    url += "linkedin_url,facebook_url,status,status_changed_at,data_source,source_record_id,is_verified,";
    url += "is_duplicate,duplicate_of_id,do_not_contact,created_at,updated_at";

    // Add pagination
    url += "&page[offset]=" + std::to_string(offset);
    url += "&page[limit]=" + std::to_string(limit);

    // Sort by id (default)
    url += "&sort=id";

    std::cout << "  [ALS] Getting prospects for franchisee: " << franchiseeId << std::endl;
    return httpGet(url);
}

ApiResponse ApiLogicServerClient::saveProspect(const ProspectDTO& prospect) {
    ProspectDTO dto = prospect;
    if (dto.id.empty()) {
        // Create new record - POST to collection endpoint
        dto.id = generateUUID();
        std::string json = dto.toJson();
        std::cout << "  [ALS] Creating new Prospect with generated UUID: " << dto.id << std::endl;
        return httpPost("/Prospect", json);
    } else {
        // Update existing record - PATCH to resource endpoint
        std::string json = dto.toJson();
        std::cout << "  [ALS] Updating existing Prospect: " << dto.id << std::endl;
        return httpPatch("/Prospect/" + dto.id, json);
    }
}

ApiResponse ApiLogicServerClient::getProspect(const std::string& id) {
    if (id.empty()) {
        ApiResponse response;
        response.success = false;
        response.statusCode = 400;
        response.errorMessage = "Prospect ID cannot be empty";
        return response;
    }
    return httpGet("/Prospect/" + id);
}

ApiResponse ApiLogicServerClient::deleteProspect(const std::string& id) {
    if (id.empty()) {
        ApiResponse response;
        response.success = false;
        response.statusCode = 400;
        response.errorMessage = "Prospect ID cannot be empty";
        return response;
    }
    return httpDelete("/Prospect/" + id);
}

std::vector<ProspectDTO> ApiLogicServerClient::parseProspects(const ApiResponse& response) {
    std::vector<ProspectDTO> prospects;

    if (!response.success || response.body.empty()) {
        return prospects;
    }

    const std::string& json = response.body;

    // Find the array start
    size_t arrayStart = json.find('[');
    if (arrayStart == std::string::npos) {
        // Single object?
        if (json.find('{') != std::string::npos) {
            prospects.push_back(ProspectDTO::fromJson(json));
        }
        return prospects;
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
                prospects.push_back(ProspectDTO::fromJson(objJson));
                objStart = std::string::npos;
            }
        } else if (c == ']' && braceCount == 0) {
            break;
        }
        pos++;
    }

    return prospects;
}

ApiResponse ApiLogicServerClient::getSavedProspect(const std::string& id) {
    if (id.empty()) {
        ApiResponse response;
        response.success = false;
        response.statusCode = 400;
        response.errorMessage = "Saved prospect ID cannot be empty";
        return response;
    }
    return httpGet("/SavedProspect/" + id);
}

ApiResponse ApiLogicServerClient::deleteSavedProspect(const std::string& id) {
    if (id.empty()) {
        ApiResponse response;
        response.success = false;
        response.statusCode = 400;
        response.errorMessage = "Saved prospect ID cannot be empty";
        return response;
    }
    return httpDelete("/SavedProspect/" + id);
}

std::vector<SavedProspectDTO> ApiLogicServerClient::parseSavedProspects(const ApiResponse& response) {
    std::vector<SavedProspectDTO> prospects;

    if (!response.success || response.body.empty()) {
        return prospects;
    }

    const std::string& json = response.body;

    // Find the array start
    size_t arrayStart = json.find('[');
    if (arrayStart == std::string::npos) {
        // Single object?
        if (json.find('{') != std::string::npos) {
            prospects.push_back(SavedProspectDTO::fromJson(json));
        }
        return prospects;
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
                prospects.push_back(SavedProspectDTO::fromJson(objJson));
                objStart = std::string::npos;
            }
        } else if (c == ']' && braceCount == 0) {
            break;
        }
        pos++;
    }

    return prospects;
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

// ============================================================================
// Generic Resource Operations
// ============================================================================

std::string ApiLogicServerClient::getResource(const std::string& resourceType,
                                               const std::string& id,
                                               const std::string& filter) {
    std::string path = "/" + resourceType;
    if (!id.empty()) {
        path += "/" + id;
    }
    if (!filter.empty()) {
        path += "?filter[" + filter.substr(0, filter.find('=')) + "]=" +
                filter.substr(filter.find('=') + 1);
    }

    std::cout << "[ALS] GET " << path << std::endl;
    auto response = httpGet(path);

    if (response.success) {
        return response.body;
    }
    return "";
}

std::string ApiLogicServerClient::createResource(const std::string& resourceType,
                                                  const std::string& jsonBody) {
    std::string path = "/" + resourceType;

    std::cout << "[ALS] POST " << path << std::endl;
    auto response = httpPost(path, jsonBody);

    if (response.success) {
        return response.body;
    }
    return "";
}

std::string ApiLogicServerClient::updateResource(const std::string& resourceType,
                                                  const std::string& id,
                                                  const std::string& jsonBody) {
    std::string path = "/" + resourceType + "/" + id;

    std::cout << "[ALS] PATCH " << path << std::endl;
    auto response = httpPatch(path, jsonBody);

    if (response.success) {
        return response.body;
    }
    return "";
}

} // namespace Services
} // namespace FranchiseAI
