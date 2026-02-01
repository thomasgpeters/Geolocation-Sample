#include "OpenStreetMapAPI.h"
#include <curl/curl.h>
#include <random>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>

namespace FranchiseAI {
namespace Services {

// Static OSM tag to BusinessType mapping
const std::map<std::string, Models::BusinessType> OpenStreetMapAPI::osmTagMapping_ = {
    // Office types
    {"office=company", Models::BusinessType::CORPORATE_OFFICE},
    {"office=corporate", Models::BusinessType::CORPORATE_OFFICE},
    {"office=headquarters", Models::BusinessType::CORPORATE_OFFICE},
    {"office=it", Models::BusinessType::TECH_COMPANY},
    {"office=telecommunication", Models::BusinessType::TECH_COMPANY},
    {"office=research", Models::BusinessType::TECH_COMPANY},
    {"office=financial", Models::BusinessType::FINANCIAL_SERVICES},
    {"office=insurance", Models::BusinessType::FINANCIAL_SERVICES},
    {"office=accountant", Models::BusinessType::FINANCIAL_SERVICES},
    {"office=lawyer", Models::BusinessType::LAW_FIRM},
    {"office=notary", Models::BusinessType::LAW_FIRM},
    {"office=government", Models::BusinessType::GOVERNMENT_OFFICE},
    {"office=ngo", Models::BusinessType::NONPROFIT},
    {"office=foundation", Models::BusinessType::NONPROFIT},
    {"office=coworking", Models::BusinessType::COWORKING_SPACE},

    // Building types
    {"building=office", Models::BusinessType::CORPORATE_OFFICE},
    {"building=commercial", Models::BusinessType::CORPORATE_OFFICE},
    {"building=industrial", Models::BusinessType::MANUFACTURING},
    {"building=warehouse", Models::BusinessType::WAREHOUSE},
    {"building=hotel", Models::BusinessType::HOTEL},
    {"building=hospital", Models::BusinessType::MEDICAL_FACILITY},
    {"building=university", Models::BusinessType::EDUCATIONAL_INSTITUTION},
    {"building=school", Models::BusinessType::EDUCATIONAL_INSTITUTION},
    {"building=government", Models::BusinessType::GOVERNMENT_OFFICE},

    // Amenity types
    {"amenity=conference_centre", Models::BusinessType::CONFERENCE_CENTER},
    {"amenity=events_venue", Models::BusinessType::CONFERENCE_CENTER},
    {"amenity=hospital", Models::BusinessType::MEDICAL_FACILITY},
    {"amenity=clinic", Models::BusinessType::MEDICAL_FACILITY},
    {"amenity=university", Models::BusinessType::EDUCATIONAL_INSTITUTION},
    {"amenity=college", Models::BusinessType::EDUCATIONAL_INSTITUTION},
    {"amenity=school", Models::BusinessType::EDUCATIONAL_INSTITUTION},
    {"amenity=coworking_space", Models::BusinessType::COWORKING_SPACE},

    // Tourism types
    {"tourism=hotel", Models::BusinessType::HOTEL},
    {"tourism=motel", Models::BusinessType::HOTEL},
    {"tourism=hostel", Models::BusinessType::HOTEL},

    // Healthcare
    {"healthcare=hospital", Models::BusinessType::MEDICAL_FACILITY},
    {"healthcare=clinic", Models::BusinessType::MEDICAL_FACILITY},
    {"healthcare=doctor", Models::BusinessType::MEDICAL_FACILITY},

    // Landuse
    {"landuse=industrial", Models::BusinessType::MANUFACTURING},
    {"landuse=commercial", Models::BusinessType::CORPORATE_OFFICE},
};

// CURL write callback
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

OpenStreetMapAPI::OpenStreetMapAPI() = default;

OpenStreetMapAPI::OpenStreetMapAPI(const OSMAPIConfig& config)
    : config_(config) {}

OpenStreetMapAPI::~OpenStreetMapAPI() = default;

void OpenStreetMapAPI::setConfig(const OSMAPIConfig& config) {
    config_ = config;
}

void OpenStreetMapAPI::searchNearby(
    double latitude,
    double longitude,
    int radiusMeters,
    POICallback callback
) {
    ++totalApiCalls_;

    // Check cache first
    std::string cacheKey = std::to_string(latitude) + "," + std::to_string(longitude) + "," + std::to_string(radiusMeters);
    if (config_.enableCaching) {
        auto it = poiCache_.find(cacheKey);
        if (it != poiCache_.end()) {
            time_t now = std::time(nullptr);
            if (now - it->second.second < config_.cacheDurationMinutes * 60) {
                if (callback) {
                    callback(it->second.first, "");
                }
                return;
            }
        }
    }

    // Build and execute real Overpass API query for catering prospects
    std::string query = buildCateringProspectQuery(latitude, longitude, radiusMeters);
    std::string response = executeOverpassQuery(query);

    // Check for error in response
    if (response.empty()) {
        if (callback) callback({}, "Overpass API request failed - no response");
        return;
    }

    // Check for error JSON (from curl failure or API error)
    if (response.find("\"error\"") != std::string::npos) {
        // Extract error message if possible
        size_t errorStart = response.find("\"error\"");
        size_t colonPos = response.find(':', errorStart);
        size_t quoteStart = response.find('"', colonPos);
        size_t quoteEnd = response.find('"', quoteStart + 1);
        std::string errorMsg = "Overpass API error";
        if (quoteStart != std::string::npos && quoteEnd != std::string::npos) {
            errorMsg = response.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
        }
        if (callback) callback({}, errorMsg);
        return;
    }

    // Parse the JSON response
    auto results = parseOverpassResponse(response);

    // Cache results
    if (config_.enableCaching && !results.empty()) {
        poiCache_[cacheKey] = {results, std::time(nullptr)};
    }

    if (callback) {
        callback(results, "");
    }
}

void OpenStreetMapAPI::searchBusinesses(
    double latitude,
    double longitude,
    double radiusMiles,
    BusinessCallback callback
) {
    int radiusMeters = static_cast<int>(radiusMiles * 1609.34);

    searchNearby(latitude, longitude, radiusMeters,
        [callback](const std::vector<OSMPoi>& pois, const std::string& error) {
            if (!error.empty()) {
                if (callback) callback({}, error);
                return;
            }

            std::vector<Models::BusinessInfo> businesses;
            businesses.reserve(pois.size());

            for (const auto& poi : pois) {
                if (!poi.name.empty()) {
                    businesses.push_back(poiToBusinessInfo(poi));
                }
            }

            // Sort by catering potential
            std::sort(businesses.begin(), businesses.end(),
                [](const Models::BusinessInfo& a, const Models::BusinessInfo& b) {
                    return a.cateringPotentialScore > b.cateringPotentialScore;
                });

            if (callback) callback(businesses, "");
        });
}

void OpenStreetMapAPI::searchByBusinessType(
    double latitude,
    double longitude,
    double radiusMiles,
    const std::vector<Models::BusinessType>& types,
    BusinessCallback callback
) {
    searchBusinesses(latitude, longitude, radiusMiles,
        [types, callback](std::vector<Models::BusinessInfo> businesses, const std::string& error) {
            if (!error.empty()) {
                if (callback) callback({}, error);
                return;
            }

            // Filter by requested types
            std::vector<Models::BusinessInfo> filtered;
            for (const auto& biz : businesses) {
                for (const auto& type : types) {
                    if (biz.type == type) {
                        filtered.push_back(biz);
                        break;
                    }
                }
            }

            if (callback) callback(filtered, "");
        });
}

void OpenStreetMapAPI::searchCateringProspects(
    double latitude,
    double longitude,
    double radiusMiles,
    BusinessCallback callback
) {
    std::vector<Models::BusinessType> cateringTypes = {
        Models::BusinessType::CORPORATE_OFFICE,
        Models::BusinessType::CONFERENCE_CENTER,
        Models::BusinessType::TECH_COMPANY,
        Models::BusinessType::FINANCIAL_SERVICES,
        Models::BusinessType::HOTEL,
        Models::BusinessType::MEDICAL_FACILITY,
        Models::BusinessType::EDUCATIONAL_INSTITUTION,
        Models::BusinessType::COWORKING_SPACE,
        Models::BusinessType::GOVERNMENT_OFFICE
    };

    searchByBusinessType(latitude, longitude, radiusMiles, cateringTypes, callback);
}

void OpenStreetMapAPI::getAreaStatistics(
    double latitude,
    double longitude,
    double radiusKm,
    AreaStatsCallback callback
) {
    ++totalApiCalls_;

    // For demo/prototype, generate sample statistics
    auto stats = generateDemoAreaStats(latitude, longitude, radiusKm);

    if (callback) {
        callback(stats, "");
    }
}

void OpenStreetMapAPI::geocodeAddress(
    const std::string& address,
    GeocodeCallback callback
) {
    ++totalApiCalls_;

    // URL encode the address
    CURL* curl = curl_easy_init();
    if (!curl) {
        if (callback) callback(0.0, 0.0, "Failed to initialize CURL");
        return;
    }

    char* encodedAddress = curl_easy_escape(curl, address.c_str(), static_cast<int>(address.length()));
    if (!encodedAddress) {
        curl_easy_cleanup(curl);
        if (callback) callback(0.0, 0.0, "Failed to encode address");
        return;
    }

    // Build Nominatim search URL
    std::string url = config_.nominatimEndpoint + "/search?format=json&limit=1&q=" + std::string(encodedAddress);
    curl_free(encodedAddress);
    curl_easy_cleanup(curl);

    // Execute the query
    std::string response = executeNominatimQuery(url);

    // Parse response
    OSMPoi poi = parseNominatimResponse(response);

    if (poi.latitude != 0.0 && poi.longitude != 0.0) {
        if (callback) callback(poi.latitude, poi.longitude, "");
    } else {
        if (callback) callback(0.0, 0.0, "Geocoding failed: no results found");
    }
}

void OpenStreetMapAPI::reverseGeocode(
    double latitude,
    double longitude,
    std::function<void(OSMPoi, std::string)> callback
) {
    ++totalApiCalls_;

    // Build Nominatim reverse geocode URL
    std::ostringstream url;
    url << std::fixed << std::setprecision(6);
    url << config_.nominatimEndpoint << "/reverse?format=json&lat=" << latitude << "&lon=" << longitude;

    // Execute the query
    std::string response = executeNominatimQuery(url.str());

    // Parse response
    OSMPoi poi = parseNominatimResponse(response);
    poi.latitude = latitude;
    poi.longitude = longitude;

    if (callback) {
        callback(poi, "");
    }
}

std::vector<Models::BusinessInfo> OpenStreetMapAPI::searchBusinessesSync(
    double latitude,
    double longitude,
    double radiusMiles
) {
    // Validate coordinates - reject invalid or default (0,0) coordinates
    if (latitude == 0.0 && longitude == 0.0) {
        // Invalid coordinates - return empty results
        return {};
    }

    // Validate reasonable coordinate ranges
    if (latitude < -90.0 || latitude > 90.0 || longitude < -180.0 || longitude > 180.0) {
        return {};
    }

    std::vector<Models::BusinessInfo> results;
    std::string errorMsg;
    searchBusinesses(latitude, longitude, radiusMiles,
        [&results, &errorMsg](std::vector<Models::BusinessInfo> businesses, const std::string& error) {
            if (!error.empty()) {
                errorMsg = error;
            }
            results = std::move(businesses);
        });

    // Log error if any (for debugging)
    if (!errorMsg.empty()) {
        // Could add logging here: std::cerr << "OSM search error: " << errorMsg << std::endl;
    }

    return results;
}

OSMAreaStats OpenStreetMapAPI::getAreaStatisticsSync(
    double latitude,
    double longitude,
    double radiusKm
) {
    OSMAreaStats stats;
    getAreaStatistics(latitude, longitude, radiusKm,
        [&stats](OSMAreaStats s, const std::string&) {
            stats = std::move(s);
        });
    return stats;
}

void OpenStreetMapAPI::clearCache() {
    poiCache_.clear();
}

int OpenStreetMapAPI::getCacheSize() const {
    return static_cast<int>(poiCache_.size());
}

void OpenStreetMapAPI::resetStatistics() {
    totalApiCalls_ = 0;
}

Models::BusinessInfo OpenStreetMapAPI::poiToBusinessInfo(const OSMPoi& poi) {
    Models::BusinessInfo business;

    // Generate unique ID from OSM data
    business.id = "osm_" + poi.osmType + "_" + std::to_string(poi.osmId);
    business.name = poi.name;
    business.source = Models::DataSource::OPENSTREETMAP;
    business.type = inferBusinessType(poi);

    // Address
    business.address.street1 = poi.houseNumber.empty() ? poi.street : poi.houseNumber + " " + poi.street;
    business.address.city = poi.city;
    business.address.state = poi.state;
    business.address.zipCode = poi.postcode;
    business.address.country = poi.country.empty() ? "USA" : poi.country;
    business.address.latitude = poi.latitude;
    business.address.longitude = poi.longitude;

    // Contact info from OSM tags
    business.contact.primaryPhone = poi.phone;
    business.contact.website = poi.website;
    business.contact.email = poi.email;

    // Set description based on type
    business.description = "Business found via OpenStreetMap";

    // Parse opening hours if available
    if (!poi.openingHours.empty()) {
        // Simplified: just store the raw string
        business.hours.monday = poi.openingHours;
    }

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

    // Set metadata
    business.dateAdded = std::time(nullptr);
    business.lastUpdated = std::time(nullptr);
    business.isVerified = true;  // OSM data is community-verified

    return business;
}

Models::BusinessType OpenStreetMapAPI::inferBusinessType(const OSMPoi& poi) {
    // Check specific tags in order of priority
    for (const auto& [tag, type] : osmTagMapping_) {
        size_t eqPos = tag.find('=');
        if (eqPos != std::string::npos) {
            std::string key = tag.substr(0, eqPos);
            std::string value = tag.substr(eqPos + 1);

            auto it = poi.tags.find(key);
            if (it != poi.tags.end() && it->second == value) {
                return type;
            }
        }
    }

    // Check by general tag categories
    if (!poi.office.empty()) {
        if (poi.office == "company" || poi.office == "corporate")
            return Models::BusinessType::CORPORATE_OFFICE;
        if (poi.office == "it" || poi.office == "telecommunication")
            return Models::BusinessType::TECH_COMPANY;
        if (poi.office == "lawyer" || poi.office == "notary")
            return Models::BusinessType::LAW_FIRM;
        if (poi.office == "financial" || poi.office == "insurance")
            return Models::BusinessType::FINANCIAL_SERVICES;
        if (poi.office == "government")
            return Models::BusinessType::GOVERNMENT_OFFICE;
        if (poi.office == "ngo" || poi.office == "foundation")
            return Models::BusinessType::NONPROFIT;
        return Models::BusinessType::CORPORATE_OFFICE;  // Default for unspecified office
    }

    if (!poi.building.empty()) {
        if (poi.building == "office" || poi.building == "commercial")
            return Models::BusinessType::CORPORATE_OFFICE;
        if (poi.building == "warehouse")
            return Models::BusinessType::WAREHOUSE;
        if (poi.building == "industrial")
            return Models::BusinessType::MANUFACTURING;
        if (poi.building == "hotel")
            return Models::BusinessType::HOTEL;
        if (poi.building == "hospital")
            return Models::BusinessType::MEDICAL_FACILITY;
        if (poi.building == "university" || poi.building == "school")
            return Models::BusinessType::EDUCATIONAL_INSTITUTION;
    }

    if (!poi.amenity.empty()) {
        if (poi.amenity == "conference_centre" || poi.amenity == "events_venue")
            return Models::BusinessType::CONFERENCE_CENTER;
        if (poi.amenity == "hospital" || poi.amenity == "clinic")
            return Models::BusinessType::MEDICAL_FACILITY;
        if (poi.amenity == "university" || poi.amenity == "college" || poi.amenity == "school")
            return Models::BusinessType::EDUCATIONAL_INSTITUTION;
        if (poi.amenity == "coworking_space")
            return Models::BusinessType::COWORKING_SPACE;
    }

    if (!poi.tourism.empty()) {
        if (poi.tourism == "hotel" || poi.tourism == "motel")
            return Models::BusinessType::HOTEL;
    }

    if (!poi.healthcare.empty()) {
        return Models::BusinessType::MEDICAL_FACILITY;
    }

    return Models::BusinessType::OTHER;
}

std::string OpenStreetMapAPI::buildOverpassQuery(
    double lat,
    double lon,
    int radiusMeters,
    const std::vector<std::string>& osmFilters
) {
    std::ostringstream query;
    query << "[out:json][timeout:25];(";

    for (const auto& filter : osmFilters) {
        query << "node[" << filter << "](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[" << filter << "](around:" << radiusMeters << "," << lat << "," << lon << ");";
    }

    query << ");out center;";
    return query.str();
}

std::string OpenStreetMapAPI::buildCateringProspectQuery(
    double lat,
    double lon,
    int radiusMeters
) {
    // Limit radius to avoid overly large queries that timeout
    int limitedRadius = std::min(radiusMeters, 16000);  // Max ~10 miles

    std::ostringstream query;
    query << std::fixed << std::setprecision(6);
    // Reduced timeout from 30s to 10s for faster feedback
    query << "[out:json][timeout:10];(";

    // Focus on high-value catering prospects only (simplified query)
    // Offices with names (more likely to be relevant)
    query << "node[\"office\"][\"name\"](around:" << limitedRadius << "," << lat << "," << lon << ");";
    query << "way[\"office\"][\"name\"](around:" << limitedRadius << "," << lat << "," << lon << ");";

    // Hotels (high catering value)
    query << "node[\"tourism\"=\"hotel\"](around:" << limitedRadius << "," << lat << "," << lon << ");";
    query << "way[\"tourism\"=\"hotel\"](around:" << limitedRadius << "," << lat << "," << lon << ");";

    // Conference centers (high catering value)
    query << "node[\"amenity\"=\"conference_centre\"](around:" << limitedRadius << "," << lat << "," << lon << ");";
    query << "way[\"amenity\"=\"conference_centre\"](around:" << limitedRadius << "," << lat << "," << lon << ");";

    // Hospitals (regular catering needs)
    query << "node[\"amenity\"=\"hospital\"](around:" << limitedRadius << "," << lat << "," << lon << ");";
    query << "way[\"amenity\"=\"hospital\"](around:" << limitedRadius << "," << lat << "," << lon << ");";

    // Universities (large catering potential)
    query << "node[\"amenity\"=\"university\"](around:" << limitedRadius << "," << lat << "," << lon << ");";
    query << "way[\"amenity\"=\"university\"](around:" << limitedRadius << "," << lat << "," << lon << ");";

    query << ");out center;";
    return query.str();
}

std::string OpenStreetMapAPI::executeOverpassQuery(const std::string& query) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        std::string url = config_.overpassEndpoint;
        std::string postData = "data=" + query;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, config_.requestTimeoutMs);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, config_.connectTimeoutMs);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, config_.userAgent.c_str());
        // Enable TCP keepalive for faster failure detection
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            response = "{\"error\": \"" + std::string(curl_easy_strerror(res)) + "\"}";
        }

        curl_easy_cleanup(curl);
    }

    return response;
}

std::string OpenStreetMapAPI::executeNominatimQuery(const std::string& endpoint) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, config_.requestTimeoutMs);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, config_.connectTimeoutMs);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, config_.userAgent.c_str());
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            response = "{\"error\": \"" + std::string(curl_easy_strerror(res)) + "\"}";
        }

        curl_easy_cleanup(curl);
    }

    return response;
}

// Helper to extract JSON string value
static std::string extractJsonString(const std::string& json, const std::string& key) {
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
        while (valueEnd < json.length() && (std::isdigit(json[valueEnd]) || json[valueEnd] == '.' || json[valueEnd] == '-')) {
            valueEnd++;
        }
        return json.substr(valueStart, valueEnd - valueStart);
    }

    return "";
}

// Helper to extract JSON number value
static double extractJsonNumber(const std::string& json, const std::string& key) {
    std::string value = extractJsonString(json, key);
    if (value.empty()) return 0.0;
    try {
        return std::stod(value);
    } catch (...) {
        return 0.0;
    }
}

// Helper to extract tags from JSON object
static std::map<std::string, std::string> extractJsonTags(const std::string& json) {
    std::map<std::string, std::string> tags;

    size_t tagsStart = json.find("\"tags\"");
    if (tagsStart == std::string::npos) return tags;

    size_t braceStart = json.find('{', tagsStart);
    if (braceStart == std::string::npos) return tags;

    // Find matching closing brace
    int braceCount = 1;
    size_t braceEnd = braceStart + 1;
    while (braceEnd < json.length() && braceCount > 0) {
        if (json[braceEnd] == '{') braceCount++;
        else if (json[braceEnd] == '}') braceCount--;
        braceEnd++;
    }

    std::string tagsJson = json.substr(braceStart + 1, braceEnd - braceStart - 2);

    // Parse key-value pairs
    size_t pos = 0;
    while (pos < tagsJson.length()) {
        // Find key
        size_t keyStart = tagsJson.find('"', pos);
        if (keyStart == std::string::npos) break;
        size_t keyEnd = tagsJson.find('"', keyStart + 1);
        if (keyEnd == std::string::npos) break;
        std::string key = tagsJson.substr(keyStart + 1, keyEnd - keyStart - 1);

        // Find value
        size_t colonPos = tagsJson.find(':', keyEnd);
        if (colonPos == std::string::npos) break;

        size_t valueStart = tagsJson.find('"', colonPos);
        if (valueStart == std::string::npos) break;
        size_t valueEnd = tagsJson.find('"', valueStart + 1);
        if (valueEnd == std::string::npos) break;
        std::string value = tagsJson.substr(valueStart + 1, valueEnd - valueStart - 1);

        tags[key] = value;
        pos = valueEnd + 1;
    }

    return tags;
}

std::vector<OSMPoi> OpenStreetMapAPI::parseOverpassResponse(const std::string& json) {
    std::vector<OSMPoi> pois;

    // Check for error
    if (json.find("\"error\"") != std::string::npos) {
        return pois;
    }

    // Find "elements" array
    size_t elementsPos = json.find("\"elements\"");
    if (elementsPos == std::string::npos) {
        return pois;
    }

    size_t arrayStart = json.find('[', elementsPos);
    if (arrayStart == std::string::npos) {
        return pois;
    }

    // Parse each element object
    size_t pos = arrayStart + 1;
    while (pos < json.length()) {
        // Find next object
        size_t objStart = json.find('{', pos);
        if (objStart == std::string::npos) break;

        // Find end of object (matching brace)
        int braceCount = 1;
        size_t objEnd = objStart + 1;
        while (objEnd < json.length() && braceCount > 0) {
            if (json[objEnd] == '{') braceCount++;
            else if (json[objEnd] == '}') braceCount--;
            objEnd++;
        }

        std::string objJson = json.substr(objStart, objEnd - objStart);

        OSMPoi poi;

        // Extract basic fields
        poi.osmType = extractJsonString(objJson, "type");
        std::string idStr = extractJsonString(objJson, "id");
        if (!idStr.empty()) {
            try {
                poi.osmId = std::stoll(idStr);
            } catch (...) {
                poi.osmId = 0;
            }
        }

        // Get coordinates - check for "center" (for ways) or direct lat/lon (for nodes)
        if (objJson.find("\"center\"") != std::string::npos) {
            size_t centerPos = objJson.find("\"center\"");
            size_t centerStart = objJson.find('{', centerPos);
            size_t centerEnd = objJson.find('}', centerStart);
            if (centerStart != std::string::npos && centerEnd != std::string::npos) {
                std::string centerJson = objJson.substr(centerStart, centerEnd - centerStart + 1);
                poi.latitude = extractJsonNumber(centerJson, "lat");
                poi.longitude = extractJsonNumber(centerJson, "lon");
            }
        } else {
            poi.latitude = extractJsonNumber(objJson, "lat");
            poi.longitude = extractJsonNumber(objJson, "lon");
        }

        // Extract tags
        poi.tags = extractJsonTags(objJson);

        // Set common tag fields
        auto tagIt = poi.tags.find("name");
        if (tagIt != poi.tags.end()) poi.name = tagIt->second;

        tagIt = poi.tags.find("amenity");
        if (tagIt != poi.tags.end()) poi.amenity = tagIt->second;

        tagIt = poi.tags.find("building");
        if (tagIt != poi.tags.end()) poi.building = tagIt->second;

        tagIt = poi.tags.find("office");
        if (tagIt != poi.tags.end()) poi.office = tagIt->second;

        tagIt = poi.tags.find("shop");
        if (tagIt != poi.tags.end()) poi.shop = tagIt->second;

        tagIt = poi.tags.find("tourism");
        if (tagIt != poi.tags.end()) poi.tourism = tagIt->second;

        tagIt = poi.tags.find("healthcare");
        if (tagIt != poi.tags.end()) poi.healthcare = tagIt->second;

        // Address info
        tagIt = poi.tags.find("addr:street");
        if (tagIt != poi.tags.end()) poi.street = tagIt->second;

        tagIt = poi.tags.find("addr:housenumber");
        if (tagIt != poi.tags.end()) poi.houseNumber = tagIt->second;

        tagIt = poi.tags.find("addr:city");
        if (tagIt != poi.tags.end()) poi.city = tagIt->second;

        tagIt = poi.tags.find("addr:postcode");
        if (tagIt != poi.tags.end()) poi.postcode = tagIt->second;

        tagIt = poi.tags.find("addr:state");
        if (tagIt != poi.tags.end()) poi.state = tagIt->second;

        tagIt = poi.tags.find("addr:country");
        if (tagIt != poi.tags.end()) poi.country = tagIt->second;

        // Contact info
        tagIt = poi.tags.find("phone");
        if (tagIt != poi.tags.end()) poi.phone = tagIt->second;

        tagIt = poi.tags.find("website");
        if (tagIt != poi.tags.end()) poi.website = tagIt->second;

        tagIt = poi.tags.find("email");
        if (tagIt != poi.tags.end()) poi.email = tagIt->second;

        tagIt = poi.tags.find("opening_hours");
        if (tagIt != poi.tags.end()) poi.openingHours = tagIt->second;

        // Only add POIs with valid coordinates and preferably a name
        if (poi.latitude != 0.0 && poi.longitude != 0.0) {
            // Generate a name from type if name is empty
            if (poi.name.empty()) {
                if (!poi.office.empty()) {
                    poi.name = "Office (" + poi.office + ")";
                } else if (!poi.building.empty()) {
                    poi.name = "Building (" + poi.building + ")";
                } else if (!poi.amenity.empty()) {
                    poi.name = poi.amenity.substr(0, 1);
                    std::transform(poi.name.begin(), poi.name.end(), poi.name.begin(), ::toupper);
                    poi.name += poi.amenity.substr(1);
                } else if (!poi.tourism.empty()) {
                    poi.name = poi.tourism;
                }
            }
            pois.push_back(poi);
        }

        pos = objEnd;
        if (pos < json.length() && json[pos] == ']') break;
    }

    // Limit results to configured maximum
    if (pois.size() > static_cast<size_t>(config_.maxResultsPerQuery)) {
        pois.resize(config_.maxResultsPerQuery);
    }

    return pois;
}

OSMPoi OpenStreetMapAPI::parseNominatimResponse(const std::string& json) {
    OSMPoi poi;

    // Check for error or empty result
    if (json.empty() || json.find("\"error\"") != std::string::npos || json == "[]") {
        return poi;
    }

    // Handle array response (search returns array)
    std::string objJson = json;
    if (json[0] == '[') {
        size_t objStart = json.find('{');
        if (objStart == std::string::npos) return poi;
        size_t objEnd = json.find('}', objStart);
        if (objEnd == std::string::npos) return poi;
        objJson = json.substr(objStart, objEnd - objStart + 1);
    }

    poi.latitude = extractJsonNumber(objJson, "lat");
    poi.longitude = extractJsonNumber(objJson, "lon");
    poi.name = extractJsonString(objJson, "display_name");

    // Extract address components if present
    size_t addrPos = objJson.find("\"address\"");
    if (addrPos != std::string::npos) {
        size_t addrStart = objJson.find('{', addrPos);
        size_t addrEnd = objJson.find('}', addrStart);
        if (addrStart != std::string::npos && addrEnd != std::string::npos) {
            std::string addrJson = objJson.substr(addrStart, addrEnd - addrStart + 1);
            poi.street = extractJsonString(addrJson, "road");
            poi.houseNumber = extractJsonString(addrJson, "house_number");
            poi.city = extractJsonString(addrJson, "city");
            if (poi.city.empty()) poi.city = extractJsonString(addrJson, "town");
            if (poi.city.empty()) poi.city = extractJsonString(addrJson, "village");
            poi.state = extractJsonString(addrJson, "state");
            poi.postcode = extractJsonString(addrJson, "postcode");
            poi.country = extractJsonString(addrJson, "country");
        }
    }

    return poi;
}

std::vector<std::string> OpenStreetMapAPI::getOSMFiltersForBusinessTypes(
    const std::vector<Models::BusinessType>& types
) {
    std::vector<std::string> filters;

    for (const auto& type : types) {
        switch (type) {
            case Models::BusinessType::CORPORATE_OFFICE:
                filters.push_back("\"office\"");
                filters.push_back("\"building\"=\"office\"");
                break;
            case Models::BusinessType::WAREHOUSE:
                filters.push_back("\"building\"=\"warehouse\"");
                break;
            case Models::BusinessType::CONFERENCE_CENTER:
                filters.push_back("\"amenity\"=\"conference_centre\"");
                filters.push_back("\"amenity\"=\"events_venue\"");
                break;
            case Models::BusinessType::HOTEL:
                filters.push_back("\"tourism\"=\"hotel\"");
                break;
            case Models::BusinessType::MEDICAL_FACILITY:
                filters.push_back("\"amenity\"=\"hospital\"");
                filters.push_back("\"amenity\"=\"clinic\"");
                break;
            case Models::BusinessType::EDUCATIONAL_INSTITUTION:
                filters.push_back("\"amenity\"=\"university\"");
                filters.push_back("\"amenity\"=\"college\"");
                break;
            case Models::BusinessType::MANUFACTURING:
                filters.push_back("\"building\"=\"industrial\"");
                filters.push_back("\"landuse\"=\"industrial\"");
                break;
            case Models::BusinessType::TECH_COMPANY:
                filters.push_back("\"office\"=\"it\"");
                filters.push_back("\"office\"=\"telecommunication\"");
                break;
            case Models::BusinessType::FINANCIAL_SERVICES:
                filters.push_back("\"office\"=\"financial\"");
                filters.push_back("\"amenity\"=\"bank\"");
                break;
            case Models::BusinessType::COWORKING_SPACE:
                filters.push_back("\"amenity\"=\"coworking_space\"");
                filters.push_back("\"office\"=\"coworking\"");
                break;
            case Models::BusinessType::GOVERNMENT_OFFICE:
                filters.push_back("\"office\"=\"government\"");
                filters.push_back("\"building\"=\"government\"");
                break;
            default:
                break;
        }
    }

    return filters;
}

// Demo data generation for prototype testing
std::vector<OSMPoi> OpenStreetMapAPI::generateDemoPOIs(
    double lat,
    double lon,
    int radiusMeters
) {
    std::vector<OSMPoi> pois;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> latDist(-0.01, 0.01);
    std::uniform_real_distribution<> lonDist(-0.01, 0.01);
    std::uniform_int_distribution<> idDist(100000, 999999);

    // Sample POI data
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> sampleData = {
        {"TechVenture Corporate HQ", "office", "company", "Technology company headquarters"},
        {"Metro Business Center", "building", "office", "Multi-tenant office building"},
        {"Innovation Hub Coworking", "amenity", "coworking_space", "Shared workspace facility"},
        {"Grand Convention Center", "amenity", "conference_centre", "Event and conference venue"},
        {"Riverside Medical Center", "amenity", "hospital", "Regional healthcare facility"},
        {"State University Main Campus", "amenity", "university", "Higher education institution"},
        {"Downtown Marriott Hotel", "tourism", "hotel", "Full-service business hotel"},
        {"First National Bank Tower", "office", "financial", "Financial services headquarters"},
        {"City Hall Complex", "office", "government", "Municipal government offices"},
        {"Pacific Logistics Warehouse", "building", "warehouse", "Distribution and fulfillment center"},
        {"Apex Manufacturing Plant", "building", "industrial", "Industrial manufacturing facility"},
        {"Healthcare Associates Clinic", "amenity", "clinic", "Medical clinic and offices"},
        {"Community Foundation Center", "office", "ngo", "Non-profit organization"},
        {"Smith and Associates Law Firm", "office", "lawyer", "Corporate law firm"},
        {"Regional Tech Park", "office", "it", "Technology business park"},
        {"Sunrise Senior Care Center", "amenity", "hospital", "Healthcare and senior services"},
        {"Enterprise Solutions Inc", "office", "company", "Business consulting firm"},
        {"Central Business Plaza", "building", "commercial", "Commercial office complex"},
        {"Valley Conference Hotel", "tourism", "hotel", "Hotel with conference facilities"},
        {"Metro Cowork Spaces", "amenity", "coworking_space", "Flexible workspace provider"}
    };

    std::vector<std::string> streets = {
        "Main Street", "Commerce Drive", "Business Park Way", "Corporate Boulevard",
        "Innovation Lane", "Enterprise Road", "Technology Circle", "Professional Parkway"
    };

    int numResults = std::min(static_cast<int>(sampleData.size()),
                              config_.maxResultsPerQuery);

    for (int i = 0; i < numResults; ++i) {
        OSMPoi poi;
        auto& [name, tagKey, tagValue, desc] = sampleData[i];

        poi.osmId = idDist(gen);
        poi.osmType = "way";
        poi.name = name;
        poi.latitude = lat + latDist(gen) * (radiusMeters / 1000.0);
        poi.longitude = lon + lonDist(gen) * (radiusMeters / 1000.0);

        // Set the appropriate tag
        poi.tags[tagKey] = tagValue;
        if (tagKey == "office") poi.office = tagValue;
        else if (tagKey == "building") poi.building = tagValue;
        else if (tagKey == "amenity") poi.amenity = tagValue;
        else if (tagKey == "tourism") poi.tourism = tagValue;

        // Address
        poi.houseNumber = std::to_string(100 + i * 50);
        poi.street = streets[i % streets.size()];
        poi.city = "Sample City";
        poi.state = "ST";
        poi.postcode = "12345";
        poi.country = "USA";

        // Contact info (random generation)
        poi.phone = "(555) " + std::to_string(100 + i) + "-" + std::to_string(1000 + i * 111);
        std::string domain = name.substr(0, name.find(' '));
        std::transform(domain.begin(), domain.end(), domain.begin(), ::tolower);
        poi.website = "www." + domain + ".com";
        poi.email = "info@" + domain + ".com";

        pois.push_back(poi);
    }

    return pois;
}

OSMAreaStats OpenStreetMapAPI::generateDemoAreaStats(
    double lat,
    double lon,
    double radiusKm
) {
    OSMAreaStats stats;
    std::random_device rd;
    std::mt19937 gen(rd());

    stats.areaName = "Sample Area";
    stats.centerLat = lat;
    stats.centerLon = lon;
    stats.radiusKm = radiusKm;

    // Generate realistic-looking statistics based on area size
    double areaFactor = radiusKm * radiusKm * 3.14159;  // Area in sq km

    std::uniform_int_distribution<> baseDist(5, 15);
    int multiplier = baseDist(gen);

    stats.offices = static_cast<int>(areaFactor * multiplier * 0.8);
    stats.restaurants = static_cast<int>(areaFactor * multiplier * 1.5);
    stats.cafes = static_cast<int>(areaFactor * multiplier * 0.9);
    stats.hotels = static_cast<int>(areaFactor * multiplier * 0.2);
    stats.conferenceVenues = static_cast<int>(areaFactor * multiplier * 0.1);
    stats.hospitals = static_cast<int>(areaFactor * multiplier * 0.05);
    stats.schools = static_cast<int>(areaFactor * multiplier * 0.3);
    stats.universities = static_cast<int>(areaFactor * multiplier * 0.02);
    stats.industrialBuildings = static_cast<int>(areaFactor * multiplier * 0.4);
    stats.warehouses = static_cast<int>(areaFactor * multiplier * 0.3);
    stats.shops = static_cast<int>(areaFactor * multiplier * 2.0);
    stats.banks = static_cast<int>(areaFactor * multiplier * 0.15);
    stats.governmentBuildings = static_cast<int>(areaFactor * multiplier * 0.1);

    // Infrastructure
    stats.parkingLots = static_cast<int>(areaFactor * multiplier * 0.6);
    stats.busStops = static_cast<int>(areaFactor * multiplier * 0.8);
    stats.railwayStations = static_cast<int>(areaFactor * multiplier * 0.02);

    stats.totalPois = stats.offices + stats.restaurants + stats.cafes +
                      stats.hotels + stats.conferenceVenues + stats.hospitals +
                      stats.schools + stats.universities + stats.industrialBuildings +
                      stats.warehouses + stats.shops + stats.banks + stats.governmentBuildings;

    stats.calculateMetrics();

    return stats;
}

void OSMAreaStats::calculateMetrics() {
    // Calculate business density
    double areaSqKm = 3.14159 * radiusKm * radiusKm;
    if (areaSqKm > 0) {
        businessDensityPerSqKm = static_cast<double>(totalPois) / areaSqKm;
    }

    // Calculate market potential score (0-100)
    int score = 0;

    // Office density scoring
    if (offices > 50) score += 15;
    else if (offices > 20) score += 10;
    else if (offices > 10) score += 5;

    // Conference venues (high value)
    if (conferenceVenues > 5) score += 15;
    else if (conferenceVenues > 2) score += 10;
    else if (conferenceVenues > 0) score += 5;

    // Hotels (catering prospects)
    if (hotels > 10) score += 10;
    else if (hotels > 5) score += 7;
    else if (hotels > 2) score += 4;

    // Healthcare (regular meal services)
    if (hospitals > 3) score += 10;
    else if (hospitals > 1) score += 6;
    else if (hospitals > 0) score += 3;

    // Educational (cafeteria/event catering)
    if (universities > 2) score += 10;
    else if (universities > 0) score += 5;
    if (schools > 10) score += 5;

    // Industrial/warehouse (employee meals)
    if (industrialBuildings + warehouses > 20) score += 10;
    else if (industrialBuildings + warehouses > 10) score += 6;
    else if (industrialBuildings + warehouses > 5) score += 3;

    // Business density bonus
    if (businessDensityPerSqKm > 100) score += 15;
    else if (businessDensityPerSqKm > 50) score += 10;
    else if (businessDensityPerSqKm > 20) score += 5;

    marketPotentialScore = std::min(100, score);
}

std::string OSMAreaStats::getMarketQualityDescription() const {
    if (marketPotentialScore >= 80) return "Excellent";
    if (marketPotentialScore >= 60) return "Very Good";
    if (marketPotentialScore >= 40) return "Good";
    if (marketPotentialScore >= 20) return "Fair";
    return "Limited";
}

// ===== GeoLocation-based API implementations =====

void OpenStreetMapAPI::searchBusinesses(
    const Models::SearchArea& searchArea,
    BusinessCallback callback
) {
    searchBusinesses(
        searchArea.center.latitude,
        searchArea.center.longitude,
        searchArea.radiusMiles,
        callback
    );
}

std::vector<Models::BusinessInfo> OpenStreetMapAPI::searchBusinessesSync(
    const Models::SearchArea& searchArea
) {
    return searchBusinessesSync(
        searchArea.center.latitude,
        searchArea.center.longitude,
        searchArea.radiusMiles
    );
}

void OpenStreetMapAPI::searchCateringProspects(
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

void OpenStreetMapAPI::getAreaStatistics(
    const Models::SearchArea& searchArea,
    AreaStatsCallback callback
) {
    getAreaStatistics(
        searchArea.center.latitude,
        searchArea.center.longitude,
        searchArea.radiusKm,
        callback
    );
}

OSMAreaStats OpenStreetMapAPI::getAreaStatisticsSync(const Models::SearchArea& searchArea) {
    return getAreaStatisticsSync(
        searchArea.center.latitude,
        searchArea.center.longitude,
        searchArea.radiusKm
    );
}

void OpenStreetMapAPI::searchNearby(
    const Models::SearchArea& searchArea,
    POICallback callback
) {
    searchNearby(
        searchArea.center.latitude,
        searchArea.center.longitude,
        searchArea.radiusMeters(),
        callback
    );
}

Models::GeoLocation OpenStreetMapAPI::poiToGeoLocation(const OSMPoi& poi) {
    Models::GeoLocation location(poi.latitude, poi.longitude);
    location.street = poi.houseNumber.empty() ? poi.street : poi.houseNumber + " " + poi.street;
    location.city = poi.city;
    location.state = poi.state;
    location.postalCode = poi.postcode;
    location.country = poi.country.empty() ? "USA" : poi.country;
    location.source = "openstreetmap";

    // Build formatted address
    std::string addr;
    if (!location.street.empty()) addr = location.street;
    if (!location.city.empty()) {
        if (!addr.empty()) addr += ", ";
        addr += location.city;
    }
    if (!location.state.empty()) {
        if (!addr.empty()) addr += ", ";
        addr += location.state;
    }
    if (!location.postalCode.empty()) {
        if (!addr.empty()) addr += " ";
        addr += location.postalCode;
    }
    location.formattedAddress = addr;

    return location;
}

void OpenStreetMapAPI::searchByCategory(
    const Models::SearchArea& searchArea,
    const std::string& category,
    POICallback callback
) {
    // Generate demo POIs filtered by category
    auto pois = searchByCategorySync(searchArea, category);
    callback(pois, "");
}

std::vector<OSMPoi> OpenStreetMapAPI::searchByCategorySync(
    const Models::SearchArea& searchArea,
    const std::string& category
) {
    ++totalApiCalls_;

    // Normalize category name
    std::string categoryLower = category;
    std::transform(categoryLower.begin(), categoryLower.end(), categoryLower.begin(), ::tolower);

    // Build Overpass query for this category
    std::ostringstream query;
    query << std::fixed << std::setprecision(6);
    query << "[out:json][timeout:30];(";

    int radiusMeters = searchArea.radiusMeters();
    double lat = searchArea.center.latitude;
    double lon = searchArea.center.longitude;

    // Category-specific queries
    if (categoryLower == "offices") {
        query << "node[\"office\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"office\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"building\"=\"office\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"building\"=\"commercial\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    } else if (categoryLower == "hotels") {
        query << "node[\"tourism\"=\"hotel\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"tourism\"=\"hotel\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "node[\"tourism\"=\"motel\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"tourism\"=\"motel\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    } else if (categoryLower == "conference") {
        query << "node[\"amenity\"=\"conference_centre\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"amenity\"=\"conference_centre\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "node[\"amenity\"=\"events_venue\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"amenity\"=\"events_venue\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    } else if (categoryLower == "hospitals") {
        query << "node[\"amenity\"=\"hospital\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"amenity\"=\"hospital\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "node[\"amenity\"=\"clinic\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"amenity\"=\"clinic\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    } else if (categoryLower == "universities") {
        query << "node[\"amenity\"=\"university\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"amenity\"=\"university\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "node[\"amenity\"=\"college\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"amenity\"=\"college\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    } else if (categoryLower == "schools") {
        query << "node[\"amenity\"=\"school\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"amenity\"=\"school\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    } else if (categoryLower == "industrial") {
        query << "way[\"building\"=\"industrial\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"landuse\"=\"industrial\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    } else if (categoryLower == "warehouses") {
        query << "way[\"building\"=\"warehouse\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    } else if (categoryLower == "banks") {
        query << "node[\"amenity\"=\"bank\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"amenity\"=\"bank\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "node[\"office\"=\"financial\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"office\"=\"financial\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    } else if (categoryLower == "government") {
        query << "node[\"office\"=\"government\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"office\"=\"government\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"building\"=\"government\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    } else if (categoryLower == "restaurants") {
        query << "node[\"amenity\"=\"restaurant\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"amenity\"=\"restaurant\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    } else if (categoryLower == "cafes") {
        query << "node[\"amenity\"=\"cafe\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
        query << "way[\"amenity\"=\"cafe\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    } else {
        // Unknown category - return empty
        return {};
    }

    query << ");out center;";

    // Execute the Overpass query
    std::string response = executeOverpassQuery(query.str());

    // Parse and return results
    return parseOverpassResponse(response);
}

} // namespace Services
} // namespace FranchiseAI
