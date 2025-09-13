/**
 * @file ThreadUtils.cpp
 * @brief Implementation of thread optimization utilities
 */

#include "ThreadUtils.h"
#include <iostream>
#include <thread>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <cstring>

#ifdef __linux__
#include <sched.h>
#include <sys/prctl.h>
#elif __APPLE__
#include <mach/thread_policy.h>
#include <mach/thread_act.h>
#include <mach/mach_init.h>
#endif

bool ThreadUtils::setHighPriority(int priority) {
    if (priority < 1 || priority > 99) {
        std::cerr << "Priority must be between 1 and 99" << std::endl;
        return false;
    }
    
#ifdef __linux__
    struct sched_param param;
    param.sched_priority = priority;
    
    int result = sched_setscheduler(0, SCHED_FIFO, &param);
    if (result != 0) {
        std::cerr << "Failed to set high priority: " << strerror(errno) << std::endl;
        std::cerr << "Note: This may require root privileges" << std::endl;
        return false;
    }
    return true;
#elif __APPLE__
    // On macOS, we can't set real-time priority without root privileges
    // Instead, we'll set the thread to high priority
    int result = pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
    if (result != 0) {
        return false;
    }
    return true;
#else
    std::cerr << "High priority setting not supported on this platform" << std::endl;
    return false;
#endif
}

bool ThreadUtils::setCpuAffinity(int cpuCore) {
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpuCore, &cpuset);
    
    int result = pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
    if (result != 0) {
        std::cerr << "Failed to set CPU affinity: " << strerror(result) << std::endl;
        return false;
    }
    return true;
#elif __APPLE__
    // On macOS, we need to use thread affinity API
    thread_affinity_policy_data_t policy = { cpuCore };
    kern_return_t result = thread_policy_set(
        mach_thread_self(),
        THREAD_AFFINITY_POLICY,
        (thread_policy_t)&policy,
        THREAD_AFFINITY_POLICY_COUNT
    );
    
    if (result != KERN_SUCCESS) {
        return false;
    }
    return true;
#else
    std::cerr << "CPU affinity setting not supported on this platform" << std::endl;
    return false;
#endif
}

bool ThreadUtils::setThreadName(const std::string& name) {
#ifdef __linux__
    int result = prctl(PR_SET_NAME, name.c_str(), 0, 0, 0);
    if (result != 0) {
        std::cerr << "Failed to set thread name: " << strerror(errno) << std::endl;
        return false;
    }
    return true;
#elif __APPLE__
    // On macOS, we can set thread name using pthread_setname_np
    int result = pthread_setname_np(name.c_str());
    if (result != 0) {
        std::cerr << "Failed to set thread name: " << strerror(result) << std::endl;
        return false;
    }
    return true;
#else
    // Thread naming not supported on this platform
    return false;
#endif
}

bool ThreadUtils::optimizeForHFT(const std::string& threadName, int cpuCore, int priority) {
    bool success = true;
    
    // Set thread name (non-critical)
    if (!setThreadName(threadName)) {
        // Silently continue - thread naming is not critical
    }
    
    // Set CPU affinity (non-critical on macOS)
    if (!setCpuAffinity(cpuCore)) {
        // Silently continue - CPU affinity may not be available
    }
    
    // Set high priority (non-critical)
    if (!setHighPriority(priority)) {
        // Silently continue - priority setting may require privileges
    }
    
    return success;
}

std::thread::id ThreadUtils::getCurrentThreadId() {
    return std::this_thread::get_id();
}

int ThreadUtils::getCpuCoreCount() {
    return std::thread::hardware_concurrency();
}

bool ThreadUtils::disableCpuFrequencyScaling() {
    // This is a system-level setting that typically requires root privileges
    // and is usually done at the OS level, not in application code
    std::cerr << "CPU frequency scaling should be disabled at the OS level" << std::endl;
    std::cerr << "Use: echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor" << std::endl;
    return false;
}

bool ThreadUtils::lockMemory() {
    int result = mlockall(MCL_CURRENT | MCL_FUTURE);
    if (result != 0) {
        std::cerr << "Failed to lock memory: " << strerror(errno) << std::endl;
        std::cerr << "Note: This may require root privileges" << std::endl;
        return false;
    }
    
    return true;
}
