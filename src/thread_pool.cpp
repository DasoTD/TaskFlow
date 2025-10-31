#include <taskflow/thread_pool.hpp>

namespace tf {

ThreadPool::ThreadPool(size_t n) {
    for (size_t i = 0; i < n; ++i) {
        workers_.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lk(mtx_);
                    cv_.wait(lk, [this]{ return stop_ || !tasks_.empty(); });
                    if (stop_ && tasks_.empty()) return;
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                try { task(); } catch (...) {}
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    { std::lock_guard<std::mutex> lk(mtx_); stop_ = true; }
    cv_.notify_all();
    for (auto& w : workers_) if (w.joinable()) w.join();
}

void ThreadPool::enqueue(std::function<void()> task) {
    { std::lock_guard<std::mutex> lk(mtx_); if (stop_) return; tasks_.push(std::move(task)); }
    cv_.notify_one();
}

}  // namespace tf