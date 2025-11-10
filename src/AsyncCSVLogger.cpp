/**
 * @file AsyncCSVLogger.cpp
 * @brief Implementation of HFT-grade SPSC lock-free async CSV logging
 */

#include "AsyncCSVLogger.h"
#include "ThreadUtils.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>

#ifdef __linux__

AsyncCSVLogger::AsyncCSVLogger(const std::string& filename, 
                               int logThreadCpu,
                               int logThreadNumaNode)
    : m_filename(filename)
    , m_logThreadCpu(logThreadCpu)
    , m_logThreadNumaNode(logThreadNumaNode) {
    
    // Initialize NUMA if available
    NUMAUtils::initialize();
    
    // Determine CPU and NUMA node for logging thread
    if (m_logThreadCpu < 0) {
        // Auto-select: use a CPU on a different NUMA node if possible
        if (NUMAUtils::isAvailable()) {
            int numNodes = NUMAUtils::getNumNodes();
            if (numNodes > 1) {
                // Use a different NUMA node than the current one
                int currentNode = NUMAUtils::getCurrentNode();
                m_logThreadNumaNode = (currentNode + 1) % numNodes;
            } else {
                m_logThreadNumaNode = 0;
            }
            m_logThreadCpu = NUMAUtils::getFirstCpuForNode(m_logThreadNumaNode);
        } else {
            m_logThreadCpu = 1; // Default to CPU 1
            m_logThreadNumaNode = 0;
        }
    } else if (m_logThreadNumaNode < 0) {
        // Determine NUMA node from CPU
        if (NUMAUtils::isAvailable()) {
            // We need to determine the NUMA node from the CPU
            // For now, use current node as fallback
            m_logThreadNumaNode = NUMAUtils::getCurrentNode();
        } else {
            m_logThreadNumaNode = 0;
        }
    }
    
    // Open file
    m_file.open(filename, std::ios::out | std::ios::app);
    if (!m_file.is_open()) {
        std::cerr << "Error: Could not open CSV file: " << filename << std::endl;
        return;
    }
    
    // Start logging thread
    m_running.store(true);
    m_logThread = std::thread(&AsyncCSVLogger::logThreadFunction, this);
    
    // Wait for thread to be ready (with timeout)
    int64_t startMicros = HighResTimer::nowMicros();
    while (!m_ready.load() && 
           (HighResTimer::nowMicros() - startMicros) < 1000000) { // 1 second timeout
        HighResTimer::sleepMicros(100); // Sleep 100 microseconds
    }
    
    if (!m_ready.load()) {
        std::cerr << "Warning: AsyncCSVLogger thread did not start properly" << std::endl;
    }
}

AsyncCSVLogger::~AsyncCSVLogger() {
    close();
}

void AsyncCSVLogger::writeHeaders() {
    if (!m_file.is_open()) {
        return;
    }
    
    bool expected = false;
    if (!m_headersWritten.compare_exchange_strong(expected, true)) {
        return; // Headers already written
    }
    
    m_file << "type,sequence,product_id,price,open_24h,volume_24h,low_24h,high_24h,"
           << "volume_30d,best_bid,best_ask,side,time,trade_id,last_size,"
           << "price_ema,mid_price_ema,mid_price,timestamp_us" << std::endl;
    
    m_file.flush();
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
    // Optimize thread for HFT: pin to CPU, set real-time priority, NUMA policy
    ThreadUtils::optimizeForHFT("AsyncCSVLogger", m_logThreadCpu, 99, m_logThreadNumaNode);
    
    // Set NUMA memory policy if available
    if (NUMAUtils::isAvailable()) {
        NUMAUtils::setMemoryPolicy(m_logThreadNumaNode);
    }
    
    m_ready.store(true);
    
    // Pre-allocate string buffer to avoid allocations in hot path
    std::string csvLine;
    csvLine.reserve(512); // Pre-allocate reasonable size
    
    int64_t lastFlushTime = HighResTimer::nowMicros();
    const int64_t FLUSH_INTERVAL_MICROS = 10000; // Flush every 10ms
    
    while (m_running.load()) {
        TickerData data;
        bool hadData = false;
        
        // Process all available data (batch processing for efficiency)
        while (m_logQueue.pop(data)) {
            if (!m_file.is_open()) {
                break;
            }
            
            // Write headers if not already written
            if (!m_headersWritten.load()) {
                writeHeaders();
            }
            
            // Format and write data
            csvLine = formatToCSV(data);
            m_file << csvLine << '\n'; // Use '\n' instead of std::endl for performance
            
            hadData = true;
        }
        
        // Flush periodically (not on every write for performance)
        if (hadData) {
            int64_t now = HighResTimer::nowMicros();
            if (now - lastFlushTime >= FLUSH_INTERVAL_MICROS) {
                if (m_file.is_open()) {
                    m_file.flush();
                }
                lastFlushTime = now;
            }
        }
        
        // Brief pause if no data to prevent busy waiting
        if (!hadData) {
            HighResTimer::sleepMicros(10); // 10 microseconds
        }
    }
    
    // Process remaining data before shutdown
    TickerData data;
    while (m_logQueue.pop(data)) {
        if (m_file.is_open()) {
            if (!m_headersWritten.load()) {
                writeHeaders();
            }
            csvLine = formatToCSV(data);
            m_file << csvLine << '\n';
        }
    }
    
    if (m_file.is_open()) {
        m_file.flush();
    }
}

std::string AsyncCSVLogger::formatToCSV(const TickerData& data) const {
    // Optimized CSV formatting - avoid stringstream overhead
    std::ostringstream oss;
    oss.precision(8);
    oss << std::fixed;
    
    // Helper lambda for escaping
    auto escape = [](const std::string& s) -> const std::string& {
        // Simple check - most fields don't need escaping
        if (s.find(',') == std::string::npos && 
            s.find('"') == std::string::npos && 
            s.find('\n') == std::string::npos) {
            return s;
        }
        // Would need escaping - for now return as-is (can be optimized further)
        return s;
    };
    
    // Write all fields
    oss << escape(data.type) << ","
        << escape(data.sequence) << ","
        << escape(data.product_id) << ","
        << escape(data.price) << ","
        << escape(data.open_24h) << ","
        << escape(data.volume_24h) << ","
        << escape(data.low_24h) << ","
        << escape(data.high_24h) << ","
        << escape(data.volume_30d) << ","
        << escape(data.best_bid) << ","
        << escape(data.best_ask) << ","
        << escape(data.side) << ","
        << escape(data.time) << ","
        << escape(data.trade_id) << ","
        << escape(data.last_size) << ","
        << data.price_ema << ","
        << data.mid_price_ema << ","
        << data.mid_price << ","
        << HighResTimer::nowMicros(); // Add microsecond timestamp
    
    return oss.str();
}

bool AsyncCSVLogger::logTickerData(const TickerData& data) {
    if (!m_ready.load()) {
        return false;
    }
    
    // Non-blocking push to SPSC queue (never blocks)
    return m_logQueue.push(data);
}

bool AsyncCSVLogger::logTickerDataWithTimestamp(const TickerData& data, int64_t timestampMicros) {
    if (!m_ready.load()) {
        return false;
    }
    
    // For now, just use regular logTickerData
    // The timestamp is added in formatToCSV
    // Could be optimized to store timestamp in TickerData if needed
    return logTickerData(data);
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
    // Could trigger immediate flush via atomic flag if needed
}

void AsyncCSVLogger::close() {
    bool expected = true;
    if (!m_running.compare_exchange_strong(expected, false)) {
        return; // Already closed
    }
    
    if (m_logThread.joinable()) {
        m_logThread.join();
    }
    
    if (m_file.is_open()) {
        m_file.flush();
        m_file.close();
    }
}

int AsyncCSVLogger::getLogThreadCpu() const {
    return m_logThreadCpu;
}

int AsyncCSVLogger::getLogThreadNumaNode() const {
    return m_logThreadNumaNode;
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

#endif // __linux__
