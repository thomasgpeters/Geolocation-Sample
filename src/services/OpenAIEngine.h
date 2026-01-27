#ifndef OPENAI_ENGINE_H
#define OPENAI_ENGINE_H

#include "AIEngine.h"
#include <map>
#include <chrono>

namespace FranchiseAI {
namespace Services {

/**
 * @brief OpenAI-specific configuration
 */
struct OpenAIConfig : public AIEngineConfig {
    OpenAIConfig() {
        provider = AIProvider::OPENAI;
        apiEndpoint = "https://api.openai.com/v1/chat/completions";
        model = "gpt-3.5-turbo";
        maxTokens = 1024;
        temperature = 0.7;
    }
};

/**
 * @brief OpenAI API implementation of AIEngine
 *
 * Uses OpenAI's Chat Completions API (GPT-3.5-turbo, GPT-4, etc.)
 */
class OpenAIEngine : public AIEngine {
public:
    OpenAIEngine();
    explicit OpenAIEngine(const AIEngineConfig& config);
    ~OpenAIEngine() override = default;

    // AIEngine interface implementation
    AIProvider getProvider() const override { return AIProvider::OPENAI; }
    std::string getProviderName() const override { return "OpenAI"; }
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

    // OpenAI-specific methods
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

#endif // OPENAI_ENGINE_H
