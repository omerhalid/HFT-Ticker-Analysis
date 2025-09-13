/**
 * @file ThreadUtils.h
 * @brief Utility functions for thread optimization in HFT applications
 */

#ifndef THREADUTILS_H
#define THREADUTILS_H

#include <thread>
#include <string>

/**
 * @brief Utility class for thread optimization
 * 
 * This class provides functions to optimize threads for high-frequency trading
 * applications, including CPU affinity, priority settings, and thread naming.
 */
class ThreadUtils {
public:
    /**
     * @brief Set high priority for current thread
     * @param priority Priority level (1-99, higher is more priority)
     * @return True if successful, false otherwise
     */
    static bool setHighPriority(int priority = 99);
    
    /**
     * @brief Set CPU affinity for current thread
     * @param cpuCore CPU core number to bind to
     * @return True if successful, false otherwise
     */
    static bool setCpuAffinity(int cpuCore);
    
    /**
     * @brief Set thread name for debugging
     * @param name Thread name
     * @return True if successful, false otherwise
     */
    static bool setThreadName(const std::string& name);
    
    /**
     * @brief Optimize thread for HFT (combines priority, affinity, and naming)
     * @param threadName Name for the thread
     * @param cpuCore CPU core to bind to
     * @param priority Priority level (1-99)
     * @return True if all optimizations successful, false otherwise
     */
    static bool optimizeForHFT(const std::string& threadName, int cpuCore = 0, int priority = 99);
    
    /**
     * @brief Get current thread ID
     * @return Thread ID
     */
    static std::thread::id getCurrentThreadId();
    
    /**
     * @brief Get number of available CPU cores
     * @return Number of CPU cores
     */
    static int getCpuCoreCount();
    
    /**
     * @brief Disable CPU frequency scaling for better performance
     * @return True if successful, false otherwise
     */
    static bool disableCpuFrequencyScaling();
    
    /**
     * @brief Set memory locking to prevent swapping
     * @return True if successful, false otherwise
     */
    static bool lockMemory();
};

#endif // THREADUTILS_H
