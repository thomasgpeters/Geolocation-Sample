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

    // For demo/prototype, generate sample data
    // In production, this would call the actual Overpass API
    auto results = generateDemoPOIs(latitude, longitude, radiusMeters);

    // Cache results
    if (config_.enableCaching) {
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

    // Demo: Return coordinates for common test locations
    // In production, this would call Nominatim API
    double lat = 39.7392;  // Denver, CO default
    double lon = -104.9903;

    // Simple pattern matching for demo
    if (address.find("New York") != std::string::npos || address.find("NYC") != std::string::npos) {
        lat = 40.7128; lon = -74.0060;
    } else if (address.find("Los Angeles") != std::string::npos || address.find("LA") != std::string::npos) {
        lat = 34.0522; lon = -118.2437;
    } else if (address.find("Chicago") != std::string::npos) {
        lat = 41.8781; lon = -87.6298;
    } else if (address.find("Houston") != std::string::npos) {
        lat = 29.7604; lon = -95.3698;
    } else if (address.find("Phoenix") != std::string::npos) {
        lat = 33.4484; lon = -112.0740;
    } else if (address.find("San Francisco") != std::string::npos || address.find("SF") != std::string::npos) {
        lat = 37.7749; lon = -122.4194;
    } else if (address.find("Seattle") != std::string::npos) {
        lat = 37.7749; lon = -122.4194;
    } else if (address.find("Austin") != std::string::npos) {
        lat = 30.2672; lon = -97.7431;
    } else if (address.find("Boston") != std::string::npos) {
        lat = 42.3601; lon = -71.0589;
    } else if (address.find("Atlanta") != std::string::npos) {
        lat = 33.7490; lon = -84.3880;
    }

    if (callback) {
        callback(lat, lon, "");
    }
}

void OpenStreetMapAPI::reverseGeocode(
    double latitude,
    double longitude,
    std::function<void(OSMPoi, std::string)> callback
) {
    ++totalApiCalls_;

    OSMPoi result;
    result.latitude = latitude;
    result.longitude = longitude;
    result.city = "Sample City";
    result.state = "ST";
    result.postcode = "12345";
    result.country = "USA";

    if (callback) {
        callback(result, "");
    }
}

std::vector<Models::BusinessInfo> OpenStreetMapAPI::searchBusinessesSync(
    double latitude,
    double longitude,
    double radiusMiles
) {
    std::vector<Models::BusinessInfo> results;
    searchBusinesses(latitude, longitude, radiusMiles,
        [&results](std::vector<Models::BusinessInfo> businesses, const std::string&) {
            results = std::move(businesses);
        });
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
    std::ostringstream query;
    query << std::fixed << std::setprecision(6);
    query << "[out:json][timeout:30];(";

    // Offices and corporate buildings
    query << "node[\"office\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    query << "way[\"office\"](around:" << radiusMeters << "," << lat << "," << lon << ");";

    // Commercial buildings
    query << "way[\"building\"=\"office\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    query << "way[\"building\"=\"commercial\"](around:" << radiusMeters << "," << lat << "," << lon << ");";

    // Conference centers and venues
    query << "node[\"amenity\"=\"conference_centre\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    query << "way[\"amenity\"=\"conference_centre\"](around:" << radiusMeters << "," << lat << "," << lon << ");";

    // Hotels
    query << "node[\"tourism\"=\"hotel\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    query << "way[\"tourism\"=\"hotel\"](around:" << radiusMeters << "," << lat << "," << lon << ");";

    // Medical facilities
    query << "node[\"amenity\"=\"hospital\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    query << "way[\"amenity\"=\"hospital\"](around:" << radiusMeters << "," << lat << "," << lon << ");";

    // Educational institutions
    query << "node[\"amenity\"=\"university\"](around:" << radiusMeters << "," << lat << "," << lon << ");";
    query << "way[\"amenity\"=\"university\"](around:" << radiusMeters << "," << lat << "," << lon << ");";

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
        curl_easy_setopt(curl, CURLOPT_USERAGENT, config_.userAgent.c_str());

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            response = "{\"error\": \"" + std::string(curl_easy_strerror(res)) + "\"}";
        }

        curl_easy_cleanup(curl);
    }

    return response;
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
    // Category to OSM tag mapping
    static const std::map<std::string, std::vector<std::pair<std::string, std::string>>> categoryTagMap = {
        {"offices", {{"office", "company"}, {"office", "corporate"}, {"office", "it"}, {"building", "office"}, {"building", "commercial"}}},
        {"hotels", {{"tourism", "hotel"}, {"building", "hotel"}}},
        {"conference", {{"amenity", "conference_centre"}, {"amenity", "events_venue"}}},
        {"hospitals", {{"amenity", "hospital"}, {"amenity", "clinic"}, {"building", "hospital"}}},
        {"universities", {{"amenity", "university"}, {"amenity", "college"}, {"building", "university"}}},
        {"schools", {{"amenity", "school"}, {"building", "school"}}},
        {"industrial", {{"building", "industrial"}, {"landuse", "industrial"}}},
        {"warehouses", {{"building", "warehouse"}}},
        {"banks", {{"amenity", "bank"}, {"office", "financial"}}},
        {"government", {{"office", "government"}, {"building", "government"}}},
        {"restaurants", {{"amenity", "restaurant"}, {"amenity", "fast_food"}}},
        {"cafes", {{"amenity", "cafe"}}}
    };

    // Normalize category name
    std::string categoryLower = category;
    std::transform(categoryLower.begin(), categoryLower.end(), categoryLower.begin(), ::tolower);

    auto tagIt = categoryTagMap.find(categoryLower);
    if (tagIt == categoryTagMap.end()) {
        return {};  // Unknown category
    }

    // Generate demo POIs for this category
    std::vector<OSMPoi> pois;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> latDist(-0.008, 0.008);
    std::uniform_real_distribution<> lonDist(-0.008, 0.008);
    std::uniform_int_distribution<> idDist(100000, 999999);
    std::uniform_int_distribution<> numDist(100, 500);

    // Sample business names by category (expanded for more POIs)
    static const std::map<std::string, std::vector<std::string>> categoryNames = {
        {"offices", {"TechVenture Corp", "Metro Business Center", "Innovation Hub", "Enterprise Solutions", "Global Dynamics", "Summit Partners", "Apex Consulting", "Premier Holdings", "DataTech Inc", "CloudWorks HQ", "Synergy Group", "Vanguard Systems", "NextGen Solutions", "Pioneer Analytics", "Quantum Enterprises", "Horizon Partners", "Cascade Business Park", "Sterling Office Complex", "Matrix Technologies", "Vertex Solutions", "Catalyst Group", "Pinnacle Partners", "Nexus Business Center", "Elevate Workspace", "Momentum Labs"}},
        {"hotels", {"Downtown Marriott", "Grand Plaza Hotel", "Valley Conference Hotel", "Business Suites Inn", "Executive Stay Hotel", "Metro Lodge", "Corporate Inn", "Parkview Hotel", "Summit Hotel & Spa", "Riverside Suites", "Capitol Hotel", "Gateway Inn", "Prestige Hotel", "Ambassador Suites", "Crown Plaza", "Heritage Hotel", "Lakeside Resort", "Urban Stay Hotel", "Beacon Hotel", "Skyline Suites", "Cornerstone Inn", "Westgate Hotel", "Eastside Suites", "Northpoint Hotel", "Central Station Hotel"}},
        {"conference", {"Grand Convention Center", "Metro Events Center", "City Conference Hall", "Business Expo Center", "Summit Meeting Center", "Premier Conference Venue", "Lakeside Convention Hall", "Executive Conference Center", "Innovation Summit Hall", "Trade Show Arena", "Corporate Events Plaza", "Business Forum Center", "Leadership Conference Hall", "Tech Summit Venue", "Regional Expo Center", "Downtown Conference Complex", "Riverside Meeting Hall", "Skyline Events Center", "Capitol Conference Center", "Unity Convention Hall"}},
        {"hospitals", {"Regional Medical Center", "St. Mary Hospital", "Community Health Center", "Valley Medical Clinic", "Metro Healthcare", "Sunrise Medical Center", "General Hospital", "University Medical Center", "Children's Hospital", "Veterans Medical Center", "Mercy Hospital", "Providence Health", "Sacred Heart Medical", "Good Samaritan Hospital", "Memorial Medical Center", "County General Hospital", "Westside Medical Center", "Eastgate Health Clinic", "Northview Hospital", "Southpark Medical"}},
        {"universities", {"State University", "Metro Technical College", "City Community College", "Regional University", "Business Academy", "Technology Institute", "Liberal Arts College", "Graduate School of Business", "Engineering University", "Medical School Campus", "Law School", "Design Institute", "Performing Arts Academy", "Science & Research University", "Agricultural College", "Maritime Academy", "Aviation Institute", "Culinary School", "Trade Technical College", "Online University Campus"}},
        {"schools", {"Central High School", "Lincoln Middle School", "Washington Elementary", "Oak Valley Academy", "Metro Prep School", "Riverside High", "Lakeview Middle School", "Jefferson Elementary", "Madison Academy", "Roosevelt High School", "Kennedy Middle School", "Franklin Elementary", "Adams Preparatory", "Hamilton High", "Monroe Middle School", "Jackson Elementary", "Wilson Academy", "Taylor High School", "Polk Middle School", "Harrison Elementary"}},
        {"industrial", {"Apex Manufacturing", "Metro Industrial Park", "Valley Production Center", "Summit Factory", "Industrial Tech Center", "Precision Manufacturing", "Advanced Materials Plant", "Heavy Industries Complex", "Assembly Line Facility", "Production Hub", "Manufacturing Solutions", "Industrial Dynamics", "Fabrication Center", "Process Industries", "Component Factory", "Quality Manufacturing", "Integrated Production", "Supply Chain Hub", "Logistics Manufacturing", "Industrial Innovation Center"}},
        {"warehouses", {"Pacific Logistics Center", "Metro Distribution", "Valley Storage Solutions", "Central Fulfillment", "Express Warehouse", "Regional Distribution Hub", "Supply Chain Warehouse", "E-Commerce Fulfillment", "Cold Storage Facility", "Bulk Storage Center", "Cross-Dock Facility", "Inventory Hub", "Distribution Solutions", "Freight Warehouse", "Import Export Storage", "Last Mile Hub", "Returns Processing Center", "Overflow Storage", "Seasonal Warehouse", "Archive Storage Facility"}},
        {"banks", {"First National Bank", "Metro Credit Union", "Valley Savings Bank", "Business Financial Center", "Regional Trust Bank", "Commerce Bank", "Citizens Bank", "Peoples Credit Union", "Investment Banking Corp", "Community Savings", "Capital One Branch", "Wells Financial", "Chase Business Center", "Bank of Commerce", "Savings & Loan", "Federal Credit Union", "Merchant Bank", "Private Banking Suite", "Corporate Finance Center", "Agricultural Credit Bank"}},
        {"government", {"City Hall Complex", "County Administration", "State Services Building", "Federal Office Building", "Municipal Center", "Department of Motor Vehicles", "Social Services Office", "Tax Assessment Office", "Planning & Zoning Dept", "Public Works Building", "Parks & Recreation Office", "Health Department", "Employment Services", "Veterans Affairs Office", "Environmental Agency", "Housing Authority", "Licensing Bureau", "Court House", "Police Administration", "Fire Department HQ"}},
        {"restaurants", {"The Corporate Grill", "Business District Cafe", "Executive Dining", "Metro Bistro", "Downtown Eatery", "Valley Kitchen", "Summit Restaurant", "Riverside Grill", "The Meeting Place", "Power Lunch Spot", "Uptown Brasserie", "Financial District Deli", "Corner Office Cafe", "The Board Room Restaurant", "Midtown Kitchen", "Professional Plaza Dining", "Commerce Cafe", "Trade Center Grill", "Convention Catering", "Lobby Lounge Restaurant"}},
        {"cafes", {"Metro Coffee House", "Business Brew", "Morning Cup Cafe", "Espresso Corner", "Valley Roasters", "Quick Cafe", "Java Junction", "The Daily Grind", "Latte Lounge", "Bean Counter Cafe", "Sunrise Coffee", "Artisan Roasters", "The Coffee Spot", "Brew & Beyond", "Mocha Moments", "Caffeine Fix", "The Percolator", "Steam Coffee Bar", "Drip Drop Cafe", "Pour Over Place"}}
    };

    auto nameIt = categoryNames.find(categoryLower);
    if (nameIt == categoryNames.end()) {
        return {};
    }

    const auto& names = nameIt->second;
    const auto& tags = tagIt->second;

    std::vector<std::string> streets = {
        "Main Street", "Commerce Drive", "Business Park Way", "Corporate Boulevard",
        "Innovation Lane", "Enterprise Road", "Technology Circle", "Professional Parkway"
    };

    int numResults = std::min(static_cast<int>(names.size()), config_.maxResultsPerQuery);

    for (int i = 0; i < numResults; ++i) {
        OSMPoi poi;
        poi.osmId = idDist(gen);
        poi.osmType = "way";
        poi.name = names[i % names.size()];
        poi.latitude = searchArea.center.latitude + latDist(gen);
        poi.longitude = searchArea.center.longitude + lonDist(gen);

        // Set appropriate tags for this category
        if (!tags.empty()) {
            const auto& [tagKey, tagValue] = tags[i % tags.size()];
            poi.tags[tagKey] = tagValue;
            if (tagKey == "office") poi.office = tagValue;
            else if (tagKey == "building") poi.building = tagValue;
            else if (tagKey == "amenity") poi.amenity = tagValue;
            else if (tagKey == "tourism") poi.tourism = tagValue;
        }

        // Address
        poi.houseNumber = std::to_string(numDist(gen));
        poi.street = streets[i % streets.size()];
        poi.city = searchArea.center.city.empty() ? "Sample City" : searchArea.center.city;
        poi.state = searchArea.center.state.empty() ? "ST" : searchArea.center.state;
        poi.postcode = searchArea.center.postalCode.empty() ? "12345" : searchArea.center.postalCode;
        poi.country = "USA";

        // Contact info
        poi.phone = "(555) " + std::to_string(100 + i) + "-" + std::to_string(1000 + i * 111);
        std::string domain = poi.name.substr(0, poi.name.find(' '));
        std::transform(domain.begin(), domain.end(), domain.begin(), ::tolower);
        // Remove special chars from domain
        domain.erase(std::remove_if(domain.begin(), domain.end(), [](char c) { return !std::isalnum(c); }), domain.end());
        poi.website = "www." + domain + ".com";
        poi.email = "info@" + domain + ".com";

        pois.push_back(poi);
    }

    return pois;
}

} // namespace Services
} // namespace FranchiseAI
