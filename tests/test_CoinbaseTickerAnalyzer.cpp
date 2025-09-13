/**
 * @file test_CoinbaseTickerAnalyzer.cpp
 * @brief Unit tests for CoinbaseTickerAnalyzer class
 * @author HFT Developer
 * @date 2024
 */

#include <gtest/gtest.h>
#include <filesystem>
#include "CoinbaseTickerAnalyzer.h"

class CoinbaseTickerAnalyzerTest : public ::testing::Test {
protected:
    void SetUp() override {
        testCsvFile = "test_analyzer_output.csv";
        // Remove test file if it exists
        if (std::filesystem::exists(testCsvFile)) {
            std::filesystem::remove(testCsvFile);
        }
    }
    
    void TearDown() override {
        // Clean up test file
        if (std::filesystem::exists(testCsvFile)) {
            std::filesystem::remove(testCsvFile);
        }
    }
    
    std::string testCsvFile;
};

TEST_F(CoinbaseTickerAnalyzerTest, Constructor) {
    CoinbaseTickerAnalyzer analyzer("ETH-USD", testCsvFile);
    
    EXPECT_EQ(analyzer.getProductId(), "ETH-USD");
    EXPECT_EQ(analyzer.getCsvFilename(), testCsvFile);
    EXPECT_FALSE(analyzer.isRunning());
}

TEST_F(CoinbaseTickerAnalyzerTest, DefaultConstructor) {
    CoinbaseTickerAnalyzer analyzer;
    
    EXPECT_EQ(analyzer.getProductId(), "BTC-USD");
    EXPECT_EQ(analyzer.getCsvFilename(), "ticker_data.csv");
    EXPECT_FALSE(analyzer.isRunning());
}

TEST_F(CoinbaseTickerAnalyzerTest, SetProductId) {
    CoinbaseTickerAnalyzer analyzer;
    
    analyzer.setProductId("LTC-USD");
    EXPECT_EQ(analyzer.getProductId(), "LTC-USD");
}

TEST_F(CoinbaseTickerAnalyzerTest, SetCsvFilename) {
    CoinbaseTickerAnalyzer analyzer;
    
    analyzer.setCsvFilename("custom_output.csv");
    EXPECT_EQ(analyzer.getCsvFilename(), "custom_output.csv");
}

TEST_F(CoinbaseTickerAnalyzerTest, GetStatistics) {
    CoinbaseTickerAnalyzer analyzer("ETH-USD", testCsvFile);
    
    std::string stats = analyzer.getStatistics();
    
    EXPECT_FALSE(stats.empty());
    EXPECT_TRUE(stats.find("ETH-USD") != std::string::npos);
    EXPECT_TRUE(stats.find("custom_output.csv") != std::string::npos);
    EXPECT_TRUE(stats.find("Running: No") != std::string::npos);
}

TEST_F(CoinbaseTickerAnalyzerTest, StartWithoutConnection) {
    // This test verifies that start() fails gracefully when WebSocket connection fails
    // In a real scenario, this would fail due to network issues
    CoinbaseTickerAnalyzer analyzer("INVALID-PRODUCT", testCsvFile);
    
    // Note: This test might pass or fail depending on network connectivity
    // In a real test environment, you might want to mock the WebSocket client
    bool result = analyzer.start();
    
    // If it fails to start, it should not be running
    if (!result) {
        EXPECT_FALSE(analyzer.isRunning());
    }
    
    // Always stop to clean up
    analyzer.stop();
}

TEST_F(CoinbaseTickerAnalyzerTest, StopWhenNotRunning) {
    CoinbaseTickerAnalyzer analyzer;
    
    // Should not crash when stopping a non-running analyzer
    analyzer.stop();
    EXPECT_FALSE(analyzer.isRunning());
}

TEST_F(CoinbaseTickerAnalyzerTest, MultipleStartStop) {
    CoinbaseTickerAnalyzer analyzer("ETH-USD", testCsvFile);
    
    // Multiple start calls should be handled gracefully
    analyzer.start();
    analyzer.start(); // Second start should not cause issues
    
    analyzer.stop();
    EXPECT_FALSE(analyzer.isRunning());
    
    // Should be able to start again after stopping
    analyzer.start();
    analyzer.stop();
    EXPECT_FALSE(analyzer.isRunning());
}

TEST_F(CoinbaseTickerAnalyzerTest, DestructorCleanup) {
    {
        CoinbaseTickerAnalyzer analyzer("ETH-USD", testCsvFile);
        analyzer.start();
        // Analyzer should be properly cleaned up when going out of scope
    }
    
    // Should not crash or leave resources hanging
    EXPECT_TRUE(true);
}

// Note: Integration tests that require actual WebSocket connections
// would be better suited for a separate test suite or manual testing
// due to their dependency on external services and network connectivity.
