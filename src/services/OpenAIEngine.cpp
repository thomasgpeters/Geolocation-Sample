#include "OpenAIEngine.h"
#include <curl/curl.h>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <regex>
#include <iomanip>

namespace FranchiseAI {
namespace Services {

namespace {
    // CURL write callback
    size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
        size_t totalSize = size * nmemb;
        response->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }

    // Simple JSON string escaping
    std::string escapeJSON(const std::string& str) {
        std::ostringstream result;
        for (char c : str) {
            switch (c) {
                case '"':  result << "\\\""; break;
                case '\\': result << "\\\\"; break;
                case '\n': result << "\\n"; break;
                case '\r': result << "\\r"; break;
                case '\t': result << "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        result << "\\u" << std::hex << std::setfill('0') << std::setw(4)
                               << static_cast<int>(static_cast<unsigned char>(c));
                    } else {
                        result << c;
                    }
            }
        }
        return result.str();
    }

    // Extract JSON string value
    std::string extractJSONString(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos) return "";

        size_t colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos) return "";

        size_t startPos = json.find('"', colonPos + 1);
        if (startPos == std::string::npos) return "";

        std::string result;
        bool escaped = false;
        for (size_t i = startPos + 1; i < json.size(); ++i) {
            char c = json[i];
            if (escaped) {
                switch (c) {
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    default: result += c;
                }
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else if (c == '"') {
                break;
            } else {
                result += c;
            }
        }
        return result;
    }

    // Extract JSON number value
    int extractJSONNumber(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos) return 0;

        size_t colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos) return 0;

        size_t startPos = colonPos + 1;
        while (startPos < json.size() && (json[startPos] == ' ' || json[startPos] == '\t')) {
            ++startPos;
        }

        std::string numStr;
        while (startPos < json.size() && (std::isdigit(json[startPos]) || json[startPos] == '-')) {
            numStr += json[startPos++];
        }

        return numStr.empty() ? 0 : std::stoi(numStr);
    }
}

OpenAIEngine::OpenAIEngine() {
    config_.provider = AIProvider::OPENAI;
    config_.apiEndpoint = "https://api.openai.com/v1/chat/completions";
    config_.model = "gpt-3.5-turbo";
    config_.maxTokens = 1024;
    config_.temperature = 0.7;
}

OpenAIEngine::OpenAIEngine(const AIEngineConfig& config) : config_(config) {
    config_.provider = AIProvider::OPENAI;
    if (config_.apiEndpoint.empty()) {
        config_.apiEndpoint = "https://api.openai.com/v1/chat/completions";
    }
    if (config_.model.empty()) {
        config_.model = "gpt-3.5-turbo";
    }
}

bool OpenAIEngine::isConfigured() const {
    return !config_.apiKey.empty();
}

void OpenAIEngine::setConfig(const AIEngineConfig& config) {
    config_ = config;
    config_.provider = AIProvider::OPENAI;
}

void OpenAIEngine::setModel(const std::string& model) {
    config_.model = model;
}

std::vector<std::string> OpenAIEngine::getAvailableModels() const {
    return {
        "gpt-4o",
        "gpt-4o-mini",
        "gpt-4-turbo",
        "gpt-4",
        "gpt-3.5-turbo"
    };
}

std::string OpenAIEngine::buildRequestJSON(
    const std::string& systemPrompt,
    const std::string& userPrompt
) {
    std::ostringstream json;
    json << "{"
         << "\"model\":\"" << escapeJSON(config_.model) << "\","
         << "\"messages\":["
         << "{\"role\":\"system\",\"content\":\"" << escapeJSON(systemPrompt) << "\"},"
         << "{\"role\":\"user\",\"content\":\"" << escapeJSON(userPrompt) << "\"}"
         << "],"
         << "\"max_tokens\":" << config_.maxTokens << ","
         << "\"temperature\":" << config_.temperature
         << "}";
    return json.str();
}

std::string OpenAIEngine::makeAPIRequest(const std::string& requestBody) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "{\"error\":{\"message\":\"Failed to initialize CURL\"}}";
    }

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string authHeader = "Authorization: Bearer " + config_.apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, config_.apiEndpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, config_.timeoutMs);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "{\"error\":{\"message\":\"CURL error: " +
               std::string(curl_easy_strerror(res)) + "\"}}";
    }

    return response;
}

AIAnalysisResponse OpenAIEngine::parseAPIResponse(const std::string& jsonResponse) {
    AIAnalysisResponse response;
    response.provider = "OpenAI";
    response.model = config_.model;

    // Check for error
    if (jsonResponse.find("\"error\"") != std::string::npos) {
        response.success = false;
        response.error = extractJSONString(jsonResponse, "message");
        if (response.error.empty()) {
            response.error = "Unknown API error";
        }
        return response;
    }

    // Extract content from choices[0].message.content
    size_t contentStart = jsonResponse.find("\"content\"");
    if (contentStart != std::string::npos) {
        response.content = extractJSONString(jsonResponse, "content");
        response.success = !response.content.empty();
    }

    // Extract token usage
    response.tokensUsed = extractJSONNumber(jsonResponse, "total_tokens");

    if (response.success) {
        response.confidenceScore = 0.85;  // Default confidence for successful responses
    }

    return response;
}

void OpenAIEngine::complete(const AIAnalysisRequest& request, AnalysisCallback callback) {
    auto response = completeSync(request);
    if (callback) {
        callback(response);
    }
}

AIAnalysisResponse OpenAIEngine::completeSync(const AIAnalysisRequest& request) {
    if (!isConfigured()) {
        AIAnalysisResponse response;
        response.success = false;
        response.error = "OpenAI API key not configured";
        response.provider = "OpenAI";
        return response;
    }

    // Check cache
    std::string cacheKey = getCacheKey(request);
    if (config_.enableCaching && isCacheValid(cacheKey)) {
        AIAnalysisResponse response;
        response.success = true;
        response.content = cache_[cacheKey].response;
        response.provider = "OpenAI (cached)";
        response.model = config_.model;
        return response;
    }

    std::string systemPrompt = request.systemPrompt.empty()
        ? "You are an AI assistant helping analyze businesses for corporate catering potential. "
          "Provide concise, actionable insights."
        : request.systemPrompt;

    std::string requestJSON = buildRequestJSON(systemPrompt, request.prompt);
    std::string apiResponse = makeAPIRequest(requestJSON);
    AIAnalysisResponse response = parseAPIResponse(apiResponse);

    // Cache successful response
    if (response.success && config_.enableCaching) {
        cacheResponse(cacheKey, response.content);
    }

    return response;
}

void OpenAIEngine::analyzeBusinessPotential(
    const Models::BusinessInfo& business,
    BusinessAnalysisCallback callback
) {
    auto result = analyzeBusinessPotentialSync(business);
    if (callback) {
        callback(result);
    }
}

BusinessAnalysisResult OpenAIEngine::analyzeBusinessPotentialSync(
    const Models::BusinessInfo& business
) {
    if (!isConfigured()) {
        // Fall back to local analysis
        return localBusinessAnalysis(business);
    }

    AIAnalysisRequest request;
    request.prompt = buildBusinessAnalysisPrompt(business);
    request.systemPrompt = "You are an expert business analyst specializing in corporate catering "
                          "market analysis. Analyze businesses for their potential as catering clients. "
                          "Consider factors like employee count, meeting facilities, company type, and location.";

    auto response = completeSync(request);

    if (!response.success) {
        // Fall back to local analysis
        return localBusinessAnalysis(business);
    }

    BusinessAnalysisResult result = parseBusinessAnalysis(response.content);
    result.confidenceScore = response.confidenceScore;

    return result;
}

void OpenAIEngine::analyzeMarketPotential(
    const std::vector<Models::DemographicData>& demographics,
    const std::vector<Models::BusinessInfo>& businesses,
    MarketAnalysisCallback callback
) {
    auto result = analyzeMarketPotentialSync(demographics, businesses);
    if (callback) {
        callback(result);
    }
}

MarketAnalysisResult OpenAIEngine::analyzeMarketPotentialSync(
    const std::vector<Models::DemographicData>& demographics,
    const std::vector<Models::BusinessInfo>& businesses
) {
    if (!isConfigured()) {
        return localMarketAnalysis(demographics, businesses);
    }

    AIAnalysisRequest request;
    request.prompt = buildMarketAnalysisPrompt(demographics, businesses);
    request.systemPrompt = "You are a market research analyst specializing in the food service industry. "
                          "Analyze geographic areas for corporate catering business opportunities. "
                          "Consider demographics, business density, and economic factors.";

    auto response = completeSync(request);

    if (!response.success) {
        return localMarketAnalysis(demographics, businesses);
    }

    return parseMarketAnalysis(response.content);
}

std::string OpenAIEngine::generateSearchSummary(
    int totalResults,
    int highPotentialCount,
    const std::vector<std::string>& businessSummaries
) {
    if (!isConfigured()) {
        std::ostringstream summary;
        summary << "Found " << totalResults << " potential catering prospects. "
                << highPotentialCount << " are high-potential leads (score 60+).";
        return summary.str();
    }

    std::ostringstream prompt;
    prompt << "Generate a brief (2-3 sentence) search results summary:\n"
           << "- Total results: " << totalResults << "\n"
           << "- High-potential leads: " << highPotentialCount << "\n"
           << "Top businesses:\n";

    int count = 0;
    for (const auto& summary : businessSummaries) {
        if (count++ >= 5) break;
        prompt << "- " << summary << "\n";
    }

    AIAnalysisRequest request;
    request.prompt = prompt.str();
    request.systemPrompt = "Generate a professional, concise summary of search results for catering prospects.";

    auto response = completeSync(request);

    if (!response.success) {
        std::ostringstream summary;
        summary << "Found " << totalResults << " potential catering prospects. "
                << highPotentialCount << " are high-potential leads.";
        return summary.str();
    }

    return response.content;
}

bool OpenAIEngine::testConnection() {
    if (!isConfigured()) {
        return false;
    }

    AIAnalysisRequest request;
    request.prompt = "Hello";
    request.systemPrompt = "Respond with 'OK' only.";

    // Temporarily disable caching for test
    bool cachingEnabled = config_.enableCaching;
    config_.enableCaching = false;

    auto response = completeSync(request);

    config_.enableCaching = cachingEnabled;

    return response.success;
}

std::string OpenAIEngine::getCacheKey(const AIAnalysisRequest& request) {
    return request.systemPrompt + "|" + request.prompt;
}

bool OpenAIEngine::isCacheValid(const std::string& key) {
    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    auto age = std::chrono::duration_cast<std::chrono::minutes>(now - it->second.timestamp);
    return age.count() < config_.cacheDurationMinutes;
}

void OpenAIEngine::cacheResponse(const std::string& key, const std::string& response) {
    CacheEntry entry;
    entry.response = response;
    entry.timestamp = std::chrono::steady_clock::now();
    cache_[key] = entry;
}

BusinessAnalysisResult OpenAIEngine::localBusinessAnalysis(const Models::BusinessInfo& business) {
    BusinessAnalysisResult result;

    std::ostringstream summary;
    summary << business.name << " is a " << business.getBusinessTypeString()
            << " with approximately " << business.employeeCount << " employees. ";

    if (business.hasConferenceRoom || business.hasEventSpace) {
        summary << "This location has ";
        if (business.hasConferenceRoom && business.hasEventSpace) {
            summary << "conference rooms and event space, ";
        } else if (business.hasConferenceRoom) {
            summary << "conference rooms, ";
        } else {
            summary << "event space, ";
        }
        summary << "ideal for corporate catering. ";
    }

    if (business.bbbAccredited) {
        summary << "BBB accredited with " << business.getBBBRatingString() << " rating. ";
    }

    result.summary = summary.str();

    // Calculate score based on factors
    int score = 50;  // Base score
    if (business.employeeCount >= 100) score += 20;
    else if (business.employeeCount >= 50) score += 10;

    if (business.hasConferenceRoom) score += 10;
    if (business.hasEventSpace) score += 10;
    if (business.bbbAccredited) score += 5;
    if (business.googleRating >= 4.5) score += 5;

    result.cateringPotentialScore = std::min(100, score);

    // Key highlights
    result.keyHighlights.push_back("Employee count: ~" + std::to_string(business.employeeCount));
    result.keyHighlights.push_back("Business type: " + business.getBusinessTypeString());
    if (business.hasConferenceRoom) {
        result.keyHighlights.push_back("Has conference facilities");
    }

    // Recommended actions
    result.recommendedActions.push_back("Research company meeting frequency");
    if (business.hasConferenceRoom) {
        result.recommendedActions.push_back("Inquire about regular meeting catering needs");
    }
    result.recommendedActions.push_back("Schedule introductory meeting with office manager");

    // Match reason
    std::ostringstream reason;
    reason << "Matched as a " << business.getBusinessTypeString();
    if (business.employeeCount > 0) {
        reason << " with " << business.employeeCount << " employees";
    }
    result.matchReason = reason.str();

    result.confidenceScore = 0.6;  // Lower confidence for local analysis

    return result;
}

MarketAnalysisResult OpenAIEngine::localMarketAnalysis(
    const std::vector<Models::DemographicData>& demographics,
    const std::vector<Models::BusinessInfo>& businesses
) {
    MarketAnalysisResult result;

    int totalBusinesses = 0;
    int totalOfficeBuildings = 0;
    int highPotential = 0;

    for (const auto& demo : demographics) {
        totalBusinesses += demo.totalBusinesses;
        totalOfficeBuildings += demo.officeBuildings;
    }

    for (const auto& biz : businesses) {
        if (biz.cateringPotentialScore >= 60) {
            ++highPotential;
        }
    }

    std::ostringstream analysis;
    analysis << "The search area shows "
             << (highPotential >= 10 ? "strong" : highPotential >= 5 ? "moderate" : "limited")
             << " catering potential with " << totalBusinesses << " businesses and "
             << totalOfficeBuildings << " office buildings. "
             << highPotential << " high-potential prospects identified.";
    result.overallAnalysis = analysis.str();

    std::ostringstream market;
    market << "Market analysis covers " << demographics.size() << " demographic zones with "
           << businesses.size() << " businesses analyzed.";
    result.marketSummary = market.str();

    result.topRecommendations.push_back("Focus on high-potential corporate offices");
    result.topRecommendations.push_back("Target conference centers for event catering");
    result.topRecommendations.push_back("Explore employee meal programs at warehouses");

    result.opportunities.push_back("Corporate meeting catering");
    result.opportunities.push_back("Regular employee lunch programs");

    result.risks.push_back("Competition from existing catering services");
    result.risks.push_back("Economic fluctuations affecting corporate spending");

    return result;
}

} // namespace Services
} // namespace FranchiseAI
