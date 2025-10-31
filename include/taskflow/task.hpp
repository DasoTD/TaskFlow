#pragma once
#include <functional>
#include <chrono>
#include <optional>
#include <memory>
#include <variant>
#include <any>
#include <atomic>
#include <future>

namespace tf {

using Task = std::function<void()>;
using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using Duration = Clock::duration;

struct TaskHandle {
    uint64_t id = 0;
    bool is_valid() const { return id != 0; }
    bool operator==(const TaskHandle& o) const { return id == o.id; }
};

template<typename T = void>
using TaskWithResult = std::function<T()>;

struct ScheduledTask {
    Task func;
    TimePoint next_run;
    Duration interval{};
    bool recurring = false;
    bool canceled = false;
    std::string cron_expr;

    std::vector<TaskHandle> dependencies;
    std::vector<TaskHandle> dependents;
    std::atomic<int> pending_deps{0};

    std::promise<std::optional<std::any>> completion;
    
    // Make it movable
    ScheduledTask() = default;
    ScheduledTask(const ScheduledTask&) = delete;
    ScheduledTask& operator=(const ScheduledTask&) = delete;
    ScheduledTask(ScheduledTask&& other) noexcept 
        : func(std::move(other.func)), next_run(other.next_run), interval(other.interval),
          recurring(other.recurring), canceled(other.canceled), cron_expr(std::move(other.cron_expr)),
          dependencies(std::move(other.dependencies)), dependents(std::move(other.dependents)),
          pending_deps(other.pending_deps.load()), completion(std::move(other.completion)) {}
    
    ScheduledTask& operator=(ScheduledTask&& other) noexcept {
        if (this != &other) {
            func = std::move(other.func);
            next_run = other.next_run;
            interval = other.interval;
            recurring = other.recurring;
            canceled = other.canceled;
            cron_expr = std::move(other.cron_expr);
            dependencies = std::move(other.dependencies);
            dependents = std::move(other.dependents);
            pending_deps = other.pending_deps.load();
            completion = std::move(other.completion);
        }
        return *this;
    }
    
    std::shared_future<std::optional<std::any>> future() { return completion.get_future(); }

    void run() {
        if (canceled) return;
        try {
            func();
            completion.set_value(std::nullopt);
        } catch (...) { 
            completion.set_exception(std::current_exception()); 
        }
    }
};

}  // namespace tf