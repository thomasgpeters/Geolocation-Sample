#ifndef GEOLOCATION_H
#define GEOLOCATION_H

#include <string>
#include <cmath>

namespace FranchiseAI {
namespace Models {

/**
 * @brief Geographic location with coordinates and optional address info
 *
 * Core model for all geolocation operations. Used by geocoding services
 * and location-based APIs (OpenStreetMap, Google, etc.)
 */
struct GeoLocation {
    // Coordinates
    double latitude = 0.0;
    double longitude = 0.0;

    // Address components (optional, populated by reverse geocoding)
    std::string formattedAddress;
    std::string street;
    std::string city;
    std::string state;
    std::string postalCode;
    std::string country;

    // Metadata
    std::string source;        // Which service provided this (e.g., "nominatim", "google")
    double accuracy = 0.0;     // Accuracy in meters (if available)
    bool isValid = false;      // Whether coordinates are valid

    // Default constructor
    GeoLocation() = default;

    // Constructor with coordinates
    GeoLocation(double lat, double lon)
        : latitude(lat), longitude(lon), isValid(true) {}

    // Constructor with full address
    GeoLocation(double lat, double lon, const std::string& city, const std::string& state)
        : latitude(lat), longitude(lon), city(city), state(state), isValid(true) {}

    /**
     * @brief Check if location has valid coordinates
     */
    bool hasValidCoordinates() const {
        return isValid &&
               latitude >= -90.0 && latitude <= 90.0 &&
               longitude >= -180.0 && longitude <= 180.0 &&
               (latitude != 0.0 || longitude != 0.0);
    }

    /**
     * @brief Calculate distance to another location in kilometers
     * Uses Haversine formula for accuracy
     */
    double distanceToKm(const GeoLocation& other) const {
        if (!hasValidCoordinates() || !other.hasValidCoordinates()) {
            return -1.0;
        }

        const double R = 6371.0;  // Earth's radius in km
        double lat1 = latitude * M_PI / 180.0;
        double lat2 = other.latitude * M_PI / 180.0;
        double dLat = (other.latitude - latitude) * M_PI / 180.0;
        double dLon = (other.longitude - longitude) * M_PI / 180.0;

        double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
                   std::cos(lat1) * std::cos(lat2) *
                   std::sin(dLon / 2) * std::sin(dLon / 2);
        double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

        return R * c;
    }

    /**
     * @brief Calculate distance to another location in miles
     */
    double distanceToMiles(const GeoLocation& other) const {
        double km = distanceToKm(other);
        return km >= 0 ? km * 0.621371 : -1.0;
    }

    /**
     * @brief Get formatted coordinates string
     */
    std::string getCoordinatesString() const {
        return std::to_string(latitude) + ", " + std::to_string(longitude);
    }

    /**
     * @brief Get display-friendly address or coordinates
     */
    std::string getDisplayString() const {
        if (!formattedAddress.empty()) {
            return formattedAddress;
        }
        if (!city.empty()) {
            std::string result = city;
            if (!state.empty()) result += ", " + state;
            if (!postalCode.empty()) result += " " + postalCode;
            return result;
        }
        return getCoordinatesString();
    }
};

/**
 * @brief Search area defined by center location and radius
 *
 * Used to define geographic search boundaries for API queries
 */
struct SearchArea {
    GeoLocation center;
    double radiusKm = 10.0;
    double radiusMiles = 6.21371;  // Default 10km in miles

    SearchArea() = default;

    SearchArea(const GeoLocation& loc, double radiusInKm)
        : center(loc), radiusKm(radiusInKm), radiusMiles(radiusInKm * 0.621371) {}

    static SearchArea fromMiles(const GeoLocation& loc, double radiusInMiles) {
        SearchArea area;
        area.center = loc;
        area.radiusMiles = radiusInMiles;
        area.radiusKm = radiusInMiles * 1.60934;
        return area;
    }

    /**
     * @brief Get radius in meters (for APIs that use meters)
     */
    int radiusMeters() const {
        return static_cast<int>(radiusKm * 1000);
    }

    /**
     * @brief Check if a location is within this search area
     */
    bool contains(const GeoLocation& location) const {
        return center.distanceToKm(location) <= radiusKm;
    }
};

/**
 * @brief Bounding box for geographic queries
 *
 * Some APIs prefer bounding box queries over radius-based
 */
struct GeoBoundingBox {
    double minLat = 0.0;
    double maxLat = 0.0;
    double minLon = 0.0;
    double maxLon = 0.0;

    GeoBoundingBox() = default;

    /**
     * @brief Create bounding box from search area
     */
    static GeoBoundingBox fromSearchArea(const SearchArea& area) {
        GeoBoundingBox box;
        // Approximate: 1 degree latitude = 111km
        double latDelta = area.radiusKm / 111.0;
        // Longitude varies by latitude
        double lonDelta = area.radiusKm / (111.0 * std::cos(area.center.latitude * M_PI / 180.0));

        box.minLat = area.center.latitude - latDelta;
        box.maxLat = area.center.latitude + latDelta;
        box.minLon = area.center.longitude - lonDelta;
        box.maxLon = area.center.longitude + lonDelta;

        return box;
    }

    /**
     * @brief Get Overpass API bbox string format: (south,west,north,east)
     */
    std::string toOverpassFormat() const {
        return "(" + std::to_string(minLat) + "," + std::to_string(minLon) + "," +
               std::to_string(maxLat) + "," + std::to_string(maxLon) + ")";
    }
};

} // namespace Models
} // namespace FranchiseAI

#endif // GEOLOCATION_H
