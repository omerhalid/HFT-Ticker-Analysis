/**
 * @file ThreadUtils.cpp
 * @brief Implementation of HFT-grade thread optimization utilities
 */

#include "ThreadUtils.h"

#ifdef __linux__

#include <sched.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#include "NUMAUtils.h"

bool ThreadUtils::optimizeForHFT(const std::string& threadName, 
                                 int cpuCore, 
                                 int priority,
                                 int numaNode) {
    bool success = true;
    
    // Set thread name
    success &= setThreadName(threadName);
    
    // Determine CPU core
    if (cpuCore < 0) {
        cpuCore = getOptimalCpu();
    }
    
    // Pin to CPU
    success &= pinToCpu(cpuCore);
    
    // Set NUMA memory policy if NUMA is available
    if (NUMAUtils::isAvailable()) {
        if (numaNode < 0) {
            // Determine NUMA node from CPU
            numaNode = NUMAUtils::getCurrentNode();
        }
        NUMAUtils::setMemoryPolicy(numaNode);
    }
    
    // Set real-time priority
    success &= setRealtimePriority(priority);
    
    // Disable CPU migration
    disableCpuMigration();
    
    return success;
}

bool ThreadUtils::pinToCpu(int cpuCore) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpuCore, &cpuset);
    
    return pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) == 0;
}

bool ThreadUtils::pinToNUMANode(int numaNode) {
    if (!NUMAUtils::isAvailable()) {
        return false;
    }
    
    int cpu = NUMAUtils::getFirstCpuForNode(numaNode);
    if (cpu < 0) {
        return false;
    }
    
    return pinToCpu(cpu);
}

bool ThreadUtils::setThreadName(const std::string& threadName) {
    // Linux thread names are limited to 15 characters + null terminator
    std::string name = threadName;
    if (name.length() > 15) {
        name = name.substr(0, 15);
    }
    
    return prctl(PR_SET_NAME, name.c_str(), 0, 0, 0) == 0;
}

bool ThreadUtils::setRealtimePriority(int priority) {
    // Clamp priority to valid range for SCHED_FIFO
    if (priority < 1) priority = 1;
    if (priority > 99) priority = 99;
    
    struct sched_param param;
    param.sched_priority = priority;
    
    return sched_setscheduler(0, SCHED_FIFO, &param) == 0;
}

bool ThreadUtils::setMaxRealtimePriority() {
    return setRealtimePriority(99);
}

bool ThreadUtils::disableCpuMigration() {
    // Use sched_setaffinity to prevent migration
    // The kernel will respect the affinity mask
    cpu_set_t cpuset;
    if (sched_getaffinity(0, sizeof(cpuset), &cpuset) == 0) {
        return sched_setaffinity(0, sizeof(cpuset), &cpuset) == 0;
    }
    return false;
}

int ThreadUtils::getCurrentCpu() {
    return sched_getcpu();
}

int ThreadUtils::getOptimalCpu(int threadId) {
    if (NUMAUtils::isAvailable()) {
        int numaNode = NUMAUtils::getOptimalNode(threadId);
        int cpu = NUMAUtils::getFirstCpuForNode(numaNode);
        if (cpu >= 0) {
            return cpu;
        }
    }
    
    // Fallback: round-robin across available CPUs
    int numCpus = sysconf(_SC_NPROCESSORS_ONLN);
    return threadId % numCpus;
}

bool ThreadUtils::setCpuAffinity(uint64_t cpuMask) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    
    for (int cpu = 0; cpu < 64; ++cpu) {
        if (cpuMask & (1ULL << cpu)) {
            CPU_SET(cpu, &cpuset);
        }
    }
    
    return pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) == 0;
}

#endif // __linux__

