/**
 * @file BranchPrediction.h
 * @brief Branch prediction hints for HFT optimization
 * 
 * Provides likely() and unlikely() macros to help CPU branch predictor
 * optimize hot paths for maximum performance.
 */

#ifndef BRANCHPREDICTION_H
#define BRANCHPREDICTION_H

// Branch prediction hints for GCC/Clang
#if defined(__GNUC__) || defined(__clang__)
    #define LIKELY(x)   __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    // No-op for other compilers
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
#endif

#endif // BRANCHPREDICTION_H

