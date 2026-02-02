#ifndef FRANCHISEE_H
#define FRANCHISEE_H

#include <string>
#include <vector>
#include "GeoLocation.h"
#include "BusinessInfo.h"

namespace FranchiseAI {
namespace Models {

/**
 * @brief Search criteria for finding prospects
 */
struct SearchCriteria {
    double radiusMiles = 5.0;
    std::vector<BusinessType> businessTypes;
    int minEmployees = 0;
    int maxEmployees = 10000;
    bool includeOpenStreetMap = true;
    bool includeGooglePlaces = false;
    bool includeBBB = false;

    SearchCriteria() {
        // Default business types for catering prospects
        businessTypes = {
            BusinessType::CORPORATE_OFFICE,
            BusinessType::CONFERENCE_CENTER,
            BusinessType::HOTEL,
            BusinessType::MEDICAL_FACILITY,
            BusinessType::EDUCATIONAL_INSTITUTION,
            BusinessType::MANUFACTURING,
            BusinessType::WAREHOUSE
        };
    }

    bool hasBusinessType(BusinessType type) const {
        for (const auto& t : businessTypes) {
            if (t == type) return true;
        }
        return false;
    }

    void addBusinessType(BusinessType type) {
        if (!hasBusinessType(type)) {
            businessTypes.push_back(type);
        }
    }

    void removeBusinessType(BusinessType type) {
        businessTypes.erase(
            std::remove(businessTypes.begin(), businessTypes.end(), type),
            businessTypes.end()
        );
    }

    void clearBusinessTypes() {
        businessTypes.clear();
    }
};

/**
 * @brief Franchisee store information
 *
 * Represents a Vocelli Pizza (or other franchise) store location
 * that serves as the center point for prospect searches.
 */
struct Franchisee {
    // Store identification
    std::string storeId;
    std::string storeName;
    std::string franchiseName = "Vocelli Pizza";

    // Location
    GeoLocation location;
    std::string address;
    std::string phone;
    std::string email;

    // Store details
    int employeeCount = 0;
    std::string ownerName;
    std::string managerName;

    // Service area
    double defaultSearchRadiusMiles = 5.0;
    SearchCriteria searchCriteria;

    // Status
    bool isConfigured = false;

    Franchisee() = default;

    Franchisee(const std::string& name, const GeoLocation& loc)
        : storeName(name), location(loc), isConfigured(true) {}

    /**
     * @brief Get display name for the store
     */
    std::string getDisplayName() const {
        if (!storeName.empty()) {
            return storeName;
        }
        if (!storeId.empty()) {
            return franchiseName + " #" + storeId;
        }
        return franchiseName;
    }

    /**
     * @brief Get location display string
     */
    std::string getLocationDisplay() const {
        if (!location.city.empty()) {
            std::string result = location.city;
            if (!location.state.empty()) {
                result += ", " + location.state;
            }
            return result;
        }
        if (!address.empty()) {
            return address;
        }
        return location.getDisplayString();
    }

    /**
     * @brief Get full formatted address for display and geocoding
     * Combines street address, city, state, and zip code
     */
    std::string getFullAddress() const {
        std::string fullAddr;
        if (!address.empty()) {
            fullAddr = address;
        }
        if (!location.city.empty()) {
            if (!fullAddr.empty()) fullAddr += ", ";
            fullAddr += location.city;
        }
        if (!location.state.empty()) {
            if (!fullAddr.empty()) fullAddr += ", ";
            fullAddr += location.state;
        }
        if (!location.postalCode.empty()) {
            if (!fullAddr.empty()) fullAddr += " ";
            fullAddr += location.postalCode;
        }
        return fullAddr;
    }

    /**
     * @brief Create a search area centered on this franchisee
     */
    SearchArea createSearchArea() const {
        return SearchArea::fromMiles(location, searchCriteria.radiusMiles);
    }

    /**
     * @brief Create a search area with custom radius
     */
    SearchArea createSearchArea(double radiusMiles) const {
        return SearchArea::fromMiles(location, radiusMiles);
    }

    /**
     * @brief Check if franchisee has valid location
     */
    bool hasValidLocation() const {
        return location.hasValidCoordinates();
    }
};

/**
 * @brief Employee count range options for filtering
 */
struct EmployeeRange {
    std::string label;
    int minEmployees;
    int maxEmployees;

    EmployeeRange(const std::string& lbl, int min, int max)
        : label(lbl), minEmployees(min), maxEmployees(max) {}

    static std::vector<EmployeeRange> getStandardRanges() {
        return {
            {"Any Size", 0, 100000},
            {"Small (1-50)", 1, 50},
            {"Medium (51-200)", 51, 200},
            {"Large (201-1000)", 201, 1000},
            {"Enterprise (1000+)", 1000, 100000}
        };
    }
};

} // namespace Models
} // namespace FranchiseAI

#endif // FRANCHISEE_H
