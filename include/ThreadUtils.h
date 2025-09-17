/**
 * @file ThreadUtils.h
 * @brief Simple thread optimization utilities
 */

#ifndef THREADUTILS_H
#define THREADUTILS_H

#include <string>

/**
 * @brief Cross-platform utility class for thread optimization
 * 
 * Provides HFT-optimized thread configuration across Linux, macOS, and Windows.
 * Sets thread name, CPU affinity, and high priority for low-latency performance.
 */
class ThreadUtils {
public:
    /**
     * @brief Optimize thread for HFT (sets name, priority, and CPU affinity)
     * @param threadName Name for the thread
     * @param cpuCore CPU core to bind to (default: 0)
     * @param priority Priority level (default: 99)
     * @return Always returns true 
     * 
     * Platform-specific behavior:
     * - Linux: Uses prctl, pthread_setaffinity_np, SCHED_FIFO
     * - macOS: Uses pthread_setname_np, thread_affinity_policy, QOS_CLASS_USER_INTERACTIVE
     * - Windows: Uses SetThreadDescription, SetThreadAffinityMask, SetThreadPriority
     */
    static bool optimizeForHFT(const std::string& threadName, int cpuCore = 0, int priority = 99);
};

#endif // THREADUTILS_H
