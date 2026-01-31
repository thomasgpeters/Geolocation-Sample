#ifndef THREAD_POOL_SETTINGS_WIDGET_H
#define THREAD_POOL_SETTINGS_WIDGET_H

#include <Wt/WContainerWidget.h>
#include <Wt/WSlider.h>
#include <Wt/WText.h>
#include <Wt/WPushButton.h>
#include <Wt/WGroupBox.h>
#include <Wt/WProgressBar.h>
#include <memory>
#include <functional>
#include "../services/ThreadPool.h"

namespace FranchiseAI {
namespace Widgets {

/**
 * @brief Widget for configuring thread pool settings
 *
 * Provides a visual interface for adjusting thread pool size
 * with real-time memory recommendations and performance hints.
 */
class ThreadPoolSettingsWidget : public Wt::WContainerWidget {
public:
    using ThreadCountChangedCallback = std::function<void(int threadCount)>;

    ThreadPoolSettingsWidget();
    ~ThreadPoolSettingsWidget() override = default;

    /**
     * @brief Set the current thread count
     */
    void setThreadCount(int count);

    /**
     * @brief Get the current thread count
     */
    int getThreadCount() const;

    /**
     * @brief Set minimum thread count
     */
    void setMinThreads(int min);

    /**
     * @brief Set maximum thread count
     */
    void setMaxThreads(int max);

    /**
     * @brief Set callback for thread count changes
     */
    void setOnThreadCountChanged(ThreadCountChangedCallback callback);

    /**
     * @brief Update thread pool metrics display
     */
    void updateMetrics(const Services::ThreadPoolMetrics& metrics);

    /**
     * @brief Enable or disable the widget
     */
    void setEnabled(bool enabled);

    /**
     * @brief Show or hide advanced metrics
     */
    void showAdvancedMetrics(bool show);

private:
    void setupUI();
    void updateRecommendations();
    void onSliderChanged();

    int minThreads_ = 1;
    int maxThreads_ = 16;
    int currentThreads_ = 4;

    ThreadCountChangedCallback onThreadCountChanged_;

    // UI components
    Wt::WSlider* threadSlider_ = nullptr;
    Wt::WText* threadCountText_ = nullptr;
    Wt::WText* memoryRecommendationText_ = nullptr;
    Wt::WText* performanceDescriptionText_ = nullptr;
    Wt::WText* optimalThreadsText_ = nullptr;

    // Advanced metrics
    Wt::WContainerWidget* advancedMetricsContainer_ = nullptr;
    Wt::WText* tasksCompletedText_ = nullptr;
    Wt::WText* tasksFailedText_ = nullptr;
    Wt::WText* avgProcessingTimeText_ = nullptr;
    Wt::WText* throughputText_ = nullptr;
    Wt::WProgressBar* queueUtilizationBar_ = nullptr;

    // Memory gauge
    Wt::WContainerWidget* memoryGaugeContainer_ = nullptr;
};

/**
 * @brief Preset configurations for common use cases
 */
struct ThreadPoolPreset {
    std::string name;
    int threadCount;
    std::string description;

    static std::vector<ThreadPoolPreset> getPresets() {
        return {
            {"Low Memory", 2, "Minimal resource usage - suitable for constrained environments"},
            {"Balanced", 4, "Good balance of speed and memory - recommended for most users"},
            {"Performance", 8, "Fast geocoding - requires more memory"},
            {"Maximum", 12, "Fastest geocoding - for high-memory systems"}
        };
    }
};

} // namespace Widgets
} // namespace FranchiseAI

#endif // THREAD_POOL_SETTINGS_WIDGET_H
