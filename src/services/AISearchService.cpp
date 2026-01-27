#include "AISearchService.h"
#include "OpenAIEngine.h"
#include "GeminiEngine.h"
#include <algorithm>
#include <numeric>
#include <sstream>
#include <chrono>
#include <thread>

namespace FranchiseAI {
namespace Services {

AISearchService::AISearchService() {
    googleAPI_.setConfig(config_.googleConfig);
    bbbAPI_.setConfig(config_.bbbConfig);
    demographicsAPI_.setConfig(config_.demographicsConfig);

    // Initialize AI engine if configured
    if (config_.aiEngineConfig.provider != AIProvider::LOCAL &&
        !config_.aiEngineConfig.apiKey.empty()) {
        aiEngine_ = createAIEngine(config_.aiEngineConfig.provider, config_.aiEngineConfig);
    }
}

AISearchService::AISearchService(const AISearchConfig& config) : config_(config) {
    googleAPI_.setConfig(config_.googleConfig);
    bbbAPI_.setConfig(config_.bbbConfig);
    demographicsAPI_.setConfig(config_.demographicsConfig);

    // Initialize AI engine if configured
    if (config_.aiEngineConfig.provider != AIProvider::LOCAL &&
        !config_.aiEngineConfig.apiKey.empty()) {
        aiEngine_ = createAIEngine(config_.aiEngineConfig.provider, config_.aiEngineConfig);
    }
}

AISearchService::~AISearchService() {
    cancelSearch();
}

void AISearchService::setConfig(const AISearchConfig& config) {
    config_ = config;
    googleAPI_.setConfig(config_.googleConfig);
    bbbAPI_.setConfig(config_.bbbConfig);
    demographicsAPI_.setConfig(config_.demographicsConfig);

    // Update AI engine if configuration changed
    if (config_.aiEngineConfig.provider != AIProvider::LOCAL &&
        !config_.aiEngineConfig.apiKey.empty()) {
        aiEngine_ = createAIEngine(config_.aiEngineConfig.provider, config_.aiEngineConfig);
    } else {
        aiEngine_.reset();
    }
}

void AISearchService::setAIEngine(std::unique_ptr<AIEngine> engine) {
    aiEngine_ = std::move(engine);
}

void AISearchService::setAIProvider(AIProvider provider, const std::string& apiKey) {
    config_.aiEngineConfig.provider = provider;
    config_.aiEngineConfig.apiKey = apiKey;

    if (provider != AIProvider::LOCAL && !apiKey.empty()) {
        aiEngine_ = createAIEngine(provider, config_.aiEngineConfig);
    } else {
        aiEngine_.reset();
    }
}

AIProvider AISearchService::getAIProvider() const {
    if (aiEngine_) {
        return aiEngine_->getProvider();
    }
    return AIProvider::LOCAL;
}

bool AISearchService::isAIEngineConfigured() const {
    return aiEngine_ && aiEngine_->isConfigured();
}

void AISearchService::search(
    const Models::SearchQuery& query,
    SearchCallback callback,
    ProgressCallback progressCallback
) {
    std::lock_guard<std::mutex> lock(searchMutex_);

    if (isSearching_) {
        return;
    }

    isSearching_ = true;
    cancelRequested_ = false;
    ++totalSearches_;

    executeSearch(query, callback, progressCallback);
}

void AISearchService::quickSearch(const std::string& location, SearchCallback callback) {
    Models::SearchQuery query;
    query.location = location;
    query.radiusMiles = config_.defaultRadius;
    query.includeGoogleMyBusiness = true;
    query.includeBBB = true;
    query.includeDemographics = true;

    search(query, callback);
}

void AISearchService::searchByBusinessType(
    const std::string& location,
    const std::vector<Models::BusinessType>& types,
    SearchCallback callback
) {
    Models::SearchQuery query;
    query.location = location;
    query.businessTypes = types;
    query.radiusMiles = config_.defaultRadius;

    search(query, callback);
}

void AISearchService::findExpansionOpportunities(
    const std::string& centerLocation,
    double radiusMiles,
    SearchCallback callback
) {
    Models::SearchQuery query;
    query.location = centerLocation;
    query.radiusMiles = radiusMiles;
    query.includeDemographics = true;
    query.includeGoogleMyBusiness = true;
    query.includeBBB = false;
    query.minCateringScore = 50;

    search(query, callback);
}

void AISearchService::analyzeBusinessPotential(
    const std::string& businessId,
    std::function<void(Models::SearchResultItem)> callback
) {
    Models::SearchResultItem item;
    item.id = businessId;

    auto business = std::make_shared<Models::BusinessInfo>();
    business->id = businessId;
    business->name = "Analyzed Business";
    business->calculateCateringPotential();

    item.business = business;
    scoreResult(item);
    generateAIInsights(item);

    if (callback) {
        callback(item);
    }
}

std::vector<std::string> AISearchService::getSearchSuggestions(const std::string& partialInput) {
    std::vector<std::string> suggestions;

    if (partialInput.length() < 2) {
        return suggestions;
    }

    // Add location-based suggestions
    suggestions.push_back(partialInput + " corporate offices");
    suggestions.push_back(partialInput + " business parks");
    suggestions.push_back(partialInput + " conference centers");
    suggestions.push_back(partialInput + " warehouses and distribution");
    suggestions.push_back(partialInput + " tech companies");

    return suggestions;
}

void AISearchService::cancelSearch() {
    cancelRequested_ = true;
}

void AISearchService::executeSearch(
    const Models::SearchQuery& query,
    SearchCallback callback,
    ProgressCallback progressCallback
) {
    auto startTime = std::chrono::high_resolution_clock::now();

    SearchProgress progress;
    std::vector<Models::BusinessInfo> googleResults;
    std::vector<Models::BusinessInfo> bbbResults;
    std::vector<Models::DemographicData> demographicResults;

    // Step 1: Google My Business search
    if (query.includeGoogleMyBusiness && !cancelRequested_) {
        progress.currentStep = "Searching Google My Business...";
        progress.percentComplete = 10;
        if (progressCallback) progressCallback(progress);

        googleResults = googleAPI_.searchBusinessesSync(query);
        progress.googleComplete = true;
        progress.googleResultCount = static_cast<int>(googleResults.size());
        progress.percentComplete = 35;
        if (progressCallback) progressCallback(progress);
    }

    // Step 2: BBB search
    if (query.includeBBB && !cancelRequested_) {
        progress.currentStep = "Searching Better Business Bureau...";
        progress.percentComplete = 40;
        if (progressCallback) progressCallback(progress);

        bbbResults = bbbAPI_.searchBusinessesSync(query);
        progress.bbbComplete = true;
        progress.bbbResultCount = static_cast<int>(bbbResults.size());
        progress.percentComplete = 60;
        if (progressCallback) progressCallback(progress);
    }

    // Step 3: Demographics search
    if (query.includeDemographics && !cancelRequested_) {
        progress.currentStep = "Analyzing demographic data...";
        progress.percentComplete = 65;
        if (progressCallback) progressCallback(progress);

        std::string zipCode = query.zipCode.empty() ? "62701" : query.zipCode;
        demographicResults = demographicsAPI_.getZipCodesInRadiusSync(zipCode, query.radiusMiles);
        progress.demographicsComplete = true;
        progress.demographicsResultCount = static_cast<int>(demographicResults.size());
        progress.percentComplete = 80;
        if (progressCallback) progressCallback(progress);
    }

    // Step 4: Aggregate and analyze results
    if (!cancelRequested_) {
        progress.currentStep = "Performing AI analysis...";
        progress.percentComplete = 85;
        if (progressCallback) progressCallback(progress);

        auto results = aggregateResults(googleResults, bbbResults, demographicResults, query);

        auto endTime = std::chrono::high_resolution_clock::now();
        results.searchDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime
        );

        // Generate AI analysis
        if (config_.enableAIAnalysis) {
            analyzeResults(results);
        }

        progress.analysisComplete = true;
        progress.percentComplete = 100;
        progress.currentStep = "Search complete";
        if (progressCallback) progressCallback(progress);

        totalResultsFound_ += results.totalResults;

        isSearching_ = false;

        if (callback) {
            callback(results);
        }
    } else {
        isSearching_ = false;
    }
}

Models::SearchResults AISearchService::aggregateResults(
    const std::vector<Models::BusinessInfo>& googleResults,
    const std::vector<Models::BusinessInfo>& bbbResults,
    const std::vector<Models::DemographicData>& demographicResults,
    const Models::SearchQuery& query
) {
    Models::SearchResults results;
    results.query = query;

    // Add Google results
    for (const auto& business : googleResults) {
        auto item = createResultItem(business, query);
        results.items.push_back(item);
    }
    results.googleResults = static_cast<int>(googleResults.size());

    // Add BBB results (merge with existing if possible)
    for (const auto& business : bbbResults) {
        bool merged = false;
        for (auto& item : results.items) {
            if (item.business && item.business->name == business.name) {
                mergeBusinessData(*item.business, business);
                item.sources.push_back(Models::DataSource::BBB);
                merged = true;
                break;
            }
        }
        if (!merged) {
            auto item = createResultItem(business, query);
            results.items.push_back(item);
        }
    }
    results.bbbResults = static_cast<int>(bbbResults.size());

    // Add demographic area results
    for (const auto& demographic : demographicResults) {
        auto item = createDemographicResultItem(demographic, query);
        results.items.push_back(item);
    }
    results.demographicResults = static_cast<int>(demographicResults.size());

    // Score and sort all results
    for (auto& item : results.items) {
        scoreResult(item);
    }

    // Sort by overall score
    std::sort(results.items.begin(), results.items.end(),
        [](const Models::SearchResultItem& a, const Models::SearchResultItem& b) {
            return a.overallScore > b.overallScore;
        });

    // Apply max results limit
    if (results.items.size() > static_cast<size_t>(config_.maxResults)) {
        results.items.resize(config_.maxResults);
    }

    results.totalResults = static_cast<int>(results.items.size());
    results.isComplete = true;

    return results;
}

void AISearchService::mergeBusinessData(
    Models::BusinessInfo& primary,
    const Models::BusinessInfo& secondary
) {
    // Merge BBB data into primary record
    if (secondary.source == Models::DataSource::BBB) {
        primary.bbbRating = secondary.bbbRating;
        primary.bbbAccredited = secondary.bbbAccredited;
        primary.bbbComplaintCount = secondary.bbbComplaintCount;
    }

    // Fill in missing data
    if (primary.yearEstablished == 0 && secondary.yearEstablished != 0) {
        primary.yearEstablished = secondary.yearEstablished;
    }
    if (primary.employeeCount == 0 && secondary.employeeCount != 0) {
        primary.employeeCount = secondary.employeeCount;
    }

    // Recalculate potential
    primary.calculateCateringPotential();
}

Models::SearchResultItem AISearchService::createResultItem(
    const Models::BusinessInfo& business,
    const Models::SearchQuery& query
) {
    Models::SearchResultItem item;
    item.id = business.id;
    item.resultType = Models::SearchResultType::BUSINESS;
    item.business = std::make_shared<Models::BusinessInfo>(business);
    item.sources.push_back(business.source);
    item.relevanceScore = calculateRelevanceScore(business, query);
    item.matchReason = generateMatchReason(business);
    item.recommendedActions = generateRecommendedActions(business);

    return item;
}

Models::SearchResultItem AISearchService::createDemographicResultItem(
    const Models::DemographicData& demographic,
    const Models::SearchQuery& query
) {
    Models::SearchResultItem item;
    item.id = "demo_" + demographic.zipCode;
    item.resultType = Models::SearchResultType::DEMOGRAPHIC_AREA;
    item.demographic = std::make_shared<Models::DemographicData>(demographic);
    item.sources.push_back(Models::DataSource::DEMOGRAPHICS);
    item.distanceMiles = demographic.distanceFromFranchise;

    // Generate insights for demographic areas
    std::ostringstream summary;
    summary << "Area with " << demographic.totalBusinesses << " businesses, "
            << demographic.officeBuildings << " office buildings, and "
            << demographic.warehouses << " warehouses. "
            << "Market potential score: " << demographic.marketPotentialScore << "/100.";
    item.aiSummary = summary.str();

    item.keyHighlights.push_back("Total businesses: " + std::to_string(demographic.totalBusinesses));
    item.keyHighlights.push_back("Office buildings: " + std::to_string(demographic.officeBuildings));
    item.keyHighlights.push_back("Conference venues: " + std::to_string(demographic.conferenceVenues));
    item.keyHighlights.push_back("Working population: " + std::to_string(demographic.workingAgePopulation));

    return item;
}

void AISearchService::analyzeResults(Models::SearchResults& results) {
    // Generate insights for each result
    for (auto& item : results.items) {
        generateAIInsights(item);
    }

    // Generate overall analysis
    generateOverallAnalysis(results);
}

void AISearchService::scoreResult(Models::SearchResultItem& item) {
    int score = 0;

    if (item.business) {
        // Business scoring
        score = item.business->cateringPotentialScore;

        // Boost for multiple data sources
        score += static_cast<int>(item.sources.size()) * 5;

        // Boost for verified businesses
        if (item.business->isVerified) score += 5;

        // Boost for BBB accredited
        if (item.business->bbbAccredited) score += 10;

        // High Google rating boost
        if (item.business->googleRating >= 4.5) score += 5;
    } else if (item.demographic) {
        // Demographic area scoring
        score = item.demographic->marketPotentialScore;
    }

    item.overallScore = std::min(score, 100);
    item.aiConfidenceScore = score / 100.0;
}

void AISearchService::generateAIInsights(Models::SearchResultItem& item) {
    if (item.business) {
        auto& biz = *item.business;

        // Use AI engine if available
        if (aiEngine_ && aiEngine_->isConfigured()) {
            auto analysis = aiEngine_->analyzeBusinessPotentialSync(biz);
            item.aiSummary = analysis.summary;
            item.keyHighlights = analysis.keyHighlights;
            item.recommendedActions = analysis.recommendedActions;
            item.matchReason = analysis.matchReason;
            item.aiConfidenceScore = analysis.confidenceScore;

            // Update business score from AI analysis if provided
            if (analysis.cateringPotentialScore > 0) {
                biz.cateringPotentialScore = analysis.cateringPotentialScore;
            }
            return;
        }

        // Fall back to local analysis
        std::ostringstream insights;

        insights << biz.name << " is a " << biz.getBusinessTypeString()
                 << " with approximately " << biz.employeeCount << " employees. ";

        if (biz.hasConferenceRoom || biz.hasEventSpace) {
            insights << "This location has ";
            if (biz.hasConferenceRoom && biz.hasEventSpace) {
                insights << "conference rooms and dedicated event space, ";
            } else if (biz.hasConferenceRoom) {
                insights << "conference rooms, ";
            } else {
                insights << "event space, ";
            }
            insights << "making it ideal for corporate catering. ";
        }

        if (biz.bbbAccredited) {
            insights << "BBB accredited with " << biz.getBBBRatingString() << " rating. ";
        }

        insights << "Catering potential: " << biz.getCateringPotentialDescription() << ".";

        item.aiSummary = insights.str();

        // Key highlights
        item.keyHighlights.clear();
        item.keyHighlights.push_back("Employee count: ~" + std::to_string(biz.employeeCount));
        item.keyHighlights.push_back("Business type: " + biz.getBusinessTypeString());
        if (biz.googleRating > 0) {
            std::ostringstream rating;
            rating.precision(1);
            rating << std::fixed << biz.googleRating;
            item.keyHighlights.push_back("Google rating: " + rating.str() + "/5");
        }
        if (biz.hasConferenceRoom) {
            item.keyHighlights.push_back("Has conference facilities");
        }
    }
}

void AISearchService::generateOverallAnalysis(Models::SearchResults& results) {
    int highPotentialCount = 0;
    int withConferenceRoom = 0;
    int bbbAccredited = 0;

    std::vector<Models::DemographicData> demographics;
    std::vector<Models::BusinessInfo> businesses;

    for (const auto& item : results.items) {
        if (item.business) {
            businesses.push_back(*item.business);
            if (item.business->cateringPotentialScore >= 60) ++highPotentialCount;
            if (item.business->hasConferenceRoom) ++withConferenceRoom;
            if (item.business->bbbAccredited) ++bbbAccredited;
        }
        if (item.demographic) {
            demographics.push_back(*item.demographic);
        }
    }

    // Use AI engine for market analysis if available
    if (aiEngine_ && aiEngine_->isConfigured() && !businesses.empty()) {
        auto marketAnalysis = aiEngine_->analyzeMarketPotentialSync(demographics, businesses);
        results.aiOverallAnalysis = marketAnalysis.overallAnalysis;
        results.topRecommendations = marketAnalysis.topRecommendations;
        results.marketSummary = marketAnalysis.marketSummary;

        // Also generate a search summary using AI
        std::vector<std::string> businessSummaries;
        for (const auto& biz : businesses) {
            if (businessSummaries.size() >= 5) break;
            businessSummaries.push_back(biz.name + " (" + biz.getBusinessTypeString() + ")");
        }
        // The market analysis already covers what we need
        return;
    }

    // Fall back to local analysis
    std::ostringstream analysis;
    analysis << "Found " << results.totalResults << " potential catering prospects. ";
    analysis << highPotentialCount << " are high-potential leads (score 60+). ";
    analysis << withConferenceRoom << " have conference facilities. ";
    analysis << bbbAccredited << " are BBB accredited businesses.";

    results.aiOverallAnalysis = analysis.str();

    // Top recommendations
    results.topRecommendations.clear();
    results.topRecommendations.push_back("Focus on high-potential corporate offices and tech companies");
    results.topRecommendations.push_back("Conference centers and hotels offer recurring event catering opportunities");
    results.topRecommendations.push_back("Warehouses and distribution centers are great for employee meal programs");

    // Market summary
    std::ostringstream market;
    market << "The search area shows strong catering potential with "
           << results.demographicResults << " demographic zones analyzed. "
           << "Average catering potential score: " << results.getAverageCateringPotential() << "/100.";
    results.marketSummary = market.str();
}

double AISearchService::calculateRelevanceScore(
    const Models::BusinessInfo& business,
    const Models::SearchQuery& query
) {
    double score = 0.5;  // Base score

    // Keyword matching
    if (!query.keywords.empty()) {
        std::string lowerName = business.name;
        std::string lowerKeywords = query.keywords;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        std::transform(lowerKeywords.begin(), lowerKeywords.end(), lowerKeywords.begin(), ::tolower);

        if (lowerName.find(lowerKeywords) != std::string::npos) {
            score += 0.3;
        }
    }

    // Business type matching
    if (!query.businessTypes.empty()) {
        for (const auto& type : query.businessTypes) {
            if (business.type == type) {
                score += 0.2;
                break;
            }
        }
    }

    // Catering score contribution
    score += business.cateringPotentialScore / 200.0;

    return std::min(score, 1.0);
}

std::vector<std::string> AISearchService::generateRecommendedActions(
    const Models::BusinessInfo& business
) {
    std::vector<std::string> actions;

    actions.push_back("Research company size and meeting frequency");

    if (business.hasConferenceRoom) {
        actions.push_back("Inquire about regular meeting catering needs");
    }

    if (business.hasEventSpace) {
        actions.push_back("Ask about upcoming corporate events");
    }

    if (business.type == Models::BusinessType::WAREHOUSE ||
        business.type == Models::BusinessType::MANUFACTURING) {
        actions.push_back("Propose employee appreciation lunch program");
    }

    if (business.employeeCount > 100) {
        actions.push_back("Suggest recurring weekly catering service");
    }

    actions.push_back("Schedule introductory meeting with office manager");

    return actions;
}

std::string AISearchService::generateMatchReason(const Models::BusinessInfo& business) {
    std::ostringstream reason;

    reason << "Matched as a " << business.getBusinessTypeString();

    if (business.employeeCount > 0) {
        reason << " with " << business.employeeCount << " employees";
    }

    if (business.hasConferenceRoom || business.hasEventSpace) {
        reason << ", includes meeting facilities";
    }

    return reason.str();
}

} // namespace Services
} // namespace FranchiseAI
