// AtomicBufferTest.cpp
#include "AtomicBuffer.hpp"
#include <iostream>
#include <thread>
#include <cassert>

void producer(AtomicBuffer<int>& buffer) {
    for (int i = 0; i < 100; ++i) {
        while (!buffer.push(i)) {
            // Wait or handle buffer being full
            std::this_thread::yield(); // Yield to other threads
        }
    }
}

void consumer(AtomicBuffer<int>& buffer, int expectedCount) {
    int item;
    for (int i = 0; i < expectedCount; ++i) {
        while (!buffer.pop(item)) {
            // Wait or handle buffer being empty
            std::this_thread::yield(); // Yield to other threads
        }
        assert(item == i); // Verify the item is as expected
        std::cout << "Consumed: " << item << std::endl;
    }
}

int main() {
    AtomicBuffer<int> buffer(10);
    const int expectedCount = 100;

    std::thread producerThread(producer, std::ref(buffer));
    std::thread consumerThread(consumer, std::ref(buffer), expectedCount);

    producerThread.join();
    consumerThread.join();

    std::cout << "All items produced and consumed successfully." << std::endl;

    return 0;
}
