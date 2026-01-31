#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <string>
#include <mutex>
#include <fstream>
#include <iostream>
#include <cstdlib>

namespace FranchiseAI {

/**
 * @brief Global application configuration
 *
 * Manages API keys and settings loaded from environment variables
 * or configuration files. Thread-safe singleton pattern.
 */
class AppConfig {
public:
    /**
     * @brief Get the singleton instance
     */
    static AppConfig& instance() {
        static AppConfig config;
        return config;
    }

    /**
     * @brief Load configuration from environment variables
     */
    void loadFromEnvironment() {
        std::lock_guard<std::mutex> lock(mutex_);

        // Load ApiLogicServer URL (infrastructure config)
        if (const char* url = std::getenv("API_LOGIC_SERVER_URL")) {
            apiLogicServerUrl_ = url;
        }
        if (const char* prefix = std::getenv("API_LOGIC_SERVER_API_PREFIX")) {
            apiLogicServerApiPrefix_ = prefix;
        }

        // Load OpenAI API key
        if (const char* key = std::getenv("OPENAI_API_KEY")) {
            openaiApiKey_ = key;
        }

        // Load Google API key
        if (const char* key = std::getenv("GOOGLE_API_KEY")) {
            googleApiKey_ = key;
        }

        // Load BBB API key
        if (const char* key = std::getenv("BBB_API_KEY")) {
            bbbApiKey_ = key;
        }

        // Load Census/Demographics API key
        if (const char* key = std::getenv("CENSUS_API_KEY")) {
            censusApiKey_ = key;
        }

        // Load Gemini API key (alternative to OpenAI)
        if (const char* key = std::getenv("GEMINI_API_KEY")) {
            geminiApiKey_ = key;
        }
    }

    /**
     * @brief Load configuration from a JSON file
     * @param filepath Path to the config file
     * @return true if loaded successfully
     */
    bool loadFromFile(const std::string& filepath) {
        std::lock_guard<std::mutex> lock(mutex_);

        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        // Simple JSON parsing for key-value pairs
        std::string line;
        while (std::getline(file, line)) {
            // Parse "key": "value" format
            auto colonPos = line.find(':');
            if (colonPos == std::string::npos) continue;

            std::string key = extractJsonString(line.substr(0, colonPos));
            std::string value = extractJsonString(line.substr(colonPos + 1));

            // Only load from file if not already set (env vars take precedence)
            // ApiLogicServer settings
            if (key == "url" && !value.empty() && apiLogicServerUrl_.empty()) {
                apiLogicServerUrl_ = value;
            } else if (key == "api_prefix" && !value.empty() && apiLogicServerApiPrefix_.empty()) {
                apiLogicServerApiPrefix_ = value;
            } else if (key == "timeout_ms" && !value.empty()) {
                try { apiLogicServerTimeoutMs_ = std::stoi(value); } catch (...) {}
            }
            // API keys
            else if (key == "openai_api_key" && !value.empty() && openaiApiKey_.empty()) {
                openaiApiKey_ = value;
            } else if (key == "google_api_key" && !value.empty() && googleApiKey_.empty()) {
                googleApiKey_ = value;
            } else if (key == "bbb_api_key" && !value.empty() && bbbApiKey_.empty()) {
                bbbApiKey_ = value;
            } else if (key == "census_api_key" && !value.empty() && censusApiKey_.empty()) {
                censusApiKey_ = value;
            } else if (key == "gemini_api_key" && !value.empty() && geminiApiKey_.empty()) {
                geminiApiKey_ = value;
            } else if (key == "openai_model" && !value.empty() && openaiModel_.empty()) {
                openaiModel_ = value;
            }
        }

        configFilePath_ = filepath;
        return true;
    }

    /**
     * @brief Save configuration to a JSON file
     * @param filepath Path to the config file (uses last loaded path if empty)
     * @return true if saved successfully
     */
    bool saveToFile(const std::string& filepath = "") {
        std::lock_guard<std::mutex> lock(mutex_);

        std::string path = filepath.empty() ? configFilePath_ : filepath;
        if (path.empty()) {
            path = "config/app_config.json";
        }

        std::ofstream file(path);
        if (!file.is_open()) {
            return false;
        }

        file << "{\n";
        file << "  \"openai_api_key\": \"" << openaiApiKey_ << "\",\n";
        file << "  \"openai_model\": \"" << openaiModel_ << "\",\n";
        file << "  \"google_api_key\": \"" << googleApiKey_ << "\",\n";
        file << "  \"bbb_api_key\": \"" << bbbApiKey_ << "\",\n";
        file << "  \"census_api_key\": \"" << censusApiKey_ << "\",\n";
        file << "  \"gemini_api_key\": \"" << geminiApiKey_ << "\"\n";
        file << "}\n";

        configFilePath_ = path;
        return true;
    }

    // Getters - ApiLogicServer
    std::string getApiLogicServerUrl() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return apiLogicServerUrl_.empty() ? "http://localhost:5656" : apiLogicServerUrl_;
    }

    std::string getApiLogicServerApiPrefix() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return apiLogicServerApiPrefix_.empty() ? "/api" : apiLogicServerApiPrefix_;
    }

    int getApiLogicServerTimeoutMs() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return apiLogicServerTimeoutMs_ > 0 ? apiLogicServerTimeoutMs_ : 30000;
    }

    std::string getApiLogicServerEndpoint() const {
        return getApiLogicServerUrl() + getApiLogicServerApiPrefix();
    }

    // Getters - API Keys
    std::string getOpenAIApiKey() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return openaiApiKey_;
    }

    std::string getOpenAIModel() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return openaiModel_.empty() ? "gpt-4o" : openaiModel_;
    }

    std::string getGoogleApiKey() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return googleApiKey_;
    }

    std::string getBBBApiKey() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return bbbApiKey_;
    }

    std::string getCensusApiKey() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return censusApiKey_;
    }

    std::string getGeminiApiKey() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return geminiApiKey_;
    }

    // Setters
    void setOpenAIApiKey(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        openaiApiKey_ = key;
    }

    void setOpenAIModel(const std::string& model) {
        std::lock_guard<std::mutex> lock(mutex_);
        openaiModel_ = model;
    }

    void setGoogleApiKey(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        googleApiKey_ = key;
    }

    void setBBBApiKey(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        bbbApiKey_ = key;
    }

    void setCensusApiKey(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        censusApiKey_ = key;
    }

    void setGeminiApiKey(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        geminiApiKey_ = key;
    }

    // Configuration status checks
    bool hasOpenAIKey() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return !openaiApiKey_.empty();
    }

    bool hasGoogleKey() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return !googleApiKey_.empty();
    }

    bool hasBBBKey() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return !bbbApiKey_.empty();
    }

    bool hasCensusKey() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return !censusApiKey_.empty();
    }

    bool hasGeminiKey() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return !geminiApiKey_.empty();
    }

    /**
     * @brief Print configuration status (for startup logging)
     */
    void printStatus() const {
        std::lock_guard<std::mutex> lock(mutex_);

        std::cout << "Infrastructure Configuration:" << std::endl;
        std::cout << "  ApiLogicServer:  " << (apiLogicServerUrl_.empty() ? "http://localhost:5656 (default)" : apiLogicServerUrl_) << std::endl;
        std::cout << "  API Prefix:      " << (apiLogicServerApiPrefix_.empty() ? "/api (default)" : apiLogicServerApiPrefix_) << std::endl;
        std::cout << std::endl;

        std::cout << "API Configuration Status:" << std::endl;
        std::cout << "  OpenAI API Key:  " << (openaiApiKey_.empty() ? "Not configured" : "Configured (" + maskKey(openaiApiKey_) + ")") << std::endl;
        std::cout << "  OpenAI Model:    " << (openaiModel_.empty() ? "gpt-4o (default)" : openaiModel_) << std::endl;
        std::cout << "  Gemini API Key:  " << (geminiApiKey_.empty() ? "Not configured" : "Configured (" + maskKey(geminiApiKey_) + ")") << std::endl;
        std::cout << "  Google API Key:  " << (googleApiKey_.empty() ? "Not configured" : "Configured (" + maskKey(googleApiKey_) + ")") << std::endl;
        std::cout << "  BBB API Key:     " << (bbbApiKey_.empty() ? "Not configured" : "Configured (" + maskKey(bbbApiKey_) + ")") << std::endl;
        std::cout << "  Census API Key:  " << (censusApiKey_.empty() ? "Not configured" : "Configured (" + maskKey(censusApiKey_) + ")") << std::endl;
        std::cout << std::endl;
    }

private:
    AppConfig() = default;
    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;

    mutable std::mutex mutex_;

    // ApiLogicServer settings (infrastructure - from local config only)
    std::string apiLogicServerUrl_;
    std::string apiLogicServerApiPrefix_;
    int apiLogicServerTimeoutMs_ = 30000;

    // API keys (from local config or environment)
    std::string openaiApiKey_;
    std::string openaiModel_;
    std::string googleApiKey_;
    std::string bbbApiKey_;
    std::string censusApiKey_;
    std::string geminiApiKey_;
    std::string configFilePath_;

    /**
     * @brief Extract string value from JSON-like format
     */
    static std::string extractJsonString(const std::string& s) {
        size_t start = s.find('"');
        if (start == std::string::npos) return "";
        size_t end = s.find('"', start + 1);
        if (end == std::string::npos) return "";
        return s.substr(start + 1, end - start - 1);
    }

    /**
     * @brief Mask API key for display (show first 4 and last 4 chars)
     */
    static std::string maskKey(const std::string& key) {
        if (key.length() <= 8) {
            return std::string(key.length(), '*');
        }
        return key.substr(0, 4) + "..." + key.substr(key.length() - 4);
    }
};

} // namespace FranchiseAI

#endif // APP_CONFIG_H
