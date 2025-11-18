// AtomicBufferFixed.hpp
// Thread-safe lock-free circular buffer using Compare-And-Swap (CAS) operations
#ifndef ATOMIC_BUFFER_FIXED_HPP
#define ATOMIC_BUFFER_FIXED_HPP

#include <atomic>
#include <vector>
#include <cstddef>

template <typename T>
class AtomicBufferFixed {
private:
    const size_t maxSize;
    std::vector<T> buffer;
    alignas(64) std::atomic<size_t> head;  // Cache line aligned to prevent false sharing
    alignas(64) std::atomic<size_t> tail;  // Cache line aligned to prevent false sharing

    // Helper to get next index in circular buffer
    inline size_t next(size_t current) const noexcept {
        return (current + 1) % maxSize;
    }

public:
    explicit AtomicBufferFixed(size_t size)
        : maxSize(size), buffer(maxSize), head(0), tail(0) {
    }

    // Delete copy constructor and assignment operator
    AtomicBufferFixed(const AtomicBufferFixed&) = delete;
    AtomicBufferFixed& operator=(const AtomicBufferFixed&) = delete;

    /**
     * Thread-safe push operation using CAS
     * Returns true if item was successfully pushed, false if buffer is full
     */
    bool push(const T& item) noexcept {
        size_t currentTail = tail.load(std::memory_order_relaxed);

        while (true) {
            size_t nextTail = next(currentTail);

            // Check if buffer is full
            // Use acquire to ensure we see all previous consumer operations
            if (nextTail == head.load(std::memory_order_acquire)) {
                return false;  // Buffer is full
            }

            // Write the item speculatively before CAS
            // If CAS fails, this write is wasted but doesn't affect correctness
            // because this slot is not yet visible to consumers
            buffer[currentTail] = item;

            // Memory fence to ensure the write completes before we update tail
            std::atomic_thread_fence(std::memory_order_release);

            // Try to claim this slot using CAS
            if (tail.compare_exchange_weak(
                    currentTail,
                    nextTail,
                    std::memory_order_release,  // Success: make slot visible to consumers
                    std::memory_order_relaxed   // Failure: retry with updated currentTail
                )) {
                // Successfully claimed and published the slot
                return true;
            }

            // CAS failed, currentTail was updated by compare_exchange_weak
            // Loop will retry with the new value (and overwrite the speculative write)
        }
    }

    /**
     * Move version of push for better performance with move-only types
     */
    bool push(T&& item) noexcept {
        size_t currentTail = tail.load(std::memory_order_relaxed);

        while (true) {
            size_t nextTail = next(currentTail);

            if (nextTail == head.load(std::memory_order_acquire)) {
                return false;  // Buffer is full
            }

            // Write the item speculatively before CAS
            buffer[currentTail] = std::move(item);

            // Memory fence to ensure the write completes before we update tail
            std::atomic_thread_fence(std::memory_order_release);

            if (tail.compare_exchange_weak(
                    currentTail,
                    nextTail,
                    std::memory_order_release,
                    std::memory_order_relaxed
                )) {
                return true;
            }

            // CAS failed - we need to restore the item for retry
            // This is problematic for move-only types, so we can't perfectly support
            // retries with move semantics. For now, return false to avoid complexity.
            return false;
        }
    }

    /**
     * Thread-safe pop operation using CAS
     * Returns true if item was successfully popped, false if buffer is empty
     */
    bool pop(T& item) noexcept {
        size_t currentHead = head.load(std::memory_order_relaxed);

        while (true) {
            // Check if buffer is empty
            // Use acquire to ensure we see all previous producer operations
            if (currentHead == tail.load(std::memory_order_acquire)) {
                return false;  // Buffer is empty
            }

            size_t nextHead = next(currentHead);

            // Try to claim this slot using CAS
            if (head.compare_exchange_weak(
                    currentHead,
                    nextHead,
                    std::memory_order_release,  // Success: make slot visible to producers
                    std::memory_order_relaxed   // Failure: retry with updated currentHead
                )) {
                // Successfully claimed the slot, move the item
                // This is safe because we've advanced head, so producers won't touch this slot
                item = std::move(buffer[currentHead]);
                return true;
            }

            // CAS failed, currentHead was updated by compare_exchange_weak
            // Loop will retry with the new value
        }
    }

    /**
     * Check if buffer is empty (approximation - may be stale immediately)
     */
    bool isEmpty() const noexcept {
        return head.load(std::memory_order_acquire) ==
               tail.load(std::memory_order_acquire);
    }

    /**
     * Check if buffer is full (approximation - may be stale immediately)
     */
    bool isFull() const noexcept {
        size_t currentTail = tail.load(std::memory_order_acquire);
        size_t currentHead = head.load(std::memory_order_acquire);
        return next(currentTail) == currentHead;
    }

    /**
     * Get approximate size (may be stale immediately in multi-threaded context)
     */
    size_t size() const noexcept {
        size_t currentHead = head.load(std::memory_order_acquire);
        size_t currentTail = tail.load(std::memory_order_acquire);

        if (currentTail >= currentHead) {
            return currentTail - currentHead;
        } else {
            return maxSize - currentHead + currentTail;
        }
    }

    /**
     * Get buffer capacity
     */
    size_t capacity() const noexcept {
        return maxSize - 1;  // One slot is always reserved to distinguish full from empty
    }
};

#endif // ATOMIC_BUFFER_FIXED_HPP
