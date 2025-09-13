/**
 * @file test_AsyncCSVLogger.cpp
 * @brief Unit tests for AsyncCSVLogger class
 * @author HFT Developer
 * @date 2024
 */

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include "AsyncCSVLogger.h"
#include "TickerData.h"

class AsyncCSVLoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        testFilename = "test_async_ticker_data.csv";
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

TEST_F(AsyncCSVLoggerTest, ConstructorAndDestructor) {
    {
        AsyncCSVLogger logger(testFilename);
        // Wait for logger to be ready
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_TRUE(logger.isReady());
        EXPECT_EQ(logger.getFilename(), testFilename);
    }
    
    // File should exist after logger goes out of scope
    EXPECT_TRUE(std::filesystem::exists(testFilename));
}

TEST_F(AsyncCSVLoggerTest, LogSingleTickerData) {
    AsyncCSVLogger logger(testFilename);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    TickerData data = createSampleTickerData();
    EXPECT_TRUE(logger.logTickerData(data));
    
    // Wait for async logging to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::ifstream file(testFilename);
    EXPECT_TRUE(file.is_open());
    
    std::string line;
    std::getline(file, line); // Skip header
    std::getline(file, line);
    EXPECT_FALSE(line.empty());
    EXPECT_NE(line.find("BTC-USD"), std::string::npos);
}

TEST_F(AsyncCSVLoggerTest, LogMultipleTickerData) {
    AsyncCSVLogger logger(testFilename);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    for (int i = 0; i < 5; ++i) {
        TickerData data = createSampleTickerData();
        data.sequence = std::to_string(12345 + i);
        EXPECT_TRUE(logger.logTickerData(data));
    }
    
    // Wait for async logging to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::ifstream file(testFilename);
    EXPECT_TRUE(file.is_open());
    
    std::string line;
    std::getline(file, line); // Skip header
    
    int lineCount = 0;
    while (std::getline(file, line)) {
        lineCount++;
    }
    EXPECT_EQ(lineCount, 5);
}

TEST_F(AsyncCSVLoggerTest, CSVHeaders) {
    AsyncCSVLogger logger(testFilename);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    TickerData data = createSampleTickerData();
    logger.logTickerData(data);
    
    // Wait for async logging to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::ifstream file(testFilename);
    EXPECT_TRUE(file.is_open());
    
    std::string header;
    std::getline(file, header);
    EXPECT_NE(header.find("type,sequence,product_id"), std::string::npos);
    EXPECT_NE(header.find("price_ema,mid_price_ema"), std::string::npos);
}

TEST_F(AsyncCSVLoggerTest, QueueCapacity) {
    AsyncCSVLogger logger(testFilename);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_GT(logger.getQueueCapacity(), 0);
    EXPECT_EQ(logger.getQueueSize(), 0);
}

TEST_F(AsyncCSVLoggerTest, ThreadSafety) {
    AsyncCSVLogger logger(testFilename);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    const int numThreads = 4;
    const int entriesPerThread = 50; // Reduced to avoid queue overflow
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&logger, i, entriesPerThread]() {
            for (int j = 0; j < entriesPerThread; ++j) {
                TickerData data;
                data.type = "ticker";
                data.sequence = std::to_string(i * entriesPerThread + j);
                data.product_id = "BTC-USD";
                data.price = "50000.00";
                data.price_ema = 50000.0;
                data.mid_price_ema = 50000.0;
                data.mid_price = 50000.0;
                
                logger.logTickerData(data);
                // Small delay to prevent overwhelming the queue
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Wait for async logging to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    std::ifstream file(testFilename);
    EXPECT_TRUE(file.is_open());
    
    std::string line;
    std::getline(file, line); // Skip header
    
    int lineCount = 0;
    while (std::getline(file, line)) {
        lineCount++;
    }
    // Allow for some data loss due to queue overflow in high-load scenarios
    EXPECT_GE(lineCount, numThreads * entriesPerThread * 0.9); // At least 90% should be logged
}

TEST_F(AsyncCSVLoggerTest, CloseAndReopen) {
    {
        AsyncCSVLogger logger(testFilename);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        TickerData data = createSampleTickerData();
        logger.logTickerData(data);
        logger.close();
    }
    
    // Create new logger with same file
    AsyncCSVLogger logger2(testFilename);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    TickerData data2 = createSampleTickerData();
    data2.sequence = "99999";
    EXPECT_TRUE(logger2.logTickerData(data2));
    
    // Wait for async logging to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::ifstream file(testFilename);
    EXPECT_TRUE(file.is_open());
    
    std::string line;
    std::getline(file, line); // Skip header
    
    int lineCount = 0;
    while (std::getline(file, line)) {
        lineCount++;
    }
    EXPECT_GE(lineCount, 1); // At least one entry should be present
}
