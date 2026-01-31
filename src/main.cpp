/**
 * @file main.cpp
 * @brief Entry point for the FranchiseAI Prospect Search Application
 *
 * This application provides franchise owners with AI-powered search
 * capabilities to find potential catering clients. It aggregates data
 * from Google My Business, Better Business Bureau, and demographic
 * databases to identify high-potential prospects.
 *
 * Usage:
 *   ./franchise_ai_search --docroot ./resources --http-address 0.0.0.0 --http-port 8080
 *
 * Features:
 *   - AI-powered prospect search
 *   - Multi-source data aggregation (Google, BBB, Demographics)
 *   - Intelligent scoring and ranking
 *   - Modern, responsive UI with sidebar navigation
 *   - Export and prospect management capabilities
 *
 * @author FranchiseAI Team
 * @version 1.0.0
 */

#include <Wt/WServer.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include "FranchiseApp.h"
#include "AppConfig.h"

/**
 * @brief Print application banner and startup information
 */
void printBanner() {
    std::cout << R"(
  _____                     _     _            _    ___
 |  ___| __ __ _ _ __   ___| |__ (_)___  ___  / \  |_ _|
 | |_ | '__/ _` | '_ \ / __| '_ \| / __|/ _ \/  \  | |
 |  _|| | | (_| | | | | (__| | | | \__ \  __/ /\ \ | |
 |_|  |_|  \__,_|_| |_|\___|_| |_|_|___/\___/_/  \_\___|

  AI-Powered Prospect Search for Franchise Owners
  Version 1.0.0

)" << std::endl;
}

/**
 * @brief Print usage instructions
 * @param programName Name of the executable
 */
void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --docroot <path>       Document root for static resources (default: ./resources)" << std::endl;
    std::cout << "  --http-address <addr>  HTTP server address (default: 0.0.0.0)" << std::endl;
    std::cout << "  --http-port <port>     HTTP server port (default: 8080)" << std::endl;
    std::cout << "  --help                 Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  " << programName << " --docroot ./resources --http-address 0.0.0.0 --http-port 8080" << std::endl;
    std::cout << std::endl;
}

/**
 * @brief Main entry point
 *
 * Initializes and runs the Wt HTTP server with the FranchiseAI application.
 *
 * @param argc Argument count
 * @param argv Argument values
 * @return Exit code (0 for success)
 */
int main(int argc, char** argv) {
    // Check for help flag
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printBanner();
            printUsage(argv[0]);
            return 0;
        }
    }

    printBanner();

    // Load configuration from environment variables and config file
    std::cout << "Loading configuration..." << std::endl;
    auto& config = FranchiseAI::AppConfig::instance();

    // First load from environment variables (these take precedence)
    config.loadFromEnvironment();

    // Then try to load from config file (won't override env vars if already set)
    if (config.loadFromFile("config/app_config.json")) {
        std::cout << "Loaded configuration from config/app_config.json" << std::endl;
    }

    // Print configuration status
    config.printStatus();

    try {
        // Create Wt server
        Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

        // Add the FranchiseAI application entry point
        server.addEntryPoint(
            Wt::EntryPointType::Application,
            FranchiseAI::createFranchiseApp,
            "",  // Path (root)
            ""   // Favicon
        );

        std::cout << "Starting FranchiseAI server..." << std::endl;
        std::cout << "Navigate to: http://localhost:8080" << std::endl;
        std::cout << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        std::cout << std::endl;

        // Run the server
        if (server.start()) {
            // Wait for shutdown signal
            int sig = Wt::WServer::waitForShutdown();
            std::cout << std::endl;
            std::cout << "Shutdown signal received (signal " << sig << ")" << std::endl;
            server.stop();
        }

        std::cout << "FranchiseAI server stopped." << std::endl;

    } catch (const Wt::WServer::Exception& e) {
        std::cerr << "Server exception: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
