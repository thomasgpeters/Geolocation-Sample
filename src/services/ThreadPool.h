#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <atomic>
#include <stdexcept>

namespace FranchiseAI {
namespace Services {

/**
 * @brief Thread pool configuration with memory recommendations
 */
struct ThreadPoolConfig {
    int threadCount = 4;                  // Number of worker threads
    int maxQueueSize = 1000;              // Maximum pending tasks
    bool enableMetrics = true;            // Track performance metrics

    /**
     * @brief Get recommended memory in MB for the thread pool
     * @param threadCount Number of threads
     * @return Recommended memory in MB
     */
    static int getRecommendedMemoryMB(int threadCount) {
        // Base memory: ~2MB per thread for stack + overhead
        // Task queue: ~1KB per queued task (assuming 1000 max)
        // HTTP connections: ~64KB per thread for CURL buffers
        // Response buffers: ~256KB per thread for JSON responses
        // Safety margin: 20%

        const int stackPerThreadMB = 2;        // Thread stack size
        const int curlBufferPerThreadKB = 64;  // CURL connection pool
        const int responseBufferPerThreadKB = 256; // Response storage
        const int queueOverheadMB = 1;         // Task queue overhead

        int perThreadMB = stackPerThreadMB +
                          (curlBufferPerThreadKB + responseBufferPerThreadKB) / 1024;

        int totalMB = (perThreadMB * threadCount) + queueOverheadMB;

        // Add 20% safety margin
        return static_cast<int>(totalMB * 1.2);
    }

    /**
     * @brief Get recommended thread count based on available memory
     * @param availableMemoryMB Available memory in MB
     * @return Recommended thread count
     */
    static int getRecommendedThreadCount(int availableMemoryMB) {
        // Inverse calculation of getRecommendedMemoryMB
        // Ensure at least 1 thread
        if (availableMemoryMB < 5) return 1;

        const int perThreadMB = 3;  // Approximate per-thread memory
        int threads = (availableMemoryMB - 1) / perThreadMB;

        // Cap at reasonable maximum (typically 2x CPU cores)
        int maxThreads = std::max(1, static_cast<int>(std::thread::hardware_concurrency() * 2));
        return std::min(std::max(1, threads), maxThreads);
    }

    /**
     * @brief Get optimal thread count for system
     * @return Optimal thread count based on CPU cores
     */
    static int getOptimalThreadCount() {
        unsigned int cores = std::thread::hardware_concurrency();
        if (cores == 0) cores = 4;  // Default if detection fails

        // For I/O-bound tasks like HTTP requests, use more threads than cores
        // Network latency means threads spend most time waiting
        return std::max(4, static_cast<int>(cores * 2));
    }

    /**
     * @brief Get description for thread count setting
     * @param threadCount Number of threads
     * @return Human-readable description
     */
    static std::string getThreadCountDescription(int threadCount) {
        int memMB = getRecommendedMemoryMB(threadCount);
        std::string desc;

        if (threadCount <= 2) {
            desc = "Low - Minimal resource usage, slower geocoding";
        } else if (threadCount <= 4) {
            desc = "Balanced - Good performance with moderate resources";
        } else if (threadCount <= 8) {
            desc = "High - Fast geocoding, higher memory usage";
        } else {
            desc = "Maximum - Fastest geocoding, significant memory usage";
        }

        desc += " (Recommended: " + std::to_string(memMB) + " MB RAM)";
        return desc;
    }
};

/**
 * @brief Thread pool performance metrics
 */
struct ThreadPoolMetrics {
    std::atomic<uint64_t> tasksSubmitted{0};
    std::atomic<uint64_t> tasksCompleted{0};
    std::atomic<uint64_t> tasksFailed{0};
    std::atomic<uint64_t> totalProcessingTimeMs{0};
    std::atomic<int> currentQueueSize{0};
    std::atomic<int> activeThreads{0};

    double getAverageProcessingTimeMs() const {
        uint64_t completed = tasksCompleted.load();
        if (completed == 0) return 0.0;
        return static_cast<double>(totalProcessingTimeMs.load()) / completed;
    }

    double getThroughputPerSecond() const {
        uint64_t completed = tasksCompleted.load();
        uint64_t totalTimeMs = totalProcessingTimeMs.load();
        if (totalTimeMs == 0) return 0.0;
        return (completed * 1000.0) / totalTimeMs;
    }

    void reset() {
        tasksSubmitted = 0;
        tasksCompleted = 0;
        tasksFailed = 0;
        totalProcessingTimeMs = 0;
        currentQueueSize = 0;
    }
};

/**
 * @brief Generic thread pool for concurrent task execution
 *
 * Provides a configurable pool of worker threads for executing
 * asynchronous tasks. Designed for I/O-bound operations like
 * geocoding API calls.
 */
class ThreadPool {
public:
    /**
     * @brief Construct thread pool with specified thread count
     * @param threadCount Number of worker threads
     */
    explicit ThreadPool(int threadCount = ThreadPoolConfig::getOptimalThreadCount());

    /**
     * @brief Construct thread pool with configuration
     * @param config Thread pool configuration
     */
    explicit ThreadPool(const ThreadPoolConfig& config);

    /**
     * @brief Destructor - waits for all tasks to complete
     */
    ~ThreadPool();

    // Non-copyable, non-movable
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    /**
     * @brief Submit a task to the thread pool
     * @param task Function to execute
     * @return Future for the task result
     */
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type>;

    /**
     * @brief Submit a task without waiting for result
     * @param task Function to execute
     */
    void execute(std::function<void()> task);

    /**
     * @brief Wait for all pending tasks to complete
     */
    void waitAll();

    /**
     * @brief Stop the thread pool gracefully
     * @param waitForTasks If true, wait for pending tasks to complete
     */
    void shutdown(bool waitForTasks = true);

    /**
     * @brief Check if thread pool is running
     */
    bool isRunning() const { return !stopped_.load(); }

    /**
     * @brief Get number of pending tasks
     */
    int getPendingTaskCount() const;

    /**
     * @brief Get number of worker threads
     */
    int getThreadCount() const { return static_cast<int>(workers_.size()); }

    /**
     * @brief Get thread pool metrics
     */
    const ThreadPoolMetrics& getMetrics() const { return metrics_; }

    /**
     * @brief Reset metrics
     */
    void resetMetrics() { metrics_.reset(); }

    /**
     * @brief Resize the thread pool
     * @param newThreadCount New number of threads
     */
    void resize(int newThreadCount);

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    mutable std::mutex queueMutex_;
    std::condition_variable condition_;
    std::condition_variable completionCondition_;

    std::atomic<bool> stopped_{false};
    std::atomic<int> pendingTasks_{0};

    ThreadPoolConfig config_;
    ThreadPoolMetrics metrics_;

    void workerThread();
    void createWorkers(int count);
};

// Template implementation
template<typename F, typename... Args>
auto ThreadPool::submit(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result<F, Args...>::type>
{
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> result = task->get_future();

    {
        std::unique_lock<std::mutex> lock(queueMutex_);

        if (stopped_) {
            throw std::runtime_error("Cannot submit to stopped thread pool");
        }

        if (config_.maxQueueSize > 0 &&
            static_cast<int>(tasks_.size()) >= config_.maxQueueSize) {
            throw std::runtime_error("Thread pool queue is full");
        }

        tasks_.emplace([task, this]() {
            (*task)();
        });

        ++pendingTasks_;
        metrics_.tasksSubmitted++;
        metrics_.currentQueueSize = static_cast<int>(tasks_.size());
    }

    condition_.notify_one();
    return result;
}

} // namespace Services
} // namespace FranchiseAI

#endif // THREAD_POOL_H
