#include "GoogleMyBusinessAPI.h"
#include <random>
#include <ctime>
#include <sstream>
#include <algorithm>

namespace FranchiseAI {
namespace Services {

GoogleMyBusinessAPI::GoogleMyBusinessAPI() = default;

GoogleMyBusinessAPI::GoogleMyBusinessAPI(const GoogleAPIConfig& config)
    : config_(config) {}

GoogleMyBusinessAPI::~GoogleMyBusinessAPI() = default;

void GoogleMyBusinessAPI::setConfig(const GoogleAPIConfig& config) {
    config_ = config;
}

void GoogleMyBusinessAPI::setApiKey(const std::string& apiKey) {
    config_.apiKey = apiKey;
}

void GoogleMyBusinessAPI::searchBusinesses(
    const Models::SearchQuery& query,
    SearchCallback callback
) {
    ++totalApiCalls_;

    // For demo purposes, generate sample data
    // In production, this would make actual API calls
    auto results = generateDemoResults(query);

    if (callback) {
        callback(results, "");
    }
}

void GoogleMyBusinessAPI::searchNearby(
    const std::string& keyword,
    double latitude,
    double longitude,
    int radiusMeters,
    SearchCallback callback
) {
    Models::SearchQuery query;
    query.keywords = keyword;
    query.latitude = latitude;
    query.longitude = longitude;
    query.radiusMiles = radiusMeters / 1609.34;  // Convert to miles

    searchBusinesses(query, callback);
}

void GoogleMyBusinessAPI::getPlaceDetails(
    const std::string& placeId,
    DetailsCallback callback
) {
    ++totalApiCalls_;

    // Generate demo details
    Models::BusinessInfo business;
    business.id = placeId;
    business.name = "Sample Business";
    business.source = Models::DataSource::GOOGLE_MY_BUSINESS;

    if (callback) {
        callback(business, "");
    }
}

void GoogleMyBusinessAPI::searchCateringProspects(
    const std::string& location,
    double radiusMiles,
    SearchCallback callback
) {
    Models::SearchQuery query;
    query.location = location;
    query.radiusMiles = radiusMiles;
    query.keywords = "corporate office conference room warehouse";
    query.minCateringScore = 40;

    searchBusinesses(query, callback);
}

void GoogleMyBusinessAPI::getAutocomplete(
    const std::string& input,
    std::function<void(std::vector<std::string>)> callback
) {
    std::vector<std::string> suggestions = {
        input + " corporate offices",
        input + " conference centers",
        input + " business parks",
        input + " warehouses",
        input + " tech companies"
    };

    if (callback) {
        callback(suggestions);
    }
}

std::vector<Models::BusinessInfo> GoogleMyBusinessAPI::searchBusinessesSync(
    const Models::SearchQuery& query
) {
    return generateDemoResults(query);
}

Models::BusinessInfo GoogleMyBusinessAPI::getPlaceDetailsSync(const std::string& placeId) {
    Models::BusinessInfo business;
    business.id = placeId;
    return business;
}

void GoogleMyBusinessAPI::clearCache() {
    // Clear internal cache
}

int GoogleMyBusinessAPI::getCacheSize() const {
    return 0;
}

void GoogleMyBusinessAPI::resetStatistics() {
    totalApiCalls_ = 0;
}

std::string GoogleMyBusinessAPI::buildSearchUrl(
    const std::string& keyword,
    double lat,
    double lng,
    int radius
) {
    std::ostringstream url;
    url << config_.placeApiEndpoint << "/nearbysearch/json?"
        << "location=" << lat << "," << lng
        << "&radius=" << radius
        << "&keyword=" << keyword
        << "&key=" << config_.apiKey;
    return url.str();
}

std::string GoogleMyBusinessAPI::buildDetailsUrl(const std::string& placeId) {
    std::ostringstream url;
    url << config_.placeApiEndpoint << "/details/json?"
        << "place_id=" << placeId
        << "&key=" << config_.apiKey;
    return url.str();
}

Models::BusinessType GoogleMyBusinessAPI::inferBusinessType(
    const std::vector<std::string>& types
) {
    for (const auto& type : types) {
        if (type == "corporate_office" || type == "office")
            return Models::BusinessType::CORPORATE_OFFICE;
        if (type == "warehouse" || type == "storage")
            return Models::BusinessType::WAREHOUSE;
        if (type == "conference_center" || type == "event_venue")
            return Models::BusinessType::CONFERENCE_CENTER;
        if (type == "hotel" || type == "lodging")
            return Models::BusinessType::HOTEL;
        if (type == "coworking_space")
            return Models::BusinessType::COWORKING_SPACE;
        if (type == "hospital" || type == "medical")
            return Models::BusinessType::MEDICAL_FACILITY;
        if (type == "university" || type == "school")
            return Models::BusinessType::EDUCATIONAL_INSTITUTION;
        if (type == "government" || type == "city_hall")
            return Models::BusinessType::GOVERNMENT_OFFICE;
        if (type == "factory" || type == "manufacturing")
            return Models::BusinessType::MANUFACTURING;
    }
    return Models::BusinessType::OTHER;
}

std::vector<Models::BusinessInfo> GoogleMyBusinessAPI::generateDemoResults(
    const Models::SearchQuery& query
) {
    std::vector<Models::BusinessInfo> results;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> ratingDist(3.5, 5.0);
    std::uniform_int_distribution<> reviewDist(10, 500);
    std::uniform_int_distribution<> empDist(25, 1000);
    std::uniform_real_distribution<> distDist(0.5, query.radiusMiles);

    // Sample business data for demo
    std::vector<std::tuple<std::string, Models::BusinessType, std::string>> sampleBusinesses = {
        {"TechCorp Headquarters", Models::BusinessType::TECH_COMPANY, "Leading technology solutions provider"},
        {"Metro Conference Center", Models::BusinessType::CONFERENCE_CENTER, "Premier event and meeting venue"},
        {"Apex Distribution Warehouse", Models::BusinessType::WAREHOUSE, "Large-scale distribution facility"},
        {"Summit Financial Group", Models::BusinessType::FINANCIAL_SERVICES, "Full-service financial advisory"},
        {"Innovation Hub Coworking", Models::BusinessType::COWORKING_SPACE, "Modern shared workspace"},
        {"Regional Medical Center", Models::BusinessType::MEDICAL_FACILITY, "Comprehensive healthcare services"},
        {"Sterling Law Partners", Models::BusinessType::LAW_FIRM, "Corporate and business law firm"},
        {"Pacific Manufacturing Inc", Models::BusinessType::MANUFACTURING, "Industrial manufacturing plant"},
        {"City Government Complex", Models::BusinessType::GOVERNMENT_OFFICE, "Municipal government offices"},
        {"Grand Hotel & Convention", Models::BusinessType::HOTEL, "Full-service hotel with meeting rooms"},
        {"Nexus Corporate Park", Models::BusinessType::CORPORATE_OFFICE, "Multi-tenant office complex"},
        {"DataStream Analytics", Models::BusinessType::TECH_COMPANY, "Data science and analytics firm"},
        {"Midwest Logistics Hub", Models::BusinessType::WAREHOUSE, "Freight and logistics center"},
        {"Community Foundation", Models::BusinessType::NONPROFIT, "Regional charitable organization"},
        {"State University Campus", Models::BusinessType::EDUCATIONAL_INSTITUTION, "Higher education institution"}
    };

    std::vector<std::string> streets = {
        "123 Business Park Dr", "456 Corporate Blvd", "789 Commerce Way",
        "1000 Industry Lane", "555 Executive Plaza", "200 Tech Center Dr",
        "350 Innovation Ave", "600 Enterprise Rd", "150 Professional Pkwy"
    };

    std::string city = query.city.empty() ? "Springfield" : query.city;
    std::string state = query.state.empty() ? "IL" : query.state;
    std::string zip = query.zipCode.empty() ? "62701" : query.zipCode;

    int numResults = std::min(static_cast<int>(sampleBusinesses.size()),
                             query.pageSize > 0 ? query.pageSize : 10);

    for (int i = 0; i < numResults; ++i) {
        Models::BusinessInfo business;
        auto& [name, type, desc] = sampleBusinesses[i];

        business.id = "gmb_" + std::to_string(i + 1) + "_" + std::to_string(std::time(nullptr));
        business.name = name;
        business.description = desc;
        business.type = type;
        business.source = Models::DataSource::GOOGLE_MY_BUSINESS;

        business.address.street1 = streets[i % streets.size()];
        business.address.city = city;
        business.address.state = state;
        business.address.zipCode = zip;
        business.address.latitude = query.latitude + (gen() % 100 - 50) / 1000.0;
        business.address.longitude = query.longitude + (gen() % 100 - 50) / 1000.0;

        business.contact.primaryPhone = "(555) " + std::to_string(100 + i) + "-" + std::to_string(1000 + i * 111);
        business.contact.website = "www." + name.substr(0, name.find(' ')) + ".com";
        business.contact.email = "info@" + name.substr(0, name.find(' ')) + ".com";
        std::transform(business.contact.website.begin(), business.contact.website.end(),
                      business.contact.website.begin(), ::tolower);
        std::transform(business.contact.email.begin(), business.contact.email.end(),
                      business.contact.email.begin(), ::tolower);

        business.googleRating = ratingDist(gen);
        business.googleReviewCount = reviewDist(gen);
        business.employeeCount = empDist(gen);

        business.hasConferenceRoom = (type == Models::BusinessType::CORPORATE_OFFICE ||
                                      type == Models::BusinessType::CONFERENCE_CENTER ||
                                      type == Models::BusinessType::HOTEL ||
                                      type == Models::BusinessType::TECH_COMPANY ||
                                      type == Models::BusinessType::COWORKING_SPACE);
        business.hasEventSpace = (type == Models::BusinessType::CONFERENCE_CENTER ||
                                  type == Models::BusinessType::HOTEL);
        business.regularMeetings = (type != Models::BusinessType::WAREHOUSE &&
                                    type != Models::BusinessType::MANUFACTURING);

        business.hours.monday = "8:00 AM - 6:00 PM";
        business.hours.tuesday = "8:00 AM - 6:00 PM";
        business.hours.wednesday = "8:00 AM - 6:00 PM";
        business.hours.thursday = "8:00 AM - 6:00 PM";
        business.hours.friday = "8:00 AM - 6:00 PM";
        business.hours.saturday = "Closed";
        business.hours.sunday = "Closed";

        business.calculateCateringPotential();
        business.isVerified = true;
        business.dateAdded = std::time(nullptr);
        business.lastUpdated = std::time(nullptr);

        results.push_back(business);
    }

    // Sort by catering potential
    std::sort(results.begin(), results.end(),
        [](const Models::BusinessInfo& a, const Models::BusinessInfo& b) {
            return a.cateringPotentialScore > b.cateringPotentialScore;
        });

    return results;
}

} // namespace Services
} // namespace FranchiseAI
