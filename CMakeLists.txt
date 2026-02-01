cmake_minimum_required(VERSION 3.16)
project(FranchiseAISearch VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Suppress deprecation warnings from Wt library headers (sprintf in WLogger.h)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    add_compile_options(-Wno-deprecated-declarations)
endif()

# Find Wt package
find_package(Wt REQUIRED COMPONENTS Wt HTTP)

# Find CURL for HTTP requests to AI APIs
find_package(CURL REQUIRED)

# Include directories (our source directories)
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/widgets
    ${CMAKE_SOURCE_DIR}/src/services
    ${CMAKE_SOURCE_DIR}/src/models
)

# Source files - Models
set(MODEL_SOURCES
    src/models/SearchResult.cpp
    src/models/BusinessInfo.cpp
)

# Source files - Services
set(SERVICE_SOURCES
    src/services/GoogleMyBusinessAPI.cpp
    src/services/GoogleGeocodingAPI.cpp
    src/services/GooglePlacesAPI.cpp
    src/services/ThreadPool.cpp
    src/services/BBBAPI.cpp
    src/services/DemographicsAPI.cpp
    src/services/OpenStreetMapAPI.cpp
    src/services/GeocodingService.cpp
    src/services/AISearchService.cpp
    src/services/AIEngine.cpp
    src/services/OpenAIEngine.cpp
    src/services/GeminiEngine.cpp
    src/services/ApiLogicServerClient.cpp
    src/services/AuthService.cpp
    src/services/AuditLogger.cpp
)

# Source files - Widgets
set(WIDGET_SOURCES
    src/widgets/Sidebar.cpp
    src/widgets/Navigation.cpp
    src/widgets/SearchPanel.cpp
    src/widgets/ResultCard.cpp
    src/widgets/ResultsDisplay.cpp
    src/widgets/LoginDialog.cpp
    src/widgets/AuditTrailPage.cpp
)

# Main application sources
set(APP_SOURCES
    src/main.cpp
    src/FranchiseApp.cpp
)

# Combine all sources
set(ALL_SOURCES
    ${MODEL_SOURCES}
    ${SERVICE_SOURCES}
    ${WIDGET_SOURCES}
    ${APP_SOURCES}
)

# Create executable
add_executable(franchise_ai_search ${ALL_SOURCES})

# Mark Wt include directories as SYSTEM to suppress warnings from library headers
get_target_property(WT_INCLUDE_DIRS Wt::Wt INTERFACE_INCLUDE_DIRECTORIES)
if(WT_INCLUDE_DIRS)
    target_include_directories(franchise_ai_search SYSTEM PRIVATE ${WT_INCLUDE_DIRS})
endif()

# Link Wt and CURL libraries
target_link_libraries(franchise_ai_search
    Wt::Wt
    Wt::HTTP
    CURL::libcurl
)

# Copy resources to build directory
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})

# Install targets
install(TARGETS franchise_ai_search RUNTIME DESTINATION bin)
install(DIRECTORY resources/ DESTINATION share/franchise_ai_search/resources)

# Custom target to run the application
add_custom_target(run
    COMMAND franchise_ai_search --docroot "${CMAKE_BINARY_DIR}/resources" --http-address 0.0.0.0 --http-port 8080
    DEPENDS franchise_ai_search
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running Franchise AI Search Application on http://localhost:8080"
)

# ============================================================================
# Test Target: ApiLogicServer Client Tests
# ============================================================================
set(TEST_SOURCES
    tests/test_als_client.cpp
    src/services/ApiLogicServerClient.cpp
)

add_executable(test_als_client ${TEST_SOURCES})

target_include_directories(test_als_client PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/services
    ${CMAKE_SOURCE_DIR}/src/models
)

target_link_libraries(test_als_client
    CURL::libcurl
)

# Custom target to run tests
add_custom_target(test
    COMMAND test_als_client
    DEPENDS test_als_client
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running ApiLogicServer Client Tests"
)

# ============================================================================
# Test Runner UI: ncurses-based test orchestration application
# ============================================================================
find_package(Curses)

if(CURSES_FOUND)
    set(TEST_RUNNER_SOURCES
        tests/test_runner_ui.cpp
        tests/TestOrchestrator.cpp
        src/services/ApiLogicServerClient.cpp
    )

    add_executable(test_runner ${TEST_RUNNER_SOURCES})

    target_include_directories(test_runner PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/services
        ${CMAKE_SOURCE_DIR}/src/models
        ${CMAKE_SOURCE_DIR}/tests
        ${CURSES_INCLUDE_DIR}
    )

    target_link_libraries(test_runner
        CURL::libcurl
        ${CURSES_LIBRARIES}
    )

    # Custom target to run the test runner UI
    add_custom_target(run_tests
        COMMAND test_runner
        DEPENDS test_runner
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running Test Runner UI"
    )
else()
    message(STATUS "ncurses not found - test_runner target will not be available")
endif()
