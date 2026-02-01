#ifndef TEST_ORCHESTRATOR_H
#define TEST_ORCHESTRATOR_H

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <chrono>

namespace FranchiseAI {
namespace Testing {

/**
 * @brief Result of a single test execution
 */
struct TestResult {
    bool passed = false;
    std::string message;
    std::chrono::milliseconds duration{0};
    std::vector<std::string> logs;
};

/**
 * @brief Represents a single test case
 */
struct TestCase {
    std::string id;
    std::string name;
    std::string description;
    std::function<TestResult()> executor;
    bool enabled = true;
    TestResult lastResult;
    bool hasRun = false;
};

/**
 * @brief Represents a test suite containing multiple test cases
 */
struct TestSuite {
    std::string id;
    std::string name;
    std::string description;
    std::vector<TestCase> tests;
    bool enabled = true;
    bool expanded = false;  // For UI tree view

    int passedCount() const {
        int count = 0;
        for (const auto& test : tests) {
            if (test.hasRun && test.lastResult.passed) count++;
        }
        return count;
    }

    int failedCount() const {
        int count = 0;
        for (const auto& test : tests) {
            if (test.hasRun && !test.lastResult.passed) count++;
        }
        return count;
    }

    int totalCount() const {
        return static_cast<int>(tests.size());
    }

    int enabledCount() const {
        int count = 0;
        for (const auto& test : tests) {
            if (test.enabled) count++;
        }
        return count;
    }
};

/**
 * @brief Overall test run statistics
 */
struct TestRunStats {
    int totalTests = 0;
    int passedTests = 0;
    int failedTests = 0;
    int skippedTests = 0;
    std::chrono::milliseconds totalDuration{0};
    std::string startTime;
    std::string endTime;
};

/**
 * @brief Test orchestrator that manages test suites and execution
 */
class TestOrchestrator {
public:
    TestOrchestrator();
    ~TestOrchestrator();

    // Suite management
    void registerSuite(const TestSuite& suite);
    void registerSuite(const std::string& id, const std::string& name,
                       const std::string& description = "");

    // Test registration
    void registerTest(const std::string& suiteId, const TestCase& test);
    void registerTest(const std::string& suiteId, const std::string& testId,
                      const std::string& name, std::function<TestResult()> executor,
                      const std::string& description = "");

    // Enable/disable
    void enableSuite(const std::string& suiteId, bool enabled);
    void enableTest(const std::string& suiteId, const std::string& testId, bool enabled);
    void enableAll(bool enabled);
    void toggleSuite(const std::string& suiteId);
    void toggleTest(const std::string& suiteId, const std::string& testId);

    // Execution
    TestRunStats runAllEnabled();
    TestRunStats runSuite(const std::string& suiteId);
    TestResult runTest(const std::string& suiteId, const std::string& testId);

    // Accessors
    std::vector<TestSuite>& getSuites() { return suites_; }
    const std::vector<TestSuite>& getSuites() const { return suites_; }
    TestSuite* findSuite(const std::string& suiteId);
    TestCase* findTest(const std::string& suiteId, const std::string& testId);

    // Statistics
    TestRunStats getLastRunStats() const { return lastRunStats_; }

    // Progress callback
    using ProgressCallback = std::function<void(const std::string& suiteName,
                                                 const std::string& testName,
                                                 int current, int total)>;
    void setProgressCallback(ProgressCallback callback) { progressCallback_ = callback; }

private:
    std::vector<TestSuite> suites_;
    TestRunStats lastRunStats_;
    ProgressCallback progressCallback_;
};

/**
 * @brief Helper macros for test creation
 */
#define TEST_PASS(msg) TestResult{true, msg, std::chrono::milliseconds(0), {}}
#define TEST_FAIL(msg) TestResult{false, msg, std::chrono::milliseconds(0), {}}

} // namespace Testing
} // namespace FranchiseAI

#endif // TEST_ORCHESTRATOR_H
