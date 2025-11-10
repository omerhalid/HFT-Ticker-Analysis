/**
 * @file AsyncCSVLogger.h
 * @brief HFT-grade SPSC lock-free async CSV logger with microsecond timestamps
 * 
 * Optimized for ultra-low latency with:
 * - True SPSC lock-free ring buffer
 * - Microsecond-precision timestamps
 * - NUMA-aware memory allocation
 * - Thread pinning and real-time scheduling
 * - Zero-copy where possible
 */

#ifndef ASYNCCSVLOGGER_H
#define ASYNCCSVLOGGER_H

#include <string>
#include <fstream>
#include <thread>
#include <atomic>
#include <memory>
#include <cstdint>
#include "TickerData.h"
#include "LockFreeRingBuffer.h"
#include "HighResTimer.h"
#include "NUMAUtils.h"

#ifdef __linux__

/**
 * @brief HFT-grade asynchronous CSV logger for ticker data
 * 
 * This class provides ultra-low latency, non-blocking CSV logging using:
 * - SPSC lock-free ring buffer (no mutexes, no locks)
 * - Dedicated logging thread with CPU pinning
 * - NUMA-aware memory allocation
 * - Microsecond-precision timestamps
 * - Real-time scheduling (SCHED_FIFO)
 * 
 * Producer: Main thread pushes TickerData
 * Consumer: Dedicated logging thread writes to file
 */
class AsyncCSVLogger {
private:
    std::string m_filename;                                    ///< Output CSV filename
    std::ofstream m_file;                                      ///< Output file stream
    std::atomic<bool> m_headersWritten{false};                ///< Whether CSV headers have been written
    
    // Lock-free SPSC ring buffer for async logging (power of 2 for fast modulo)
    static constexpr size_t LOG_BUFFER_SIZE = 16384;          ///< Size of log buffer (power of 2)
    LockFreeRingBuffer<TickerData, LOG_BUFFER_SIZE> m_logQueue; ///< SPSC queue for ticker data
    
    // Threading
    std::thread m_logThread;                                   ///< Dedicated logging thread
    std::atomic<bool> m_running{false};                       ///< Logger running status
    std::atomic<bool> m_ready{false};                          ///< Logger ready status
    
    // Thread configuration
    int m_logThreadCpu;                                        ///< CPU core for logging thread
    int m_logThreadNumaNode;                                    ///< NUMA node for logging thread
    
    /**
     * @brief Write CSV headers to file
     */
    void writeHeaders();
    
    /**
     * @brief Escape CSV field value
     * @param value String value to escape
     * @return Escaped string
     */
    std::string escapeCSVField(const std::string& value) const;
    
    /**
     * @brief Logging thread function (consumer)
     * 
     * Runs on dedicated thread with:
     * - CPU pinning
     * - Real-time scheduling
     * - NUMA memory policy
     */
    void logThreadFunction();
    
    /**
     * @brief Write CSV headers to file
     */
    void writeHeaders();
    
    /**
     * @brief Format TickerData to CSV string (optimized)
     * @param data Ticker data to format
     * @return CSV formatted string
     */
    std::string formatToCSV(const TickerData& data) const;

public:
    /**
     * @brief Constructor
     * @param filename Output CSV filename
     * @param logThreadCpu CPU core for logging thread (-1 for auto)
     * @param logThreadNumaNode NUMA node for logging thread (-1 for auto)
     */
    explicit AsyncCSVLogger(const std::string& filename, 
                           int logThreadCpu = -1,
                           int logThreadNumaNode = -1);
    
    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~AsyncCSVLogger();
    
    /**
     * @brief Log ticker data to CSV file (non-blocking, SPSC)
     * @param data Ticker data to log
     * @return True if queued successfully, false if queue is full
     * 
     * This is the producer side of the SPSC queue. It never blocks.
     */
    bool logTickerData(const TickerData& data);
    
    /**
     * @brief Log ticker data with microsecond timestamp (non-blocking)
     * @param data Ticker data to log
     * @param timestampMicros Microsecond timestamp (from HighResTimer)
     * @return True if queued successfully, false if queue is full
     */
    bool logTickerDataWithTimestamp(const TickerData& data, int64_t timestampMicros);
    
    /**
     * @brief Check if logger is ready for writing
     * @return True if logger is ready
     */
    bool isReady() const;
    
    /**
     * @brief Check if logger is running
     * @return True if logger is running
     */
    bool isRunning() const;
    
    /**
     * @brief Flush the output buffer
     */
    void flush();
    
    /**
     * @brief Close the log file and stop logging thread
     */
    void close();
    
    /**
     * @brief Get the current filename
     * @return Current filename
     */
    std::string getFilename() const;
    
    /**
     * @brief Get current queue size
     * @return Number of items in queue
     */
    size_t getQueueSize() const;
    
    /**
     * @brief Get queue capacity
     * @return Maximum queue capacity
     */
    size_t getQueueCapacity() const;
    
    /**
     * @brief Get logging thread CPU
     * @return CPU core ID
     */
    int getLogThreadCpu() const;
    
    /**
     * @brief Get logging thread NUMA node
     * @return NUMA node ID
     */
    int getLogThreadNumaNode() const;
};

#endif // __linux__

#endif // ASYNCCSVLOGGER_H
