#include "DemographicsAPI.h"
#include <random>
#include <ctime>
#include <algorithm>
#include <cmath>

namespace FranchiseAI {
namespace Services {

DemographicsAPI::DemographicsAPI() = default;

DemographicsAPI::DemographicsAPI(const DemographicsAPIConfig& config) : config_(config) {}

DemographicsAPI::~DemographicsAPI() = default;

void DemographicsAPI::setConfig(const DemographicsAPIConfig& config) {
    config_ = config;
}

void DemographicsAPI::setApiKey(const std::string& apiKey) {
    config_.apiKey = apiKey;
}

void DemographicsAPI::getByZipCode(const std::string& zipCode, DemographicCallback callback) {
    ++totalApiCalls_;

    auto data = generateDemoData(zipCode);

    if (callback) {
        callback(data, "");
    }
}

void DemographicsAPI::getByCity(
    const std::string& city,
    const std::string& state,
    DemographicCallback callback
) {
    ++totalApiCalls_;

    Models::DemographicData data;
    data.city = city;
    data.state = state;
    data.zipCode = "00000";  // Placeholder

    // Generate demo data
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> popDist(50000, 500000);
    std::uniform_real_distribution<> incomeDist(45000, 120000);
    std::uniform_int_distribution<> bizDist(500, 5000);

    data.totalPopulation = popDist(gen);
    data.workingAgePopulation = static_cast<int>(data.totalPopulation * 0.65);
    data.medianHouseholdIncome = incomeDist(gen);
    data.totalBusinesses = bizDist(gen);
    data.officeBuildings = data.totalBusinesses / 10;
    data.warehouses = data.totalBusinesses / 20;
    data.conferenceVenues = data.totalBusinesses / 50;

    data.marketPotentialScore = data.calculateMarketPotential();

    if (callback) {
        callback(data, "");
    }
}

void DemographicsAPI::getMultipleZipCodes(
    const std::vector<std::string>& zipCodes,
    MultiDemographicCallback callback
) {
    ++totalApiCalls_;

    std::vector<Models::DemographicData> results;
    for (const auto& zip : zipCodes) {
        results.push_back(generateDemoData(zip));
    }

    if (callback) {
        callback(results, "");
    }
}

void DemographicsAPI::getZipCodesInRadius(
    const std::string& centerZip,
    double radiusMiles,
    MultiDemographicCallback callback
) {
    ++totalApiCalls_;

    auto results = generateDemoAreaData(centerZip, radiusMiles);

    if (callback) {
        callback(results, "");
    }
}

void DemographicsAPI::findHighPotentialAreas(
    const std::string& centerLocation,
    double radiusMiles,
    int minScore,
    MultiDemographicCallback callback
) {
    ++totalApiCalls_;

    auto allAreas = generateDemoAreaData(centerLocation, radiusMiles);

    // Filter by minimum score
    std::vector<Models::DemographicData> highPotential;
    std::copy_if(allAreas.begin(), allAreas.end(), std::back_inserter(highPotential),
        [minScore](const Models::DemographicData& data) {
            return data.marketPotentialScore >= minScore;
        });

    // Sort by market potential
    std::sort(highPotential.begin(), highPotential.end(),
        [](const Models::DemographicData& a, const Models::DemographicData& b) {
            return a.marketPotentialScore > b.marketPotentialScore;
        });

    if (callback) {
        callback(highPotential, "");
    }
}

void DemographicsAPI::getBusinessDensity(const std::string& zipCode, DemographicCallback callback) {
    ++totalApiCalls_;

    auto data = generateDemoData(zipCode);

    if (callback) {
        callback(data, "");
    }
}

void DemographicsAPI::getEmploymentBySector(const std::string& zipCode, DemographicCallback callback) {
    ++totalApiCalls_;

    auto data = generateDemoData(zipCode);
    populateEmploymentData(data);

    if (callback) {
        callback(data, "");
    }
}

Models::DemographicData DemographicsAPI::getByZipCodeSync(const std::string& zipCode) {
    return generateDemoData(zipCode);
}

std::vector<Models::DemographicData> DemographicsAPI::getZipCodesInRadiusSync(
    const std::string& centerZip,
    double radiusMiles
) {
    return generateDemoAreaData(centerZip, radiusMiles);
}

void DemographicsAPI::clearCache() {
    // Clear internal cache
}

int DemographicsAPI::getCacheSize() const {
    return 0;
}

Models::DemographicData DemographicsAPI::generateDemoData(const std::string& zipCode) {
    Models::DemographicData data;
    std::random_device rd;
    std::mt19937 gen(rd());

    // Use ZIP code to seed some variation
    int zipSeed = 0;
    for (char c : zipCode) {
        if (std::isdigit(c)) {
            zipSeed += c - '0';
        }
    }
    gen.seed(zipSeed + rd());

    std::uniform_int_distribution<> popDist(15000, 80000);
    std::uniform_real_distribution<> incomeDist(35000, 150000);
    std::uniform_int_distribution<> bizDist(100, 2000);
    std::uniform_real_distribution<> growthDist(-1.0, 5.0);
    std::uniform_real_distribution<> ageDist(32.0, 45.0);

    data.zipCode = zipCode;
    data.city = "Sample City";
    data.state = "IL";
    data.county = "Sample County";

    data.totalPopulation = popDist(gen);
    data.workingAgePopulation = static_cast<int>(data.totalPopulation * 0.62);
    data.medianAge = ageDist(gen);

    data.medianHouseholdIncome = incomeDist(gen);
    data.averageHouseholdIncome = data.medianHouseholdIncome * 1.2;
    data.perCapitaIncome = data.medianHouseholdIncome / 2.5;

    std::uniform_real_distribution<> unempDist(2.5, 8.0);
    data.unemploymentRate = unempDist(gen);

    data.totalBusinesses = bizDist(gen);
    data.officeBuildings = data.totalBusinesses / 8;
    data.warehouses = data.totalBusinesses / 15;
    data.conferenceVenues = std::max(1, data.totalBusinesses / 40);
    data.corporateHeadquarters = std::max(0, data.totalBusinesses / 100);

    data.populationGrowthRate = growthDist(gen);
    data.businessGrowthRate = growthDist(gen);
    data.economicGrowthIndex = 50 + growthDist(gen) * 10;

    populateEmploymentData(data);

    data.marketPotentialScore = data.calculateMarketPotential();

    std::uniform_real_distribution<> distDist(1.0, 25.0);
    data.distanceFromFranchise = distDist(gen);

    return data;
}

std::vector<Models::DemographicData> DemographicsAPI::generateDemoAreaData(
    const std::string& centerZip,
    double radiusMiles
) {
    std::vector<Models::DemographicData> results;
    std::random_device rd;
    std::mt19937 gen(rd());

    // Generate ZIP codes based on the center
    int baseZip = 0;
    try {
        baseZip = std::stoi(centerZip);
    } catch (...) {
        baseZip = 62700;  // Default
    }

    // Generate nearby ZIP codes
    int numAreas = static_cast<int>(radiusMiles / 3) + 5;
    numAreas = std::min(numAreas, 20);

    std::vector<std::string> cities = {
        "Downtown District", "Business Park Area", "Industrial Zone",
        "Tech Corridor", "Commerce Center", "Corporate Plaza",
        "Metro Heights", "Enterprise District", "Financial Center",
        "Innovation Park"
    };

    for (int i = 0; i < numAreas; ++i) {
        int zipVariation = (i % 2 == 0 ? i : -i) * (i + 1);
        std::string newZip = std::to_string(baseZip + zipVariation);

        // Pad with zeros if needed
        while (newZip.length() < 5) {
            newZip = "0" + newZip;
        }

        auto data = generateDemoData(newZip);
        data.city = cities[i % cities.size()];

        // Calculate distance from center
        std::uniform_real_distribution<> distDist(0.5, radiusMiles);
        data.distanceFromFranchise = distDist(gen);

        results.push_back(data);
    }

    // Sort by market potential
    std::sort(results.begin(), results.end(),
        [](const Models::DemographicData& a, const Models::DemographicData& b) {
            return a.marketPotentialScore > b.marketPotentialScore;
        });

    return results;
}

void DemographicsAPI::populateEmploymentData(Models::DemographicData& data) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> empDist(500, 5000);

    data.employmentBySector[Models::IndustrySector::TECHNOLOGY] = empDist(gen);
    data.employmentBySector[Models::IndustrySector::HEALTHCARE] = empDist(gen);
    data.employmentBySector[Models::IndustrySector::FINANCE] = empDist(gen);
    data.employmentBySector[Models::IndustrySector::MANUFACTURING] = empDist(gen);
    data.employmentBySector[Models::IndustrySector::RETAIL] = empDist(gen);
    data.employmentBySector[Models::IndustrySector::PROFESSIONAL] = empDist(gen);
    data.employmentBySector[Models::IndustrySector::EDUCATION] = empDist(gen);
    data.employmentBySector[Models::IndustrySector::GOVERNMENT] = empDist(gen);
    data.employmentBySector[Models::IndustrySector::LOGISTICS] = empDist(gen);
    data.employmentBySector[Models::IndustrySector::HOSPITALITY] = empDist(gen);
}

} // namespace Services
} // namespace FranchiseAI
