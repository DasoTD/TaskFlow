#pragma once
#include "task.hpp"
#include "thread_pool.hpp"
#include "cron_parser.hpp"
#include <memory>
#include <vector>
#include <atomic>
#include <future>
#include <any>
#include <sstream>
#include <iomanip>
#include <type_traits>

namespace tf {

class Scheduler {
public:
    Scheduler(size_t threads = std::thread::hardware_concurrency());
    ~Scheduler();

    void start();
    void stop();
    void wait();

    // one-time
    TaskHandle schedule_once(const std::string& iso, Task task,
                            const std::vector<TaskHandle>& deps = {});
    TaskHandle schedule_once(TimePoint tp, Task task,
                            const std::vector<TaskHandle>& deps = {});

    // recurring
    TaskHandle schedule_recurring(const std::string& cron, Task task,
                                 const std::vector<TaskHandle>& deps = {});
    TaskHandle schedule_every(Duration d, Task task,
                             const std::vector<TaskHandle>& deps = {});

    // result-bearing
    template<class F>
    auto schedule_once(const std::string& iso, F f,
                       const std::vector<TaskHandle>& deps = {})
        -> TaskHandle {
        std::tm tm{}; 
        std::istringstream ss(iso);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        
        // Convert from system_clock to steady_clock
        auto system_now = std::chrono::system_clock::now();
        auto steady_now = Clock::now();
        auto duration_from_now = tp - system_now;
        auto steady_target = steady_now + duration_from_now;
        
        using R = decltype(f());
        if constexpr (std::is_void_v<R>) {
            Task task = [f = std::move(f)]() mutable { f(); };
            return schedule_once(steady_target, task, deps);
        } else {
            // For non-void return types, we'd need to handle result storage
            Task task = [f = std::move(f)]() mutable { f(); };
            return schedule_once(steady_target, task, deps);
        }
    }

    void wait_for(TaskHandle h);
    template<class T = void>
    T get_result(TaskHandle h);

private:
    TaskHandle create_task(ScheduledTask&& st);
    void run_loop();

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace tf