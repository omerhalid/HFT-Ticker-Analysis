/**
 * @file test_EMACalculator.cpp
 * @brief Unit tests for EMACalculator class
 * @author HFT Developer
 * @date 2024
 */

#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include "EMACalculator.h"

class EMACalculatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        calculator = std::make_unique<EMACalculator>(5); // 5-second interval
    }
    
    void TearDown() override {
        calculator.reset();
    }
    
    std::unique_ptr<EMACalculator> calculator;
};

TEST_F(EMACalculatorTest, InitialState) {
    EXPECT_FALSE(calculator->isPriceInitialized());
    EXPECT_EQ(calculator->getPriceEMA(), 0.0);
    EXPECT_EQ(calculator->getMidPriceEMA(), 0.0);
}

TEST_F(EMACalculatorTest, FirstPriceUpdate) {
    auto now = std::chrono::system_clock::now();
    double price = 50000.0;
    
    double ema = calculator->updatePriceEMA(price, now);
    
    EXPECT_TRUE(calculator->isPriceInitialized());
    EXPECT_EQ(ema, price);
    EXPECT_EQ(calculator->getPriceEMA(), price);
}

TEST_F(EMACalculatorTest, FirstMidPriceUpdate) {
    auto now = std::chrono::system_clock::now();
    double midPrice = 50025.0;
    
    double ema = calculator->updateMidPriceEMA(midPrice, now);
    
    EXPECT_TRUE(calculator->isMidPriceInitialized());
    EXPECT_EQ(ema, midPrice);
    EXPECT_EQ(calculator->getMidPriceEMA(), midPrice);
}

TEST_F(EMACalculatorTest, MultipleUpdatesWithinInterval) {
    auto now = std::chrono::system_clock::now();
    double price1 = 50000.0;
    double price2 = 50100.0;
    
    // First update
    double ema1 = calculator->updatePriceEMA(price1, now);
    EXPECT_EQ(ema1, price1);
    
    // Second update within same interval should not change EMA
    double ema2 = calculator->updatePriceEMA(price2, now);
    EXPECT_EQ(ema2, price1); // Should remain the same
}

TEST_F(EMACalculatorTest, MultipleUpdatesAfterInterval) {
    auto now1 = std::chrono::system_clock::now();
    auto now2 = now1 + std::chrono::seconds(6); // 6 seconds later
    
    double price1 = 50000.0;
    double price2 = 50100.0;
    
    // First update
    double ema1 = calculator->updatePriceEMA(price1, now1);
    EXPECT_EQ(ema1, price1);
    
    // Second update after interval
    double ema2 = calculator->updatePriceEMA(price2, now2);
    
    // EMA should be updated with smoothing
    EXPECT_GT(ema2, price1);
    EXPECT_LT(ema2, price2);
}

TEST_F(EMACalculatorTest, ResetFunctionality) {
    auto now = std::chrono::system_clock::now();
    
    // Update with some values
    calculator->updatePriceEMA(50000.0, now);
    calculator->updateMidPriceEMA(50025.0, now);
    
    EXPECT_TRUE(calculator->isPriceInitialized());
    
    // Reset
    calculator->reset();
    
    EXPECT_FALSE(calculator->isPriceInitialized());
    EXPECT_EQ(calculator->getPriceEMA(), 0.0);
    EXPECT_EQ(calculator->getMidPriceEMA(), 0.0);
}

TEST_F(EMACalculatorTest, ThreadSafety) {
    const int numThreads = 4;
    const int updatesPerThread = 100;
    std::vector<std::thread> threads;
    
    auto now = std::chrono::system_clock::now();
    
    // Start multiple threads updating EMA
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, now, i, updatesPerThread]() {
            for (int j = 0; j < updatesPerThread; ++j) {
                double price = 50000.0 + (i * updatesPerThread + j);
                auto updateTime = now + std::chrono::seconds(j);
                calculator->updatePriceEMA(price, updateTime);
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Should not crash and should have some reasonable value
    EXPECT_TRUE(calculator->isPriceInitialized());
    EXPECT_GT(calculator->getPriceEMA(), 0.0);
}
