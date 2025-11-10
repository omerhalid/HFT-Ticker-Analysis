/**
 * @file HighResTimer.cpp
 * @brief Implementation of HFT-grade high-resolution timing with RDTSC
 */

#include "HighResTimer.h"
#include <iostream>
#include <thread>
#include <cmath>

#if HAVE_RDTSC
#include <immintrin.h>  // For _mm_pause()
#endif

#ifdef __linux__
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#endif

// Static members
bool HighResTimer::s_initialized = false;
double HighResTimer::s_tscFrequencyGHz = 0.0;
int64_t HighResTimer::s_tscOffset = 0;

void HighResTimer::initialize() {
    if (s_initialized) {
        return;
    }
    
    initializeRDTSC();
    s_initialized = true;
}

void HighResTimer::initializeRDTSC() {
#if HAVE_RDTSC
    // Calibrate RDTSC by comparing with CLOCK_MONOTONIC_RAW
    // This gives us the TSC frequency and an offset
    #ifdef __linux__
    struct timespec ts1, ts2;
    uint64_t tsc1, tsc2;
    
    // Read TSC and wall clock time
    tsc1 = readTSC();
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts1);
    
    // Wait a bit (100ms) for accurate calibration
    usleep(100000);
    
    tsc2 = readTSC();
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts2);
    
    // Calculate TSC frequency
    uint64_t tscDiff = tsc2 - tsc1;
    int64_t timeDiffNanos = (ts2.tv_sec - ts1.tv_sec) * 1000000000LL + 
                           (ts2.tv_nsec - ts1.tv_nsec);
    
    if (tscDiff > 0 && timeDiffNanos > 0) {
        // Calculate TSC frequency: cycles per nanosecond
        s_tscFrequencyGHz = static_cast<double>(tscDiff) / static_cast<double>(timeDiffNanos);
        
        // Calculate offset: what wall clock time corresponds to TSC=0?
        int64_t wallTime1Nanos = ts1.tv_sec * 1000000000LL + ts1.tv_nsec;
        s_tscOffset = wallTime1Nanos - static_cast<int64_t>(static_cast<double>(tsc1) / s_tscFrequencyGHz);
    } else {
        // Fallback: assume 2.5 GHz (common CPU frequency)
        s_tscFrequencyGHz = 2.5;
        s_tscOffset = 0;
    }
    #else
    // Non-Linux: assume 2.5 GHz
    s_tscFrequencyGHz = 2.5;
    s_tscOffset = 0;
    #endif
#else
    // No RDTSC support
    s_tscFrequencyGHz = 0.0;
    s_tscOffset = 0;
#endif
}

uint64_t HighResTimer::nowCycles() {
#if HAVE_RDTSC
    if (!s_initialized) {
        initialize();
    }
    return readTSC();
#else
    return 0;
#endif
}

int64_t HighResTimer::cyclesToNanos(uint64_t cycles) {
#if HAVE_RDTSC
    if (!s_initialized) {
        initialize();
    }
    if (s_tscFrequencyGHz > 0.0) {
        // Convert cycles to nanoseconds: cycles / (cycles per nanosecond)
        return static_cast<int64_t>(static_cast<double>(cycles) / s_tscFrequencyGHz);
    }
#endif
    return 0;
}

int64_t HighResTimer::nowNanos() {
#if HAVE_RDTSC
    if (!s_initialized) {
        initialize();
    }
    
    if (s_tscFrequencyGHz > 0.0) {
        // Use RDTSC for ultra-low latency (no system call)
        uint64_t cycles = readTSC();
        return static_cast<int64_t>(static_cast<double>(cycles) / s_tscFrequencyGHz) + s_tscOffset;
    }
#endif

    // Fallback to clock_gettime
#ifdef __linux__
    struct timespec ts;
    // Use CLOCK_MONOTONIC_RAW for maximum precision without NTP adjustments
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == 0) {
        return static_cast<int64_t>(ts.tv_sec) * 1000000000LL + static_cast<int64_t>(ts.tv_nsec);
    }
    // Fallback to CLOCK_MONOTONIC
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return static_cast<int64_t>(ts.tv_sec) * 1000000000LL + static_cast<int64_t>(ts.tv_nsec);
    }
#endif
    // Fallback to std::chrono
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}

int64_t HighResTimer::nowMicros() {
    return nanosToMicros(nowNanos());
}

int64_t HighResTimer::nowMillis() {
    return nanosToMillis(nowNanos());
}

int64_t HighResTimer::diffNanos(int64_t start, int64_t end) {
    return end - start;
}

int64_t HighResTimer::diffMicros(int64_t start, int64_t end) {
    return nanosToMicros(end - start);
}

int64_t HighResTimer::diffMillis(int64_t start, int64_t end) {
    return nanosToMillis(end - start);
}

int64_t HighResTimer::nanosToMicros(int64_t nanos) {
    return nanos / 1000;
}

int64_t HighResTimer::nanosToMillis(int64_t nanos) {
    return nanos / 1000000;
}

void HighResTimer::sleepNanos(int64_t nanos) {
    if (nanos <= 0) {
        return;
    }
    
    // For very short durations, use busy-wait with RDTSC for precision
    if (nanos < 10000) { // Less than 10 microseconds
#if HAVE_RDTSC
        if (s_tscFrequencyGHz > 0.0) {
            // Use RDTSC for precise busy-wait
            uint64_t targetCycles = static_cast<uint64_t>(static_cast<double>(nanos) * s_tscFrequencyGHz);
            uint64_t startCycles = readTSC();
            uint64_t target = startCycles + targetCycles;
            
            while (readTSC() < target) {
                // Busy-wait with CPU pause instruction
                _mm_pause();
            }
            return;
        }
#endif
        // Fallback: use nanos-based busy-wait
        int64_t start = nowNanos();
        while (nowNanos() - start < nanos) {
            // Busy-wait with CPU pause instruction (x86/x86_64)
            #if defined(__x86_64__) || defined(__i386__)
            asm volatile("pause" ::: "memory");
            #elif defined(__aarch64__)
            asm volatile("yield" ::: "memory");
            #else
            std::this_thread::yield();
            #endif
        }
        return;
    }
    
#ifdef __linux__
    struct timespec ts;
    ts.tv_sec = nanos / 1000000000LL;
    ts.tv_nsec = nanos % 1000000000LL;
    nanosleep(&ts, nullptr);
#else
    std::this_thread::sleep_for(std::chrono::nanoseconds(nanos));
#endif
}

void HighResTimer::sleepMicros(int64_t micros) {
    sleepNanos(micros * 1000);
}

ScopedTimer::ScopedTimer(const char* label)
    : m_start(HighResTimer::nowNanos())
    , m_label(label) {
}

ScopedTimer::~ScopedTimer() {
    if (m_label) {
        int64_t elapsed = elapsedMicros();
        std::cout << "[ScopedTimer] " << m_label << ": " << elapsed << " us" << std::endl;
    }
}

int64_t ScopedTimer::elapsedNanos() const {
    return HighResTimer::diffNanos(m_start, HighResTimer::nowNanos());
}

int64_t ScopedTimer::elapsedMicros() const {
    return HighResTimer::diffMicros(m_start, HighResTimer::nowNanos());
}

void ScopedTimer::reset() {
    m_start = HighResTimer::nowNanos();
}

