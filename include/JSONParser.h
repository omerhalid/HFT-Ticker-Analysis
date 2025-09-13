/**
 * @file JSONParser.h
 * @brief JSON parsing functionality for Coinbase ticker messages
 */

#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <string>
#include <nlohmann/json.hpp>
#include "TickerData.h"

/**
 * @brief JSON parser for Coinbase ticker messages
 * 
 * This class handles parsing of JSON messages received from Coinbase WebSocket
 * and converts them to TickerData structures.
 */
class JSONParser {
public:
    /**
     * @brief Parse JSON string to TickerData structure
     * @param jsonString JSON string to parse
     * @param tickerData Output TickerData structure
     * @return True if parsing was successful
     */
    static bool parseTickerMessage(const std::string& jsonString, TickerData& tickerData);
    
    /**
     * @brief Create subscription message JSON
     * @param productId Product ID to subscribe to (e.g., "BTC-USD")
     * @return JSON subscription message string
     */
    static std::string createSubscriptionMessage(const std::string& productId);
    
    /**
     * @brief Validate if JSON string is a ticker message
     * @param jsonString JSON string to validate
     * @return True if it's a valid ticker message
     */
    static bool isTickerMessage(const std::string& jsonString);
    
    /**
     * @brief Extract string value from JSON with error handling
     * @param json JSON object
     * @param key Key to extract
     * @param defaultValue Default value if key not found
     * @return Extracted string value or default
     */
    static std::string getStringValue(const nlohmann::json& json, 
                                    const std::string& key, 
                                    const std::string& defaultValue = "");
    
    /**
     * @brief Extract double value from JSON with error handling
     * @param json JSON object
     * @param key Key to extract
     * @param defaultValue Default value if key not found
     * @return Extracted double value or default
     */
    static double getDoubleValue(const nlohmann::json& json, 
                               const std::string& key, 
                               double defaultValue = 0.0);

private:
    /**
     * @brief Parse timestamp string to system_clock time_point
     * @param timeString ISO 8601 timestamp string
     * @return Parsed time_point
     */
    static std::chrono::system_clock::time_point parseTimestamp(const std::string& timeString);
    
    /**
     * @brief Convert string to double with error handling
     * @param str String to convert
     * @param defaultValue Default value if conversion fails
     * @return Converted double value or default
     */
    static double stringToDouble(const std::string& str, double defaultValue = 0.0);
};

#endif // JSONPARSER_H
