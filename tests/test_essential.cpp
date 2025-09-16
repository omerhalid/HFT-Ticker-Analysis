/**
 * @file test_essential.cpp
 * @brief Essential tests for Coinbase Ticker Analyzer
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "EMACalculator.h"
#include "JSONParser.h"
#include "TickerData.h"
#include "LockFreeRingBuffer.h"

// Test EMA Calculator
TEST(EMATest, BasicCalculation) {
    EMACalculator calculator(5); // 5 second interval
    auto now = std::chrono::system_clock::now();
    
    // First update should initialize
    calculator.updatePriceEMA(100.0, now);
    EXPECT_TRUE(calculator.isPriceInitialized());
    EXPECT_DOUBLE_EQ(calculator.getPriceEMA(), 100.0);
    
    // Test that calculator is working
    EXPECT_FALSE(calculator.isMidPriceInitialized());
}

// Test 5-Second Rule
TEST(EMATest, FiveSecondRule) {
    EMACalculator calculator(5);
    auto start_time = std::chrono::system_clock::now();
    
    // First update - should work
    double ema1 = calculator.updatePriceEMA(100.0, start_time);
    EXPECT_DOUBLE_EQ(ema1, 100.0);
    
    // Update immediately after - should NOT update (less than 5 seconds)
    auto immediate_time = start_time + std::chrono::milliseconds(100);
    double ema2 = calculator.updatePriceEMA(200.0, immediate_time);
    EXPECT_DOUBLE_EQ(ema2, 100.0); // Should return old value
    
    // Update after 5+ seconds - should update
    auto later_time = start_time + std::chrono::seconds(6);
    double ema3 = calculator.updatePriceEMA(200.0, later_time);
    EXPECT_NE(ema3, 100.0); // Should be different (updated)
    EXPECT_GT(ema3, 100.0); // Should be closer to 200.0
}

// Test EMA Mathematics 
TEST(EMATest, EMAMathematics) {
    EMACalculator calculator(5);
    auto time1 = std::chrono::system_clock::now();
    auto time2 = time1 + std::chrono::seconds(6);
    
    // First value: 100.0
    double ema1 = calculator.updatePriceEMA(100.0, time1);
    EXPECT_DOUBLE_EQ(ema1, 100.0);
    
    // Second value: 200.0 after 6 seconds
    // EMA formula: new_ema = alpha * new_value + (1 - alpha) * old_ema
    // alpha = 2/(5+1) = 1/3 = 0.333...
    double ema2 = calculator.updatePriceEMA(200.0, time2);
    
    // Expected: 0.333 * 200 + 0.667 * 100 = 66.67 + 66.67 = 133.33
    double expected = (1.0/3.0) * 200.0 + (2.0/3.0) * 100.0;
    EXPECT_NEAR(ema2, expected, 0.01);
}

// Test Thread Safety - Concurrent Access
TEST(EMATest, ThreadSafety) {
    EMACalculator calculator(5);
    auto base_time = std::chrono::system_clock::now();
    
    std::vector<std::thread> threads;
    std::vector<double> results(10);
    
    // Launch multiple threads updating EMA simultaneously
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&calculator, &results, i, base_time]() {
            auto thread_time = base_time + std::chrono::seconds(i);
            results[i] = calculator.updatePriceEMA(100.0 + i, thread_time);
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Should not crash and should have some valid results
    bool hasValidResults = false;
    for (double result : results) {
        if (result > 0) {
            hasValidResults = true;
            break;
        }
    }
    EXPECT_TRUE(hasValidResults);
}

// Test JSON Parser
TEST(JSONTest, ParseTickerMessage) {
    std::string json = R"({
        "type": "ticker",
        "sequence": 12345,
        "product_id": "BTC-USD",
        "price": "50000.00",
        "best_bid": "49999.50",
        "best_ask": "50000.50"
    })";
    
    TickerData data;
    EXPECT_TRUE(JSONParser::parseTickerMessage(json, data));
    EXPECT_EQ(data.product_id, "BTC-USD");
    EXPECT_EQ(data.price, "50000.00");
    EXPECT_EQ(data.best_bid, "49999.50");
    EXPECT_EQ(data.best_ask, "50000.50");
}

// Test TickerData 
TEST(TickerDataTest, MidPriceCalculation) {
    TickerData data;
    data.best_bid = "100.0";
    data.best_ask = "102.0";
    
    EXPECT_DOUBLE_EQ(data.calculateMidPrice(), 101.0);
}

// Test CSV Format 
TEST(TickerDataTest, CSVOutput) {
    TickerData data;
    data.type = "ticker";
    data.product_id = "BTC-USD";
    data.price = "50000.0";
    data.price_ema = 49950.0;
    data.mid_price_ema = 49975.0;
    
    std::string csv = data.toCSV();
    EXPECT_TRUE(csv.find("BTC-USD") != std::string::npos);
    EXPECT_TRUE(csv.find("50000") != std::string::npos);
    EXPECT_TRUE(csv.find("49950") != std::string::npos);
    EXPECT_TRUE(csv.find("49975") != std::string::npos);
}

// Test LockFreeRingBuffer 
TEST(LockFreeRingBufferTest, BasicOperations) {
    LockFreeRingBuffer<int, 8> buffer;
    
    // Test empty buffer
    EXPECT_TRUE(buffer.empty());
    EXPECT_FALSE(buffer.full());
    EXPECT_EQ(buffer.size(), 0);
    
    // Test push/pop
    EXPECT_TRUE(buffer.push(42));
    EXPECT_FALSE(buffer.empty());
    EXPECT_EQ(buffer.size(), 1);
    
    int value;
    EXPECT_TRUE(buffer.pop(value));
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(buffer.empty());
}

// Test LockFreeRingBuffer 
TEST(LockFreeRingBufferTest, ThreadSafety) {
    LockFreeRingBuffer<int, 16> buffer;
    std::atomic<bool> start_flag{false};
    std::atomic<int> push_count{0};
    std::atomic<int> pop_count{0};
    std::atomic<bool> producer_done{false};
    
    // Producer thread
    std::thread producer([&]() {
        while (!start_flag.load()) std::this_thread::yield();
        
        for (int i = 0; i < 50; ++i) {
            if (buffer.push(i)) {
                push_count.fetch_add(1);
            }
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
        producer_done.store(true);
    });
    
    // Consumer thread
    std::thread consumer([&]() {
        while (!start_flag.load()) std::this_thread::yield();
        
        int value;
        while (!producer_done.load() || !buffer.empty()) {
            if (buffer.pop(value)) {
                pop_count.fetch_add(1);
            }
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    });
    
    // Start both threads
    start_flag.store(true);
    
    producer.join();
    consumer.join();
    
    // Should have processed some items without crashes
    EXPECT_GT(push_count.load(), 0);
    EXPECT_GT(pop_count.load(), 0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
