#include "AIEngine.h"
#include "OpenAIEngine.h"
#include "GeminiEngine.h"
#include <sstream>
#include <algorithm>
#include <regex>
#include <iomanip>

namespace FranchiseAI {
namespace Services {

std::string AIEngine::buildBusinessAnalysisPrompt(const Models::BusinessInfo& business) {
    std::ostringstream prompt;

    prompt << "Analyze the following business for corporate catering potential:\n\n";
    prompt << "Business Name: " << business.name << "\n";
    prompt << "Type: " << business.getBusinessTypeString() << "\n";
    prompt << "Description: " << business.description << "\n";
    prompt << "Employee Count: " << business.employeeCount << "\n";
    prompt << "Location: " << business.address.city << ", " << business.address.state << "\n";

    if (business.hasConferenceRoom) {
        prompt << "Has Conference Room: Yes\n";
    }
    if (business.hasEventSpace) {
        prompt << "Has Event Space: Yes\n";
    }
    if (business.regularMeetings) {
        prompt << "Regular Meetings: Yes\n";
    }

    if (business.googleRating > 0) {
        prompt << "Google Rating: " << business.googleRating << "/5 ("
               << business.googleReviewCount << " reviews)\n";
    }

    if (business.bbbAccredited) {
        prompt << "BBB Accredited: Yes, Rating: " << business.getBBBRatingString() << "\n";
    }

    if (business.yearEstablished > 0) {
        prompt << "Year Established: " << business.yearEstablished << "\n";
    }

    prompt << "\nProvide your analysis in the following format:\n";
    prompt << "SUMMARY: [2-3 sentence summary of catering potential]\n";
    prompt << "SCORE: [0-100 catering potential score]\n";
    prompt << "HIGHLIGHTS:\n- [key highlight 1]\n- [key highlight 2]\n- [key highlight 3]\n";
    prompt << "ACTIONS:\n- [recommended action 1]\n- [recommended action 2]\n- [recommended action 3]\n";
    prompt << "MATCH_REASON: [why this business is a good catering prospect]\n";

    return prompt.str();
}

std::string AIEngine::buildMarketAnalysisPrompt(
    const std::vector<Models::DemographicData>& demographics,
    const std::vector<Models::BusinessInfo>& businesses
) {
    std::ostringstream prompt;

    prompt << "Analyze the following market area for corporate catering opportunities:\n\n";

    // Summarize demographics
    prompt << "DEMOGRAPHIC DATA:\n";
    int totalPopulation = 0;
    int totalBusinesses = 0;
    int totalOfficeBuildings = 0;
    double avgIncome = 0;

    for (const auto& demo : demographics) {
        totalPopulation += demo.totalPopulation;
        totalBusinesses += demo.totalBusinesses;
        totalOfficeBuildings += demo.officeBuildings;
        avgIncome += demo.medianHouseholdIncome;
    }

    if (!demographics.empty()) {
        avgIncome /= demographics.size();
    }

    prompt << "- Total Population: " << totalPopulation << "\n";
    prompt << "- Total Businesses: " << totalBusinesses << "\n";
    prompt << "- Office Buildings: " << totalOfficeBuildings << "\n";
    prompt << "- Avg Household Income: $" << static_cast<int>(avgIncome) << "\n";
    prompt << "- Zip Codes Covered: " << demographics.size() << "\n\n";

    // Summarize businesses
    prompt << "TOP BUSINESSES:\n";
    int count = 0;
    for (const auto& biz : businesses) {
        if (count >= 10) break;
        prompt << "- " << biz.name << " (" << biz.getBusinessTypeString()
               << ", " << biz.employeeCount << " employees)\n";
        ++count;
    }

    prompt << "\nProvide your analysis in the following format:\n";
    prompt << "OVERALL_ANALYSIS: [3-4 sentence market analysis]\n";
    prompt << "MARKET_SUMMARY: [brief market summary]\n";
    prompt << "RECOMMENDATIONS:\n- [recommendation 1]\n- [recommendation 2]\n- [recommendation 3]\n";
    prompt << "OPPORTUNITIES:\n- [opportunity 1]\n- [opportunity 2]\n";
    prompt << "RISKS:\n- [risk 1]\n- [risk 2]\n";

    return prompt.str();
}

BusinessAnalysisResult AIEngine::parseBusinessAnalysis(const std::string& response) {
    BusinessAnalysisResult result;

    // Parse SUMMARY - use simple string search instead of regex with dotall
    size_t summaryStart = response.find("SUMMARY:");
    if (summaryStart != std::string::npos) {
        summaryStart += 8; // Length of "SUMMARY:"
        size_t summaryEnd = response.find("\nSCORE:", summaryStart);
        if (summaryEnd == std::string::npos) summaryEnd = response.find("\nHIGHLIGHTS:", summaryStart);
        if (summaryEnd == std::string::npos) summaryEnd = response.find("\nACTIONS:", summaryStart);
        if (summaryEnd == std::string::npos) summaryEnd = response.find("\nMATCH_REASON:", summaryStart);
        if (summaryEnd == std::string::npos) summaryEnd = response.length();
        result.summary = response.substr(summaryStart, summaryEnd - summaryStart);
        // Trim whitespace
        result.summary.erase(0, result.summary.find_first_not_of(" \n\r\t"));
        if (!result.summary.empty()) {
            result.summary.erase(result.summary.find_last_not_of(" \n\r\t") + 1);
        }
    }

    // Parse SCORE
    std::regex scoreRegex("SCORE:\\s*(\\d+)");
    std::smatch scoreMatch;
    if (std::regex_search(response, scoreMatch, scoreRegex)) {
        result.cateringPotentialScore = std::stoi(scoreMatch[1].str());
        result.cateringPotentialScore = std::min(100, std::max(0, result.cateringPotentialScore));
    }

    // Parse HIGHLIGHTS
    std::regex highlightsRegex("HIGHLIGHTS:\\s*\\n((?:-[^\\n]+\\n?)+)");
    std::smatch highlightsMatch;
    if (std::regex_search(response, highlightsMatch, highlightsRegex)) {
        std::string highlights = highlightsMatch[1].str();
        std::regex itemRegex("-\\s*(.+)");
        std::sregex_iterator it(highlights.begin(), highlights.end(), itemRegex);
        std::sregex_iterator end;
        while (it != end) {
            std::string item = (*it)[1].str();
            item.erase(0, item.find_first_not_of(" \t"));
            item.erase(item.find_last_not_of(" \t\r\n") + 1);
            if (!item.empty()) {
                result.keyHighlights.push_back(item);
            }
            ++it;
        }
    }

    // Parse ACTIONS
    std::regex actionsRegex("ACTIONS:\\s*\\n((?:-[^\\n]+\\n?)+)");
    std::smatch actionsMatch;
    if (std::regex_search(response, actionsMatch, actionsRegex)) {
        std::string actions = actionsMatch[1].str();
        std::regex itemRegex("-\\s*(.+)");
        std::sregex_iterator it(actions.begin(), actions.end(), itemRegex);
        std::sregex_iterator end;
        while (it != end) {
            std::string item = (*it)[1].str();
            item.erase(0, item.find_first_not_of(" \t"));
            item.erase(item.find_last_not_of(" \t\r\n") + 1);
            if (!item.empty()) {
                result.recommendedActions.push_back(item);
            }
            ++it;
        }
    }

    // Parse MATCH_REASON
    std::regex matchRegex("MATCH_REASON:\\s*(.+?)(?=\\n|$)");
    std::smatch matchMatch;
    if (std::regex_search(response, matchMatch, matchRegex)) {
        result.matchReason = matchMatch[1].str();
        result.matchReason.erase(0, result.matchReason.find_first_not_of(" \t"));
        result.matchReason.erase(result.matchReason.find_last_not_of(" \t\r\n") + 1);
    }

    result.confidenceScore = result.cateringPotentialScore / 100.0;

    return result;
}

MarketAnalysisResult AIEngine::parseMarketAnalysis(const std::string& response) {
    MarketAnalysisResult result;

    // Parse OVERALL_ANALYSIS - use simple string search instead of regex with dotall
    size_t analysisStart = response.find("OVERALL_ANALYSIS:");
    if (analysisStart != std::string::npos) {
        analysisStart += 17; // Length of "OVERALL_ANALYSIS:"
        size_t analysisEnd = response.find("\nMARKET_SUMMARY:", analysisStart);
        if (analysisEnd == std::string::npos) analysisEnd = response.find("\nRECOMMENDATIONS:", analysisStart);
        if (analysisEnd == std::string::npos) analysisEnd = response.find("\nOPPORTUNITIES:", analysisStart);
        if (analysisEnd == std::string::npos) analysisEnd = response.find("\nRISKS:", analysisStart);
        if (analysisEnd == std::string::npos) analysisEnd = response.length();
        result.overallAnalysis = response.substr(analysisStart, analysisEnd - analysisStart);
        result.overallAnalysis.erase(0, result.overallAnalysis.find_first_not_of(" \n\r\t"));
        if (!result.overallAnalysis.empty()) {
            result.overallAnalysis.erase(result.overallAnalysis.find_last_not_of(" \n\r\t") + 1);
        }
    }

    // Parse MARKET_SUMMARY
    std::regex summaryRegex("MARKET_SUMMARY:\\s*(.+?)(?=\\n(?:RECOMMENDATIONS|OPPORTUNITIES|RISKS|$))");
    std::smatch summaryMatch;
    if (std::regex_search(response, summaryMatch, summaryRegex)) {
        result.marketSummary = summaryMatch[1].str();
        result.marketSummary.erase(0, result.marketSummary.find_first_not_of(" \t"));
        result.marketSummary.erase(result.marketSummary.find_last_not_of(" \t\r\n") + 1);
    }

    // Parse RECOMMENDATIONS
    std::regex recRegex("RECOMMENDATIONS:\\s*\\n((?:-[^\\n]+\\n?)+)");
    std::smatch recMatch;
    if (std::regex_search(response, recMatch, recRegex)) {
        std::string recs = recMatch[1].str();
        std::regex itemRegex("-\\s*(.+)");
        std::sregex_iterator it(recs.begin(), recs.end(), itemRegex);
        std::sregex_iterator end;
        while (it != end) {
            std::string item = (*it)[1].str();
            item.erase(0, item.find_first_not_of(" \t"));
            item.erase(item.find_last_not_of(" \t\r\n") + 1);
            if (!item.empty()) {
                result.topRecommendations.push_back(item);
            }
            ++it;
        }
    }

    // Parse OPPORTUNITIES
    std::regex oppRegex("OPPORTUNITIES:\\s*\\n((?:-[^\\n]+\\n?)+)");
    std::smatch oppMatch;
    if (std::regex_search(response, oppMatch, oppRegex)) {
        std::string opps = oppMatch[1].str();
        std::regex itemRegex("-\\s*(.+)");
        std::sregex_iterator it(opps.begin(), opps.end(), itemRegex);
        std::sregex_iterator end;
        while (it != end) {
            std::string item = (*it)[1].str();
            item.erase(0, item.find_first_not_of(" \t"));
            item.erase(item.find_last_not_of(" \t\r\n") + 1);
            if (!item.empty()) {
                result.opportunities.push_back(item);
            }
            ++it;
        }
    }

    // Parse RISKS
    std::regex riskRegex("RISKS:\\s*\\n((?:-[^\\n]+\\n?)+)");
    std::smatch riskMatch;
    if (std::regex_search(response, riskMatch, riskRegex)) {
        std::string risks = riskMatch[1].str();
        std::regex itemRegex("-\\s*(.+)");
        std::sregex_iterator it(risks.begin(), risks.end(), itemRegex);
        std::sregex_iterator end;
        while (it != end) {
            std::string item = (*it)[1].str();
            item.erase(0, item.find_first_not_of(" \t"));
            item.erase(item.find_last_not_of(" \t\r\n") + 1);
            if (!item.empty()) {
                result.risks.push_back(item);
            }
            ++it;
        }
    }

    return result;
}

std::string aiProviderToString(AIProvider provider) {
    switch (provider) {
        case AIProvider::OPENAI: return "OpenAI";
        case AIProvider::GEMINI: return "Gemini";
        case AIProvider::LOCAL: return "Local";
        default: return "Unknown";
    }
}

AIProvider stringToAIProvider(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "openai" || lower == "gpt") {
        return AIProvider::OPENAI;
    } else if (lower == "gemini" || lower == "google") {
        return AIProvider::GEMINI;
    }
    return AIProvider::LOCAL;
}

std::unique_ptr<AIEngine> createAIEngine(AIProvider provider, const AIEngineConfig& config) {
    switch (provider) {
        case AIProvider::OPENAI:
            return std::make_unique<OpenAIEngine>(config);
        case AIProvider::GEMINI:
            return std::make_unique<GeminiEngine>(config);
        case AIProvider::LOCAL:
        default:
            // For LOCAL provider, we return nullptr and the caller should use local analysis
            return nullptr;
    }
}

} // namespace Services
} // namespace FranchiseAI
