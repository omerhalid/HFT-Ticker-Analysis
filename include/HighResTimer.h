/**
 * @file HighResTimer.h
 * @brief HFT-grade high-resolution timing utilities with RDTSC support
 * 
 * Uses RDTSC (Read Time-Stamp Counter) on x86/x86_64 for cycle-accurate,
 * ultra-low latency timing. Falls back to CLOCK_MONOTONIC_RAW on Linux
 * or std::chrono on other platforms.
 */

#ifndef HIGHRESTIMER_H
#define HIGHRESTIMER_H

#include <cstdint>
#include <chrono>

#ifdef __linux__
#include <time.h>
#endif

#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
#define HAVE_RDTSC 1
#include <x86intrin.h>
#endif

/**
 * @brief HFT-grade high-resolution timer with RDTSC support
 * 
 * Provides cycle-accurate, nanosecond-precision timestamps using:
 * - RDTSC on x86/x86_64 (fastest, no system call overhead)
 * - CLOCK_MONOTONIC_RAW on Linux (fallback)
 * - std::chrono on other platforms
 * 
 * RDTSC is calibrated at startup to convert cycles to nanoseconds.
 */
class HighResTimer {
private:
    static bool s_initialized;
    static double s_tscFrequencyGHz;  // TSC frequency in GHz
    static int64_t s_tscOffset;      // Offset for calibration
    
    /**
     * @brief Initialize RDTSC calibration (called once at startup)
     */
    static void initializeRDTSC();
    
    /**
     * @brief Read TSC register (x86/x86_64 only)
     * @return TSC value (cycles)
     */
    static inline uint64_t readTSC() {
#if HAVE_RDTSC
        return __rdtsc();
#else
        return 0;
#endif
    }
    
public:
    /**
     * @brief Initialize timer (call once at startup for RDTSC calibration)
     */
    static void initialize();
    /**
     * @brief Get current timestamp in nanoseconds
     * @return Timestamp in nanoseconds since an arbitrary point
     * 
     * Uses RDTSC on x86/x86_64 for maximum performance (no system call).
     * Falls back to clock_gettime(CLOCK_MONOTONIC_RAW) on Linux.
     */
    static int64_t nowNanos();
    
    /**
     * @brief Get current TSC value (cycles) - x86/x86_64 only
     * @return TSC cycles, or 0 if not available
     */
    static uint64_t nowCycles();
    
    /**
     * @brief Convert TSC cycles to nanoseconds
     * @param cycles TSC cycles
     * @return Nanoseconds
     */
    static int64_t cyclesToNanos(uint64_t cycles);
    
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

