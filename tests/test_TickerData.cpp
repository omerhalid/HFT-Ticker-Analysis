/**
 * @file test_TickerData.cpp
 * @brief Unit tests for TickerData structure
 * @author HFT Developer
 * @date 2024
 */

#include <gtest/gtest.h>
#include "TickerData.h"

class TickerDataTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create sample ticker data
        tickerData.type = "ticker";
        tickerData.sequence = "12345";
        tickerData.product_id = "BTC-USD";
        tickerData.price = "50000.00";
        tickerData.open_24h = "49000.00";
        tickerData.volume_24h = "1000.5";
        tickerData.low_24h = "48000.00";
        tickerData.high_24h = "51000.00";
        tickerData.volume_30d = "30000.0";
        tickerData.best_bid = "49999.50";
        tickerData.best_ask = "50000.50";
        tickerData.side = "buy";
        tickerData.time = "2024-01-01T12:00:00.000Z";
        tickerData.trade_id = "67890";
        tickerData.last_size = "0.1";
        tickerData.price_ema = 49950.0;
        tickerData.mid_price_ema = 49975.0;
        tickerData.mid_price = 50000.0;
    }
    
    TickerData tickerData;
};

TEST_F(TickerDataTest, DefaultConstructor) {
    TickerData data;
    
    EXPECT_EQ(data.price_ema, 0.0);
    EXPECT_EQ(data.mid_price_ema, 0.0);
    EXPECT_EQ(data.mid_price, 0.0);
    EXPECT_TRUE(data.type.empty());
    EXPECT_TRUE(data.product_id.empty());
}

TEST_F(TickerDataTest, CalculateMidPrice) {
    double midPrice = tickerData.calculateMidPrice();
    double expectedMidPrice = (49999.50 + 50000.50) / 2.0;
    
    EXPECT_DOUBLE_EQ(midPrice, expectedMidPrice);
    EXPECT_DOUBLE_EQ(midPrice, 50000.0);
}

TEST_F(TickerDataTest, CalculateMidPriceWithInvalidBidAsk) {
    TickerData data;
    data.best_bid = "invalid";
    data.best_ask = "invalid";
    
    double midPrice = data.calculateMidPrice();
    
    EXPECT_EQ(midPrice, 0.0);
}

TEST_F(TickerDataTest, CalculateMidPriceWithEmptyBidAsk) {
    TickerData data;
    data.best_bid = "";
    data.best_ask = "";
    
    double midPrice = data.calculateMidPrice();
    
    EXPECT_EQ(midPrice, 0.0);
}

TEST_F(TickerDataTest, ToCSVFormat) {
    std::string csv = tickerData.toCSV();
    
    EXPECT_FALSE(csv.empty());
    EXPECT_TRUE(csv.find("ticker") != std::string::npos);
    EXPECT_TRUE(csv.find("BTC-USD") != std::string::npos);
    EXPECT_TRUE(csv.find("50000.00") != std::string::npos);
    EXPECT_TRUE(csv.find("49950.00000000") != std::string::npos);
    EXPECT_TRUE(csv.find("49975.00000000") != std::string::npos);
    EXPECT_TRUE(csv.find("50000.00000000") != std::string::npos);
}

TEST_F(TickerDataTest, ToCSVWithSpecialCharacters) {
    TickerData data = tickerData;
    data.product_id = "BTC,USD"; // Contains comma
    data.side = "buy\"sell"; // Contains quote
    
    std::string csv = data.toCSV();
    
    EXPECT_TRUE(csv.find("\"BTC,USD\"") != std::string::npos);
    EXPECT_TRUE(csv.find("\"buy\"\"sell\"") != std::string::npos);
}

TEST_F(TickerDataTest, ToCSVWithNewlines) {
    TickerData data = tickerData;
    data.side = "buy\nsell"; // Contains newline
    
    std::string csv = data.toCSV();
    
    EXPECT_TRUE(csv.find("\"buy\nsell\"") != std::string::npos);
}

TEST_F(TickerDataTest, ToCSVPrecision) {
    TickerData data = tickerData;
    data.price_ema = 123.456789012345;
    data.mid_price_ema = 987.654321098765;
    data.mid_price = 555.123456789;
    
    std::string csv = data.toCSV();
    
    // Should have 8 decimal places
    EXPECT_TRUE(csv.find("123.45678901") != std::string::npos);
    EXPECT_TRUE(csv.find("987.65432110") != std::string::npos);
    EXPECT_TRUE(csv.find("555.12345679") != std::string::npos);
}

TEST_F(TickerDataTest, CSVFieldCount) {
    std::string csv = tickerData.toCSV();
    
    // Count commas to verify field count
    int commaCount = 0;
    for (char c : csv) {
        if (c == ',') {
            commaCount++;
        }
    }
    
    // Should have 17 commas (18 fields total)
    EXPECT_EQ(commaCount, 17);
}
