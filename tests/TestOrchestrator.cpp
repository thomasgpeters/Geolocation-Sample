#include "TestOrchestrator.h"
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace FranchiseAI {
namespace Testing {

TestOrchestrator::TestOrchestrator() {}

TestOrchestrator::~TestOrchestrator() {}

void TestOrchestrator::registerSuite(const TestSuite& suite) {
    suites_.push_back(suite);
}

void TestOrchestrator::registerSuite(const std::string& id, const std::string& name,
                                      const std::string& description) {
    TestSuite suite;
    suite.id = id;
    suite.name = name;
    suite.description = description;
    suite.enabled = true;
    suite.expanded = false;
    suites_.push_back(suite);
}

void TestOrchestrator::registerTest(const std::string& suiteId, const TestCase& test) {
    auto* suite = findSuite(suiteId);
    if (suite) {
        suite->tests.push_back(test);
    }
}

void TestOrchestrator::registerTest(const std::string& suiteId, const std::string& testId,
                                     const std::string& name, std::function<TestResult()> executor,
                                     const std::string& description) {
    TestCase test;
    test.id = testId;
    test.name = name;
    test.description = description;
    test.executor = executor;
    test.enabled = true;
    test.hasRun = false;
    registerTest(suiteId, test);
}

void TestOrchestrator::enableSuite(const std::string& suiteId, bool enabled) {
    auto* suite = findSuite(suiteId);
    if (suite) {
        suite->enabled = enabled;
        for (auto& test : suite->tests) {
            test.enabled = enabled;
        }
    }
}

void TestOrchestrator::enableTest(const std::string& suiteId, const std::string& testId, bool enabled) {
    auto* test = findTest(suiteId, testId);
    if (test) {
        test->enabled = enabled;
    }
}

void TestOrchestrator::enableAll(bool enabled) {
    for (auto& suite : suites_) {
        suite.enabled = enabled;
        for (auto& test : suite.tests) {
            test.enabled = enabled;
        }
    }
}

void TestOrchestrator::toggleSuite(const std::string& suiteId) {
    auto* suite = findSuite(suiteId);
    if (suite) {
        enableSuite(suiteId, !suite->enabled);
    }
}

void TestOrchestrator::toggleTest(const std::string& suiteId, const std::string& testId) {
    auto* test = findTest(suiteId, testId);
    if (test) {
        test->enabled = !test->enabled;
    }
}

TestRunStats TestOrchestrator::runAllEnabled() {
    TestRunStats stats;
    auto startTime = std::chrono::steady_clock::now();

    // Get current time as string
    auto now = std::time(nullptr);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
    stats.startTime = oss.str();

    // Count total enabled tests
    for (const auto& suite : suites_) {
        if (suite.enabled) {
            for (const auto& test : suite.tests) {
                if (test.enabled) {
                    stats.totalTests++;
                }
            }
        }
    }

    int currentTest = 0;

    // Run all enabled tests
    for (auto& suite : suites_) {
        if (!suite.enabled) continue;

        for (auto& test : suite.tests) {
            if (!test.enabled) {
                stats.skippedTests++;
                continue;
            }

            currentTest++;
            if (progressCallback_) {
                progressCallback_(suite.name, test.name, currentTest, stats.totalTests);
            }

            auto testStart = std::chrono::steady_clock::now();

            try {
                test.lastResult = test.executor();
            } catch (const std::exception& e) {
                test.lastResult.passed = false;
                test.lastResult.message = std::string("Exception: ") + e.what();
            } catch (...) {
                test.lastResult.passed = false;
                test.lastResult.message = "Unknown exception";
            }

            auto testEnd = std::chrono::steady_clock::now();
            test.lastResult.duration = std::chrono::duration_cast<std::chrono::milliseconds>(testEnd - testStart);
            test.hasRun = true;

            if (test.lastResult.passed) {
                stats.passedTests++;
            } else {
                stats.failedTests++;
            }
        }
    }

    auto endTime = std::chrono::steady_clock::now();
    stats.totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    now = std::time(nullptr);
    oss.str("");
    oss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
    stats.endTime = oss.str();

    lastRunStats_ = stats;
    return stats;
}

TestRunStats TestOrchestrator::runSuite(const std::string& suiteId) {
    TestRunStats stats;
    auto* suite = findSuite(suiteId);
    if (!suite) return stats;

    auto startTime = std::chrono::steady_clock::now();

    for (auto& test : suite->tests) {
        if (!test.enabled) {
            stats.skippedTests++;
            continue;
        }

        stats.totalTests++;

        if (progressCallback_) {
            progressCallback_(suite->name, test.name, stats.totalTests, static_cast<int>(suite->tests.size()));
        }

        auto testStart = std::chrono::steady_clock::now();

        try {
            test.lastResult = test.executor();
        } catch (const std::exception& e) {
            test.lastResult.passed = false;
            test.lastResult.message = std::string("Exception: ") + e.what();
        } catch (...) {
            test.lastResult.passed = false;
            test.lastResult.message = "Unknown exception";
        }

        auto testEnd = std::chrono::steady_clock::now();
        test.lastResult.duration = std::chrono::duration_cast<std::chrono::milliseconds>(testEnd - testStart);
        test.hasRun = true;

        if (test.lastResult.passed) {
            stats.passedTests++;
        } else {
            stats.failedTests++;
        }
    }

    auto endTime = std::chrono::steady_clock::now();
    stats.totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    return stats;
}

TestResult TestOrchestrator::runTest(const std::string& suiteId, const std::string& testId) {
    auto* test = findTest(suiteId, testId);
    if (!test) {
        return TEST_FAIL("Test not found");
    }

    auto testStart = std::chrono::steady_clock::now();

    try {
        test->lastResult = test->executor();
    } catch (const std::exception& e) {
        test->lastResult.passed = false;
        test->lastResult.message = std::string("Exception: ") + e.what();
    } catch (...) {
        test->lastResult.passed = false;
        test->lastResult.message = "Unknown exception";
    }

    auto testEnd = std::chrono::steady_clock::now();
    test->lastResult.duration = std::chrono::duration_cast<std::chrono::milliseconds>(testEnd - testStart);
    test->hasRun = true;

    return test->lastResult;
}

TestSuite* TestOrchestrator::findSuite(const std::string& suiteId) {
    for (auto& suite : suites_) {
        if (suite.id == suiteId) {
            return &suite;
        }
    }
    return nullptr;
}

TestCase* TestOrchestrator::findTest(const std::string& suiteId, const std::string& testId) {
    auto* suite = findSuite(suiteId);
    if (!suite) return nullptr;

    for (auto& test : suite->tests) {
        if (test.id == testId) {
            return &test;
        }
    }
    return nullptr;
}

} // namespace Testing
} // namespace FranchiseAI
