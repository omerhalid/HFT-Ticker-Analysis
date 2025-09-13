/**
 * @file EMACalculator.h
 * @brief Exponential Moving Average calculator for ticker data
 */

#ifndef EMACALCULATOR_H
#define EMACALCULATOR_H

#include <chrono>
#include <mutex>
#include <atomic>

/**
 * @brief Class for calculating Exponential Moving Average (EMA)
 * 
 * This class provides thread-safe EMA calculations for price and mid-price data
 * with a configurable time interval. Uses a 5-second interval by default.
 */
class EMACalculator {
private:
    std::mutex m_mutex;                                    ///< Mutex for thread safety
    std::chrono::seconds m_interval;                       ///< EMA calculation interval
    
    // EMA values
    std::atomic<double> m_priceEMA;                        ///< EMA of price field
    std::atomic<double> m_midPriceEMA;                     ///< EMA of mid-price
    
    // EMA parameters
    double m_alpha;                                        ///< Smoothing factor (2/(n+1))
    bool m_priceInitialized;                               ///< Whether price EMA has been initialized
    bool m_midPriceInitialized;                            ///< Whether mid-price EMA has been initialized
    std::chrono::system_clock::time_point m_priceLastUpdate;    ///< Last price EMA update timestamp
    std::chrono::system_clock::time_point m_midPriceLastUpdate; ///< Last mid-price EMA update timestamp
    
    /**
     * @brief Calculate smoothing factor alpha based on interval
     * @param intervalSeconds Time interval in seconds
     * @return Smoothing factor alpha
     */
    double calculateAlpha(int intervalSeconds) const;
    
    /**
     * @brief Check if enough time has passed for price EMA update
     * @param currentTime Current timestamp
     * @return True if update should be performed
     */
    bool shouldUpdatePrice(const std::chrono::system_clock::time_point& currentTime) const;
    
    /**
     * @brief Check if enough time has passed for mid-price EMA update
     * @param currentTime Current timestamp
     * @return True if update should be performed
     */
    bool shouldUpdateMidPrice(const std::chrono::system_clock::time_point& currentTime) const;

public:
    /**
     * @brief Constructor
     * @param intervalSeconds Time interval for EMA calculation (default: 5 seconds)
     */
    explicit EMACalculator(int intervalSeconds = 5);
    
    /**
     * @brief Destructor
     */
    ~EMACalculator() = default;
    
    /**
     * @brief Update EMA with new price value
     * @param price New price value
     * @param currentTime Current timestamp
     * @return Updated EMA value
     */
    double updatePriceEMA(double price, const std::chrono::system_clock::time_point& currentTime);
    
    /**
     * @brief Update EMA with new mid-price value
     * @param midPrice New mid-price value
     * @param currentTime Current timestamp
     * @return Updated EMA value
     */
    double updateMidPriceEMA(double midPrice, const std::chrono::system_clock::time_point& currentTime);
    
    /**
     * @brief Get current price EMA
     * @return Current price EMA value
     */
    double getPriceEMA() const;
    
    /**
     * @brief Get current mid-price EMA
     * @return Current mid-price EMA value
     */
    double getMidPriceEMA() const;
    
    /**
     * @brief Reset EMA calculations
     */
    void reset();
    
    /**
     * @brief Check if price EMA is initialized
     * @return True if price EMA has been initialized with data
     */
    bool isPriceInitialized() const;
    
    /**
     * @brief Check if mid-price EMA is initialized
     * @return True if mid-price EMA has been initialized with data
     */
    bool isMidPriceInitialized() const;
};

#endif // EMACALCULATOR_H
