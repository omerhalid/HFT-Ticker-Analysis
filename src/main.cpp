/**
 * @file main.cpp
 * @brief Main entry point for Coinbase Ticker Analyzer application
 */

#include <iostream>
#include <string>
#include <signal.h>
#include <memory>
#include "CoinbaseTickerAnalyzer.h"

// Global analyzer instance for signal handling
std::unique_ptr<CoinbaseTickerAnalyzer> g_analyzer;

/**
 * @brief Signal handler for graceful shutdown
 * @param signal Signal number
 */
void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Shutting down gracefully..." << std::endl;
    if (g_analyzer) {
        g_analyzer->stop();
    }
}

/**
 * @brief Print usage information
 * @param programName Name of the program
 */
void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -p, --product <ID>    Product ID to analyze (default: BTC-USD)" << std::endl;
    std::cout << "  -o, --output <file>   Output CSV filename (default: ticker_data.csv)" << std::endl;
    std::cout << "  -h, --help           Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " -p ETH-USD -o eth_data.csv" << std::endl;
    std::cout << "  " << programName << " --product BTC-USD --output btc_ticker.csv" << std::endl;
}

/**
 * @brief Main function
 * @param argc Argument count
 * @param argv Argument vector
 * @return Exit code
 */
int main(int argc, char* argv[]) {
    std::string productId = "BTC-USD";
    std::string outputFile = "ticker_data.csv";
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-p" || arg == "--product") {
            if (i + 1 < argc) {
                productId = argv[++i];
            } else {
                std::cerr << "Error: --product requires a value" << std::endl;
                return 1;
            }
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                outputFile = argv[++i];
            } else {
                std::cerr << "Error: --output requires a value" << std::endl;
                return 1;
            }
        } else {
            std::cerr << "Error: Unknown argument " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "=== Coinbase Ticker Analyzer ===" << std::endl;
    std::cout << "Product ID: " << productId << std::endl;
    std::cout << "Output File: " << outputFile << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    std::cout << std::endl;
    
    try {
        // Create and start the analyzer
        g_analyzer = std::make_unique<CoinbaseTickerAnalyzer>(productId, outputFile);
        
        if (!g_analyzer->start()) {
            std::cerr << "Failed to start the analyzer" << std::endl;
            return 1;
        }
        
        // Keep the main thread alive
        while (g_analyzer->isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Application terminated successfully" << std::endl;
    return 0;
}
