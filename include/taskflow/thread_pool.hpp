#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

namespace tf {

class ThreadPool {
public:
    explicit ThreadPool(size_t n = std::thread::hardware_concurrency());
    ~ThreadPool();
    void enqueue(std::function<void()> task);
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};
};

}  // namespace tf