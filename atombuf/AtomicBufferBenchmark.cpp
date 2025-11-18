// AtomicBufferBenchmark.cpp
// Performance benchmarking for AtomicBuffer implementations
#include "AtomicBufferFixed.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <sstream>

// Benchmark configuration
struct BenchmarkConfig {
    size_t bufferSize;
    int numProducers;
    int numConsumers;
    int itemsPerProducer;
    std::string name;
};

// Benchmark result
struct BenchmarkResult {
    std::string name;
    double durationSeconds;
    size_t totalItems;
    double throughputOpsPerSec;
    double avgLatencyMicros;
};

// RAII timer class
class Timer {
    std::chrono::high_resolution_clock::time_point start;
public:
    Timer() : start(std::chrono::high_resolution_clock::now()) {}

    double elapsed() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - start).count();
    }

    double elapsedMillis() const {
        return elapsed() * 1000.0;
    }

    double elapsedMicros() const {
        return elapsed() * 1000000.0;
    }
};

// Producer function for benchmark
template<typename BufferType>
void benchmarkProducer(BufferType& buffer, int producerId, int itemsPerProducer, std::atomic<size_t>& produced) {
    for (int i = 0; i < itemsPerProducer; ++i) {
        int item = producerId * itemsPerProducer + i;
        while (!buffer.push(item)) {
            std::this_thread::yield();
        }
        produced.fetch_add(1, std::memory_order_relaxed);
    }
}

// Consumer function for benchmark
template<typename BufferType>
void benchmarkConsumer(BufferType& buffer, size_t expectedTotal, std::atomic<size_t>& consumed) {
    int item;
    while (consumed.load(std::memory_order_relaxed) < expectedTotal) {
        if (buffer.pop(item)) {
            consumed.fetch_add(1, std::memory_order_relaxed);
        } else {
            std::this_thread::yield();
        }
    }
}

// Run a single benchmark
template<typename BufferType>
BenchmarkResult runBenchmark(const BenchmarkConfig& config) {
    BufferType buffer(config.bufferSize);
    std::atomic<size_t> itemsProduced{0};
    std::atomic<size_t> itemsConsumed{0};

    std::vector<std::thread> threads;
    threads.reserve(config.numProducers + config.numConsumers);

    size_t expectedTotal = config.numProducers * config.itemsPerProducer;

    Timer timer;

    // Launch producers
    for (int i = 0; i < config.numProducers; ++i) {
        threads.emplace_back(benchmarkProducer<BufferType>,
                           std::ref(buffer), i, config.itemsPerProducer,
                           std::ref(itemsProduced));
    }

    // Launch consumers
    for (int i = 0; i < config.numConsumers; ++i) {
        threads.emplace_back(benchmarkConsumer<BufferType>,
                           std::ref(buffer), expectedTotal,
                           std::ref(itemsConsumed));
    }

    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }

    double duration = timer.elapsed();

    BenchmarkResult result;
    result.name = config.name;
    result.durationSeconds = duration;
    result.totalItems = expectedTotal;
    result.throughputOpsPerSec = expectedTotal / duration;
    result.avgLatencyMicros = (duration * 1000000.0) / expectedTotal;

    return result;
}

// Print benchmark results
void printResults(const std::vector<BenchmarkResult>& results) {
    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << "BENCHMARK RESULTS\n";
    std::cout << std::string(100, '=') << "\n\n";

    std::cout << std::left << std::setw(40) << "Benchmark"
              << std::right << std::setw(15) << "Duration (s)"
              << std::setw(15) << "Total Items"
              << std::setw(18) << "Throughput (op/s)"
              << std::setw(20) << "Avg Latency (μs)"
              << "\n";
    std::cout << std::string(100, '-') << "\n";

    for (const auto& result : results) {
        std::cout << std::left << std::setw(40) << result.name
                  << std::right << std::setw(15) << std::fixed << std::setprecision(3) << result.durationSeconds
                  << std::setw(15) << result.totalItems
                  << std::setw(18) << std::fixed << std::setprecision(0) << result.throughputOpsPerSec
                  << std::setw(20) << std::fixed << std::setprecision(3) << result.avgLatencyMicros
                  << "\n";
    }

    std::cout << std::string(100, '=') << "\n";
}

// Print system info
void printSystemInfo() {
    std::cout << "\nSYSTEM INFORMATION\n";
    std::cout << std::string(100, '-') << "\n";
    std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << " threads\n";
    std::cout << "C++ Standard: " << __cplusplus << "\n";

#ifdef __GNUC__
    std::cout << "Compiler: GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << "\n";
#elif defined(__clang__)
    std::cout << "Compiler: Clang " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__ << "\n";
#elif defined(_MSC_VER)
    std::cout << "Compiler: MSVC " << _MSC_VER << "\n";
#endif

    std::cout << std::string(100, '-') << "\n\n";
}

int main() {
    printSystemInfo();

    std::vector<BenchmarkResult> results;

    std::cout << "Running benchmarks...\n\n";

    // Benchmark 1: Single Producer, Single Consumer (SPSC)
    {
        std::cout << "Running SPSC benchmark...\n";
        BenchmarkConfig config{128, 1, 1, 1000000, "SPSC (1P/1C, 1M items)"};
        results.push_back(runBenchmark<AtomicBufferFixed<int>>(config));
    }

    // Benchmark 2: Multiple Producers, Single Consumer (MPSC)
    {
        std::cout << "Running MPSC benchmark...\n";
        BenchmarkConfig config{128, 4, 1, 250000, "MPSC (4P/1C, 1M items)"};
        results.push_back(runBenchmark<AtomicBufferFixed<int>>(config));
    }

    // Benchmark 3: Single Producer, Multiple Consumers (SPMC)
    {
        std::cout << "Running SPMC benchmark...\n";
        BenchmarkConfig config{128, 1, 4, 1000000, "SPMC (1P/4C, 1M items)"};
        results.push_back(runBenchmark<AtomicBufferFixed<int>>(config));
    }

    // Benchmark 4: Multiple Producers, Multiple Consumers (MPMC)
    {
        std::cout << "Running MPMC benchmark...\n";
        BenchmarkConfig config{128, 4, 4, 250000, "MPMC (4P/4C, 1M items)"};
        results.push_back(runBenchmark<AtomicBufferFixed<int>>(config));
    }

    // Benchmark 5: High Contention (many threads, small buffer)
    {
        std::cout << "Running high contention benchmark...\n";
        BenchmarkConfig config{16, 8, 8, 125000, "High Contention (8P/8C, small buffer)"};
        results.push_back(runBenchmark<AtomicBufferFixed<int>>(config));
    }

    // Benchmark 6: Low Contention (few threads, large buffer)
    {
        std::cout << "Running low contention benchmark...\n";
        BenchmarkConfig config{4096, 2, 2, 500000, "Low Contention (2P/2C, large buffer)"};
        results.push_back(runBenchmark<AtomicBufferFixed<int>>(config));
    }

    // Benchmark 7: Stress test (many threads, many items)
    {
        std::cout << "Running stress test benchmark...\n";
        BenchmarkConfig config{256, 16, 16, 62500, "Stress Test (16P/16C, 1M items)"};
        results.push_back(runBenchmark<AtomicBufferFixed<int>>(config));
    }

    // Benchmark 8: Burst workload (small bursts)
    {
        std::cout << "Running burst workload benchmark...\n";
        BenchmarkConfig config{64, 8, 8, 12500, "Burst Workload (8P/8C, 100K items)"};
        results.push_back(runBenchmark<AtomicBufferFixed<int>>(config));
    }

    printResults(results);

    std::cout << "\nBenchmark suite completed successfully.\n";

    return 0;
}
