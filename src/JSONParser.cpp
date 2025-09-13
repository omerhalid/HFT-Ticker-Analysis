/**
 * @file JSONParser.cpp
 * @brief Implementation of JSON parsing functionality
 */

#include "JSONParser.h"
#include <iostream>
#include <sstream>
#include <iomanip>

bool JSONParser::parseTickerMessage(const std::string& jsonString, TickerData& tickerData) {
    try {
        nlohmann::json json = nlohmann::json::parse(jsonString);
        
        // Check if it's a ticker message
        if (!isTickerMessage(jsonString)) {
            return false;
        }
        
        // Parse all fields
        tickerData.type = getStringValue(json, "type");
        tickerData.sequence = getStringValue(json, "sequence");
        tickerData.product_id = getStringValue(json, "product_id");
        tickerData.price = getStringValue(json, "price");
        tickerData.open_24h = getStringValue(json, "open_24h");
        tickerData.volume_24h = getStringValue(json, "volume_24h");
        tickerData.low_24h = getStringValue(json, "low_24h");
        tickerData.high_24h = getStringValue(json, "high_24h");
        tickerData.volume_30d = getStringValue(json, "volume_30d");
        tickerData.best_bid = getStringValue(json, "best_bid");
        tickerData.best_ask = getStringValue(json, "best_ask");
        tickerData.side = getStringValue(json, "side");
        tickerData.time = getStringValue(json, "time");
        tickerData.trade_id = getStringValue(json, "trade_id");
        tickerData.last_size = getStringValue(json, "last_size");
        
        // Calculate mid-price
        tickerData.mid_price = tickerData.calculateMidPrice();
        
        // Parse timestamp
        tickerData.timestamp = parseTimestamp(tickerData.time);
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
    }
}

std::string JSONParser::createSubscriptionMessage(const std::string& productId) {
    nlohmann::json subscription;
    subscription["type"] = "subscribe";
    subscription["product_ids"] = nlohmann::json::array({productId});
    subscription["channels"] = nlohmann::json::array({"ticker"});
    
    return subscription.dump();
}

bool JSONParser::isTickerMessage(const std::string& jsonString) {
    try {
        nlohmann::json json = nlohmann::json::parse(jsonString);
        
        // Check if it has the required fields for a ticker message
        return json.contains("type") && 
               json.contains("product_id") && 
               json.contains("price") &&
               json["type"] == "ticker";
    } catch (const std::exception&) {
        return false;
    }
}

std::string JSONParser::getStringValue(const nlohmann::json& json, 
                                     const std::string& key, 
                                     const std::string& defaultValue) {
    try {
        if (json.contains(key) && json[key].is_string()) {
            return json[key].get<std::string>();
        } else if (json.contains(key) && json[key].is_number()) {
            // Convert number to string
            return std::to_string(json[key].get<double>());
        }
    } catch (const std::exception&) {
        // Return default value on any error
    }
    return defaultValue;
}

double JSONParser::getDoubleValue(const nlohmann::json& json, 
                                const std::string& key, 
                                double defaultValue) {
    try {
        if (json.contains(key)) {
            if (json[key].is_number()) {
                return json[key].get<double>();
            } else if (json[key].is_string()) {
                return stringToDouble(json[key].get<std::string>(), defaultValue);
            }
        }
    } catch (const std::exception&) {
        // Return default value on any error
    }
    return defaultValue;
}

std::chrono::system_clock::time_point JSONParser::parseTimestamp(const std::string& timeString) {
    try {
        // Parse ISO 8601 timestamp (e.g., "2024-01-01T12:00:00.000Z")
        std::tm tm = {};
        std::istringstream ss(timeString);
        
        // Remove 'Z' if present
        std::string cleanTime = timeString;
        if (cleanTime.back() == 'Z') {
            cleanTime.pop_back();
        }
        
        // Parse the timestamp
        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        
        if (ss.fail()) {
            // Try with milliseconds
            ss.clear();
            ss.str(cleanTime);
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        }
        
        // Convert to time_point
        auto time_t = std::mktime(&tm);
        return std::chrono::system_clock::from_time_t(time_t);
    } catch (const std::exception&) {
        // Return current time if parsing fails
        return std::chrono::system_clock::now();
    }
}

double JSONParser::stringToDouble(const std::string& str, double defaultValue) {
    try {
        return std::stod(str);
    } catch (const std::exception&) {
        return defaultValue;
    }
}
