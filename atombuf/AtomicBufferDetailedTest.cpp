// AtomicBufferDetailedTest.cpp
#include "AtomicBuffer.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <cassert>

constexpr int BUFFER_SIZE = 10;
constexpr int PRODUCER_COUNT = 5;
constexpr int CONSUMER_COUNT = 5;
constexpr int ITEMS_PER_PRODUCER = 100;

std::atomic<int> itemsProduced(0);
std::atomic<int> itemsConsumed(0);
std::mutex cout_mutex;
std::vector<int> producerCount(PRODUCER_COUNT, 0);
std::vector<int> consumerCount(CONSUMER_COUNT, 0);

void producer(AtomicBuffer<int>& buffer, int producer_id) {
    for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
        int item = producer_id * ITEMS_PER_PRODUCER + i;
        while (!buffer.push(item)) {
            std::this_thread::yield();
        }
        itemsProduced.fetch_add(1, std::memory_order_relaxed);
        producerCount[producer_id]++;
        {
            std::lock_guard<std::mutex> guard(cout_mutex);
            std::cout << "Producer " << producer_id << " produced item: " << item << std::endl;
        }
    }
}

void consumer(AtomicBuffer<int>& buffer, int consumer_id) {
    int item;
    while (itemsConsumed.load(std::memory_order_relaxed) < PRODUCER_COUNT * ITEMS_PER_PRODUCER) {
        if (buffer.pop(item)) {
            itemsConsumed.fetch_add(1, std::memory_order_relaxed);
            consumerCount[consumer_id]++;
            {
                std::lock_guard<std::mutex> guard(cout_mutex);
                std::cout << "Consumer " << consumer_id << " consumed item: " << item << std::endl;
            }
        } else {
            std::this_thread::yield();
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

    // Detailed logging of each producer and consumer count
    {
        std::lock_guard<std::mutex> guard(cout_mutex);
        for (int i = 0; i < PRODUCER_COUNT; ++i) {
            std::cout << "Producer " << i << " produced " << producerCount[i] << " items." << std::endl;
        }
        for (int i = 0; i < CONSUMER_COUNT; ++i) {
            std::cout << "Consumer " << i << " consumed " << consumerCount[i] << " items." << std::endl;
        }
    }

    // Check if all items have been consumed
    assert(itemsProduced.load(std::memory_order_relaxed) == PRODUCER_COUNT * ITEMS_PER_PRODUCER);
    assert(itemsConsumed.load(std::memory_order_relaxed) == PRODUCER_COUNT * ITEMS_PER_PRODUCER);
    std::cout << "All items produced have been consumed successfully." << std::endl;

    return 0;
}
