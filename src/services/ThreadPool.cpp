#include "ThreadPool.h"
#include <chrono>

namespace FranchiseAI {
namespace Services {

ThreadPool::ThreadPool(int threadCount) {
    config_.threadCount = std::max(1, threadCount);
    createWorkers(config_.threadCount);
}

ThreadPool::ThreadPool(const ThreadPoolConfig& config)
    : config_(config) {
    config_.threadCount = std::max(1, config_.threadCount);
    createWorkers(config_.threadCount);
}

ThreadPool::~ThreadPool() {
    shutdown(true);
}

void ThreadPool::createWorkers(int count) {
    workers_.reserve(count);
    for (int i = 0; i < count; ++i) {
        workers_.emplace_back(&ThreadPool::workerThread, this);
    }
}

void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queueMutex_);

            condition_.wait(lock, [this] {
                return stopped_ || !tasks_.empty();
            });

            if (stopped_ && tasks_.empty()) {
                return;
            }

            if (!tasks_.empty()) {
                task = std::move(tasks_.front());
                tasks_.pop();
                metrics_.currentQueueSize = static_cast<int>(tasks_.size());
                metrics_.activeThreads++;
            }
        }

        if (task) {
            auto startTime = std::chrono::high_resolution_clock::now();

            try {
                task();
                metrics_.tasksCompleted++;
            } catch (...) {
                metrics_.tasksFailed++;
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                endTime - startTime).count();
            metrics_.totalProcessingTimeMs += duration;

            metrics_.activeThreads--;
            --pendingTasks_;

            completionCondition_.notify_all();
        }
    }
}

void ThreadPool::execute(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex_);

        if (stopped_) {
            throw std::runtime_error("Cannot execute on stopped thread pool");
        }

        if (config_.maxQueueSize > 0 &&
            static_cast<int>(tasks_.size()) >= config_.maxQueueSize) {
            throw std::runtime_error("Thread pool queue is full");
        }

        tasks_.emplace(std::move(task));
        ++pendingTasks_;
        metrics_.tasksSubmitted++;
        metrics_.currentQueueSize = static_cast<int>(tasks_.size());
    }

    condition_.notify_one();
}

void ThreadPool::waitAll() {
    std::unique_lock<std::mutex> lock(queueMutex_);
    completionCondition_.wait(lock, [this] {
        return pendingTasks_.load() == 0;
    });
}

void ThreadPool::shutdown(bool waitForTasks) {
    {
        std::unique_lock<std::mutex> lock(queueMutex_);

        if (stopped_) {
            return;
        }

        if (!waitForTasks) {
            // Clear pending tasks
            while (!tasks_.empty()) {
                tasks_.pop();
            }
            pendingTasks_ = 0;
        }

        stopped_ = true;
    }

    condition_.notify_all();

    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    workers_.clear();
}

int ThreadPool::getPendingTaskCount() const {
    return pendingTasks_.load();
}

void ThreadPool::resize(int newThreadCount) {
    newThreadCount = std::max(1, newThreadCount);

    if (newThreadCount == static_cast<int>(workers_.size())) {
        return;
    }

    // Shutdown existing pool
    shutdown(true);

    // Reset state
    stopped_ = false;
    pendingTasks_ = 0;
    metrics_.reset();

    // Create new workers
    config_.threadCount = newThreadCount;
    createWorkers(newThreadCount);
}

} // namespace Services
} // namespace FranchiseAI
