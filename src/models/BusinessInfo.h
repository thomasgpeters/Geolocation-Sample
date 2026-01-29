#ifndef BUSINESS_INFO_H
#define BUSINESS_INFO_H

#include <string>
#include <vector>
#include <ctime>

namespace FranchiseAI {
namespace Models {

/**
 * @brief Business type enumeration for categorizing potential clients
 */
enum class BusinessType {
    CORPORATE_OFFICE,
    WAREHOUSE,
    CONFERENCE_CENTER,
    HOTEL,
    COWORKING_SPACE,
    MEDICAL_FACILITY,
    EDUCATIONAL_INSTITUTION,
    GOVERNMENT_OFFICE,
    MANUFACTURING,
    TECH_COMPANY,
    FINANCIAL_SERVICES,
    LAW_FIRM,
    NONPROFIT,
    OTHER
};

/**
 * @brief Data source enumeration
 */
enum class DataSource {
    GOOGLE_MY_BUSINESS,
    BBB,
    DEMOGRAPHICS,
    OPENSTREETMAP,
    MANUAL_ENTRY,
    IMPORTED
};

/**
 * @brief BBB Rating enumeration
 */
enum class BBBRating {
    A_PLUS,
    A,
    A_MINUS,
    B_PLUS,
    B,
    B_MINUS,
    C_PLUS,
    C,
    C_MINUS,
    D_PLUS,
    D,
    D_MINUS,
    F,
    NOT_RATED,
    NOT_ACCREDITED
};

/**
 * @brief Contact information for a business
 */
struct ContactInfo {
    std::string primaryPhone;
    std::string secondaryPhone;
    std::string email;
    std::string website;
    std::string contactPerson;
    std::string contactTitle;
};

/**
 * @brief Physical address information
 */
struct Address {
    std::string street1;
    std::string street2;
    std::string city;
    std::string state;
    std::string zipCode;
    std::string country = "USA";
    double latitude = 0.0;
    double longitude = 0.0;

    std::string getFullAddress() const {
        std::string addr = street1;
        if (!street2.empty()) addr += ", " + street2;
        addr += ", " + city + ", " + state + " " + zipCode;
        return addr;
    }
};

/**
 * @brief Business hours structure
 */
struct BusinessHours {
    std::string monday;
    std::string tuesday;
    std::string wednesday;
    std::string thursday;
    std::string friday;
    std::string saturday;
    std::string sunday;
    bool isOpen24Hours = false;
};

/**
 * @brief Detailed business information
 *
 * Comprehensive data model for potential catering clients,
 * combining data from multiple sources.
 */
class BusinessInfo {
public:
    BusinessInfo() = default;
    ~BusinessInfo() = default;

    // Basic identification
    std::string id;
    std::string name;
    std::string description;
    BusinessType type = BusinessType::OTHER;
    DataSource source = DataSource::MANUAL_ENTRY;

    // Location
    Address address;

    // Contact
    ContactInfo contact;

    // Business details
    std::string category;
    std::vector<std::string> subcategories;
    int employeeCount = 0;
    int yearEstablished = 0;
    double annualRevenue = 0.0;

    // Ratings and reviews
    double googleRating = 0.0;
    int googleReviewCount = 0;
    BBBRating bbbRating = BBBRating::NOT_RATED;
    bool bbbAccredited = false;
    int bbbComplaintCount = 0;

    // Operating hours
    BusinessHours hours;

    // Catering potential indicators
    bool hasConferenceRoom = false;
    bool hasEventSpace = false;
    bool regularMeetings = false;
    int estimatedEmployeesOnSite = 0;

    // AI-generated insights
    int cateringPotentialScore = 0;  // 0-100
    std::string aiInsights;
    std::vector<std::string> suggestedApproach;

    // Metadata
    std::time_t lastUpdated = 0;
    std::time_t dateAdded = 0;
    bool isVerified = false;

    // Helper methods
    std::string getBusinessTypeString() const;
    std::string getDataSourceString() const;
    std::string getBBBRatingString() const;
    std::string getCateringPotentialDescription() const;

    // Scoring method
    void calculateCateringPotential();
};

/**
 * @brief Convert BusinessType enum to string
 */
inline std::string businessTypeToString(BusinessType type) {
    switch (type) {
        case BusinessType::CORPORATE_OFFICE: return "Corporate Office";
        case BusinessType::WAREHOUSE: return "Warehouse";
        case BusinessType::CONFERENCE_CENTER: return "Conference Center";
        case BusinessType::HOTEL: return "Hotel";
        case BusinessType::COWORKING_SPACE: return "Coworking Space";
        case BusinessType::MEDICAL_FACILITY: return "Medical Facility";
        case BusinessType::EDUCATIONAL_INSTITUTION: return "Educational Institution";
        case BusinessType::GOVERNMENT_OFFICE: return "Government Office";
        case BusinessType::MANUFACTURING: return "Manufacturing";
        case BusinessType::TECH_COMPANY: return "Tech Company";
        case BusinessType::FINANCIAL_SERVICES: return "Financial Services";
        case BusinessType::LAW_FIRM: return "Law Firm";
        case BusinessType::NONPROFIT: return "Non-Profit Organization";
        default: return "Other";
    }
}

/**
 * @brief Convert DataSource enum to string
 */
inline std::string dataSourceToString(DataSource source) {
    switch (source) {
        case DataSource::GOOGLE_MY_BUSINESS: return "Google My Business";
        case DataSource::BBB: return "Better Business Bureau";
        case DataSource::DEMOGRAPHICS: return "Demographics Data";
        case DataSource::OPENSTREETMAP: return "OpenStreetMap";
        case DataSource::MANUAL_ENTRY: return "Manual Entry";
        case DataSource::IMPORTED: return "Imported";
        default: return "Unknown";
    }
}

/**
 * @brief Convert BBBRating enum to string
 */
inline std::string bbbRatingToString(BBBRating rating) {
    switch (rating) {
        case BBBRating::A_PLUS: return "A+";
        case BBBRating::A: return "A";
        case BBBRating::A_MINUS: return "A-";
        case BBBRating::B_PLUS: return "B+";
        case BBBRating::B: return "B";
        case BBBRating::B_MINUS: return "B-";
        case BBBRating::C_PLUS: return "C+";
        case BBBRating::C: return "C";
        case BBBRating::C_MINUS: return "C-";
        case BBBRating::D_PLUS: return "D+";
        case BBBRating::D: return "D";
        case BBBRating::D_MINUS: return "D-";
        case BBBRating::F: return "F";
        case BBBRating::NOT_RATED: return "Not Rated";
        case BBBRating::NOT_ACCREDITED: return "Not Accredited";
        default: return "Unknown";
    }
}

} // namespace Models
} // namespace FranchiseAI

#endif // BUSINESS_INFO_H
