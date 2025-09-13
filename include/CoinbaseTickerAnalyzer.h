/**
 * @file CoinbaseTickerAnalyzer.h
 * @brief Main application class for Coinbase ticker analysis
 */

#ifndef COINBASETICKERANALYZER_H
#define COINBASETICKERANALYZER_H

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "WebSocketClient.h"
#include "JSONParser.h"
#include "EMACalculator.h"
#include "CSVLogger.h"
#include "TickerData.h"

/**
 * @brief Main application class for Coinbase ticker analysis
 * 
 * This class orchestrates the entire ticker analysis process:
 * - WebSocket connection to Coinbase
 * - JSON message parsing
 * - EMA calculations
 * - CSV logging
 * - Multithreaded data processing
 */
class CoinbaseTickerAnalyzer {
private:
    // Core components
    std::unique_ptr<WebSocketClient> m_websocketClient;    ///< WebSocket client
    std::unique_ptr<EMACalculator> m_emaCalculator;       ///< EMA calculator
    std::unique_ptr<CSVLogger> m_csvLogger;               ///< CSV logger
    
    // Threading components
    std::thread m_dataProcessingThread;                   ///< Data processing thread
    std::queue<TickerData> m_dataQueue;                   ///< Queue for ticker data
    std::mutex m_queueMutex;                              ///< Mutex for queue access
    std::condition_variable m_queueCondition;             ///< Condition variable for queue
    std::atomic<bool> m_running;                          ///< Application running status
    std::atomic<bool> m_processingEnabled;                ///< Data processing enabled flag
    
    // Configuration
    std::string m_productId;                              ///< Product ID to analyze
    std::string m_csvFilename;                            ///< CSV output filename
    
    /**
     * @brief Handle incoming WebSocket message
     * @param message Received message string
     */
    void handleWebSocketMessage(const std::string& message);
    
    /**
     * @brief Process ticker data in separate thread
     */
    void processDataThread();
    
    /**
     * @brief Process single ticker data item
     * @param data Ticker data to process
     */
    void processTickerData(TickerData& data);
    
    /**
     * @brief Initialize all components
     * @return True if initialization successful
     */
    bool initializeComponents();
    
    /**
     * @brief Cleanup all components
     */
    void cleanupComponents();

public:
    /**
     * @brief Constructor
     * @param productId Product ID to analyze (e.g., "BTC-USD")
     * @param csvFilename Output CSV filename
     */
    CoinbaseTickerAnalyzer(const std::string& productId = "BTC-USD", 
                          const std::string& csvFilename = "ticker_data.csv");
    
    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~CoinbaseTickerAnalyzer();
    
    /**
     * @brief Start the ticker analysis
     * @return True if started successfully
     */
    bool start();
    
    /**
     * @brief Stop the ticker analysis
     */
    void stop();
    
    /**
     * @brief Check if analyzer is running
     * @return True if running
     */
    bool isRunning() const;
    
    /**
     * @brief Get current product ID
     * @return Current product ID
     */
    std::string getProductId() const;
    
    /**
     * @brief Set product ID
     * @param productId New product ID
     */
    void setProductId(const std::string& productId);
    
    /**
     * @brief Get current CSV filename
     * @return Current CSV filename
     */
    std::string getCsvFilename() const;
    
    /**
     * @brief Set CSV filename
     * @param filename New CSV filename
     */
    void setCsvFilename(const std::string& filename);
    
    /**
     * @brief Get statistics about processed data
     * @return String containing statistics
     */
    std::string getStatistics() const;
};

#endif // COINBASETICKERANALYZER_H
