// AtomicBufferLoggedTest.cpp
#include "AtomicBufferLogged.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <cassert>

constexpr int BUFFER_SIZE = 10;
constexpr int PRODUCER_COUNT = 5;
constexpr int CONSUMER_COUNT = 5;
constexpr int ITEMS_PER_PRODUCER = 100;

std::atomic<int> itemsProduced(0);
std::atomic<int> itemsConsumed(0);
std::vector<int> producerCount(PRODUCER_COUNT, 0);
std::vector<int> consumerCount(CONSUMER_COUNT, 0);

void producer(AtomicBuffer<int>& buffer, int producer_id) {
    for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
        int item = producer_id * ITEMS_PER_PRODUCER + i;
        while (!buffer.push(item)) {
            std::this_thread::yield(); // Yield to other threads if the buffer is full
        }
        itemsProduced.fetch_add(1, std::memory_order_relaxed);
        producerCount[producer_id]++;
    }
}

void consumer(AtomicBuffer<int>& buffer, int consumer_id) {
    int item;
    while (itemsConsumed.load(std::memory_order_relaxed) < PRODUCER_COUNT * ITEMS_PER_PRODUCER) {
        if (buffer.pop(item)) {
            itemsConsumed.fetch_add(1, std::memory_order_relaxed);
            consumerCount[consumer_id]++;
        } else {
            std::this_thread::yield(); // Yield to other threads if the buffer is empty
        }
    }
}

int main() {
    AtomicBuffer<int> buffer(BUFFER_SIZE);
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // Start producer threads
    for (int i = 0; i < PRODUCER_COUNT; ++i) {
        producers.emplace_back(producer, std::ref(buffer), i);
    }

    // Start consumer threads
    for (int i = 0; i < CONSUMER_COUNT; ++i) {
        consumers.emplace_back(consumer, std::ref(buffer), i);
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
    assert(itemsProduced.load(std::memory_order_relaxed) == PRODUCER_COUNT * ITEMS_PER_PRODUCER);
    assert(itemsConsumed.load(std::memory_order_relaxed) == PRODUCER_COUNT * ITEMS_PER_PRODUCER);

    std::cout << "Test completed successfully. All items produced have been consumed." << std::endl;

    return 0;
}
