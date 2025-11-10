/**
 * @file LockFreeRingBuffer.h
 * @brief HFT-grade lock-free SPSC ring buffer with cache line optimization
 * 
 * Optimized for microsecond latency with:
 * - Cache line alignment to prevent false sharing
 * - Proper memory barriers for SPSC semantics
 * - Power-of-2 size for fast modulo operations
 */

#ifndef LOCKFREERINGBUFFER_H
#define LOCKFREERINGBUFFER_H

#include <array>
#include <atomic>
#include <memory>
#include <cstddef>
#include <cstdint>

#ifdef __linux__
#include <numa.h>
#endif

// Cache line size (typically 64 bytes on modern CPUs)
#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

// Align to cache line boundary
#define ALIGN_CACHE_LINE alignas(CACHE_LINE_SIZE)

/**
 * @brief HFT-grade lock-free ring buffer for single-producer single-consumer scenarios
 * 
 * This class provides ultra-low latency, lock-free ring buffer optimized for
 * HFT applications. Features:
 * - Cache line alignment to prevent false sharing
 * - Proper memory ordering for SPSC semantics
 * - Power-of-2 size for fast modulo (bitwise AND)
 * - NUMA-aware allocation support
 * 
 * @tparam T Type of elements stored in the buffer
 * @tparam Size Size of the ring buffer (must be power of 2 for optimal performance)
 */
template<typename T, size_t Size>
class LockFreeRingBuffer {
    static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2");
    static_assert(Size > 0, "Size must be greater than 0");

private:
    // Align buffer to cache line to prevent false sharing
    ALIGN_CACHE_LINE std::array<T, Size> m_buffer;   ///< Internal buffer storage
    
    // Separate cache lines for head and tail to prevent false sharing
    ALIGN_CACHE_LINE std::atomic<size_t> m_head{0}; ///< Consumer index (read position)
    ALIGN_CACHE_LINE std::atomic<size_t> m_tail{0};  ///< Producer index (write position)

public:
    /**
     * @brief Constructor
     */
    LockFreeRingBuffer() = default;
    
    /**
     * @brief Destructor
     */
    ~LockFreeRingBuffer() = default;
    
    // Non-copyable, non-movable
    LockFreeRingBuffer(const LockFreeRingBuffer&) = delete;
    LockFreeRingBuffer& operator=(const LockFreeRingBuffer&) = delete;
    LockFreeRingBuffer(LockFreeRingBuffer&&) = delete;
    LockFreeRingBuffer& operator=(LockFreeRingBuffer&&) = delete;
    
    /**
     * @brief Push an item to the buffer (producer thread only)
     * @param item Item to push
     * @return True if successful, false if buffer is full
     * 
     * Uses acquire-release semantics for SPSC correctness:
     * - Relaxed load of tail (only producer writes it)
     * - Acquire load of head (to see consumer's updates)
     * - Release store of tail (to make item visible to consumer)
     */
    bool push(const T& item) noexcept {
        const size_t current_tail = m_tail.load(std::memory_order_relaxed);
        const size_t next_tail = (current_tail + 1) & (Size - 1); // Fast modulo for power of 2
        
        // Acquire barrier: ensure we see the latest head value
        if (next_tail == m_head.load(std::memory_order_acquire)) {
            return false; // Buffer full
        }
        
        // Store item (no barrier needed - only producer writes here)
        m_buffer[current_tail] = item;
        
        // Release barrier: make the item visible to consumer
        m_tail.store(next_tail, std::memory_order_release);
        return true;
    }
    
    /**
     * @brief Push an item to the buffer (move semantics)
     * @param item Item to push
     * @return True if successful, false if buffer is full
     */
    bool push(T&& item) noexcept {
        const size_t current_tail = m_tail.load(std::memory_order_relaxed);
        const size_t next_tail = (current_tail + 1) & (Size - 1);
        
        if (next_tail == m_head.load(std::memory_order_acquire)) {
            return false;
        }
        
        m_buffer[current_tail] = std::move(item);
        m_tail.store(next_tail, std::memory_order_release);
        return true;
    }
    
    /**
     * @brief Pop an item from the buffer (consumer thread only)
     * @param item Reference to store the popped item
     * @return True if successful, false if buffer is empty
     * 
     * Uses acquire-release semantics for SPSC correctness:
     * - Relaxed load of head (only consumer writes it)
     * - Acquire load of tail (to see producer's updates)
     * - Release store of head (to make slot available to producer)
     */
    bool pop(T& item) noexcept {
        const size_t current_head = m_head.load(std::memory_order_relaxed);
        
        // Acquire barrier: ensure we see the latest tail value
        if (current_head == m_tail.load(std::memory_order_acquire)) {
            return false; // Buffer empty
        }
        
        // Load item (no barrier needed - only consumer reads here)
        item = std::move(m_buffer[current_head]);
        
        // Release barrier: make the slot available to producer
        m_head.store((current_head + 1) & (Size - 1), std::memory_order_release);
        return true;
    }
    
    /**
     * @brief Check if buffer is empty
     * @return True if empty, false otherwise
     */
    bool empty() const noexcept {
        return m_head.load(std::memory_order_acquire) == 
               m_tail.load(std::memory_order_acquire);
    }
    
    /**
     * @brief Check if buffer is full
     * @return True if full, false otherwise
     */
    bool full() const noexcept {
        const size_t current_tail = m_tail.load(std::memory_order_acquire);
        const size_t next_tail = (current_tail + 1) & (Size - 1);
        return next_tail == m_head.load(std::memory_order_acquire);
    }
    
    /**
     * @brief Get current size (number of elements in buffer)
     * @return Current size
     */
    size_t size() const noexcept {
        const size_t current_tail = m_tail.load(std::memory_order_acquire);
        const size_t current_head = m_head.load(std::memory_order_acquire);
        return (current_tail - current_head) & (Size - 1);
    }
    
    /**
     * @brief Get maximum capacity
     * @return Maximum capacity
     */
    constexpr size_t capacity() const noexcept {
        return Size - 1; // One slot reserved to distinguish full from empty
    }
    
    /**
     * @brief Clear the buffer (not thread-safe, use with caution)
     */
    void clear() noexcept {
        m_head.store(0, std::memory_order_relaxed);
        m_tail.store(0, std::memory_order_relaxed);
    }
};

#endif // LOCKFREERINGBUFFER_H
