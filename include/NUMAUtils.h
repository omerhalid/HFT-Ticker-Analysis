/**
 * @file NUMAUtils.h
 * @brief NUMA-aware utilities for HFT applications (Linux-only)
 * 
 * Provides NUMA topology detection, memory allocation on specific nodes,
 * and thread-to-NUMA-node pinning for optimal latency.
 */

#ifndef NUMAUTILS_H
#define NUMAUTILS_H

#include <cstddef>
#include <cstdint>
#include <vector>

#ifdef __linux__

/**
 * @brief NUMA topology information
 */
struct NUMATopology {
    int numNodes;                           ///< Number of NUMA nodes
    std::vector<int> cpusPerNode;           ///< CPUs per NUMA node
    std::vector<std::vector<int>> nodeCpus; ///< CPU list per node
    int currentNode;                        ///< Current NUMA node
    int currentCpu;                        ///< Current CPU
};

/**
 * @brief NUMA-aware utilities for HFT applications
 * 
 * Provides functions for:
 * - Detecting NUMA topology
 * - Allocating memory on specific NUMA nodes
 * - Pinning threads to NUMA nodes
 * - Getting optimal CPU for a NUMA node
 */
class NUMAUtils {
public:
    /**
     * @brief Initialize NUMA subsystem
     * @return True if NUMA is available, false otherwise
     */
    static bool initialize();
    
    /**
     * @brief Check if NUMA is available
     * @return True if NUMA is available
     */
    static bool isAvailable();
    
    /**
     * @brief Get NUMA topology
     * @return NUMA topology structure
     */
    static NUMATopology getTopology();
    
    /**
     * @brief Get number of NUMA nodes
     * @return Number of NUMA nodes
     */
    static int getNumNodes();
    
    /**
     * @brief Get current NUMA node
     * @return Current NUMA node ID
     */
    static int getCurrentNode();
    
    /**
     * @brief Get CPUs for a specific NUMA node
     * @param nodeId NUMA node ID
     * @return Vector of CPU IDs
     */
    static std::vector<int> getCpusForNode(int nodeId);
    
    /**
     * @brief Get first CPU for a NUMA node
     * @param nodeId NUMA node ID
     * @return CPU ID, or -1 if not found
     */
    static int getFirstCpuForNode(int nodeId);
    
    /**
     * @brief Allocate memory on a specific NUMA node
     * @param size Size in bytes
     * @param nodeId NUMA node ID (-1 for current node)
     * @return Pointer to allocated memory, or nullptr on failure
     */
    static void* allocateOnNode(size_t size, int nodeId = -1);
    
    /**
     * @brief Free NUMA-allocated memory
     * @param ptr Pointer to memory
     * @param size Size in bytes
     */
    static void freeOnNode(void* ptr, size_t size);
    
    /**
     * @brief Pin current thread to a NUMA node
     * @param nodeId NUMA node ID
     * @return True if successful
     */
    static bool pinThreadToNode(int nodeId);
    
    /**
     * @brief Pin current thread to a specific CPU
     * @param cpuId CPU ID
     * @return True if successful
     */
    static bool pinThreadToCpu(int cpuId);
    
    /**
     * @brief Get optimal NUMA node for a thread (round-robin)
     * @param threadId Thread identifier (for round-robin assignment)
     * @return NUMA node ID
     */
    static int getOptimalNode(int threadId = 0);
    
    /**
     * @brief Set memory policy for current thread
     * @param nodeId NUMA node ID
     * @return True if successful
     */
    static bool setMemoryPolicy(int nodeId);
    
    /**
     * @brief Touch memory to ensure it's allocated on the correct node
     * @param ptr Pointer to memory
     * @param size Size in bytes
     */
    static void touchMemory(void* ptr, size_t size);
};

#endif // __linux__

#endif // NUMAUTILS_H

