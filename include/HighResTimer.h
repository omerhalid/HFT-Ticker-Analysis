/**
 * @file HighResTimer.h
 * @brief High-resolution timing utilities for microsecond-precision latency measurement
 * 
 * Uses CLOCK_MONOTONIC_RAW on Linux for maximum precision without NTP adjustments.
 */

#ifndef HIGHRESTIMER_H
#define HIGHRESTIMER_H

#include <cstdint>
#include <chrono>

#ifdef __linux__
#include <time.h>
#endif

/**
 * @brief High-resolution timer for microsecond-precision measurements
 * 
 * Provides nanosecond-precision timestamps using the best available clock
 * source on the platform. On Linux, uses CLOCK_MONOTONIC_RAW for maximum
 * precision without NTP adjustments.
 */
class HighResTimer {
public:
    /**
     * @brief Get current timestamp in nanoseconds
     * @return Timestamp in nanoseconds since an arbitrary point
     */
    static int64_t nowNanos();
    
    /**
     * @brief Get current timestamp in microseconds
     * @return Timestamp in microseconds since an arbitrary point
     */
    static int64_t nowMicros();
    
    /**
     * @brief Get current timestamp in milliseconds
     * @return Timestamp in milliseconds since an arbitrary point
     */
    static int64_t nowMillis();
    
    /**
     * @brief Get timestamp difference in nanoseconds
     * @param start Start timestamp (from nowNanos())
     * @param end End timestamp (from nowNanos())
     * @return Difference in nanoseconds
     */
    static int64_t diffNanos(int64_t start, int64_t end);
    
    /**
     * @brief Get timestamp difference in microseconds
     * @param start Start timestamp (from nowNanos())
     * @param end End timestamp (from nowNanos())
     * @return Difference in microseconds
     */
    static int64_t diffMicros(int64_t start, int64_t end);
    
    /**
     * @brief Get timestamp difference in milliseconds
     * @param start Start timestamp (from nowNanos())
     * @param end End timestamp (from nowNanos())
     * @return Difference in milliseconds
     */
    static int64_t diffMillis(int64_t start, int64_t end);
    
    /**
     * @brief Convert nanoseconds to microseconds
     * @param nanos Nanoseconds
     * @return Microseconds
     */
    static int64_t nanosToMicros(int64_t nanos);
    
    /**
     * @brief Convert nanoseconds to milliseconds
     * @param nanos Nanoseconds
     * @return Milliseconds
     */
    static int64_t nanosToMillis(int64_t nanos);
    
    /**
     * @brief Sleep for a specific number of nanoseconds (busy-wait for short durations)
     * @param nanos Nanoseconds to sleep
     */
    static void sleepNanos(int64_t nanos);
    
    /**
     * @brief Sleep for a specific number of microseconds
     * @param micros Microseconds to sleep
     */
    static void sleepMicros(int64_t micros);
};

/**
 * @brief RAII timer for measuring code block latency
 */
class ScopedTimer {
private:
    int64_t m_start;
    const char* m_label;
    
public:
    /**
     * @brief Constructor - starts timer
     * @param label Label for this timer (for logging)
     */
    explicit ScopedTimer(const char* label = nullptr);
    
    /**
     * @brief Destructor - logs elapsed time
     */
    ~ScopedTimer();
    
    /**
     * @brief Get elapsed time in nanoseconds
     * @return Elapsed nanoseconds
     */
    int64_t elapsedNanos() const;
    
    /**
     * @brief Get elapsed time in microseconds
     * @return Elapsed microseconds
     */
    int64_t elapsedMicros() const;
    
    /**
     * @brief Reset the timer
     */
    void reset();
};

#endif // HIGHRESTIMER_H

