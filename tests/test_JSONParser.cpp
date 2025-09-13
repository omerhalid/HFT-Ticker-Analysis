/**
 * @file test_JSONParser.cpp
 * @brief Unit tests for JSONParser class
 * @author HFT Developer
 * @date 2024
 */

#include <gtest/gtest.h>
#include "JSONParser.h"
#include "TickerData.h"

class JSONParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Sample valid ticker JSON
        validTickerJson = R"({
            "type": "ticker",
            "sequence": "12345",
            "product_id": "BTC-USD",
            "price": "50000.00",
            "open_24h": "49000.00",
            "volume_24h": "1000.5",
            "low_24h": "48000.00",
            "high_24h": "51000.00",
            "volume_30d": "30000.0",
            "best_bid": "49999.50",
            "best_ask": "50000.50",
            "side": "buy",
            "time": "2024-01-01T12:00:00.000Z",
            "trade_id": "67890",
            "last_size": "0.1"
        })";
        
        // Invalid JSON
        invalidJson = "invalid json string";
        
        // Non-ticker JSON
        nonTickerJson = R"({
            "type": "subscriptions",
            "channels": ["ticker"]
        })";
    }
    
    std::string validTickerJson;
    std::string invalidJson;
    std::string nonTickerJson;
};

TEST_F(JSONParserTest, ParseValidTickerMessage) {
    TickerData tickerData;
    
    bool result = JSONParser::parseTickerMessage(validTickerJson, tickerData);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(tickerData.type, "ticker");
    EXPECT_EQ(tickerData.sequence, "12345");
    EXPECT_EQ(tickerData.product_id, "BTC-USD");
    EXPECT_EQ(tickerData.price, "50000.00");
    EXPECT_EQ(tickerData.best_bid, "49999.50");
    EXPECT_EQ(tickerData.best_ask, "50000.50");
    EXPECT_EQ(tickerData.side, "buy");
    EXPECT_EQ(tickerData.trade_id, "67890");
    EXPECT_EQ(tickerData.last_size, "0.1");
}

TEST_F(JSONParserTest, ParseInvalidJson) {
    TickerData tickerData;
    
    bool result = JSONParser::parseTickerMessage(invalidJson, tickerData);
    
    EXPECT_FALSE(result);
}

TEST_F(JSONParserTest, ParseNonTickerMessage) {
    TickerData tickerData;
    
    bool result = JSONParser::parseTickerMessage(nonTickerJson, tickerData);
    
    EXPECT_FALSE(result);
}

TEST_F(JSONParserTest, IsTickerMessageValid) {
    EXPECT_TRUE(JSONParser::isTickerMessage(validTickerJson));
    EXPECT_FALSE(JSONParser::isTickerMessage(invalidJson));
    EXPECT_FALSE(JSONParser::isTickerMessage(nonTickerJson));
}

TEST_F(JSONParserTest, CreateSubscriptionMessage) {
    std::string productId = "ETH-USD";
    std::string subscription = JSONParser::createSubscriptionMessage(productId);
    
    EXPECT_FALSE(subscription.empty());
    EXPECT_TRUE(subscription.find("subscribe") != std::string::npos);
    EXPECT_TRUE(subscription.find("ETH-USD") != std::string::npos);
    EXPECT_TRUE(subscription.find("ticker") != std::string::npos);
}

TEST_F(JSONParserTest, MidPriceCalculation) {
    TickerData tickerData;
    
    bool result = JSONParser::parseTickerMessage(validTickerJson, tickerData);
    
    EXPECT_TRUE(result);
    
    double expectedMidPrice = (49999.50 + 50000.50) / 2.0;
    EXPECT_DOUBLE_EQ(tickerData.mid_price, expectedMidPrice);
}

TEST_F(JSONParserTest, HandleMissingFields) {
    std::string incompleteJson = R"({
        "type": "ticker",
        "product_id": "BTC-USD",
        "price": "50000.00"
    })";
    
    TickerData tickerData;
    
    bool result = JSONParser::parseTickerMessage(incompleteJson, tickerData);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(tickerData.type, "ticker");
    EXPECT_EQ(tickerData.product_id, "BTC-USD");
    EXPECT_EQ(tickerData.price, "50000.00");
    EXPECT_EQ(tickerData.sequence, ""); // Should be empty for missing field
    EXPECT_EQ(tickerData.best_bid, ""); // Should be empty for missing field
}

TEST_F(JSONParserTest, HandleNumericFieldsAsStrings) {
    std::string numericJson = R"({
        "type": "ticker",
        "product_id": "BTC-USD",
        "price": 50000.00,
        "best_bid": 49999.50,
        "best_ask": 50000.50
    })";
    
    TickerData tickerData;
    
    bool result = JSONParser::parseTickerMessage(numericJson, tickerData);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(tickerData.price, "50000.000000");
    EXPECT_EQ(tickerData.best_bid, "49999.500000");
    EXPECT_EQ(tickerData.best_ask, "50000.500000");
}
