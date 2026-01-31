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

        // Load ApiLogicServer settings (infrastructure config)
        if (const char* host = std::getenv("API_LOGIC_SERVER_HOST")) {
            apiLogicServerHost_ = host;
        }
        if (const char* port = std::getenv("API_LOGIC_SERVER_PORT")) {
            try { apiLogicServerPort_ = std::stoi(port); } catch (...) {}
        }
        if (const char* protocol = std::getenv("API_LOGIC_SERVER_PROTOCOL")) {
            apiLogicServerProtocol_ = protocol;
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
            std::cerr << "  [Config] Failed to open: " << filepath << std::endl;
            return false;
        }

        std::cout << "  [Config] Reading from: " << filepath << std::endl;

        // Simple JSON parsing for key-value pairs
        std::string line;
        while (std::getline(file, line)) {
            // Parse "key": value format (handles both quoted strings and numbers)
            auto colonPos = line.find(':');
            if (colonPos == std::string::npos) continue;

            std::string key = extractJsonKey(line);
            std::string value = extractJsonValue(line);

            // Debug output for ALS settings
            if (key == "host" || key == "port" || key == "protocol" || key == "api_prefix") {
                std::cout << "  [Config] Found: " << key << " = '" << value << "'" << std::endl;
            }

            // Only load from file if not already set (env vars take precedence)
            // ApiLogicServer settings
            if (key == "host" && !value.empty() && apiLogicServerHost_.empty()) {
                apiLogicServerHost_ = value;
                std::cout << "  [Config] Set host = " << value << std::endl;
            } else if (key == "port" && !value.empty() && apiLogicServerPort_ == 0) {
                try {
                    apiLogicServerPort_ = std::stoi(value);
                    std::cout << "  [Config] Set port = " << apiLogicServerPort_ << std::endl;
                } catch (...) {
                    std::cerr << "  [Config] Failed to parse port: " << value << std::endl;
                }
            } else if (key == "protocol" && !value.empty() && apiLogicServerProtocol_.empty()) {
                apiLogicServerProtocol_ = value;
                std::cout << "  [Config] Set protocol = " << value << std::endl;
            } else if (key == "api_prefix" && !value.empty() && apiLogicServerApiPrefix_.empty()) {
                apiLogicServerApiPrefix_ = value;
                std::cout << "  [Config] Set api_prefix = " << value << std::endl;
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

    // Getters - ApiLogicServer (individual components)
    std::string getApiLogicServerHost() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return apiLogicServerHost_.empty() ? "localhost" : apiLogicServerHost_;
    }

    int getApiLogicServerPort() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return apiLogicServerPort_ > 0 ? apiLogicServerPort_ : 5656;
    }

    std::string getApiLogicServerProtocol() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return apiLogicServerProtocol_.empty() ? "http" : apiLogicServerProtocol_;
    }

    std::string getApiLogicServerApiPrefix() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return apiLogicServerApiPrefix_.empty() ? "/api" : apiLogicServerApiPrefix_;
    }

    int getApiLogicServerTimeoutMs() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return apiLogicServerTimeoutMs_ > 0 ? apiLogicServerTimeoutMs_ : 30000;
    }

    // Constructed endpoint (assembled from individual components)
    std::string getApiLogicServerEndpoint() const {
        return getApiLogicServerProtocol() + "://" +
               getApiLogicServerHost() + ":" +
               std::to_string(getApiLogicServerPort()) +
               getApiLogicServerApiPrefix();
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

        // Build the effective values
        std::string effectiveHost = apiLogicServerHost_.empty() ? "localhost" : apiLogicServerHost_;
        int effectivePort = apiLogicServerPort_ > 0 ? apiLogicServerPort_ : 5656;
        std::string effectiveProtocol = apiLogicServerProtocol_.empty() ? "http" : apiLogicServerProtocol_;
        std::string effectivePrefix = apiLogicServerApiPrefix_.empty() ? "/api" : apiLogicServerApiPrefix_;

        // Build the full endpoint URL
        std::string endpointUrl = effectiveProtocol + "://" + effectiveHost + ":" +
                                  std::to_string(effectivePort) + effectivePrefix;

        std::cout << "ApiLogicServer Configuration:" << std::endl;
        std::cout << "  Host:            " << effectiveHost << (apiLogicServerHost_.empty() ? " (default)" : "") << std::endl;
        std::cout << "  Port:            " << effectivePort << (apiLogicServerPort_ == 0 ? " (default)" : "") << std::endl;
        std::cout << "  Protocol:        " << effectiveProtocol << (apiLogicServerProtocol_.empty() ? " (default)" : "") << std::endl;
        std::cout << "  API Prefix:      " << effectivePrefix << (apiLogicServerApiPrefix_.empty() ? " (default)" : "") << std::endl;
        std::cout << "  Endpoint URL:    " << endpointUrl << std::endl;
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
    std::string apiLogicServerHost_;
    int apiLogicServerPort_ = 0;
    std::string apiLogicServerProtocol_;
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
     * @brief Extract value from JSON-like format (handles quoted strings and unquoted numbers)
     */
    static std::string extractJsonValue(const std::string& s) {
        // Find the colon first - value is always after the colon
        size_t colonPos = s.find(':');
        if (colonPos == std::string::npos) {
            return "";
        }

        // Get everything after the colon
        std::string afterColon = s.substr(colonPos + 1);

        // Remove leading whitespace
        size_t startPos = afterColon.find_first_not_of(" \t");
        if (startPos == std::string::npos) return "";
        afterColon = afterColon.substr(startPos);

        // Check if it's a quoted string value
        if (!afterColon.empty() && afterColon[0] == '"') {
            size_t endQuote = afterColon.find('"', 1);
            if (endQuote != std::string::npos) {
                return afterColon.substr(1, endQuote - 1);
            }
        }

        // Otherwise it's an unquoted value (number, true, false, null)
        // Remove trailing whitespace and punctuation
        size_t endPos = afterColon.find_last_not_of(" \t\n\r,}]");
        if (endPos != std::string::npos) {
            afterColon = afterColon.substr(0, endPos + 1);
        }
        return afterColon;
    }

    /**
     * @brief Extract key from JSON-like format
     */
    static std::string extractJsonKey(const std::string& s) {
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
