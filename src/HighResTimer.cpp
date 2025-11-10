/**
 * @file HighResTimer.cpp
 * @brief Implementation of high-resolution timing utilities
 */

#include "HighResTimer.h"
#include <iostream>
#include <thread>

#ifdef __linux__
#include <time.h>
#include <unistd.h>
#endif

int64_t HighResTimer::nowNanos() {
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
    
    // For very short durations, use busy-wait
    if (nanos < 10000) { // Less than 10 microseconds
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

