// AtomicBufferMPMC.hpp
// Thread-safe MPMC lock-free circular buffer using per-slot sequence numbers
// Based on Dmitry Vyukov's bounded MPMC queue
#ifndef ATOMIC_BUFFER_MPMC_HPP
#define ATOMIC_BUFFER_MPMC_HPP

#include <atomic>
#include <vector>
#include <cstddef>
#include <memory>
#include <stdexcept>

template <typename T>
class AtomicBufferMPMC {
private:
    struct Cell {
        std::atomic<size_t> sequence;
        T data;
    };

    using CellBuffer = std::vector<Cell>;

    const size_t bufferMask;
    alignas(64) CellBuffer buffer;
    alignas(64) std::atomic<size_t> enqueuePos;
    alignas(64) std::atomic<size_t> dequeuePos;

public:
    explicit AtomicBufferMPMC(size_t size)
        : bufferMask(size - 1),
          buffer(size),
          enqueuePos(0),
          dequeuePos(0) {

        // Size must be a power of 2
        if (size == 0 || (size & (size - 1)) != 0) {
            throw std::invalid_argument("Buffer size must be a power of 2");
        }

        // Initialize sequence numbers
        for (size_t i = 0; i < size; ++i) {
            buffer[i].sequence.store(i, std::memory_order_relaxed);
        }
    }

    // Delete copy constructor and assignment
    AtomicBufferMPMC(const AtomicBufferMPMC&) = delete;
    AtomicBufferMPMC& operator=(const AtomicBufferMPMC&) = delete;

    /**
     * Push an item into the buffer
     * Returns true on success, false if buffer is full
     */
    bool push(const T& item) {
        Cell* cell;
        size_t pos = enqueuePos.load(std::memory_order_relaxed);

        for (;;) {
            cell = &buffer[pos & bufferMask];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);

            if (diff == 0) {
                // Cell is ready for writing
                if (enqueuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    break;
                }
            }
            else if (diff < 0) {
                // Buffer is full
                return false;
            }
            else {
                // Another thread is ahead, update position
                pos = enqueuePos.load(std::memory_order_relaxed);
            }
        }

        // Write data
        cell->data = item;

        // Make data visible to consumers
        cell->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }

    /**
     * Push with move semantics
     */
    bool push(T&& item) {
        Cell* cell;
        size_t pos = enqueuePos.load(std::memory_order_relaxed);

        for (;;) {
            cell = &buffer[pos & bufferMask];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);

            if (diff == 0) {
                if (enqueuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    break;
                }
            }
            else if (diff < 0) {
                return false;
            }
            else {
                pos = enqueuePos.load(std::memory_order_relaxed);
            }
        }

        cell->data = std::move(item);
        cell->sequence.store(pos + 1, std::memory_order_release);
        return true;
    }

    /**
     * Pop an item from the buffer
     * Returns true on success with item in output parameter, false if empty
     */
    bool pop(T& item) {
        Cell* cell;
        size_t pos = dequeuePos.load(std::memory_order_relaxed);

        for (;;) {
            cell = &buffer[pos & bufferMask];
            size_t seq = cell->sequence.load(std::memory_order_acquire);
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos + 1);

            if (diff == 0) {
                // Cell is ready for reading
                if (dequeuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    break;
                }
            }
            else if (diff < 0) {
                // Buffer is empty
                return false;
            }
            else {
                // Another thread is ahead, update position
                pos = dequeuePos.load(std::memory_order_relaxed);
            }
        }

        // Read data
        item = std::move(cell->data);

        // Make slot available for producers
        cell->sequence.store(pos + bufferMask + 1, std::memory_order_release);
        return true;
    }

    /**
     * Get approximate size (may be stale)
     */
    size_t size() const {
        size_t enq = enqueuePos.load(std::memory_order_relaxed);
        size_t deq = dequeuePos.load(std::memory_order_relaxed);
        return enq - deq;
    }

    /**
     * Check if approximately empty
     */
    bool isEmpty() const {
        return size() == 0;
    }

    /**
     * Get capacity
     */
    size_t capacity() const {
        return bufferMask + 1;
    }
};

#endif // ATOMIC_BUFFER_MPMC_HPP
