/**
 * @file CoinbaseTickerAnalyzer.cpp
 * @brief Implementation of main application class for Coinbase ticker analysis
 */

#include "CoinbaseTickerAnalyzer.h"
#include <iostream>
#include <chrono>

CoinbaseTickerAnalyzer::CoinbaseTickerAnalyzer(const std::string& productId, 
                                             const std::string& csvFilename)
    : m_productId(productId)
    , m_csvFilename(csvFilename)
    , m_running(false)
    , m_processingEnabled(false) {
}

CoinbaseTickerAnalyzer::~CoinbaseTickerAnalyzer() {
    stop();
}

bool CoinbaseTickerAnalyzer::initializeComponents() {
    try {
        // Initialize WebSocket client
        m_websocketClient = std::make_unique<WebSocketClient>();
        m_websocketClient->setMessageCallback([this](const std::string& message) {
            handleWebSocketMessage(message);
        });
        
        // Initialize EMA calculator
        m_emaCalculator = std::make_unique<EMACalculator>(5); // 5-second interval
        
        // Initialize CSV logger
        m_csvLogger = std::make_unique<CSVLogger>(m_csvFilename);
        
        if (!m_csvLogger->isReady()) {
            std::cerr << "Failed to initialize CSV logger" << std::endl;
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize components: " << e.what() << std::endl;
        return false;
    }
}

void CoinbaseTickerAnalyzer::cleanupComponents() {
    m_processingEnabled.store(false);
    
    // Notify processing thread to wake up
    m_queueCondition.notify_all();
    
    // Wait for processing thread to finish
    if (m_dataProcessingThread.joinable()) {
        m_dataProcessingThread.join();
    }
    
    // Clean up components
    if (m_websocketClient) {
        m_websocketClient->disconnect();
    }
    
    if (m_csvLogger) {
        m_csvLogger->close();
    }
}

void CoinbaseTickerAnalyzer::handleWebSocketMessage(const std::string& message) {
    TickerData tickerData;
    
    if (JSONParser::parseTickerMessage(message, tickerData)) {
        // Add to processing queue
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_dataQueue.push(tickerData);
        }
        
        // Notify processing thread
        m_queueCondition.notify_one();
    }
}

void CoinbaseTickerAnalyzer::processDataThread() {
    while (m_processingEnabled.load()) {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        
        // Wait for data or shutdown signal
        m_queueCondition.wait(lock, [this] {
            return !m_dataQueue.empty() || !m_processingEnabled.load();
        });
        
        // Process all available data
        while (!m_dataQueue.empty() && m_processingEnabled.load()) {
            TickerData data = m_dataQueue.front();
            m_dataQueue.pop();
            lock.unlock();
            
            processTickerData(data);
            
            lock.lock();
        }
    }
}

void CoinbaseTickerAnalyzer::processTickerData(TickerData& data) {
    try {
        // Calculate EMAs
        data.price_ema = m_emaCalculator->updatePriceEMA(
            std::stod(data.price), data.timestamp);
        data.mid_price_ema = m_emaCalculator->updateMidPriceEMA(
            data.mid_price, data.timestamp);
        
        // Log to CSV
        m_csvLogger->logTickerData(data);
        
        // Print to console for monitoring
        std::cout << "Processed: " << data.product_id 
                  << " Price: " << data.price
                  << " Price EMA: " << data.price_ema
                  << " Mid-Price EMA: " << data.mid_price_ema << std::endl;
                  
    } catch (const std::exception& e) {
        std::cerr << "Error processing ticker data: " << e.what() << std::endl;
    }
}

bool CoinbaseTickerAnalyzer::start() {
    if (m_running.load()) {
        std::cout << "Analyzer is already running" << std::endl;
        return true;
    }
    
    if (!initializeComponents()) {
        return false;
    }
    
    // Start data processing thread
    m_processingEnabled.store(true);
    m_dataProcessingThread = std::thread(&CoinbaseTickerAnalyzer::processDataThread, this);
    
    // Connect to Coinbase WebSocket
    const std::string coinbaseUri = "wss://ws-feed.exchange.coinbase.com";
    if (!m_websocketClient->connect(coinbaseUri)) {
        std::cerr << "Failed to connect to Coinbase WebSocket" << std::endl;
        cleanupComponents();
        return false;
    }
    
    // Wait a moment for connection to establish
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Subscribe to ticker channel
    if (!m_websocketClient->subscribeToTicker(m_productId)) {
        std::cerr << "Failed to subscribe to ticker channel" << std::endl;
        cleanupComponents();
        return false;
    }
    
    m_running.store(true);
    std::cout << "Coinbase Ticker Analyzer started successfully" << std::endl;
    std::cout << "Monitoring product: " << m_productId << std::endl;
    std::cout << "Logging to: " << m_csvFilename << std::endl;
    
    return true;
}

void CoinbaseTickerAnalyzer::stop() {
    if (!m_running.load()) {
        return;
    }
    
    std::cout << "Stopping Coinbase Ticker Analyzer..." << std::endl;
    
    m_running.store(false);
    cleanupComponents();
    
    std::cout << "Coinbase Ticker Analyzer stopped" << std::endl;
}

bool CoinbaseTickerAnalyzer::isRunning() const {
    return m_running.load();
}

std::string CoinbaseTickerAnalyzer::getProductId() const {
    return m_productId;
}

void CoinbaseTickerAnalyzer::setProductId(const std::string& productId) {
    m_productId = productId;
}

std::string CoinbaseTickerAnalyzer::getCsvFilename() const {
    return m_csvFilename;
}

void CoinbaseTickerAnalyzer::setCsvFilename(const std::string& filename) {
    m_csvFilename = filename;
}

std::string CoinbaseTickerAnalyzer::getStatistics() const {
    std::ostringstream oss;
    oss << "Product ID: " << m_productId << std::endl;
    oss << "CSV File: " << m_csvFilename << std::endl;
    oss << "Running: " << (m_running.load() ? "Yes" : "No") << std::endl;
    oss << "Connected: " << (m_websocketClient && m_websocketClient->isConnected() ? "Yes" : "No") << std::endl;
    
    if (m_emaCalculator) {
        oss << "Price EMA: " << m_emaCalculator->getPriceEMA() << std::endl;
        oss << "Mid-Price EMA: " << m_emaCalculator->getMidPriceEMA() << std::endl;
    }
    
    return oss.str();
}
