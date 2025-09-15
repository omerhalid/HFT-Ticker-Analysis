/**
 * @file AsyncCSVLogger.cpp
 * @brief Implementation of asynchronous CSV logging functionality
 */

#include "AsyncCSVLogger.h"
#include <iostream>
#include <sstream>
#include <chrono>

AsyncCSVLogger::AsyncCSVLogger(const std::string& filename) 
    : m_filename(filename)
    , m_headersWritten(false)
    , m_running(false)
    , m_ready(false) {
    
    // Open file
    m_file.open(filename, std::ios::out | std::ios::app);
    if (!m_file.is_open()) {
        std::cerr << "Error: Could not open CSV file: " << filename << std::endl;
        return;
    }
    
    // Start logging thread
    m_running = true;
    m_logThread = std::thread(&AsyncCSVLogger::logThreadFunction, this);
    
    // Wait for thread to be ready
    auto start = std::chrono::steady_clock::now();
    while (!m_ready.load() && 
           std::chrono::steady_clock::now() - start < std::chrono::seconds(1)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    if (!m_ready.load()) {
        std::cerr << "Warning: AsyncCSVLogger thread did not start properly" << std::endl;
    }
}

AsyncCSVLogger::~AsyncCSVLogger() {
    close();
}

void AsyncCSVLogger::writeHeaders() {
    if (!m_file.is_open() || m_headersWritten) {
        return;
    }
    
    m_file << "type,sequence,product_id,price,open_24h,volume_24h,low_24h,high_24h,"
           << "volume_30d,best_bid,best_ask,side,time,trade_id,last_size,"
           << "price_ema,mid_price_ema,mid_price" << std::endl;
    
    m_file.flush();
    m_headersWritten = true;
}

std::string AsyncCSVLogger::escapeCSVField(const std::string& value) const {
    if (value.find(',') != std::string::npos || 
        value.find('"') != std::string::npos || 
        value.find('\n') != std::string::npos) {
        
        std::string escaped = "\"";
        for (char c : value) {
            if (c == '"') {
                escaped += "\"\"";
            } else {
                escaped += c;
            }
        }
        escaped += "\"";
        return escaped;
    }
    return value;
}

void AsyncCSVLogger::logThreadFunction() {
    // Set thread name for debugging
    #ifdef __linux__
    pthread_setname_np(pthread_self(), "AsyncCSVLogger");
    #endif
    
    m_ready = true;
    
    while (m_running.load()) {
        TickerData data;
        
        // Process all available data
        while (m_logQueue.pop(data)) {
            if (!m_file.is_open()) {
                break;
            }
            
            // Write headers if not already written
            if (!m_headersWritten) {
                writeHeaders();
            }
            
            // Write data
            m_file << data.toCSV() << std::endl;
        }
        
        // Flush periodically
        if (m_file.is_open()) {
            m_file.flush();
        }
        
        // Brief sleep to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    
    // Process remaining data before shutdown
    TickerData data;
    while (m_logQueue.pop(data)) {
        if (m_file.is_open()) {
            if (!m_headersWritten) {
                writeHeaders();
            }
            m_file << data.toCSV() << std::endl;
        }
    }
    
    if (m_file.is_open()) {
        m_file.flush();
    }
}

bool AsyncCSVLogger::logTickerData(const TickerData& data) {
    if (!m_ready.load()) {
        return false;
    }
    
    // Non-blocking push to queue
    if (!m_logQueue.push(data)) {
        // Queue full - drop oldest entry to make space
        TickerData dummy;
        m_logQueue.pop(dummy);
        return m_logQueue.push(data);
    }
    
    return true;
}

bool AsyncCSVLogger::isReady() const {
    return m_ready.load() && m_file.is_open();
}

bool AsyncCSVLogger::isRunning() const {
    return m_running.load();
}

void AsyncCSVLogger::flush() {
    // Flush is handled automatically by the logging thread
    // This method is kept for compatibility
}

void AsyncCSVLogger::close() {
    if (!m_running.load()) {
        return;
    }
    
    m_running = false;
    
    if (m_logThread.joinable()) {
        m_logThread.join();
    }
    
    if (m_file.is_open()) {
        m_file.flush();
        m_file.close();
    }
}

std::string AsyncCSVLogger::getFilename() const {
    return m_filename;
}

size_t AsyncCSVLogger::getQueueSize() const {
    return m_logQueue.size();
}

size_t AsyncCSVLogger::getQueueCapacity() const {
    return m_logQueue.capacity();
}
