/**
 * @file AsyncCSVLogger.h
 * @brief Asynchronous CSV logger for high-performance ticker data logging
 */

#ifndef ASYNCCSVLOGGER_H
#define ASYNCCSVLOGGER_H

#include <string>
#include <fstream>
#include <thread>
#include <atomic>
#include <memory>
#include "TickerData.h"
#include "LockFreeRingBuffer.h"

/**
 * @brief Asynchronous CSV logger for ticker data
 * 
 * This class provides high-performance, non-blocking CSV logging by using
 * a dedicated logging thread and lock-free ring buffer. This prevents
 * file I/O operations from blocking the main processing thread.
 */
class AsyncCSVLogger {
private:
    std::string m_filename;                                    ///< Output CSV filename
    std::unique_ptr<std::ofstream> m_file;                     ///< Output file stream
    bool m_headersWritten;                                     ///< Whether CSV headers have been written
    
    // Lock-free ring buffer for async logging
    static constexpr size_t LOG_BUFFER_SIZE = 8192;           ///< Size of log buffer (power of 2)
    LockFreeRingBuffer<TickerData, LOG_BUFFER_SIZE> m_logQueue; ///< Queue for ticker data
    
    // Threading
    std::thread m_logThread;                                   ///< Dedicated logging thread
    std::atomic<bool> m_running;                               ///< Logger running status
    std::atomic<bool> m_ready;                                 ///< Logger ready status
    
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
     * @brief Logging thread function
     */
    void logThreadFunction();

public:
    /**
     * @brief Constructor
     * @param filename Output CSV filename
     */
    explicit AsyncCSVLogger(const std::string& filename);
    
    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~AsyncCSVLogger();
    
    /**
     * @brief Log ticker data to CSV file (non-blocking)
     * @param data Ticker data to log
     * @return True if queued successfully, false if queue is full
     */
    bool logTickerData(const TickerData& data);
    
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
};

#endif // ASYNCCSVLOGGER_H
