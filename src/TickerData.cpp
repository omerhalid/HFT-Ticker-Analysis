/**
 * @file TickerData.cpp
 * @brief Implementation of TickerData structure methods
 */

#include "TickerData.h"
#include <sstream>
#include <iomanip>

double TickerData::calculateMidPrice() const {
    try {
        double bid = std::stod(best_bid);
        double ask = std::stod(best_ask);
        return (bid + ask) / 2.0;
    } catch (const std::exception&) {
        return 0.0;
    }
}

std::string TickerData::toCSV() const {
    std::ostringstream oss;
    
    // Helper lambda to escape CSV fields
    auto escapeField = [](const std::string& field) -> std::string {
        if (field.find(',') != std::string::npos || 
            field.find('"') != std::string::npos || 
            field.find('\n') != std::string::npos) {
            std::string escaped = "\"";
            for (char c : field) {
                if (c == '"') {
                    escaped += "\"\"";
                } else {
                    escaped += c;
                }
            }
            escaped += "\"";
            return escaped;
        }
        return field;
    };
    
    // Write all fields in CSV format
    oss << escapeField(type) << ","
        << escapeField(sequence) << ","
        << escapeField(product_id) << ","
        << escapeField(price) << ","
        << escapeField(open_24h) << ","
        << escapeField(volume_24h) << ","
        << escapeField(low_24h) << ","
        << escapeField(high_24h) << ","
        << escapeField(volume_30d) << ","
        << escapeField(best_bid) << ","
        << escapeField(best_ask) << ","
        << escapeField(side) << ","
        << escapeField(time) << ","
        << escapeField(trade_id) << ","
        << escapeField(last_size) << ","
        << std::fixed << std::setprecision(8) << price_ema << ","
        << std::fixed << std::setprecision(8) << mid_price_ema << ","
        << std::fixed << std::setprecision(8) << mid_price;
    
    return oss.str();
}
