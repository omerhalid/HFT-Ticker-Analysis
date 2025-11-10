/**
 * @file ThreadUtils.h
 * @brief HFT-grade thread optimization utilities (Linux-optimized)
 * 
 * Provides NUMA-aware thread pinning, CPU isolation, and real-time scheduling
 * for ultra-low latency HFT applications.
 */

#ifndef THREADUTILS_H
#define THREADUTILS_H

#include <string>
#include <cstdint>

#ifdef __linux__

/**
 * @brief Thread optimization utilities for HFT applications
 * 
 * Provides functions for:
 * - Thread naming
 * - CPU pinning with NUMA awareness
 * - Real-time scheduling (SCHED_FIFO)
 * - CPU isolation support
 * - Thread priority management
 */
class ThreadUtils {
public:
    /**
     * @brief Optimize thread for HFT (Linux-optimized)
     * @param threadName Name for the thread
     * @param cpuCore CPU core to bind to (-1 for automatic NUMA-aware selection)
     * @param priority Priority level (1-99, default: 99 for SCHED_FIFO)
     * @param numaNode NUMA node ID (-1 for automatic selection based on CPU)
     * @return True if successful, false otherwise
     * 
     * Sets:
     * - Thread name via prctl
     * - CPU affinity via pthread_setaffinity_np
     * - Real-time scheduling via SCHED_FIFO
     * - NUMA memory policy if NUMA is available
     */
    static bool optimizeForHFT(const std::string& threadName, 
                                int cpuCore = -1, 
                                int priority = 99,
                                int numaNode = -1);
    
    /**
     * @brief Pin thread to a specific CPU core
     * @param cpuCore CPU core ID
     * @return True if successful
     */
    static bool pinToCpu(int cpuCore);
    
    /**
     * @brief Pin thread to a NUMA node (selects first CPU on that node)
     * @param numaNode NUMA node ID
     * @return True if successful
     */
    static bool pinToNUMANode(int numaNode);
    
    /**
     * @brief Set thread name
     * @param threadName Thread name (max 15 characters on Linux)
     * @return True if successful
     */
    static bool setThreadName(const std::string& threadName);
    
    /**
     * @brief Set real-time scheduling policy
     * @param priority Priority level (1-99 for SCHED_FIFO)
     * @return True if successful
     */
    static bool setRealtimePriority(int priority);
    
    /**
     * @brief Set thread to SCHED_FIFO with maximum priority
     * @return True if successful
     */
    static bool setMaxRealtimePriority();
    
    /**
     * @brief Disable CPU migration (prevent kernel from moving thread)
     * @return True if successful
     */
    static bool disableCpuMigration();
    
    /**
     * @brief Get current CPU core
     * @return CPU core ID, or -1 on error
     */
    static int getCurrentCpu();
    
    /**
     * @brief Get optimal CPU for a thread (round-robin across NUMA nodes)
     * @param threadId Thread identifier
     * @return CPU core ID
     */
    static int getOptimalCpu(int threadId = 0);
    
    /**
     * @brief Set CPU affinity mask
     * @param cpuMask CPU mask (bitmask)
     * @return True if successful
     */
    static bool setCpuAffinity(uint64_t cpuMask);
};

#endif // __linux__

#endif // THREADUTILS_H
