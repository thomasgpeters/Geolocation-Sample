#ifndef DEMOGRAPHIC_DATA_H
#define DEMOGRAPHIC_DATA_H

#include <string>
#include <vector>
#include <map>

namespace FranchiseAI {
namespace Models {

/**
 * @brief Demographic data for a geographic area
 *
 * Contains population, income, and business statistics
 * useful for identifying potential catering clients.
 */
struct DemographicData {
    std::string zipCode;
    std::string city;
    std::string state;
    std::string county;

    // Population metrics
    int totalPopulation = 0;
    int workingAgePopulation = 0;  // 18-65
    double medianAge = 0.0;

    // Economic metrics
    double medianHouseholdIncome = 0.0;
    double averageHouseholdIncome = 0.0;
    double perCapitaIncome = 0.0;
    double unemploymentRate = 0.0;

    // Business metrics
    int totalBusinesses = 0;
    int officeBuildings = 0;
    int warehouses = 0;
    int conferenceVenues = 0;
    int corporateHeadquarters = 0;

    // Employment by sector
    std::map<std::string, int> employmentBySector;

    // Growth indicators
    double populationGrowthRate = 0.0;
    double businessGrowthRate = 0.0;
    double economicGrowthIndex = 0.0;

    // Catering market potential score (0-100)
    int marketPotentialScore = 0;

    // Distance from franchise location (miles)
    double distanceFromFranchise = 0.0;

    /**
     * @brief Calculate market potential based on demographics
     * @return Score from 0-100 indicating catering market potential
     */
    int calculateMarketPotential() const {
        int score = 0;

        // Weight factors for catering potential
        if (totalBusinesses > 100) score += 15;
        if (totalBusinesses > 500) score += 10;
        if (officeBuildings > 20) score += 15;
        if (warehouses > 10) score += 10;
        if (conferenceVenues > 5) score += 15;
        if (corporateHeadquarters > 3) score += 10;
        if (medianHouseholdIncome > 75000) score += 10;
        if (workingAgePopulation > 10000) score += 10;
        if (businessGrowthRate > 2.0) score += 5;

        return std::min(score, 100);
    }

    /**
     * @brief Get a text description of the market potential
     */
    std::string getMarketPotentialDescription() const {
        int score = marketPotentialScore > 0 ? marketPotentialScore : calculateMarketPotential();

        if (score >= 80) return "Excellent";
        if (score >= 60) return "Very Good";
        if (score >= 40) return "Good";
        if (score >= 20) return "Fair";
        return "Limited";
    }
};

/**
 * @brief Industry sector classifications for employment data
 */
struct IndustrySector {
    static constexpr const char* TECHNOLOGY = "Technology";
    static constexpr const char* HEALTHCARE = "Healthcare";
    static constexpr const char* FINANCE = "Finance & Insurance";
    static constexpr const char* MANUFACTURING = "Manufacturing";
    static constexpr const char* RETAIL = "Retail Trade";
    static constexpr const char* PROFESSIONAL = "Professional Services";
    static constexpr const char* EDUCATION = "Education";
    static constexpr const char* GOVERNMENT = "Government";
    static constexpr const char* LOGISTICS = "Logistics & Warehousing";
    static constexpr const char* HOSPITALITY = "Hospitality";
};

} // namespace Models
} // namespace FranchiseAI

#endif // DEMOGRAPHIC_DATA_H
