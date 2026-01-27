#ifndef GEMINI_ENGINE_H
#define GEMINI_ENGINE_H

#include "AIEngine.h"
#include <map>
#include <chrono>

namespace FranchiseAI {
namespace Services {

/**
 * @brief Google Gemini-specific configuration
 */
struct GeminiConfig : public AIEngineConfig {
    GeminiConfig() {
        provider = AIProvider::GEMINI;
        apiEndpoint = "https://generativelanguage.googleapis.com/v1beta/models";
        model = "gemini-pro";
        maxTokens = 1024;
        temperature = 0.7;
    }
};

/**
 * @brief Google Gemini API implementation of AIEngine
 *
 * Uses Google's Generative Language API (Gemini Pro, Gemini Ultra, etc.)
 */
class GeminiEngine : public AIEngine {
public:
    GeminiEngine();
    explicit GeminiEngine(const AIEngineConfig& config);
    ~GeminiEngine() override = default;

    // AIEngine interface implementation
    AIProvider getProvider() const override { return AIProvider::GEMINI; }
    std::string getProviderName() const override { return "Google Gemini"; }
    bool isConfigured() const override;
    void setConfig(const AIEngineConfig& config) override;
    AIEngineConfig getConfig() const override { return config_; }

    void complete(const AIAnalysisRequest& request, AnalysisCallback callback) override;
    AIAnalysisResponse completeSync(const AIAnalysisRequest& request) override;

    void analyzeBusinessPotential(
        const Models::BusinessInfo& business,
        BusinessAnalysisCallback callback
    ) override;
    BusinessAnalysisResult analyzeBusinessPotentialSync(
        const Models::BusinessInfo& business
    ) override;

    void analyzeMarketPotential(
        const std::vector<Models::DemographicData>& demographics,
        const std::vector<Models::BusinessInfo>& businesses,
        MarketAnalysisCallback callback
    ) override;
    MarketAnalysisResult analyzeMarketPotentialSync(
        const std::vector<Models::DemographicData>& demographics,
        const std::vector<Models::BusinessInfo>& businesses
    ) override;

    std::string generateSearchSummary(
        int totalResults,
        int highPotentialCount,
        const std::vector<std::string>& businessSummaries
    ) override;

    bool testConnection() override;

    // Gemini-specific methods
    void setModel(const std::string& model);
    std::string getModel() const { return config_.model; }
    std::vector<std::string> getAvailableModels() const;

private:
    AIEngineConfig config_;

    // Response cache
    struct CacheEntry {
        std::string response;
        std::chrono::steady_clock::time_point timestamp;
    };
    std::map<std::string, CacheEntry> cache_;

    // Build the full API URL for a specific model
    std::string buildAPIUrl() const;

    // HTTP request helper
    std::string makeAPIRequest(const std::string& requestBody);

    // JSON helpers
    std::string buildRequestJSON(
        const std::string& systemPrompt,
        const std::string& userPrompt
    );
    AIAnalysisResponse parseAPIResponse(const std::string& jsonResponse);

    // Cache helpers
    std::string getCacheKey(const AIAnalysisRequest& request);
    bool isCacheValid(const std::string& key);
    void cacheResponse(const std::string& key, const std::string& response);

    // Fallback to local analysis when API is unavailable
    BusinessAnalysisResult localBusinessAnalysis(const Models::BusinessInfo& business);
    MarketAnalysisResult localMarketAnalysis(
        const std::vector<Models::DemographicData>& demographics,
        const std::vector<Models::BusinessInfo>& businesses
    );
};

} // namespace Services
} // namespace FranchiseAI

#endif // GEMINI_ENGINE_H
