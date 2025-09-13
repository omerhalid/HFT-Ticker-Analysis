/**
 * @file test_CSVLogger.cpp
 * @brief Unit tests for CSVLogger class
 * @author HFT Developer
 * @date 2024
 */

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <thread>
#include "CSVLogger.h"
#include "TickerData.h"

class CSVLoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        testFilename = "test_ticker_data.csv";
        // Remove test file if it exists
        if (std::filesystem::exists(testFilename)) {
            std::filesystem::remove(testFilename);
        }
    }
    
    void TearDown() override {
        // Clean up test file
        if (std::filesystem::exists(testFilename)) {
            std::filesystem::remove(testFilename);
        }
    }
    
    std::string testFilename;
    
    TickerData createSampleTickerData() {
        TickerData data;
        data.type = "ticker";
        data.sequence = "12345";
        data.product_id = "BTC-USD";
        data.price = "50000.00";
        data.open_24h = "49000.00";
        data.volume_24h = "1000.5";
        data.low_24h = "48000.00";
        data.high_24h = "51000.00";
        data.volume_30d = "30000.0";
        data.best_bid = "49999.50";
        data.best_ask = "50000.50";
        data.side = "buy";
        data.time = "2024-01-01T12:00:00.000Z";
        data.trade_id = "67890";
        data.last_size = "0.1";
        data.price_ema = 49950.0;
        data.mid_price_ema = 49975.0;
        data.mid_price = 50000.0;
        return data;
    }
};

TEST_F(CSVLoggerTest, ConstructorAndDestructor) {
    {
        CSVLogger logger(testFilename);
        EXPECT_TRUE(logger.isReady());
        EXPECT_EQ(logger.getFilename(), testFilename);
    }
    
    // File should exist after logger goes out of scope
    EXPECT_TRUE(std::filesystem::exists(testFilename));
}

TEST_F(CSVLoggerTest, LogSingleTickerData) {
    CSVLogger logger(testFilename);
    TickerData data = createSampleTickerData();
    
    logger.logTickerData(data);
    logger.flush();
    
    // Check if file was created and contains data
    EXPECT_TRUE(std::filesystem::exists(testFilename));
    
    std::ifstream file(testFilename);
    std::string line;
    std::getline(file, line); // Header line
    std::getline(file, line); // Data line
    
    EXPECT_FALSE(line.empty());
    EXPECT_TRUE(line.find("ticker") != std::string::npos);
    EXPECT_TRUE(line.find("BTC-USD") != std::string::npos);
    EXPECT_TRUE(line.find("50000.00") != std::string::npos);
}

TEST_F(CSVLoggerTest, LogMultipleTickerData) {
    CSVLogger logger(testFilename);
    
    // Log multiple entries
    for (int i = 0; i < 5; ++i) {
        TickerData data = createSampleTickerData();
        data.sequence = std::to_string(12345 + i);
        data.price = std::to_string(50000.00 + i * 100);
        logger.logTickerData(data);
    }
    
    logger.flush();
    
    // Count lines in file (should be 1 header + 5 data lines)
    std::ifstream file(testFilename);
    std::string line;
    int lineCount = 0;
    while (std::getline(file, line)) {
        lineCount++;
    }
    
    EXPECT_EQ(lineCount, 6); // 1 header + 5 data lines
}

TEST_F(CSVLoggerTest, CSVHeaders) {
    CSVLogger logger(testFilename);
    TickerData data = createSampleTickerData();
    
    logger.logTickerData(data);
    logger.flush();
    
    std::ifstream file(testFilename);
    std::string headerLine;
    std::getline(file, headerLine);
    
    EXPECT_TRUE(headerLine.find("type") != std::string::npos);
    EXPECT_TRUE(headerLine.find("product_id") != std::string::npos);
    EXPECT_TRUE(headerLine.find("price") != std::string::npos);
    EXPECT_TRUE(headerLine.find("price_ema") != std::string::npos);
    EXPECT_TRUE(headerLine.find("mid_price_ema") != std::string::npos);
    EXPECT_TRUE(headerLine.find("mid_price") != std::string::npos);
}

TEST_F(CSVLoggerTest, CSVFieldEscaping) {
    CSVLogger logger(testFilename);
    TickerData data = createSampleTickerData();
    
    // Add fields that need escaping
    data.product_id = "BTC,USD"; // Contains comma
    data.side = "buy\"sell"; // Contains quote
    
    logger.logTickerData(data);
    logger.flush();
    
    std::ifstream file(testFilename);
    std::string line;
    std::getline(file, line); // Skip header
    std::getline(file, line);
    
    // Check that fields are properly escaped
    EXPECT_TRUE(line.find("\"BTC,USD\"") != std::string::npos);
    EXPECT_TRUE(line.find("\"buy\"\"sell\"") != std::string::npos);
}

TEST_F(CSVLoggerTest, ThreadSafety) {
    CSVLogger logger(testFilename);
    const int numThreads = 4;
    const int entriesPerThread = 50;
    std::vector<std::thread> threads;
    
    // Start multiple threads logging data
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&logger, i, entriesPerThread]() {
            for (int j = 0; j < entriesPerThread; ++j) {
                TickerData data;
                data.type = "ticker";
                data.sequence = std::to_string(i * entriesPerThread + j);
                data.product_id = "BTC-USD";
                data.price = std::to_string(50000.0 + j);
                data.price_ema = 50000.0 + j;
                data.mid_price_ema = 50000.0 + j;
                data.mid_price = 50000.0 + j;
                
                logger.logTickerData(data);
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    logger.flush();
    
    // Count total lines (should be 1 header + 200 data lines)
    std::ifstream file(testFilename);
    std::string line;
    int lineCount = 0;
    while (std::getline(file, line)) {
        lineCount++;
    }
    
    EXPECT_EQ(lineCount, 201); // 1 header + 200 data lines
}

TEST_F(CSVLoggerTest, CloseAndReopen) {
    {
        CSVLogger logger(testFilename);
        TickerData data = createSampleTickerData();
        logger.logTickerData(data);
        logger.close();
    }
    
    // File should exist and be closed
    EXPECT_TRUE(std::filesystem::exists(testFilename));
    
    // Should be able to create new logger with same file
    CSVLogger logger2(testFilename);
    EXPECT_TRUE(logger2.isReady());
}
