#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include "models/BusinessInfo.h"
#include "models/DemographicData.h"

namespace FranchiseAI {
namespace Services {

/**
 * @brief AI engine provider type
 */
enum class AIProvider {
    OPENAI,
    GEMINI,
    LOCAL  // Fallback to local rule-based analysis
};

/**
 * @brief Configuration for AI engine
 */
struct AIEngineConfig {
    AIProvider provider = AIProvider::LOCAL;
    std::string apiKey;
    std::string model;  // e.g., "gpt-4", "gpt-3.5-turbo", "gemini-pro"
    std::string apiEndpoint;
    int maxTokens = 1024;
    double temperature = 0.7;
    int timeoutMs = 30000;
    bool enableCaching = true;
    int cacheDurationMinutes = 60;
};

/**
 * @brief Request structure for AI analysis
 */
struct AIAnalysisRequest {
    std::string prompt;
    std::string systemPrompt;
    std::map<std::string, std::string> context;
};

/**
 * @brief Response structure from AI analysis
 */
struct AIAnalysisResponse {
    bool success = false;
    std::string content;
    std::string error;
    int tokensUsed = 0;
    double confidenceScore = 0.0;
    std::string model;
    std::string provider;
};

/**
 * @brief Business analysis result from AI
 */
struct BusinessAnalysisResult {
    std::string summary;
    std::vector<std::string> keyHighlights;
    std::vector<std::string> recommendedActions;
    std::string matchReason;
    int cateringPotentialScore = 0;
    double confidenceScore = 0.0;
};

/**
 * @brief Market analysis result from AI
 */
struct MarketAnalysisResult {
    std::string overallAnalysis;
    std::vector<std::string> topRecommendations;
    std::string marketSummary;
    std::vector<std::string> opportunities;
    std::vector<std::string> risks;
};

/**
 * @brief Abstract base class for AI engines
 *
 * Provides a common interface for different AI providers (OpenAI, Gemini, etc.)
 */
class AIEngine {
public:
    using AnalysisCallback = std::function<void(const AIAnalysisResponse&)>;
    using BusinessAnalysisCallback = std::function<void(const BusinessAnalysisResult&)>;
    using MarketAnalysisCallback = std::function<void(const MarketAnalysisResult&)>;

    virtual ~AIEngine() = default;

    /**
     * @brief Get the AI provider type
     */
    virtual AIProvider getProvider() const = 0;

    /**
     * @brief Get provider name as string
     */
    virtual std::string getProviderName() const = 0;

    /**
     * @brief Check if the engine is properly configured
     */
    virtual bool isConfigured() const = 0;

    /**
     * @brief Set the configuration
     */
    virtual void setConfig(const AIEngineConfig& config) = 0;

    /**
     * @brief Get the current configuration
     */
    virtual AIEngineConfig getConfig() const = 0;

    /**
     * @brief Perform a raw AI completion request
     * @param request The analysis request
     * @param callback Callback with response
     */
    virtual void complete(const AIAnalysisRequest& request, AnalysisCallback callback) = 0;

    /**
     * @brief Synchronous version of complete
     */
    virtual AIAnalysisResponse completeSync(const AIAnalysisRequest& request) = 0;

    /**
     * @brief Analyze a business for catering potential
     * @param business The business to analyze
     * @param callback Callback with analysis result
     */
    virtual void analyzeBusinessPotential(
        const Models::BusinessInfo& business,
        BusinessAnalysisCallback callback
    ) = 0;

    /**
     * @brief Synchronous version of analyzeBusinessPotential
     */
    virtual BusinessAnalysisResult analyzeBusinessPotentialSync(
        const Models::BusinessInfo& business
    ) = 0;

    /**
     * @brief Analyze market potential for an area
     * @param demographics Demographic data for the area
     * @param businesses Businesses in the area
     * @param callback Callback with analysis result
     */
    virtual void analyzeMarketPotential(
        const std::vector<Models::DemographicData>& demographics,
        const std::vector<Models::BusinessInfo>& businesses,
        MarketAnalysisCallback callback
    ) = 0;

    /**
     * @brief Synchronous version of analyzeMarketPotential
     */
    virtual MarketAnalysisResult analyzeMarketPotentialSync(
        const std::vector<Models::DemographicData>& demographics,
        const std::vector<Models::BusinessInfo>& businesses
    ) = 0;

    /**
     * @brief Generate a summary for search results
     * @param totalResults Total number of results
     * @param highPotentialCount Number of high-potential leads
     * @param businessSummaries Brief summaries of top businesses
     * @return Generated summary text
     */
    virtual std::string generateSearchSummary(
        int totalResults,
        int highPotentialCount,
        const std::vector<std::string>& businessSummaries
    ) = 0;

    /**
     * @brief Test the connection to the AI provider
     * @return True if connection is successful
     */
    virtual bool testConnection() = 0;

protected:
    /**
     * @brief Build a prompt for business analysis
     */
    static std::string buildBusinessAnalysisPrompt(const Models::BusinessInfo& business);

    /**
     * @brief Build a prompt for market analysis
     */
    static std::string buildMarketAnalysisPrompt(
        const std::vector<Models::DemographicData>& demographics,
        const std::vector<Models::BusinessInfo>& businesses
    );

    /**
     * @brief Parse business analysis from AI response
     */
    static BusinessAnalysisResult parseBusinessAnalysis(const std::string& response);

    /**
     * @brief Parse market analysis from AI response
     */
    static MarketAnalysisResult parseMarketAnalysis(const std::string& response);
};

/**
 * @brief Factory function to create AI engine based on provider
 */
std::unique_ptr<AIEngine> createAIEngine(AIProvider provider, const AIEngineConfig& config);

/**
 * @brief Get string representation of AIProvider
 */
std::string aiProviderToString(AIProvider provider);

/**
 * @brief Parse AIProvider from string
 */
AIProvider stringToAIProvider(const std::string& str);

} // namespace Services
} // namespace FranchiseAI

#endif // AI_ENGINE_H
