/**
 * @file LockFreeRingBuffer.h
 * @brief Lock-free single-producer single-consumer ring buffer for high-performance data processing
 */

#ifndef LOCKFREERINGBUFFER_H
#define LOCKFREERINGBUFFER_H

#include <array>
#include <atomic>
#include <memory>

/**
 * @brief Lock-free ring buffer for single-producer single-consumer scenarios
 * 
 * This class provides a high-performance, lock-free ring buffer optimized for
 * HFT applications where low latency is critical. Uses atomic operations and
 * memory ordering to ensure thread safety without mutexes.
 * 
 * @tparam T Type of elements stored in the buffer
 * @tparam Size Size of the ring buffer (must be power of 2 for optimal performance)
 */
template<typename T, size_t Size>
class LockFreeRingBuffer {
    static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2");
    static_assert(Size > 0, "Size must be greater than 0");

private:
    std::array<T, Size> m_buffer;                    ///< Internal buffer storage
    std::atomic<size_t> m_head{0};                   ///< Consumer index (read position)
    std::atomic<size_t> m_tail{0};                   ///< Producer index (write position)
    
    // Cache line padding to avoid false sharing
    char m_padding1[64 - sizeof(std::atomic<size_t>)];
    char m_padding2[64 - sizeof(std::atomic<size_t>)];

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
     */
    bool push(const T& item) noexcept {
        const size_t current_tail = m_tail.load(std::memory_order_relaxed);
        const size_t next_tail = (current_tail + 1) & (Size - 1); // Use bitwise AND for power of 2
        
        // Check if buffer is full
        if (next_tail == m_head.load(std::memory_order_acquire)) {
            return false;
        }
        
        m_buffer[current_tail] = item;
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
     */
    bool pop(T& item) noexcept {
        const size_t current_head = m_head.load(std::memory_order_relaxed);
        
        // Check if buffer is empty
        if (current_head == m_tail.load(std::memory_order_acquire)) {
            return false;
        }
        
        item = std::move(m_buffer[current_head]);
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
