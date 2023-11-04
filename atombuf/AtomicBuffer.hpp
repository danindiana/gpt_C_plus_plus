#include <atomic>
#include <iostream>
#include <vector>
#include <thread>

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
};

// Example usage
int main() {
    AtomicBuffer<int> buffer(10);

    // Producer thread
    std::thread producer([&buffer]() {
        for (int i = 0; i < 100; ++i) {
            while (!buffer.push(i)) {
                // Wait or handle buffer being full
            }
        }
    });

    // Consumer thread
    std::thread consumer([&buffer]() {
        int item;
        for (int i = 0; i < 100; ++i) {
            while (!buffer.pop(item)) {
                // Wait or handle buffer being empty
            }
            std::cout << "Consumed: " << item << std::endl;
        }
    });

    producer.join();
    consumer.join();

    return 0;
}
