/**
 * @file ThreadUtils.h
 * @brief Simple thread optimization utilities
 */

#ifndef THREADUTILS_H
#define THREADUTILS_H

#include <string>

/**
 * @brief Simple utility class for thread optimization
 */
class ThreadUtils {
public:
    /**
     * @brief Optimize thread for HFT (sets name, priority, and CPU affinity)
     * @param threadName Name for the thread
     * @param cpuCore CPU core to bind to (default: 0)
     * @param priority Priority level (default: 99)
     * @return Always returns true 
     */
    static bool optimizeForHFT(const std::string& threadName, int cpuCore = 0, int priority = 99);
};

#endif // THREADUTILS_H
