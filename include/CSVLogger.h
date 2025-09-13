/**
 * @file CSVLogger.h
 * @brief CSV logging functionality for ticker data
 */

#ifndef CSVLOGGER_H
#define CSVLOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include "TickerData.h"

/**
 * @brief Thread-safe CSV logger for ticker data
 * 
 * This class provides thread-safe logging of ticker data to CSV files.
 * It automatically creates the CSV file with headers and handles concurrent writes.
 */
class CSVLogger {
private:
    std::string m_filename;                    ///< Output CSV filename
    std::unique_ptr<std::ofstream> m_file;     ///< Output file stream
    std::mutex m_mutex;                        ///< Mutex for thread safety
    bool m_headersWritten;                     ///< Whether CSV headers have been written
    
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

public:
    /**
     * @brief Constructor
     * @param filename Output CSV filename
     */
    explicit CSVLogger(const std::string& filename);
    
    /**
     * @brief Destructor - ensures file is properly closed
     */
    ~CSVLogger();
    
    /**
     * @brief Log ticker data to CSV file
     * @param data Ticker data to log
     */
    void logTickerData(const TickerData& data);
    
    /**
     * @brief Check if logger is ready for writing
     * @return True if file is open and ready
     */
    bool isReady() const;
    
    /**
     * @brief Flush the output buffer
     */
    void flush();
    
    /**
     * @brief Close the log file
     */
    void close();
    
    /**
     * @brief Get the current filename
     * @return Current filename
     */
    std::string getFilename() const;
};

#endif // CSVLOGGER_H
