/**
 * @file ThreadUtils.cpp
 * @brief Thread optimization utilities
 */

#include "ThreadUtils.h"

#ifdef __linux__
#include <sched.h>
#include <sys/prctl.h>
#include <pthread.h>
#elif __APPLE__
#include <mach/thread_policy.h>
#include <mach/thread_act.h>
#include <mach/mach_init.h>
#include <pthread.h>
#elif _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#endif

bool ThreadUtils::optimizeForHFT(const std::string& threadName, int cpuCore, int priority) {
    // Set thread name 
#ifdef __linux__
    prctl(PR_SET_NAME, threadName.c_str(), 0, 0, 0);
#elif __APPLE__
    pthread_setname_np(threadName.c_str());
#elif _WIN32
    #ifdef _MSC_VER
    const std::wstring wname(threadName.begin(), threadName.end());
    SetThreadDescription(GetCurrentThread(), wname.c_str());
    #endif
#endif

    // Set CPU affinity 
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpuCore, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
#elif __APPLE__
    thread_affinity_policy_data_t policy = { cpuCore };
    thread_policy_set(mach_thread_self(), THREAD_AFFINITY_POLICY, 
                     (thread_policy_t)&policy, THREAD_AFFINITY_POLICY_COUNT);
#elif _WIN32
    DWORD_PTR mask = 1ULL << cpuCore;
    SetThreadAffinityMask(GetCurrentThread(), mask);
#endif

    // Set high priority 
#ifdef __linux__
    struct sched_param param;
    param.sched_priority = priority;
    sched_setscheduler(0, SCHED_FIFO, &param);
#elif __APPLE__
    pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
#elif _WIN32
    // Windows priority mapping: 99 -> THREAD_PRIORITY_TIME_CRITICAL
    int winPriority = THREAD_PRIORITY_HIGHEST;
    if (priority >= 90) {
        winPriority = THREAD_PRIORITY_TIME_CRITICAL;
    } else if (priority >= 80) {
        winPriority = THREAD_PRIORITY_HIGHEST;
    } else if (priority >= 70) {
        winPriority = THREAD_PRIORITY_ABOVE_NORMAL;
    }
    SetThreadPriority(GetCurrentThread(), winPriority);
#endif

    return true;
}
