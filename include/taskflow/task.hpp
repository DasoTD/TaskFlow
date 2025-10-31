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
    std::variant<Task, TaskWithResult<void>> func;
    TimePoint next_run;
    Duration interval{};
    bool recurring = false;
    bool canceled = false;
    std::string cron_expr;

    std::vector<TaskHandle> dependencies;
    std::vector<TaskHandle> dependents;
    std::atomic<int> pending_deps{0};

    std::promise<std::optional<std::any>> completion;
    std::shared_future<std::optional<std::any>> future() const { return completion.get_future(); }

    void run() {
        if (canceled) return;
        try {
            if (auto* t = std::get_if<Task>(&func)) { (*t)(); completion.set_value(std::nullopt); }
            else if (auto* t = std::get_if<TaskWithResult<void>>(&func)) { (*t)(); completion.set_value(std::nullopt); }
        } catch (...) { completion.set_exception(std::current_exception()); }
    }
};

}  // namespace tf