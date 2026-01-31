#include "ThreadPoolSettingsWidget.h"
#include <Wt/WVBoxLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WBreak.h>
#include <Wt/WLabel.h>
#include <sstream>
#include <iomanip>

namespace FranchiseAI {
namespace Widgets {

ThreadPoolSettingsWidget::ThreadPoolSettingsWidget() {
    setupUI();
}

void ThreadPoolSettingsWidget::setupUI() {
    addStyleClass("thread-pool-settings");

    // Main container with vertical layout
    auto mainLayout = setLayout(std::make_unique<Wt::WVBoxLayout>());
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Title
    auto title = mainLayout->addWidget(std::make_unique<Wt::WText>(
        "<h4>Thread Pool Configuration</h4>"
    ));
    title->setTextFormat(Wt::TextFormat::XHTML);

    // Description
    auto desc = mainLayout->addWidget(std::make_unique<Wt::WText>(
        "Adjust the number of background threads for geocoding operations. "
        "More threads improve performance but require more memory."
    ));
    desc->addStyleClass("text-muted");

    mainLayout->addWidget(std::make_unique<Wt::WBreak>());

    // Thread count slider section
    auto sliderContainer = mainLayout->addWidget(std::make_unique<Wt::WContainerWidget>());
    auto sliderLayout = sliderContainer->setLayout(std::make_unique<Wt::WVBoxLayout>());

    // Thread count label and value
    auto labelRow = sliderLayout->addWidget(std::make_unique<Wt::WContainerWidget>());
    auto labelLayout = labelRow->setLayout(std::make_unique<Wt::WHBoxLayout>());
    labelLayout->setContentsMargins(0, 0, 0, 0);

    auto label = labelLayout->addWidget(std::make_unique<Wt::WText>("Thread Count:"));
    label->addStyleClass("form-label");

    threadCountText_ = labelLayout->addWidget(std::make_unique<Wt::WText>());
    threadCountText_->addStyleClass("badge bg-primary");

    labelLayout->addStretch(1);

    // Optimal threads hint
    optimalThreadsText_ = labelLayout->addWidget(std::make_unique<Wt::WText>());
    optimalThreadsText_->addStyleClass("text-info small");

    // Slider
    threadSlider_ = sliderLayout->addWidget(std::make_unique<Wt::WSlider>());
    threadSlider_->setMinimum(minThreads_);
    threadSlider_->setMaximum(maxThreads_);
    threadSlider_->setValue(currentThreads_);
    threadSlider_->setTickPosition(Wt::WSlider::TickPosition::TicksAbove);
    threadSlider_->setTickInterval(1);
    threadSlider_->resize(Wt::WLength(100, Wt::LengthUnit::Percentage), 40);

    threadSlider_->valueChanged().connect([this]() {
        onSliderChanged();
    });

    // Scale labels
    auto scaleRow = sliderLayout->addWidget(std::make_unique<Wt::WContainerWidget>());
    auto scaleLayout = scaleRow->setLayout(std::make_unique<Wt::WHBoxLayout>());
    scaleLayout->setContentsMargins(0, 0, 0, 0);

    auto minLabel = scaleLayout->addWidget(std::make_unique<Wt::WText>(
        std::to_string(minThreads_) + " thread"
    ));
    minLabel->addStyleClass("small text-muted");

    scaleLayout->addStretch(1);

    auto maxLabel = scaleLayout->addWidget(std::make_unique<Wt::WText>(
        std::to_string(maxThreads_) + " threads"
    ));
    maxLabel->addStyleClass("small text-muted");

    mainLayout->addWidget(std::make_unique<Wt::WBreak>());

    // Recommendations section
    auto recommendationsBox = mainLayout->addWidget(std::make_unique<Wt::WGroupBox>("Recommendations"));
    recommendationsBox->addStyleClass("mb-3");

    auto recLayout = recommendationsBox->setLayout(std::make_unique<Wt::WVBoxLayout>());

    // Memory recommendation
    auto memRow = recLayout->addWidget(std::make_unique<Wt::WContainerWidget>());
    auto memLayout = memRow->setLayout(std::make_unique<Wt::WHBoxLayout>());
    memLayout->setContentsMargins(0, 0, 0, 0);

    auto memIcon = memLayout->addWidget(std::make_unique<Wt::WText>("💾 "));
    memoryRecommendationText_ = memLayout->addWidget(std::make_unique<Wt::WText>());
    memoryRecommendationText_->addStyleClass("fw-bold");

    // Performance description
    performanceDescriptionText_ = recLayout->addWidget(std::make_unique<Wt::WText>());
    performanceDescriptionText_->addStyleClass("text-muted");

    // Memory gauge visualization
    memoryGaugeContainer_ = recLayout->addWidget(std::make_unique<Wt::WContainerWidget>());
    memoryGaugeContainer_->addStyleClass("mt-2");
    auto gaugeLayout = memoryGaugeContainer_->setLayout(std::make_unique<Wt::WHBoxLayout>());
    gaugeLayout->setContentsMargins(0, 0, 0, 0);

    auto gaugeLabel = gaugeLayout->addWidget(std::make_unique<Wt::WText>("Memory Usage: "));
    gaugeLabel->addStyleClass("small");

    auto gaugeBar = gaugeLayout->addWidget(std::make_unique<Wt::WProgressBar>());
    gaugeBar->setRange(0, 100);
    gaugeBar->setValue(25);  // Will be updated dynamically
    gaugeBar->setFormat("");
    gaugeBar->resize(Wt::WLength(150, Wt::LengthUnit::Pixel), 10);

    mainLayout->addWidget(std::make_unique<Wt::WBreak>());

    // Advanced metrics (hidden by default)
    advancedMetricsContainer_ = mainLayout->addWidget(std::make_unique<Wt::WContainerWidget>());
    advancedMetricsContainer_->hide();

    auto metricsBox = advancedMetricsContainer_->addWidget(
        std::make_unique<Wt::WGroupBox>("Thread Pool Metrics")
    );
    auto metricsLayout = metricsBox->setLayout(std::make_unique<Wt::WVBoxLayout>());

    // Tasks completed
    auto tasksRow = metricsLayout->addWidget(std::make_unique<Wt::WContainerWidget>());
    auto tasksLayout = tasksRow->setLayout(std::make_unique<Wt::WHBoxLayout>());
    tasksLayout->addWidget(std::make_unique<Wt::WText>("Tasks Completed: "));
    tasksCompletedText_ = tasksLayout->addWidget(std::make_unique<Wt::WText>("0"));
    tasksLayout->addStretch(1);
    tasksLayout->addWidget(std::make_unique<Wt::WText>("Failed: "));
    tasksFailedText_ = tasksLayout->addWidget(std::make_unique<Wt::WText>("0"));

    // Processing time
    auto timeRow = metricsLayout->addWidget(std::make_unique<Wt::WContainerWidget>());
    auto timeLayout = timeRow->setLayout(std::make_unique<Wt::WHBoxLayout>());
    timeLayout->addWidget(std::make_unique<Wt::WText>("Avg Processing Time: "));
    avgProcessingTimeText_ = timeLayout->addWidget(std::make_unique<Wt::WText>("0 ms"));
    timeLayout->addStretch(1);
    timeLayout->addWidget(std::make_unique<Wt::WText>("Throughput: "));
    throughputText_ = timeLayout->addWidget(std::make_unique<Wt::WText>("0/sec"));

    // Queue utilization
    auto queueRow = metricsLayout->addWidget(std::make_unique<Wt::WContainerWidget>());
    auto queueLayout = queueRow->setLayout(std::make_unique<Wt::WHBoxLayout>());
    queueLayout->addWidget(std::make_unique<Wt::WText>("Queue Utilization: "));
    queueUtilizationBar_ = queueLayout->addWidget(std::make_unique<Wt::WProgressBar>());
    queueUtilizationBar_->setRange(0, 100);
    queueUtilizationBar_->setValue(0);
    queueUtilizationBar_->resize(Wt::WLength(200, Wt::LengthUnit::Pixel), 15);

    // Presets buttons
    mainLayout->addWidget(std::make_unique<Wt::WBreak>());

    auto presetsContainer = mainLayout->addWidget(std::make_unique<Wt::WContainerWidget>());
    auto presetsLayout = presetsContainer->setLayout(std::make_unique<Wt::WHBoxLayout>());
    presetsLayout->setContentsMargins(0, 0, 0, 0);

    presetsLayout->addWidget(std::make_unique<Wt::WText>("Quick Presets: "));

    for (const auto& preset : ThreadPoolPreset::getPresets()) {
        auto btn = presetsLayout->addWidget(std::make_unique<Wt::WPushButton>(preset.name));
        btn->addStyleClass("btn-sm btn-outline-secondary me-1");
        btn->setToolTip(preset.description);

        int threadCount = preset.threadCount;
        btn->clicked().connect([this, threadCount]() {
            setThreadCount(threadCount);
        });
    }

    presetsLayout->addStretch(1);

    // Initialize display
    updateRecommendations();

    // Set optimal threads hint
    int optimal = Services::ThreadPoolConfig::getOptimalThreadCount();
    optimalThreadsText_->setText("(Optimal for this system: " + std::to_string(optimal) + ")");
}

void ThreadPoolSettingsWidget::setThreadCount(int count) {
    currentThreads_ = std::max(minThreads_, std::min(maxThreads_, count));
    threadSlider_->setValue(currentThreads_);
    updateRecommendations();

    if (onThreadCountChanged_) {
        onThreadCountChanged_(currentThreads_);
    }
}

int ThreadPoolSettingsWidget::getThreadCount() const {
    return currentThreads_;
}

void ThreadPoolSettingsWidget::setMinThreads(int min) {
    minThreads_ = std::max(1, min);
    threadSlider_->setMinimum(minThreads_);
}

void ThreadPoolSettingsWidget::setMaxThreads(int max) {
    maxThreads_ = std::max(minThreads_, max);
    threadSlider_->setMaximum(maxThreads_);
}

void ThreadPoolSettingsWidget::setOnThreadCountChanged(ThreadCountChangedCallback callback) {
    onThreadCountChanged_ = callback;
}

void ThreadPoolSettingsWidget::onSliderChanged() {
    currentThreads_ = threadSlider_->value();
    updateRecommendations();

    if (onThreadCountChanged_) {
        onThreadCountChanged_(currentThreads_);
    }
}

void ThreadPoolSettingsWidget::updateRecommendations() {
    // Update thread count display
    threadCountText_->setText(" " + std::to_string(currentThreads_) + " ");

    // Get memory recommendation
    int memoryMB = Services::ThreadPoolConfig::getRecommendedMemoryMB(currentThreads_);
    memoryRecommendationText_->setText("Recommended RAM: " + std::to_string(memoryMB) + " MB");

    // Get performance description
    std::string description = Services::ThreadPoolConfig::getThreadCountDescription(currentThreads_);
    performanceDescriptionText_->setText(description);

    // Update memory gauge (assuming 128MB max for visualization)
    if (memoryGaugeContainer_->count() > 1) {
        auto gaugeBar = dynamic_cast<Wt::WProgressBar*>(
            memoryGaugeContainer_->widget(1));
        if (gaugeBar) {
            int percentage = std::min(100, (memoryMB * 100) / 128);
            gaugeBar->setValue(percentage);

            // Color based on memory usage
            gaugeBar->removeStyleClass("bg-success");
            gaugeBar->removeStyleClass("bg-warning");
            gaugeBar->removeStyleClass("bg-danger");

            if (percentage < 40) {
                gaugeBar->addStyleClass("bg-success");
            } else if (percentage < 70) {
                gaugeBar->addStyleClass("bg-warning");
            } else {
                gaugeBar->addStyleClass("bg-danger");
            }
        }
    }
}

void ThreadPoolSettingsWidget::updateMetrics(const Services::ThreadPoolMetrics& metrics) {
    if (!advancedMetricsContainer_->isVisible()) {
        return;
    }

    tasksCompletedText_->setText(std::to_string(metrics.tasksCompleted.load()));
    tasksFailedText_->setText(std::to_string(metrics.tasksFailed.load()));

    std::ostringstream avgTime;
    avgTime << std::fixed << std::setprecision(1) << metrics.getAverageProcessingTimeMs() << " ms";
    avgProcessingTimeText_->setText(avgTime.str());

    std::ostringstream throughput;
    throughput << std::fixed << std::setprecision(1) << metrics.getThroughputPerSecond() << "/sec";
    throughputText_->setText(throughput.str());

    // Queue utilization (assuming max 100 queue size)
    int queuePercent = std::min(100, metrics.currentQueueSize.load());
    queueUtilizationBar_->setValue(queuePercent);
}

void ThreadPoolSettingsWidget::setEnabled(bool enabled) {
    threadSlider_->setEnabled(enabled);
}

void ThreadPoolSettingsWidget::showAdvancedMetrics(bool show) {
    if (show) {
        advancedMetricsContainer_->show();
    } else {
        advancedMetricsContainer_->hide();
    }
}

} // namespace Widgets
} // namespace FranchiseAI
