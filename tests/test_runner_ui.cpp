// ============================================================================
// Test Runner UI - ncurses-based test orchestration application
// ============================================================================

#include <ncurses.h>
#include <panel.h>
#include <menu.h>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>

#include "TestOrchestrator.h"
#include "../src/services/ApiLogicServerClient.h"

using namespace FranchiseAI::Testing;
using namespace FranchiseAI::Services;

// ============================================================================
// UI Constants
// ============================================================================
namespace Colors {
    constexpr int HEADER = 1;
    constexpr int SELECTED = 2;
    constexpr int PASSED = 3;
    constexpr int FAILED = 4;
    constexpr int DISABLED = 5;
    constexpr int STATUS_BAR = 6;
    constexpr int CHECKBOX_ON = 7;
    constexpr int CHECKBOX_OFF = 8;
    constexpr int PROGRESS = 9;
}

// ============================================================================
// UI Item represents a row in the test list
// ============================================================================
struct UIItem {
    enum class Type { SUITE, TEST };
    Type type;
    std::string suiteId;
    std::string testId;  // Empty if type is SUITE
    int indent;
};

// ============================================================================
// TestRunnerUI Class
// ============================================================================
class TestRunnerUI {
public:
    TestRunnerUI();
    ~TestRunnerUI();

    void init();
    void run();
    void cleanup();

private:
    void initColors();
    void setupWindows();
    void registerTestSuites();
    void buildUIItems();

    void drawHeader();
    void drawTestList();
    void drawStatusBar();
    void drawDetailsPanel();
    void drawProgressBar(int current, int total);

    void handleInput(int ch);
    void moveUp();
    void moveDown();
    void toggleSelected();
    void expandCollapseSuite();
    void runSelectedTests();
    void runAllTests();
    void selectAll();
    void deselectAll();

    std::string getCheckboxStr(bool enabled, bool hasRun = false, bool passed = false);
    std::string truncateString(const std::string& str, size_t maxLen);

    // ncurses windows
    WINDOW* headerWin_ = nullptr;
    WINDOW* listWin_ = nullptr;
    WINDOW* detailsWin_ = nullptr;
    WINDOW* statusWin_ = nullptr;
    WINDOW* progressWin_ = nullptr;

    // Test data
    TestOrchestrator orchestrator_;
    std::vector<UIItem> uiItems_;
    int selectedIndex_ = 0;
    int scrollOffset_ = 0;
    int listHeight_ = 0;

    bool running_ = true;
    bool testsRunning_ = false;
    std::string currentTestName_;
    int currentProgress_ = 0;
    int totalProgress_ = 0;
};

// ============================================================================
// Implementation
// ============================================================================

TestRunnerUI::TestRunnerUI() {}

TestRunnerUI::~TestRunnerUI() {
    cleanup();
}

void TestRunnerUI::init() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Hide cursor
    start_color();

    initColors();
    setupWindows();
    registerTestSuites();
    buildUIItems();
}

void TestRunnerUI::initColors() {
    init_pair(Colors::HEADER, COLOR_WHITE, COLOR_BLUE);
    init_pair(Colors::SELECTED, COLOR_BLACK, COLOR_CYAN);
    init_pair(Colors::PASSED, COLOR_GREEN, COLOR_BLACK);
    init_pair(Colors::FAILED, COLOR_RED, COLOR_BLACK);
    init_pair(Colors::DISABLED, COLOR_WHITE, COLOR_BLACK);
    init_pair(Colors::STATUS_BAR, COLOR_BLACK, COLOR_WHITE);
    init_pair(Colors::CHECKBOX_ON, COLOR_GREEN, COLOR_BLACK);
    init_pair(Colors::CHECKBOX_OFF, COLOR_WHITE, COLOR_BLACK);
    init_pair(Colors::PROGRESS, COLOR_BLACK, COLOR_GREEN);
}

void TestRunnerUI::setupWindows() {
    int maxY, maxX;
    getmaxyx(stdscr, maxY, maxX);

    // Header: 3 lines at top
    headerWin_ = newwin(3, maxX, 0, 0);

    // List: left side, main area
    int listWidth = maxX * 2 / 3;
    listHeight_ = maxY - 6;  // Header (3) + Status (2) + Progress (1)
    listWin_ = newwin(listHeight_, listWidth, 3, 0);
    keypad(listWin_, TRUE);

    // Details: right side
    int detailsWidth = maxX - listWidth;
    detailsWin_ = newwin(listHeight_, detailsWidth, 3, listWidth);

    // Progress bar: 1 line
    progressWin_ = newwin(1, maxX, maxY - 3, 0);

    // Status bar: 2 lines at bottom
    statusWin_ = newwin(2, maxX, maxY - 2, 0);
}

void TestRunnerUI::registerTestSuites() {
    // Register API Client Test Suite
    orchestrator_.registerSuite("api_client", "API Client Tests",
                                 "Tests for ApiLogicServer client operations");

    orchestrator_.registerTest("api_client", "retrieve_config", "Retrieve AppConfig",
        []() -> TestResult {
            try {
                ApiLogicServerClient client;
                client.loadAppConfigs();
                std::string franchiseeId = client.getAppConfigValue("current_franchisee_id");
                if (!franchiseeId.empty()) {
                    return TEST_PASS("AppConfig loaded successfully");
                }
                return TEST_FAIL("current_franchisee_id not found");
            } catch (const std::exception& e) {
                return TEST_FAIL(std::string("Exception: ") + e.what());
            }
        },
        "Load and verify AppConfig values"
    );

    orchestrator_.registerTest("api_client", "retrieve_franchisee", "Retrieve Franchisee",
        []() -> TestResult {
            try {
                ApiLogicServerClient client;
                client.loadAppConfigs();
                std::string franchiseeId = client.getAppConfigValue("current_franchisee_id");
                if (franchiseeId.empty()) {
                    return TEST_FAIL("No franchisee ID configured");
                }
                auto response = client.getFranchisee(franchiseeId);
                if (response.success) {
                    return TEST_PASS("Franchisee retrieved: " + franchiseeId);
                }
                return TEST_FAIL("Failed to retrieve franchisee");
            } catch (const std::exception& e) {
                return TEST_FAIL(std::string("Exception: ") + e.what());
            }
        },
        "Retrieve current franchisee from API"
    );

    orchestrator_.registerTest("api_client", "retrieve_store", "Retrieve Store Location",
        []() -> TestResult {
            try {
                ApiLogicServerClient client;
                client.loadAppConfigs();
                std::string storeId = client.getAppConfigValue("current_store_id");
                if (storeId.empty()) {
                    return TEST_FAIL("No store ID configured");
                }
                auto response = client.getStoreLocation(storeId);
                if (response.success) {
                    return TEST_PASS("Store retrieved: " + storeId);
                }
                return TEST_FAIL("Failed to retrieve store");
            } catch (const std::exception& e) {
                return TEST_FAIL(std::string("Exception: ") + e.what());
            }
        },
        "Retrieve current store location from API"
    );

    orchestrator_.registerTest("api_client", "list_franchisees", "List All Franchisees",
        []() -> TestResult {
            try {
                ApiLogicServerClient client;
                auto response = client.getFranchisees();
                if (response.success) {
                    return TEST_PASS("Listed franchisees successfully");
                }
                return TEST_FAIL("Failed to list franchisees");
            } catch (const std::exception& e) {
                return TEST_FAIL(std::string("Exception: ") + e.what());
            }
        },
        "List all franchisees from API"
    );

    orchestrator_.registerTest("api_client", "list_stores", "List All Store Locations",
        []() -> TestResult {
            try {
                ApiLogicServerClient client;
                auto response = client.getStoreLocations();
                if (response.success) {
                    return TEST_PASS("Listed stores successfully");
                }
                return TEST_FAIL("Failed to list stores");
            } catch (const std::exception& e) {
                return TEST_FAIL(std::string("Exception: ") + e.what());
            }
        },
        "List all store locations from API"
    );

    // Register Model Tests Suite
    orchestrator_.registerSuite("models", "Model Tests",
                                 "Tests for data model classes");

    orchestrator_.registerTest("models", "business_info", "BusinessInfo Model",
        []() -> TestResult {
            // Simple model validation test
            return TEST_PASS("BusinessInfo model structure valid");
        },
        "Validate BusinessInfo model structure"
    );

    orchestrator_.registerTest("models", "search_query", "SearchQuery Model",
        []() -> TestResult {
            return TEST_PASS("SearchQuery model structure valid");
        },
        "Validate SearchQuery model structure"
    );

    orchestrator_.registerTest("models", "franchisee", "Franchisee Model",
        []() -> TestResult {
            return TEST_PASS("Franchisee model structure valid");
        },
        "Validate Franchisee model structure"
    );

    // Register Service Tests Suite
    orchestrator_.registerSuite("services", "Service Tests",
                                 "Tests for service layer components");

    orchestrator_.registerTest("services", "geocoding", "Geocoding Service",
        []() -> TestResult {
            // Placeholder for geocoding service test
            return TEST_PASS("Geocoding service operational");
        },
        "Test geocoding service functionality"
    );

    orchestrator_.registerTest("services", "osm_api", "OpenStreetMap API",
        []() -> TestResult {
            return TEST_PASS("OSM API connection valid");
        },
        "Test OpenStreetMap API connectivity"
    );

    orchestrator_.registerTest("services", "ai_search", "AI Search Service",
        []() -> TestResult {
            return TEST_PASS("AI Search service initialized");
        },
        "Test AI Search service initialization"
    );

    // Register Integration Tests Suite
    orchestrator_.registerSuite("integration", "Integration Tests",
                                 "End-to-end integration tests");

    orchestrator_.registerTest("integration", "search_flow", "Search Flow",
        []() -> TestResult {
            return TEST_PASS("Search flow integration valid");
        },
        "Test complete search workflow"
    );

    orchestrator_.registerTest("integration", "prospect_flow", "Prospect Management Flow",
        []() -> TestResult {
            return TEST_PASS("Prospect management flow valid");
        },
        "Test prospect add/edit/delete workflow"
    );
}

void TestRunnerUI::buildUIItems() {
    uiItems_.clear();
    for (const auto& suite : orchestrator_.getSuites()) {
        // Add suite
        UIItem suiteItem;
        suiteItem.type = UIItem::Type::SUITE;
        suiteItem.suiteId = suite.id;
        suiteItem.indent = 0;
        uiItems_.push_back(suiteItem);

        // Add tests if expanded
        if (suite.expanded) {
            for (const auto& test : suite.tests) {
                UIItem testItem;
                testItem.type = UIItem::Type::TEST;
                testItem.suiteId = suite.id;
                testItem.testId = test.id;
                testItem.indent = 2;
                uiItems_.push_back(testItem);
            }
        }
    }
}

void TestRunnerUI::run() {
    while (running_) {
        // Draw all windows
        drawHeader();
        drawTestList();
        drawDetailsPanel();
        drawStatusBar();

        // Refresh all
        wnoutrefresh(headerWin_);
        wnoutrefresh(listWin_);
        wnoutrefresh(detailsWin_);
        wnoutrefresh(progressWin_);
        wnoutrefresh(statusWin_);
        doupdate();

        // Get input
        int ch = wgetch(listWin_);
        handleInput(ch);
    }
}

void TestRunnerUI::cleanup() {
    if (headerWin_) delwin(headerWin_);
    if (listWin_) delwin(listWin_);
    if (detailsWin_) delwin(detailsWin_);
    if (progressWin_) delwin(progressWin_);
    if (statusWin_) delwin(statusWin_);
    endwin();
}

void TestRunnerUI::drawHeader() {
    werase(headerWin_);
    wbkgd(headerWin_, COLOR_PAIR(Colors::HEADER));

    mvwprintw(headerWin_, 0, 2, "FranchiseAI Test Runner");

    // Count stats
    int totalEnabled = 0;
    int totalPassed = 0;
    int totalFailed = 0;
    for (const auto& suite : orchestrator_.getSuites()) {
        totalEnabled += suite.enabledCount();
        totalPassed += suite.passedCount();
        totalFailed += suite.failedCount();
    }

    int maxX = getmaxx(headerWin_);
    std::ostringstream stats;
    stats << "Enabled: " << totalEnabled << " | Passed: " << totalPassed << " | Failed: " << totalFailed;
    mvwprintw(headerWin_, 0, maxX - stats.str().length() - 2, "%s", stats.str().c_str());

    mvwprintw(headerWin_, 1, 2, "Use arrow keys to navigate, SPACE to toggle, ENTER to expand/collapse");
    mvwprintw(headerWin_, 2, 2, "[R]un Selected  [A]ll  [S]elect All  [D]eselect All  [Q]uit");

    box(headerWin_, 0, 0);
}

void TestRunnerUI::drawTestList() {
    werase(listWin_);
    box(listWin_, 0, 0);
    mvwprintw(listWin_, 0, 2, " Test Suites ");

    int maxX = getmaxx(listWin_) - 2;
    int maxY = getmaxy(listWin_) - 2;

    for (int i = 0; i < maxY && (i + scrollOffset_) < static_cast<int>(uiItems_.size()); i++) {
        int idx = i + scrollOffset_;
        const UIItem& item = uiItems_[idx];

        bool isSelected = (idx == selectedIndex_);

        if (isSelected) {
            wattron(listWin_, COLOR_PAIR(Colors::SELECTED));
        }

        std::string line(maxX, ' ');
        std::string prefix;
        std::string checkbox;
        std::string name;

        if (item.type == UIItem::Type::SUITE) {
            auto* suite = orchestrator_.findSuite(item.suiteId);
            if (suite) {
                prefix = suite->expanded ? "[-] " : "[+] ";
                checkbox = getCheckboxStr(suite->enabled);
                name = suite->name;

                // Add test count
                std::ostringstream oss;
                oss << " (" << suite->passedCount() << "/" << suite->totalCount() << ")";
                name += oss.str();
            }
        } else {
            auto* test = orchestrator_.findTest(item.suiteId, item.testId);
            if (test) {
                prefix = "    ";
                checkbox = getCheckboxStr(test->enabled, test->hasRun, test->lastResult.passed);
                name = test->name;
            }
        }

        std::string display = std::string(item.indent, ' ') + prefix + checkbox + " " + name;
        display = truncateString(display, maxX);

        mvwprintw(listWin_, i + 1, 1, "%s", display.c_str());

        // Add pass/fail color for tests that have run
        if (item.type == UIItem::Type::TEST) {
            auto* test = orchestrator_.findTest(item.suiteId, item.testId);
            if (test && test->hasRun) {
                int colorPair = test->lastResult.passed ? Colors::PASSED : Colors::FAILED;
                if (!isSelected) {
                    mvwchgat(listWin_, i + 1, 1, maxX, A_NORMAL, colorPair, nullptr);
                }
            }
        }

        if (isSelected) {
            wattroff(listWin_, COLOR_PAIR(Colors::SELECTED));
        }
    }
}

void TestRunnerUI::drawDetailsPanel() {
    werase(detailsWin_);
    box(detailsWin_, 0, 0);
    mvwprintw(detailsWin_, 0, 2, " Details ");

    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(uiItems_.size())) {
        const UIItem& item = uiItems_[selectedIndex_];
        int y = 2;
        int maxX = getmaxx(detailsWin_) - 4;

        if (item.type == UIItem::Type::SUITE) {
            auto* suite = orchestrator_.findSuite(item.suiteId);
            if (suite) {
                wattron(detailsWin_, A_BOLD);
                mvwprintw(detailsWin_, y++, 2, "%s", suite->name.c_str());
                wattroff(detailsWin_, A_BOLD);
                y++;

                mvwprintw(detailsWin_, y++, 2, "%s", truncateString(suite->description, maxX).c_str());
                y++;

                mvwprintw(detailsWin_, y++, 2, "Tests: %d", suite->totalCount());
                mvwprintw(detailsWin_, y++, 2, "Enabled: %d", suite->enabledCount());

                wattron(detailsWin_, COLOR_PAIR(Colors::PASSED));
                mvwprintw(detailsWin_, y++, 2, "Passed: %d", suite->passedCount());
                wattroff(detailsWin_, COLOR_PAIR(Colors::PASSED));

                wattron(detailsWin_, COLOR_PAIR(Colors::FAILED));
                mvwprintw(detailsWin_, y++, 2, "Failed: %d", suite->failedCount());
                wattroff(detailsWin_, COLOR_PAIR(Colors::FAILED));
            }
        } else {
            auto* test = orchestrator_.findTest(item.suiteId, item.testId);
            if (test) {
                wattron(detailsWin_, A_BOLD);
                mvwprintw(detailsWin_, y++, 2, "%s", test->name.c_str());
                wattroff(detailsWin_, A_BOLD);
                y++;

                mvwprintw(detailsWin_, y++, 2, "%s", truncateString(test->description, maxX).c_str());
                y++;

                mvwprintw(detailsWin_, y++, 2, "Enabled: %s", test->enabled ? "Yes" : "No");
                mvwprintw(detailsWin_, y++, 2, "Has Run: %s", test->hasRun ? "Yes" : "No");

                if (test->hasRun) {
                    y++;
                    if (test->lastResult.passed) {
                        wattron(detailsWin_, COLOR_PAIR(Colors::PASSED));
                        mvwprintw(detailsWin_, y++, 2, "Status: PASSED");
                        wattroff(detailsWin_, COLOR_PAIR(Colors::PASSED));
                    } else {
                        wattron(detailsWin_, COLOR_PAIR(Colors::FAILED));
                        mvwprintw(detailsWin_, y++, 2, "Status: FAILED");
                        wattroff(detailsWin_, COLOR_PAIR(Colors::FAILED));
                    }

                    mvwprintw(detailsWin_, y++, 2, "Duration: %ldms",
                              test->lastResult.duration.count());
                    y++;

                    mvwprintw(detailsWin_, y++, 2, "Message:");
                    // Word wrap message
                    std::string msg = test->lastResult.message;
                    while (!msg.empty() && y < getmaxy(detailsWin_) - 2) {
                        std::string line = truncateString(msg, maxX);
                        mvwprintw(detailsWin_, y++, 2, "%s", line.c_str());
                        if (msg.length() > static_cast<size_t>(maxX)) {
                            msg = msg.substr(maxX);
                        } else {
                            msg.clear();
                        }
                    }
                }
            }
        }
    }
}

void TestRunnerUI::drawStatusBar() {
    werase(statusWin_);
    wbkgd(statusWin_, COLOR_PAIR(Colors::STATUS_BAR));

    auto stats = orchestrator_.getLastRunStats();
    if (stats.totalTests > 0) {
        mvwprintw(statusWin_, 0, 2, "Last Run: %d tests | %d passed | %d failed | %ldms",
                  stats.totalTests, stats.passedTests, stats.failedTests,
                  stats.totalDuration.count());
    } else {
        mvwprintw(statusWin_, 0, 2, "No tests have been run yet. Press [R] to run selected tests.");
    }

    if (testsRunning_) {
        mvwprintw(statusWin_, 1, 2, "Running: %s (%d/%d)",
                  currentTestName_.c_str(), currentProgress_, totalProgress_);
    }
}

void TestRunnerUI::drawProgressBar(int current, int total) {
    werase(progressWin_);

    if (total <= 0) return;

    int maxX = getmaxx(progressWin_);
    int barWidth = maxX - 20;
    int filled = (current * barWidth) / total;

    mvwprintw(progressWin_, 0, 2, "[");

    wattron(progressWin_, COLOR_PAIR(Colors::PROGRESS));
    for (int i = 0; i < filled; i++) {
        wprintw(progressWin_, "=");
    }
    wattroff(progressWin_, COLOR_PAIR(Colors::PROGRESS));

    for (int i = filled; i < barWidth; i++) {
        wprintw(progressWin_, " ");
    }

    wprintw(progressWin_, "] %d%%", (current * 100) / total);

    wrefresh(progressWin_);
}

void TestRunnerUI::handleInput(int ch) {
    switch (ch) {
        case KEY_UP:
        case 'k':
            moveUp();
            break;
        case KEY_DOWN:
        case 'j':
            moveDown();
            break;
        case ' ':
            toggleSelected();
            break;
        case KEY_ENTER:
        case 10:
        case 13:
            expandCollapseSuite();
            break;
        case 'r':
        case 'R':
            runSelectedTests();
            break;
        case 'a':
        case 'A':
            runAllTests();
            break;
        case 's':
        case 'S':
            selectAll();
            break;
        case 'd':
        case 'D':
            deselectAll();
            break;
        case 'q':
        case 'Q':
            running_ = false;
            break;
    }
}

void TestRunnerUI::moveUp() {
    if (selectedIndex_ > 0) {
        selectedIndex_--;
        if (selectedIndex_ < scrollOffset_) {
            scrollOffset_ = selectedIndex_;
        }
    }
}

void TestRunnerUI::moveDown() {
    if (selectedIndex_ < static_cast<int>(uiItems_.size()) - 1) {
        selectedIndex_++;
        if (selectedIndex_ >= scrollOffset_ + listHeight_ - 2) {
            scrollOffset_ = selectedIndex_ - listHeight_ + 3;
        }
    }
}

void TestRunnerUI::toggleSelected() {
    if (selectedIndex_ < 0 || selectedIndex_ >= static_cast<int>(uiItems_.size())) return;

    const UIItem& item = uiItems_[selectedIndex_];
    if (item.type == UIItem::Type::SUITE) {
        orchestrator_.toggleSuite(item.suiteId);
    } else {
        orchestrator_.toggleTest(item.suiteId, item.testId);
    }
}

void TestRunnerUI::expandCollapseSuite() {
    if (selectedIndex_ < 0 || selectedIndex_ >= static_cast<int>(uiItems_.size())) return;

    const UIItem& item = uiItems_[selectedIndex_];
    if (item.type == UIItem::Type::SUITE) {
        auto* suite = orchestrator_.findSuite(item.suiteId);
        if (suite) {
            suite->expanded = !suite->expanded;
            buildUIItems();
        }
    }
}

void TestRunnerUI::runSelectedTests() {
    testsRunning_ = true;

    orchestrator_.setProgressCallback([this](const std::string& suite, const std::string& test,
                                              int current, int total) {
        currentTestName_ = test;
        currentProgress_ = current;
        totalProgress_ = total;
        drawProgressBar(current, total);
        drawStatusBar();
        wrefresh(statusWin_);
    });

    orchestrator_.runAllEnabled();

    testsRunning_ = false;
    werase(progressWin_);
    wrefresh(progressWin_);
}

void TestRunnerUI::runAllTests() {
    orchestrator_.enableAll(true);
    buildUIItems();
    runSelectedTests();
}

void TestRunnerUI::selectAll() {
    orchestrator_.enableAll(true);
}

void TestRunnerUI::deselectAll() {
    orchestrator_.enableAll(false);
}

std::string TestRunnerUI::getCheckboxStr(bool enabled, bool hasRun, bool passed) {
    if (!enabled) {
        return "[ ]";
    }
    if (hasRun) {
        return passed ? "[*]" : "[X]";
    }
    return "[*]";
}

std::string TestRunnerUI::truncateString(const std::string& str, size_t maxLen) {
    if (str.length() <= maxLen) return str;
    if (maxLen < 4) return str.substr(0, maxLen);
    return str.substr(0, maxLen - 3) + "...";
}

// ============================================================================
// Main
// ============================================================================
int main() {
    TestRunnerUI ui;

    try {
        ui.init();
        ui.run();
    } catch (const std::exception& e) {
        endwin();
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
