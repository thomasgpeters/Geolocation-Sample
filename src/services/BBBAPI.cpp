#include "BBBAPI.h"
#include <random>
#include <ctime>
#include <algorithm>

namespace FranchiseAI {
namespace Services {

BBBAPI::BBBAPI() = default;

BBBAPI::BBBAPI(const BBBAPIConfig& config) : config_(config) {}

BBBAPI::~BBBAPI() = default;

void BBBAPI::setConfig(const BBBAPIConfig& config) {
    config_ = config;
}

void BBBAPI::setApiKey(const std::string& apiKey) {
    config_.apiKey = apiKey;
}

void BBBAPI::searchBusinesses(const Models::SearchQuery& query, SearchCallback callback) {
    ++totalApiCalls_;

    auto results = generateDemoResults(query);

    if (callback) {
        callback(results, "");
    }
}

void BBBAPI::searchByName(
    const std::string& businessName,
    const std::string& city,
    const std::string& state,
    SearchCallback callback
) {
    Models::SearchQuery query;
    query.keywords = businessName;
    query.city = city;
    query.state = state;

    searchBusinesses(query, callback);
}

void BBBAPI::searchAccreditedBusinesses(
    const std::string& zipCode,
    double radiusMiles,
    SearchCallback callback
) {
    Models::SearchQuery query;
    query.zipCode = zipCode;
    query.radiusMiles = radiusMiles;

    searchBusinesses(query, callback);
}

void BBBAPI::getBusinessProfile(const std::string& bbbId, DetailsCallback callback) {
    ++totalApiCalls_;

    Models::BusinessInfo business;
    business.id = bbbId;
    business.name = "BBB Listed Business";
    business.bbbAccredited = true;
    business.bbbRating = Models::BBBRating::A;
    business.source = Models::DataSource::BBB;

    if (callback) {
        callback(business, "");
    }
}

void BBBAPI::getComplaintHistory(const std::string& bbbId, ComplaintsCallback callback) {
    ++totalApiCalls_;

    auto complaints = generateDemoComplaints(bbbId);

    if (callback) {
        callback(complaints, "");
    }
}

bool BBBAPI::checkAccreditation(
    const std::string& businessName,
    const std::string& city,
    const std::string& state
) {
    // Demo implementation - would check actual BBB API
    return true;
}

std::vector<Models::BusinessInfo> BBBAPI::searchBusinessesSync(const Models::SearchQuery& query) {
    return generateDemoResults(query);
}

Models::BusinessInfo BBBAPI::getBusinessProfileSync(const std::string& bbbId) {
    Models::BusinessInfo business;
    business.id = bbbId;
    return business;
}

void BBBAPI::clearCache() {
    // Clear internal cache
}

Models::BBBRating BBBAPI::parseRating(const std::string& ratingStr) {
    if (ratingStr == "A+") return Models::BBBRating::A_PLUS;
    if (ratingStr == "A") return Models::BBBRating::A;
    if (ratingStr == "A-") return Models::BBBRating::A_MINUS;
    if (ratingStr == "B+") return Models::BBBRating::B_PLUS;
    if (ratingStr == "B") return Models::BBBRating::B;
    if (ratingStr == "B-") return Models::BBBRating::B_MINUS;
    if (ratingStr == "C+") return Models::BBBRating::C_PLUS;
    if (ratingStr == "C") return Models::BBBRating::C;
    if (ratingStr == "C-") return Models::BBBRating::C_MINUS;
    if (ratingStr == "D+") return Models::BBBRating::D_PLUS;
    if (ratingStr == "D") return Models::BBBRating::D;
    if (ratingStr == "D-") return Models::BBBRating::D_MINUS;
    if (ratingStr == "F") return Models::BBBRating::F;
    return Models::BBBRating::NOT_RATED;
}

std::vector<Models::BusinessInfo> BBBAPI::generateDemoResults(const Models::SearchQuery& query) {
    std::vector<Models::BusinessInfo> results;
    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<Models::BBBRating> ratings = {
        Models::BBBRating::A_PLUS,
        Models::BBBRating::A,
        Models::BBBRating::A_MINUS,
        Models::BBBRating::B_PLUS,
        Models::BBBRating::A
    };

    std::vector<std::tuple<std::string, Models::BusinessType, bool>> sampleBusinesses = {
        {"Enterprise Solutions Corp", Models::BusinessType::CORPORATE_OFFICE, true},
        {"Premier Event Services", Models::BusinessType::CONFERENCE_CENTER, true},
        {"Global Logistics Partners", Models::BusinessType::WAREHOUSE, true},
        {"Pinnacle Investment Group", Models::BusinessType::FINANCIAL_SERVICES, true},
        {"Creative Workspace Collective", Models::BusinessType::COWORKING_SPACE, false},
        {"Healthcare Excellence Center", Models::BusinessType::MEDICAL_FACILITY, true},
        {"Thompson & Associates Law", Models::BusinessType::LAW_FIRM, true},
        {"Industrial Innovations LLC", Models::BusinessType::MANUFACTURING, false},
        {"Civic Center Administration", Models::BusinessType::GOVERNMENT_OFFICE, false},
        {"Luxury Resort & Spa", Models::BusinessType::HOTEL, true}
    };

    std::string city = query.city.empty() ? "Springfield" : query.city;
    std::string state = query.state.empty() ? "IL" : query.state;
    std::string zip = query.zipCode.empty() ? "62701" : query.zipCode;

    std::uniform_int_distribution<> complaintDist(0, 5);
    std::uniform_int_distribution<> yearDist(1990, 2020);
    std::uniform_int_distribution<> empDist(20, 800);

    int numResults = std::min(static_cast<int>(sampleBusinesses.size()),
                             query.pageSize > 0 ? query.pageSize : 8);

    for (int i = 0; i < numResults; ++i) {
        Models::BusinessInfo business;
        auto& [name, type, accredited] = sampleBusinesses[i];

        business.id = "bbb_" + std::to_string(i + 1) + "_" + std::to_string(std::time(nullptr));
        business.name = name;
        business.type = type;
        business.source = Models::DataSource::BBB;

        business.address.street1 = std::to_string(100 + i * 50) + " Commerce Street";
        business.address.city = city;
        business.address.state = state;
        business.address.zipCode = zip;

        business.contact.primaryPhone = "(555) " + std::to_string(200 + i) + "-" + std::to_string(2000 + i * 111);

        business.bbbAccredited = accredited;
        business.bbbRating = ratings[i % ratings.size()];
        business.bbbComplaintCount = complaintDist(gen);
        business.yearEstablished = yearDist(gen);
        business.employeeCount = empDist(gen);

        business.hasConferenceRoom = (type == Models::BusinessType::CORPORATE_OFFICE ||
                                      type == Models::BusinessType::CONFERENCE_CENTER ||
                                      type == Models::BusinessType::HOTEL);
        business.hasEventSpace = (type == Models::BusinessType::CONFERENCE_CENTER ||
                                  type == Models::BusinessType::HOTEL);

        business.calculateCateringPotential();
        business.dateAdded = std::time(nullptr);
        business.lastUpdated = std::time(nullptr);

        results.push_back(business);
    }

    // Sort by BBB rating quality
    std::sort(results.begin(), results.end(),
        [](const Models::BusinessInfo& a, const Models::BusinessInfo& b) {
            return static_cast<int>(a.bbbRating) < static_cast<int>(b.bbbRating);
        });

    return results;
}

std::vector<BBBComplaint> BBBAPI::generateDemoComplaints(const std::string& bbbId) {
    std::vector<BBBComplaint> complaints;

    // Generate 0-3 sample complaints
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> countDist(0, 3);

    int numComplaints = countDist(gen);

    std::vector<std::string> types = {
        "Service Issues",
        "Billing/Collection Issues",
        "Delivery Issues",
        "Product Quality"
    };

    std::vector<std::string> statuses = {"Resolved", "Answered", "Closed"};

    for (int i = 0; i < numComplaints; ++i) {
        BBBComplaint complaint;
        complaint.id = "complaint_" + std::to_string(i + 1);
        complaint.type = types[i % types.size()];
        complaint.status = statuses[i % statuses.size()];
        complaint.dateOpened = "2024-0" + std::to_string(i + 1) + "-15";
        complaint.dateClosed = "2024-0" + std::to_string(i + 2) + "-01";
        complaint.resolution = "BBB determined the business made a good faith effort to resolve";

        complaints.push_back(complaint);
    }

    return complaints;
}

} // namespace Services
} // namespace FranchiseAI
