#ifndef GOOGLE_GEOCODING_API_H
#define GOOGLE_GEOCODING_API_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include "GeocodingService.h"
#include "ThreadPool.h"
#include "models/GeoLocation.h"

namespace FranchiseAI {
namespace Services {

/**
 * @brief Configuration for Google Geocoding API
 */
struct GoogleGeocodingConfig {
    std::string apiKey;
    std::string endpoint = "https://maps.googleapis.com/maps/api/geocode/json";
    int requestTimeoutMs = 5000;           // 5 seconds (Google is fast)
    bool enableCaching = true;
    int cacheDurationMinutes = 1440;       // 24 hours
    std::string userAgent = "FranchiseAI/1.0";

    // Thread pool settings for background geocoding
    int threadPoolSize = 4;                // Number of concurrent geocoding threads
    int maxQueuedRequests = 100;           // Maximum pending requests

    // Rate limiting (Google allows 50 QPS for paid tier)
    int maxRequestsPerSecond = 50;

    // Retry settings
    int maxRetries = 3;
    int retryDelayMs = 100;

    /**
     * @brief Check if API key is configured
     */
    bool isConfigured() const {
        return !apiKey.empty();
    }

    /**
     * @brief Get recommended memory for thread pool
     */
    int getRecommendedMemoryMB() const {
        return ThreadPoolConfig::getRecommendedMemoryMB(threadPoolSize);
    }
};

/**
 * @brief Statistics for Google Geocoding API usage
 */
struct GoogleGeocodingStats {
    std::atomic<int> totalRequests{0};
    std::atomic<int> successfulRequests{0};
    std::atomic<int> failedRequests{0};
    std::atomic<int> cacheHits{0};
    std::atomic<int> cacheMisses{0};
    std::atomic<int> rateLimitHits{0};
    std::atomic<int64_t> totalLatencyMs{0};

    double getAverageLatencyMs() const {
        int successful = successfulRequests.load();
        if (successful == 0) return 0.0;
        return static_cast<double>(totalLatencyMs.load()) / successful;
    }

    double getCacheHitRate() const {
        int hits = cacheHits.load();
        int total = hits + cacheMisses.load();
        if (total == 0) return 0.0;
        return static_cast<double>(hits) / total * 100.0;
    }

    double getSuccessRate() const {
        int total = totalRequests.load();
        if (total == 0) return 100.0;
        return static_cast<double>(successfulRequests.load()) / total * 100.0;
    }

    void reset() {
        totalRequests = 0;
        successfulRequests = 0;
        failedRequests = 0;
        cacheHits = 0;
        cacheMisses = 0;
        rateLimitHits = 0;
        totalLatencyMs = 0;
    }
};

/**
 * @brief Result of a batch geocoding operation
 */
struct BatchGeocodeResult {
    std::vector<Models::GeoLocation> results;
    std::vector<std::string> errors;
    int successCount = 0;
    int failureCount = 0;
    int64_t totalTimeMs = 0;
};

/**
 * @brief Google Geocoding API service with thread pool support
 *
 * Provides high-performance geocoding using Google's Geocoding API
 * with multi-threaded background processing, caching, and automatic
 * retry logic.
 */
class GoogleGeocodingAPI : public IGeocodingService {
public:
    using BatchCallback = std::function<void(BatchGeocodeResult)>;
    using ProgressCallback = std::function<void(int completed, int total)>;

    GoogleGeocodingAPI();
    explicit GoogleGeocodingAPI(const GoogleGeocodingConfig& config);
    ~GoogleGeocodingAPI() override;

    // Configuration
    void setConfig(const GoogleGeocodingConfig& config);
    GoogleGeocodingConfig getConfig() const { return config_; }

    // IGeocodingService interface
    void geocode(const std::string& address, GeocodeCallback callback) override;
    void reverseGeocode(double latitude, double longitude, ReverseGeocodeCallback callback) override;
    Models::GeoLocation geocodeSync(const std::string& address) override;
    Models::GeoLocation reverseGeocodeSync(double latitude, double longitude) override;
    std::string getProviderName() const override { return "Google Maps Geocoding"; }
    bool isConfigured() const override { return config_.isConfigured(); }

    /**
     * @brief Geocode multiple addresses concurrently using thread pool
     * @param addresses List of addresses to geocode
     * @param callback Callback with batch results
     * @param progressCallback Optional progress callback
     */
    void geocodeBatch(
        const std::vector<std::string>& addresses,
        BatchCallback callback,
        ProgressCallback progressCallback = nullptr
    );

    /**
     * @brief Geocode multiple addresses synchronously
     * @param addresses List of addresses to geocode
     * @return Batch geocode result
     */
    BatchGeocodeResult geocodeBatchSync(const std::vector<std::string>& addresses);

    /**
     * @brief Pre-warm cache with addresses (async background processing)
     * @param addresses Addresses to pre-cache
     */
    void prewarmCache(const std::vector<std::string>& addresses);

    /**
     * @brief Set thread pool size for background geocoding
     * @param threadCount Number of threads
     */
    void setThreadPoolSize(int threadCount);

    /**
     * @brief Get current thread pool size
     */
    int getThreadPoolSize() const;

    /**
     * @brief Get recommended memory for current thread pool size
     */
    int getRecommendedMemoryMB() const;

    /**
     * @brief Get thread pool metrics
     */
    const ThreadPoolMetrics& getThreadPoolMetrics() const;

    // Cache management
    void clearCache();
    int getCacheSize() const;

    // Statistics
    const GoogleGeocodingStats& getStats() const { return stats_; }
    void resetStats() { stats_.reset(); }

    // Utility
    static std::string normalizeAddress(const std::string& address);

private:
    GoogleGeocodingConfig config_;
    GoogleGeocodingStats stats_;

    std::unique_ptr<ThreadPool> threadPool_;
    std::mutex threadPoolMutex_;

    // Cache: address -> (location, timestamp)
    std::unordered_map<std::string, std::pair<Models::GeoLocation, time_t>> cache_;
    mutable std::mutex cacheMutex_;

    // Internal methods
    Models::GeoLocation callGoogleAPI(const std::string& address);
    Models::GeoLocation callGoogleReverseAPI(double lat, double lon);
    std::string buildGeocodeUrl(const std::string& address);
    std::string buildReverseGeocodeUrl(double lat, double lon);
    Models::GeoLocation parseGeocodeResponse(const std::string& json, const std::string& originalAddress);
    std::string executeHttpRequest(const std::string& url);

    void initializeThreadPool();
    Models::GeoLocation getCachedResult(const std::string& normalizedAddress);
    void cacheResult(const std::string& normalizedAddress, const Models::GeoLocation& location);
};

} // namespace Services
} // namespace FranchiseAI

#endif // GOOGLE_GEOCODING_API_H
