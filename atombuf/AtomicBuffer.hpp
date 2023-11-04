// AtomicBuffer.hpp
#ifndef ATOMIC_BUFFER_HPP
#define ATOMIC_BUFFER_HPP

#include <atomic>
#include <vector>

template <typename T>
class AtomicBuffer {
private:
    std::vector<T> buffer;
    std::atomic<int> head;
    std::atomic<int> tail;
    const int maxSize;

public:
    AtomicBuffer(int size) : maxSize(size), head(0), tail(0) {
        buffer.resize(maxSize);
    }

    bool push(const T& item) {
        int currentTail = tail.load(std::memory_order_relaxed);
        int nextTail = (currentTail + 1) % maxSize;

        // Check if buffer is full
        if (nextTail == head.load(std::memory_order_acquire)) {
            return false; // Buffer is full
        }

        buffer[currentTail] = item;
        tail.store(nextTail, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        int currentHead = head.load(std::memory_order_relaxed);

        // Check if buffer is empty
        if (currentHead == tail.load(std::memory_order_acquire)) {
            return false; // Buffer is empty
        }

        item = buffer[currentHead];
        head.store((currentHead + 1) % maxSize, std::memory_order_release);
        return true;
    }

    // Additional utility functions can be added here if necessary
};

#endif // ATOMIC_BUFFER_HPP
