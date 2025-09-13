/**
 * @file TickerData.h
 * @brief Data structures for Coinbase ticker information
 */

#ifndef TICKERDATA_H
#define TICKERDATA_H

#include <string>
#include <chrono>

/**
 * @brief Structure to hold ticker data from Coinbase WebSocket
 * 
 * This structure contains all the fields from the Coinbase ticker message
 * plus calculated EMA values for price and mid-price.
 */
struct TickerData {
    std::string type;           ///< Message type (usually "ticker")
    std::string sequence;       ///< Sequence number
    std::string product_id;     ///< Trading pair (e.g., "BTC-USD")
    std::string price;          ///< Last trade price
    std::string open_24h;       ///< 24-hour opening price
    std::string volume_24h;     ///< 24-hour volume
    std::string low_24h;        ///< 24-hour low price
    std::string high_24h;       ///< 24-hour high price
    std::string volume_30d;     ///< 30-day volume
    std::string best_bid;       ///< Best bid price
    std::string best_ask;       ///< Best ask price
    std::string side;           ///< Trade side (buy/sell)
    std::string time;           ///< Timestamp
    std::string trade_id;       ///< Trade ID
    std::string last_size;      ///< Last trade size
    
    // Calculated fields
    double price_ema;           ///< EMA of price field
    double mid_price_ema;       ///< EMA of mid-price (best_bid + best_ask) / 2
    double mid_price;           ///< Current mid-price
    
    // Timestamp for internal use
    std::chrono::system_clock::time_point timestamp;
    
    /**
     * @brief Default constructor
     */
    TickerData() : price_ema(0.0), mid_price_ema(0.0), mid_price(0.0) {}
    
    /**
     * @brief Calculate mid-price from best bid and ask
     * @return Calculated mid-price
     */
    double calculateMidPrice() const;
    
    /**
     * @brief Convert to CSV string for logging
     * @return CSV formatted string
     */
    std::string toCSV() const;
};

#endif // TICKERDATA_H
