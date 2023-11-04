//AtomicBufferStressTest.cpp
#include "AtomicBuffer.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <cassert>

constexpr int BUFFER_SIZE = 10;
constexpr int PRODUCER_COUNT = 5;
constexpr int CONSUMER_COUNT = 5;
constexpr int ITEMS_PER_PRODUCER = 100;

void producer(AtomicBuffer<int>& buffer, int producer_id) {
    for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
        int item = producer_id * ITEMS_PER_PRODUCER + i; // Unique item for each producer
        while (!buffer.push(item)) {
            std::this_thread::yield(); // Yield to other threads if the buffer is full
        }
    }
}

void consumer(AtomicBuffer<int>& buffer, int& consumed_count) {
    int item;
    while (consumed_count < PRODUCER_COUNT * ITEMS_PER_PRODUCER) {
        if (buffer.pop(item)) {
            ++consumed_count;
        } else {
            std::this_thread::yield(); // Yield to other threads if the buffer is empty
        }
    }
}

int main() {
    AtomicBuffer<int> buffer(BUFFER_SIZE);
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    int total_consumed_count = 0;

    // Start producer threads
    for (int i = 0; i < PRODUCER_COUNT; ++i) {
        producers.emplace_back(producer, std::ref(buffer), i);
    }

    // Start consumer threads
    for (int i = 0; i < CONSUMER_COUNT; ++i) {
        consumers.emplace_back(consumer, std::ref(buffer), std::ref(total_consumed_count));
    }

    // Join producer threads
    for (auto& producer_thread : producers) {
        producer_thread.join();
    }

    // Join consumer threads
    for (auto& consumer_thread : consumers) {
        consumer_thread.join();
    }

    // Check if all items have been consumed
    assert(total_consumed_count == PRODUCER_COUNT * ITEMS_PER_PRODUCER);
    std::cout << "All items produced have been consumed successfully." << std::endl;

    return 0;
}
