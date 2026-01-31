#ifndef AI_SEARCH_SERVICE_H
#define AI_SEARCH_SERVICE_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>
#include "models/SearchResult.h"
#include "models/BusinessInfo.h"
#include "models/DemographicData.h"
#include "GoogleMyBusinessAPI.h"
#include "BBBAPI.h"
#include "DemographicsAPI.h"
#include "OpenStreetMapAPI.h"
#include "GeocodingService.h"
#include "AIEngine.h"
#include "models/GeoLocation.h"

namespace FranchiseAI {
namespace Services {

/**
 * @brief AI Search configuration
 */
struct AISearchConfig {
    // API configurations
    GoogleAPIConfig googleConfig;
    BBBAPIConfig bbbConfig;
    DemographicsAPIConfig demographicsConfig;
    OSMAPIConfig osmConfig;
    GeocodingConfig geocodingConfig;

    // AI Engine configuration
    AIEngineConfig aiEngineConfig;

    // Search preferences
    int defaultRadius = 25;  // miles
    int maxResults = 50;
    double minRelevanceScore = 0.3;

    // AI analysis settings
    bool enableAIAnalysis = true;
    bool enableScoring = true;
    bool combineDataSources = true;
};

/**
 * @brief Search progress information
 */
struct SearchProgress {
    bool googleComplete = false;
    bool bbbComplete = false;
    bool demographicsComplete = false;
    bool osmComplete = false;
    bool analysisComplete = false;

    int googleResultCount = 0;
    int bbbResultCount = 0;
    int demographicsResultCount = 0;
    int osmResultCount = 0;

    std::string currentStep;
    int percentComplete = 0;

    bool isComplete() const {
        return googleComplete && bbbComplete && demographicsComplete && osmComplete && analysisComplete;
    }
};

/**
 * @brief AI-powered search service for finding catering prospects
 *
 * This service aggregates data from multiple sources (Google My Business,
 * BBB, Demographics) and uses AI analysis to identify and rank potential
 * catering clients for franchise owners.
 */
class AISearchService {
public:
    using SearchCallback = std::function<void(Models::SearchResults)>;
    using ProgressCallback = std::function<void(SearchProgress)>;

    AISearchService();
    explicit AISearchService(const AISearchConfig& config);
    ~AISearchService();

    // Configuration
    void setConfig(const AISearchConfig& config);
    AISearchConfig getConfig() const { return config_; }

    /**
     * @brief Perform AI-powered search for catering prospects
     * @param query Search parameters
     * @param callback Callback with search results
     * @param progressCallback Optional progress callback
     */
    void search(
        const Models::SearchQuery& query,
        SearchCallback callback,
        ProgressCallback progressCallback = nullptr
    );

    /**
     * @brief Perform quick search with default settings
     * @param location Location string (city, state or ZIP)
     * @param callback Callback with results
     */
    void quickSearch(const std::string& location, SearchCallback callback);

    /**
     * @brief Search for specific business types
     * @param location Location string
     * @param types Business types to search for
     * @param callback Callback with results
     */
    void searchByBusinessType(
        const std::string& location,
        const std::vector<Models::BusinessType>& types,
        SearchCallback callback
    );

    /**
     * @brief Find high-potential areas for catering expansion
     * @param centerLocation Center location
     * @param radiusMiles Search radius
     * @param callback Callback with results
     */
    void findExpansionOpportunities(
        const std::string& centerLocation,
        double radiusMiles,
        SearchCallback callback
    );

    /**
     * @brief Analyze a specific business for catering potential
     * @param businessId Business ID from any source
     * @param callback Callback with detailed analysis
     */
    void analyzeBusinessPotential(
        const std::string& businessId,
        std::function<void(Models::SearchResultItem)> callback
    );

    /**
     * @brief Get search suggestions based on partial input
     * @param partialInput Partial search input
     * @return List of suggestions
     */
    std::vector<std::string> getSearchSuggestions(const std::string& partialInput);

    /**
     * @brief Cancel ongoing search
     */
    void cancelSearch();

    /**
     * @brief Check if search is in progress
     */
    bool isSearching() const { return isSearching_; }

    // API access
    GoogleMyBusinessAPI& getGoogleAPI() { return googleAPI_; }
    BBBAPI& getBBBAPI() { return bbbAPI_; }
    DemographicsAPI& getDemographicsAPI() { return demographicsAPI_; }
    OpenStreetMapAPI& getOSMAPI() { return osmAPI_; }
    IGeocodingService& getGeocodingService() { return *geocodingService_; }

    /**
     * @brief Geocode an address to GeoLocation
     * @param address Address string (city, state or full address)
     * @return GeoLocation with coordinates
     */
    Models::GeoLocation geocodeAddress(const std::string& address);

    /**
     * @brief Create a search area from address and radius
     * @param address Address string
     * @param radiusMiles Search radius in miles
     * @return SearchArea ready for API queries
     */
    Models::SearchArea createSearchArea(const std::string& address, double radiusMiles);

    // AI Engine access
    AIEngine* getAIEngine() { return aiEngine_.get(); }
    void setAIEngine(std::unique_ptr<AIEngine> engine);
    void setAIProvider(AIProvider provider, const std::string& apiKey);
    AIProvider getAIProvider() const;
    bool isAIEngineConfigured() const;

    // Statistics
    int getTotalSearches() const { return totalSearches_; }
    int getTotalResultsFound() const { return totalResultsFound_; }

private:
    AISearchConfig config_;
    GoogleMyBusinessAPI googleAPI_;
    BBBAPI bbbAPI_;
    DemographicsAPI demographicsAPI_;
    OpenStreetMapAPI osmAPI_;
    std::unique_ptr<IGeocodingService> geocodingService_;

    // AI Engine for analysis
    std::unique_ptr<AIEngine> aiEngine_;

    bool isSearching_ = false;
    bool cancelRequested_ = false;
    std::mutex searchMutex_;

    int totalSearches_ = 0;
    int totalResultsFound_ = 0;

    // Internal methods
    void executeSearch(
        const Models::SearchQuery& query,
        SearchCallback callback,
        ProgressCallback progressCallback
    );

    Models::SearchResults aggregateResults(
        const std::vector<Models::BusinessInfo>& googleResults,
        const std::vector<Models::BusinessInfo>& bbbResults,
        const std::vector<Models::BusinessInfo>& osmResults,
        const std::vector<Models::DemographicData>& demographicResults,
        const Models::SearchQuery& query
    );

    void mergeBusinessData(
        Models::BusinessInfo& primary,
        const Models::BusinessInfo& secondary
    );

    Models::SearchResultItem createResultItem(
        const Models::BusinessInfo& business,
        const Models::SearchQuery& query
    );

    Models::SearchResultItem createDemographicResultItem(
        const Models::DemographicData& demographic,
        const Models::SearchQuery& query
    );

    void analyzeResults(Models::SearchResults& results);
    void scoreResult(Models::SearchResultItem& item);
    void generateLocalInsights(Models::SearchResultItem& item);
    void generateAIInsights(Models::SearchResultItem& item);
    void generateOverallAnalysis(Models::SearchResults& results);

    double calculateRelevanceScore(
        const Models::BusinessInfo& business,
        const Models::SearchQuery& query
    );

    std::vector<std::string> generateRecommendedActions(const Models::BusinessInfo& business);
    std::string generateMatchReason(const Models::BusinessInfo& business);
};

} // namespace Services
} // namespace FranchiseAI

#endif // AI_SEARCH_SERVICE_H
