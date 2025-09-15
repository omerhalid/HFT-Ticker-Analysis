/**
 * @file ThreadUtils.cpp
 * @brief Simple thread optimization utilities
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
#endif

bool ThreadUtils::optimizeForHFT(const std::string& threadName, int cpuCore, int priority) {
    // Set thread name 
#ifdef __linux__
    prctl(PR_SET_NAME, threadName.c_str(), 0, 0, 0);
#elif __APPLE__
    pthread_setname_np(threadName.c_str());
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
#endif

    // Set high priority 
#ifdef __linux__
    struct sched_param param;
    param.sched_priority = priority;
    sched_setscheduler(0, SCHED_FIFO, &param);
#elif __APPLE__
    pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
#endif

    return true;
}
