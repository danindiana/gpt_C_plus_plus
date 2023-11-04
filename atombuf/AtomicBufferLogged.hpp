// AtomicBufferLogged.hpp
#ifndef ATOMIC_BUFFER_LOGGED_HPP
#define ATOMIC_BUFFER_LOGGED_HPP

#include <atomic>
#include <vector>
#include <iostream>
#include <mutex>

template <typename T>
class AtomicBuffer {
private:
    std::vector<T> buffer;
    std::atomic<int> head;
    std::atomic<int> tail;
    const int maxSize;
    std::mutex log_mutex;

    // Utility function to log the buffer state
    void logBufferState(const std::string& operation, int index, const T& item) {
        std::lock_guard<std::mutex> guard(log_mutex);
        std::cout << "Buffer " << operation << " - Index: " << index << ", Item: " << item << ", Buffer State: [";
        for (int i = 0; i < maxSize; ++i) {
            if (i > 0) std::cout << ", ";
            if (i >= head.load() && i < tail.load()) {
                std::cout << buffer[i % maxSize];
            } else {
                std::cout << "_";
            }
        }
        std::cout << "]" << std::endl;
    }

public:
    AtomicBuffer(int size) : maxSize(size), head(0), tail(0), buffer(size) {}

    bool push(const T& item) {
        int currentTail = tail.load();
        int nextTail = (currentTail + 1) % maxSize;
        if (nextTail != head.load()) {
            buffer[currentTail] = item;
            tail.store(nextTail);
            logBufferState("pushed", currentTail, item);
            return true;
        }
        return false;
    }

    bool pop(T& item) {
        int currentHead = head.load();
        if (currentHead == tail.load()) {
            return false; // Buffer is empty
        }
        item = buffer[currentHead];
        head.store((currentHead + 1) % maxSize);
        logBufferState("popped", currentHead, item);
        return true;
    }
};

#endif // ATOMIC_BUFFER_LOGGED_HPP
