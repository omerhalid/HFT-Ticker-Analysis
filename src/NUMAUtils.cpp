/**
 * @file NUMAUtils.cpp
 * @brief Implementation of NUMA-aware utilities for HFT applications
 */

#include "NUMAUtils.h"

#ifdef __linux__

#include <numa.h>
#include <numaif.h>
#include <sched.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

static bool g_numaInitialized = false;
static bool g_numaAvailable = false;

bool NUMAUtils::initialize() {
    if (g_numaInitialized) {
        return g_numaAvailable;
    }
    
    g_numaInitialized = true;
    
    if (numa_available() < 0) {
        g_numaAvailable = false;
        return false;
    }
    
    g_numaAvailable = true;
    return true;
}

bool NUMAUtils::isAvailable() {
    if (!g_numaInitialized) {
        initialize();
    }
    return g_numaAvailable;
}

NUMATopology NUMAUtils::getTopology() {
    NUMATopology topology;
    
    if (!isAvailable()) {
        topology.numNodes = 1;
        topology.currentNode = 0;
        topology.currentCpu = sched_getcpu();
        return topology;
    }
    
    topology.numNodes = numa_max_node() + 1;
    topology.currentNode = numa_node_of_cpu(sched_getcpu());
    topology.currentCpu = sched_getcpu();
    
    // Get CPUs for each node
    topology.nodeCpus.resize(topology.numNodes);
    topology.cpusPerNode.resize(topology.numNodes);
    
    for (int node = 0; node < topology.numNodes; ++node) {
        struct bitmask* cpus = numa_allocate_cpumask();
        if (numa_node_to_cpus(node, cpus) == 0) {
            for (int cpu = 0; cpu < numa_num_possible_cpus(); ++cpu) {
                if (numa_bitmask_isbitset(cpus, cpu)) {
                    topology.nodeCpus[node].push_back(cpu);
                }
            }
        }
        topology.cpusPerNode[node] = topology.nodeCpus[node].size();
        numa_free_cpumask(cpus);
    }
    
    return topology;
}

int NUMAUtils::getNumNodes() {
    if (!isAvailable()) {
        return 1;
    }
    return numa_max_node() + 1;
}

int NUMAUtils::getCurrentNode() {
    if (!isAvailable()) {
        return 0;
    }
    return numa_node_of_cpu(sched_getcpu());
}

std::vector<int> NUMAUtils::getCpusForNode(int nodeId) {
    std::vector<int> cpus;
    
    if (!isAvailable()) {
        // Single node system - return all CPUs
        int numCpus = sysconf(_SC_NPROCESSORS_ONLN);
        for (int i = 0; i < numCpus; ++i) {
            cpus.push_back(i);
        }
        return cpus;
    }
    
    struct bitmask* cpumask = numa_allocate_cpumask();
    if (numa_node_to_cpus(nodeId, cpumask) == 0) {
        for (int cpu = 0; cpu < numa_num_possible_cpus(); ++cpu) {
            if (numa_bitmask_isbitset(cpumask, cpu)) {
                cpus.push_back(cpu);
            }
        }
    }
    numa_free_cpumask(cpumask);
    
    return cpus;
}

int NUMAUtils::getFirstCpuForNode(int nodeId) {
    auto cpus = getCpusForNode(nodeId);
    return cpus.empty() ? -1 : cpus[0];
}

void* NUMAUtils::allocateOnNode(size_t size, int nodeId) {
    if (!isAvailable()) {
        return ::malloc(size);
    }
    
    if (nodeId < 0) {
        nodeId = getCurrentNode();
    }
    
    void* ptr = numa_alloc_onnode(size, nodeId);
    if (ptr) {
        // Touch memory to ensure it's allocated on the correct node
        touchMemory(ptr, size);
    }
    
    return ptr;
}

void NUMAUtils::freeOnNode(void* ptr, size_t size) {
    if (!isAvailable()) {
        ::free(ptr);
        return;
    }
    
    numa_free(ptr, size);
}

bool NUMAUtils::pinThreadToNode(int nodeId) {
    if (!isAvailable()) {
        return false;
    }
    
    auto cpus = getCpusForNode(nodeId);
    if (cpus.empty()) {
        return false;
    }
    
    // Pin to first CPU of the node
    return pinThreadToCpu(cpus[0]);
}

bool NUMAUtils::pinThreadToCpu(int cpuId) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpuId, &cpuset);
    
    return pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) == 0;
}

int NUMAUtils::getOptimalNode(int threadId) {
    if (!isAvailable()) {
        return 0;
    }
    
    int numNodes = getNumNodes();
    return threadId % numNodes;
}

bool NUMAUtils::setMemoryPolicy(int nodeId) {
    if (!isAvailable()) {
        return false;
    }
    
    struct bitmask* nodemask = numa_allocate_nodemask();
    numa_bitmask_setbit(nodemask, nodeId);
    
    int result = set_mempolicy(MPOL_BIND, nodemask->maskp, nodemask->size + 1);
    
    numa_free_nodemask(nodemask);
    
    return result == 0;
}

void NUMAUtils::touchMemory(void* ptr, size_t size) {
    // Touch each page to ensure it's allocated on the correct NUMA node
    const size_t pageSize = getpagesize();
    char* p = static_cast<char*>(ptr);
    
    for (size_t i = 0; i < size; i += pageSize) {
        volatile char c = p[i];
        (void)c; // Suppress unused variable warning
    }
}

#endif // __linux__

