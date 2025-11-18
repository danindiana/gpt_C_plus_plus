#pragma once

#include <thread>
#include <queue>
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <memory>
#include <atomic>

namespace dfs_search {

/**
 * @brief Modern C++20 thread pool implementation using std::jthread
 *
 * Provides a reusable thread pool for parallel task execution.
 * Uses C++20 std::jthread for automatic cleanup and stop tokens.
 */
class ThreadPool {
public:
    /**
     * @brief Construct thread pool with specified number of threads
     * @param num_threads Number of worker threads (default: hardware concurrency)
     */
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency())
        : stop_(false) {

        if (num_threads == 0) {
            num_threads = std::thread::hardware_concurrency();
        }

        workers_.reserve(num_threads);

        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] { this->worker_thread(); });
        }
    }

    /**
     * @brief Destructor - stops all threads and waits for completion
     */
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();

        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    // Delete copy and move operations
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    /**
     * @brief Enqueue a task for execution
     * @tparam F Callable type
     * @tparam Args Argument types
     * @param f Function to execute
     * @param args Arguments to pass to function
     * @return Future for the result
     */
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>> {

        using return_type = typename std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> result = task->get_future();

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            if (stop_) {
                throw std::runtime_error("Cannot enqueue on stopped ThreadPool");
            }

            tasks_.emplace([task]() { (*task)(); });
        }

        condition_.notify_one();
        return result;
    }

    /**
     * @brief Get number of worker threads
     */
    [[nodiscard]] size_t size() const {
        return workers_.size();
    }

    /**
     * @brief Get number of pending tasks
     */
    [[nodiscard]] size_t pending() const {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        return tasks_.size();
    }

private:
    void worker_thread() {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(queue_mutex_);

                condition_.wait(lock, [this] {
                    return stop_ || !tasks_.empty();
                });

                if (stop_ && tasks_.empty()) {
                    return;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            task();
        }
    }

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
};

} // namespace dfs_search
