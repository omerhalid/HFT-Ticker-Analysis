/**
 * @file CSVLogger.cpp
 * @brief Implementation of CSV logging functionality
 */

#include "CSVLogger.h"
#include <iostream>
#include <sstream>

CSVLogger::CSVLogger(const std::string& filename) 
    : m_filename(filename)
    , m_headersWritten(false) {
    
    m_file = std::make_unique<std::ofstream>(filename, std::ios::out | std::ios::app);
    if (!m_file->is_open()) {
        std::cerr << "Error: Could not open CSV file: " << filename << std::endl;
    }
}

CSVLogger::~CSVLogger() {
    close();
}

void CSVLogger::writeHeaders() {
    if (!m_file || !m_file->is_open() || m_headersWritten) {
        return;
    }
    
    *m_file << "type,sequence,product_id,price,open_24h,volume_24h,low_24h,high_24h,"
            << "volume_30d,best_bid,best_ask,side,time,trade_id,last_size,"
            << "price_ema,mid_price_ema,mid_price" << std::endl;
    
    m_file->flush();
    m_headersWritten = true;
}

std::string CSVLogger::escapeCSVField(const std::string& value) const {
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

void CSVLogger::logTickerData(const TickerData& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_file || !m_file->is_open()) {
        return;
    }
    
    // Write headers if not already written
    if (!m_headersWritten) {
        writeHeaders();
    }
    
    // Write data
    *m_file << data.toCSV() << std::endl;
}

bool CSVLogger::isReady() const {
    return m_file && m_file->is_open();
}

void CSVLogger::flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file && m_file->is_open()) {
        m_file->flush();
    }
}

void CSVLogger::close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file && m_file->is_open()) {
        m_file->flush();
        m_file->close();
    }
}

std::string CSVLogger::getFilename() const {
    return m_filename;
}
