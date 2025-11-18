# AtomicBuffer Usage Examples

This document provides practical examples of using the AtomicBuffer library in various scenarios.

## Table of Contents

1. [Basic Usage](#basic-usage)
2. [Producer-Consumer Patterns](#producer-consumer-patterns)
3. [Real-World Applications](#real-world-applications)
4. [Performance Optimization Tips](#performance-optimization-tips)
5. [Error Handling](#error-handling)

## Basic Usage

### Simple Push and Pop

```cpp
#include "AtomicBufferFixed.hpp"
#include <iostream>

int main() {
    // Create a buffer that can hold up to 9 items (10 - 1 reserved)
    AtomicBufferFixed<int> buffer(10);

    // Push items
    for (int i = 0; i < 5; ++i) {
        if (buffer.push(i)) {
            std::cout << "Pushed: " << i << std::endl;
        } else {
            std::cout << "Buffer full!" << std::endl;
        }
    }

    // Pop items
    int item;
    while (buffer.pop(item)) {
        std::cout << "Popped: " << item << std::endl;
    }

    return 0;
}
```

### Using Move Semantics

```cpp
#include "AtomicBufferFixed.hpp"
#include <memory>
#include <string>

struct Message {
    std::unique_ptr<std::string> data;
    int priority;

    Message(std::string text, int p)
        : data(std::make_unique<std::string>(std::move(text))),
          priority(p) {}

    // Move-only type
    Message(Message&&) = default;
    Message& operator=(Message&&) = default;
};

int main() {
    AtomicBufferFixed<Message> buffer(100);

    // Push with move
    Message msg("Important message", 1);
    buffer.push(std::move(msg));

    // Pop with move
    Message received(Message("", 0));
    if (buffer.pop(received)) {
        std::cout << "Received: " << *received.data
                  << " (priority: " << received.priority << ")" << std::endl;
    }

    return 0;
}
```

## Producer-Consumer Patterns

### Single Producer, Single Consumer (SPSC)

```cpp
#include "AtomicBufferFixed.hpp"
#include <thread>
#include <iostream>
#include <chrono>

void spscExample() {
    AtomicBufferFixed<int> buffer(128);

    // Producer thread
    std::thread producer([&buffer]() {
        for (int i = 0; i < 1000; ++i) {
            while (!buffer.push(i)) {
                // Buffer full, wait a bit
                std::this_thread::yield();
            }

            // Simulate work
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        std::cout << "Producer finished" << std::endl;
    });

    // Consumer thread
    std::thread consumer([&buffer]() {
        int item;
        int count = 0;
        while (count < 1000) {
            if (buffer.pop(item)) {
                std::cout << "Consumed: " << item << std::endl;
                count++;

                // Simulate processing
                std::this_thread::sleep_for(std::chrono::microseconds(15));
            } else {
                // Buffer empty, wait a bit
                std::this_thread::yield();
            }
        }
        std::cout << "Consumer finished" << std::endl;
    });

    producer.join();
    consumer.join();
}
```

### Multiple Producers, Multiple Consumers (MPMC)

```cpp
#include "AtomicBufferFixed.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <iostream>

void mpmcExample() {
    constexpr int NUM_PRODUCERS = 4;
    constexpr int NUM_CONSUMERS = 4;
    constexpr int ITEMS_PER_PRODUCER = 1000;

    AtomicBufferFixed<int> buffer(256);
    std::atomic<int> totalProduced{0};
    std::atomic<int> totalConsumed{0};

    std::vector<std::thread> threads;

    // Launch producers
    for (int p = 0; p < NUM_PRODUCERS; ++p) {
        threads.emplace_back([&, p]() {
            for (int i = 0; i < ITEMS_PER_PRODUCER; ++i) {
                int item = p * ITEMS_PER_PRODUCER + i;

                while (!buffer.push(item)) {
                    std::this_thread::yield();
                }

                totalProduced.fetch_add(1, std::memory_order_relaxed);
            }
            std::cout << "Producer " << p << " finished" << std::endl;
        });
    }

    // Launch consumers
    for (int c = 0; c < NUM_CONSUMERS; ++c) {
        threads.emplace_back([&, c]() {
            int item;
            int consumed = 0;

            while (totalConsumed.load(std::memory_order_relaxed) <
                   NUM_PRODUCERS * ITEMS_PER_PRODUCER) {

                if (buffer.pop(item)) {
                    consumed++;
                    totalConsumed.fetch_add(1, std::memory_order_relaxed);
                } else {
                    std::this_thread::yield();
                }
            }

            std::cout << "Consumer " << c << " processed " << consumed
                      << " items" << std::endl;
        });
    }

    // Wait for completion
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Total produced: " << totalProduced.load() << std::endl;
    std::cout << "Total consumed: " << totalConsumed.load() << std::endl;
}
```

## Real-World Applications

### 1. Log Message Aggregator

```cpp
#include "AtomicBufferFixed.hpp"
#include <thread>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>

enum class LogLevel {
    DEBUG, INFO, WARNING, ERROR, CRITICAL
};

struct LogMessage {
    LogLevel level;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    std::thread::id threadId;

    LogMessage(LogLevel lvl, std::string msg)
        : level(lvl),
          message(std::move(msg)),
          timestamp(std::chrono::system_clock::now()),
          threadId(std::this_thread::get_id()) {}

    LogMessage(LogMessage&&) = default;
    LogMessage& operator=(LogMessage&&) = default;
};

class AsyncLogger {
    AtomicBufferFixed<LogMessage> buffer;
    std::thread writerThread;
    std::atomic<bool> running;
    std::ofstream logFile;

public:
    explicit AsyncLogger(const std::string& filename, size_t bufferSize = 1024)
        : buffer(bufferSize),
          running(true),
          logFile(filename, std::ios::app) {

        writerThread = std::thread([this]() {
            LogMessage msg(LogLevel::INFO, "");

            while (running.load(std::memory_order_relaxed) ||
                   !buffer.isEmpty()) {

                if (buffer.pop(msg)) {
                    writeToFile(msg);
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        });
    }

    ~AsyncLogger() {
        running.store(false, std::memory_order_relaxed);
        if (writerThread.joinable()) {
            writerThread.join();
        }
    }

    void log(LogLevel level, const std::string& message) {
        LogMessage msg(level, message);
        while (!buffer.push(std::move(msg))) {
            std::this_thread::yield();
        }
    }

private:
    void writeToFile(const LogMessage& msg) {
        auto time = std::chrono::system_clock::to_time_t(msg.timestamp);
        logFile << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
                << " [" << levelToString(msg.level) << "] "
                << msg.message << std::endl;
    }

    static const char* levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::CRITICAL: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }
};

// Usage
void loggerExample() {
    AsyncLogger logger("app.log");

    std::vector<std::thread> workers;
    for (int i = 0; i < 10; ++i) {
        workers.emplace_back([&logger, i]() {
            for (int j = 0; j < 100; ++j) {
                logger.log(LogLevel::INFO,
                          "Worker " + std::to_string(i) +
                          " iteration " + std::to_string(j));
            }
        });
    }

    for (auto& t : workers) {
        t.join();
    }
}
```

### 2. Task Queue for Thread Pool

```cpp
#include "AtomicBufferFixed.hpp"
#include <thread>
#include <vector>
#include <functional>
#include <iostream>

class ThreadPool {
    using Task = std::function<void()>;

    AtomicBufferFixed<Task> taskQueue;
    std::vector<std::thread> workers;
    std::atomic<bool> running;

public:
    explicit ThreadPool(size_t numThreads, size_t queueSize = 1024)
        : taskQueue(queueSize),
          running(true) {

        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this]() {
                Task task;
                while (running.load(std::memory_order_relaxed)) {
                    if (taskQueue.pop(task)) {
                        try {
                            task();
                        } catch (const std::exception& e) {
                            std::cerr << "Task exception: " << e.what() << std::endl;
                        }
                    } else {
                        std::this_thread::yield();
                    }
                }
            });
        }
    }

    ~ThreadPool() {
        running.store(false, std::memory_order_relaxed);
        for (auto& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    bool enqueue(Task task) {
        return taskQueue.push(std::move(task));
    }

    bool tryEnqueue(Task task) {
        return taskQueue.push(std::move(task));
    }

    size_t pendingTasks() const {
        return taskQueue.size();
    }
};

// Usage
void threadPoolExample() {
    ThreadPool pool(4);  // 4 worker threads

    std::atomic<int> completedTasks{0};

    // Enqueue 100 tasks
    for (int i = 0; i < 100; ++i) {
        pool.enqueue([i, &completedTasks]() {
            // Simulate work
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::cout << "Task " << i << " completed by thread "
                      << std::this_thread::get_id() << std::endl;
            completedTasks.fetch_add(1, std::memory_order_relaxed);
        });
    }

    // Wait for all tasks to complete
    while (completedTasks.load(std::memory_order_relaxed) < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "All tasks completed!" << std::endl;
}
```

### 3. Real-Time Data Processing Pipeline

```cpp
#include "AtomicBufferFixed.hpp"
#include <thread>
#include <vector>
#include <cmath>
#include <random>

struct SensorReading {
    int sensorId;
    double value;
    uint64_t timestamp;
};

struct ProcessedData {
    int sensorId;
    double smoothedValue;
    double derivative;
    uint64_t timestamp;
};

class DataPipeline {
    AtomicBufferFixed<SensorReading> rawBuffer;
    AtomicBufferFixed<ProcessedData> processedBuffer;
    std::thread processingThread;
    std::atomic<bool> running;

    std::vector<double> history;
    constexpr static size_t HISTORY_SIZE = 10;

public:
    DataPipeline()
        : rawBuffer(1024),
          processedBuffer(1024),
          running(true),
          history(HISTORY_SIZE, 0.0) {

        processingThread = std::thread([this]() {
            SensorReading reading;
            while (running.load(std::memory_order_relaxed)) {
                if (rawBuffer.pop(reading)) {
                    ProcessedData processed = process(reading);
                    while (!processedBuffer.push(std::move(processed))) {
                        std::this_thread::yield();
                    }
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    ~DataPipeline() {
        running.store(false, std::memory_order_relaxed);
        if (processingThread.joinable()) {
            processingThread.join();
        }
    }

    bool addReading(const SensorReading& reading) {
        return rawBuffer.push(reading);
    }

    bool getProcessed(ProcessedData& data) {
        return processedBuffer.pop(data);
    }

private:
    ProcessedData process(const SensorReading& reading) {
        // Update history
        for (size_t i = HISTORY_SIZE - 1; i > 0; --i) {
            history[i] = history[i - 1];
        }
        history[0] = reading.value;

        // Calculate smoothed value (simple moving average)
        double smoothed = 0.0;
        for (double val : history) {
            smoothed += val;
        }
        smoothed /= HISTORY_SIZE;

        // Calculate derivative (rate of change)
        double derivative = history[0] - history[1];

        return ProcessedData{
            reading.sensorId,
            smoothed,
            derivative,
            reading.timestamp
        };
    }
};

// Usage
void pipelineExample() {
    DataPipeline pipeline;

    // Producer: Simulate sensor data
    std::thread sensorSimulator([&pipeline]() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> dist(25.0, 2.0);

        for (int i = 0; i < 1000; ++i) {
            SensorReading reading{
                1,  // sensor ID
                dist(gen),
                static_cast<uint64_t>(i)
            };

            while (!pipeline.addReading(reading)) {
                std::this_thread::yield();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    // Consumer: Process results
    std::thread dataConsumer([&pipeline]() {
        ProcessedData data;
        int count = 0;

        while (count < 1000) {
            if (pipeline.getProcessed(data)) {
                std::cout << "Sensor " << data.sensorId
                          << ": smoothed=" << data.smoothedValue
                          << ", derivative=" << data.derivative << std::endl;
                count++;
            } else {
                std::this_thread::yield();
            }
        }
    });

    sensorSimulator.join();
    dataConsumer.join();
}
```

## Performance Optimization Tips

### 1. Buffer Sizing

```cpp
// Bad: Buffer too small causes frequent contention
AtomicBufferFixed<int> buffer(4);  // Only 3 usable slots

// Good: Appropriate size for workload
AtomicBufferFixed<int> buffer(128);  // 127 usable slots

// Better: Power of 2 for better cache alignment
AtomicBufferFixed<int> buffer(256);  // 255 usable slots
```

### 2. Adaptive Backoff Strategy

```cpp
#include "AtomicBufferFixed.hpp"
#include <thread>
#include <chrono>

// Simple exponential backoff
class BackoffStrategy {
    int attempts = 0;
    static constexpr int MAX_ATTEMPTS = 10;

public:
    void wait() {
        if (attempts < MAX_ATTEMPTS) {
            for (int i = 0; i < (1 << attempts); ++i) {
                std::this_thread::yield();
            }
            attempts++;
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }

    void reset() {
        attempts = 0;
    }
};

void optimizedPush(AtomicBufferFixed<int>& buffer, int item) {
    BackoffStrategy backoff;

    while (!buffer.push(item)) {
        backoff.wait();
    }

    backoff.reset();
}
```

### 3. Batch Processing

```cpp
#include "AtomicBufferFixed.hpp"
#include <vector>

template<typename T>
void batchPush(AtomicBufferFixed<T>& buffer,
               const std::vector<T>& items) {
    for (const auto& item : items) {
        while (!buffer.push(item)) {
            std::this_thread::yield();
        }
    }
}

template<typename T>
std::vector<T> batchPop(AtomicBufferFixed<T>& buffer,
                        size_t maxItems) {
    std::vector<T> result;
    result.reserve(maxItems);

    T item;
    for (size_t i = 0; i < maxItems; ++i) {
        if (buffer.pop(item)) {
            result.push_back(std::move(item));
        } else {
            break;
        }
    }

    return result;
}
```

## Error Handling

### Graceful Degradation

```cpp
#include "AtomicBufferFixed.hpp"
#include <optional>
#include <chrono>

template<typename T>
std::optional<T> tryPopWithTimeout(AtomicBufferFixed<T>& buffer,
                                   std::chrono::milliseconds timeout) {
    auto start = std::chrono::steady_clock::now();
    T item;

    while (std::chrono::steady_clock::now() - start < timeout) {
        if (buffer.pop(item)) {
            return item;
        }
        std::this_thread::yield();
    }

    return std::nullopt;  // Timeout
}

// Usage
void timeoutExample(AtomicBufferFixed<int>& buffer) {
    auto result = tryPopWithTimeout(buffer, std::chrono::milliseconds(100));

    if (result) {
        std::cout << "Got item: " << *result << std::endl;
    } else {
        std::cout << "Timeout: no item available" << std::endl;
    }
}
```

### Monitoring and Metrics

```cpp
#include "AtomicBufferFixed.hpp"
#include <atomic>
#include <chrono>

class MonitoredBuffer {
    AtomicBufferFixed<int> buffer;
    std::atomic<uint64_t> pushAttempts{0};
    std::atomic<uint64_t> pushSuccesses{0};
    std::atomic<uint64_t> popAttempts{0};
    std::atomic<uint64_t> popSuccesses{0};

public:
    explicit MonitoredBuffer(size_t size) : buffer(size) {}

    bool push(int item) {
        pushAttempts.fetch_add(1, std::memory_order_relaxed);
        bool success = buffer.push(item);
        if (success) {
            pushSuccesses.fetch_add(1, std::memory_order_relaxed);
        }
        return success;
    }

    bool pop(int& item) {
        popAttempts.fetch_add(1, std::memory_order_relaxed);
        bool success = buffer.pop(item);
        if (success) {
            popSuccesses.fetch_add(1, std::memory_order_relaxed);
        }
        return success;
    }

    void printMetrics() const {
        std::cout << "Push success rate: "
                  << (100.0 * pushSuccesses.load() / pushAttempts.load())
                  << "%\n";
        std::cout << "Pop success rate: "
                  << (100.0 * popSuccesses.load() / popAttempts.load())
                  << "%\n";
        std::cout << "Current size: " << buffer.size() << "\n";
    }
};
```

## Conclusion

The AtomicBuffer provides a powerful foundation for lock-free inter-thread communication. Choose the appropriate pattern based on your use case:

- **SPSC**: Highest performance, use when possible
- **MPSC/SPMC**: Good performance, moderate contention
- **MPMC**: Most flexible, highest contention

Always profile your specific workload to determine optimal buffer size and backoff strategies.
