/**
 * @file test_essential.cpp
 * @brief Essential tests for Coinbase Ticker Analyzer
 */

#include <gtest/gtest.h>
#include "EMACalculator.h"
#include "JSONParser.h"
#include "TickerData.h"

// Test EMA Calculator - Core functionality
TEST(EMATest, BasicCalculation) {
    EMACalculator calculator(5.0); // 5 second interval
    auto now = std::chrono::system_clock::now();
    
    // First update should initialize
    calculator.updatePriceEMA(100.0, now);
    EXPECT_TRUE(calculator.isPriceInitialized());
    EXPECT_DOUBLE_EQ(calculator.getPriceEMA(), 100.0);
    
    // Test that calculator is working
    EXPECT_FALSE(calculator.isMidPriceInitialized());
}

// Test JSON Parser - Core functionality
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

// Test TickerData - Core functionality
TEST(TickerDataTest, MidPriceCalculation) {
    TickerData data;
    data.best_bid = "100.0";
    data.best_ask = "102.0";
    
    EXPECT_DOUBLE_EQ(data.calculateMidPrice(), 101.0);
}

// Test CSV Format - Core functionality
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

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
