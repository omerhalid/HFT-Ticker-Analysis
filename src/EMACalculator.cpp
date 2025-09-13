/**
 * @file EMACalculator.cpp
 * @brief Implementation of EMA calculator for ticker data
 */

#include "EMACalculator.h"
#include <algorithm>
#include <cmath>

EMACalculator::EMACalculator(int intervalSeconds) 
    : m_interval(std::chrono::seconds(intervalSeconds))
    , m_priceEMA(0.0)
    , m_midPriceEMA(0.0)
    , m_alpha(calculateAlpha(intervalSeconds))
    , m_priceInitialized(false)
    , m_midPriceInitialized(false) {
}

double EMACalculator::calculateAlpha(int intervalSeconds) const {
    // For EMA: alpha = 2 / (n + 1)
    // For 5-second interval, we use n = 5
    return 2.0 / (intervalSeconds + 1.0);
}

bool EMACalculator::shouldUpdatePrice(const std::chrono::system_clock::time_point& currentTime) const {
    if (!m_priceInitialized) {
        return true;
    }
    
    auto timeDiff = currentTime - m_priceLastUpdate;
    return timeDiff >= m_interval;
}

bool EMACalculator::shouldUpdateMidPrice(const std::chrono::system_clock::time_point& currentTime) const {
    if (!m_midPriceInitialized) {
        return true;
    }
    
    auto timeDiff = currentTime - m_midPriceLastUpdate;
    return timeDiff >= m_interval;
}

double EMACalculator::updatePriceEMA(double price, const std::chrono::system_clock::time_point& currentTime) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!shouldUpdatePrice(currentTime)) {
        return m_priceEMA.load();
    }
    
    if (!m_priceInitialized) {
        m_priceEMA.store(price);
        m_priceInitialized = true;
    } else {
        double currentEMA = m_priceEMA.load();
        double newEMA = m_alpha * price + (1.0 - m_alpha) * currentEMA;
        m_priceEMA.store(newEMA);
    }
    
    m_priceLastUpdate = currentTime;
    return m_priceEMA.load();
}

double EMACalculator::updateMidPriceEMA(double midPrice, const std::chrono::system_clock::time_point& currentTime) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!shouldUpdateMidPrice(currentTime)) {
        return m_midPriceEMA.load();
    }
    
    if (!m_midPriceInitialized) {
        m_midPriceEMA.store(midPrice);
        m_midPriceInitialized = true;
    } else {
        double currentEMA = m_midPriceEMA.load();
        double newEMA = m_alpha * midPrice + (1.0 - m_alpha) * currentEMA;
        m_midPriceEMA.store(newEMA);
    }
    
    m_midPriceLastUpdate = currentTime;
    return m_midPriceEMA.load();
}

double EMACalculator::getPriceEMA() const {
    return m_priceEMA.load();
}

double EMACalculator::getMidPriceEMA() const {
    return m_midPriceEMA.load();
}

void EMACalculator::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_priceEMA.store(0.0);
    m_midPriceEMA.store(0.0);
    m_priceInitialized = false;
    m_midPriceInitialized = false;
}

bool EMACalculator::isPriceInitialized() const {
    return m_priceInitialized;
}

bool EMACalculator::isMidPriceInitialized() const {
    return m_midPriceInitialized;
}
