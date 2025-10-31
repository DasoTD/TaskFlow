#include <taskflow/scheduler.hpp>
#include <unordered_map>
#include <shared_mutex>
#include <format>
#include <sstream>
#include <iomanip>

namespace tf {

struct Scheduler::Impl {
    ThreadPool pool;
    std::vector<ScheduledTask> tasks;
    std::unordered_map<uint64_t, size_t> id2idx;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> running{false};
    std::thread worker;
    uint64_t next_id{1};

    Impl(size_t n) : pool(n) {}

    TaskHandle add(ScheduledTask st) {
        std::lock_guard<std::mutex> lk(mtx);
        st.pending_deps = static_cast<int>(st.dependencies.size());
        size_t idx = tasks.size();
        TaskHandle h{next_id++};
        
        // Set up dependency relationships - add this task as a dependent of its dependencies
        for (auto dep_handle : st.dependencies) {
            auto dep_it = id2idx.find(dep_handle.id);
            if (dep_it != id2idx.end()) {
                tasks[dep_it->second].dependents.push_back(h);
            }
        }
        
        tasks.push_back(std::move(st));
        id2idx[h.id] = idx;
        return h;
    }

    void completed(uint64_t id) {
        std::unique_lock<std::mutex> lk(mtx);
        auto it = id2idx.find(id);
        if (it == id2idx.end()) return;
        size_t idx = it->second;
        for (auto dep : tasks[idx].dependents) {
            auto dit = id2idx.find(dep.id);
            if (dit == id2idx.end()) continue;
            if (--tasks[dit->second].pending_deps == 0) cv.notify_one();
        }
    }

    void loop() {
        while (running) {
            auto now = Clock::now();
            std::vector<size_t> ready;
            { std::lock_guard<std::mutex> lk(mtx);
                for (size_t i = 0; i < tasks.size(); ++i) {
                    auto& t = tasks[i];
                    if (!t.canceled && t.next_run <= now && t.pending_deps == 0) ready.push_back(i);
                }
            }
            for (auto i : ready) {
                auto& task = tasks[i];
                uint64_t task_id = 0;
                { std::lock_guard<std::mutex> lk(mtx);
                  for (const auto& [id, idx] : id2idx) {
                      if (idx == i) { task_id = id; break; }
                  }
                }
                pool.enqueue([this, i, task_id]() {
                    auto& t = tasks[i];
                    t.run();
                    if (t.recurring) {
                        // Reschedule the same task instead of creating a new one
                        std::lock_guard<std::mutex> lk(mtx);
                        t.next_run = Clock::now() + t.interval;
                        // Create new promise/future for next execution
                        t.completion = std::promise<std::optional<std::any>>{};
                    }
                    completed(task_id);
                });
                // Only set next_run to max for non-recurring tasks
                { std::lock_guard<std::mutex> lk(mtx); 
                  if (!tasks[i].recurring) {
                      tasks[i].next_run = TimePoint::max(); 
                  }
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};

Scheduler::Scheduler(size_t n) { impl_ = std::make_unique<Impl>(n); }
Scheduler::~Scheduler() { stop(); }

void Scheduler::start() {
    if (impl_->running) return;
    impl_->running = true;
    impl_->worker = std::thread(&Impl::loop, impl_.get());
}
void Scheduler::stop() {
    if (!impl_->running) return;
    impl_->running = false;
    impl_->cv.notify_all();
    if (impl_->worker.joinable()) impl_->worker.join();
}
void Scheduler::wait() { if (impl_->worker.joinable()) impl_->worker.join(); }

TaskHandle Scheduler::create_task(ScheduledTask&& st) { return impl_->add(std::move(st)); }

TaskHandle Scheduler::schedule_once(const std::string& iso, Task t,
                                   const std::vector<TaskHandle>& d) {
    std::tm tm{}; 
    std::istringstream ss(iso);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    
    // Convert from system_clock to steady_clock
    auto system_now = std::chrono::system_clock::now();
    auto steady_now = Clock::now();
    auto duration_from_now = tp - system_now;
    auto steady_target = steady_now + duration_from_now;
    
    return schedule_once(steady_target, std::move(t), d);
}
TaskHandle Scheduler::schedule_once(TimePoint tp, Task t,
                                   const std::vector<TaskHandle>& d) {
    ScheduledTask st;
    st.func = t;
    st.next_run = tp;
    st.dependencies = d;
    return create_task(std::move(st));
}
TaskHandle Scheduler::schedule_recurring(const std::string& cron, Task t,
                                        const std::vector<TaskHandle>& d) {
    auto s = parse_cron(cron); 
    if (!s.valid) return {};
    auto nxt = next_cron_time(s);
    
    // Convert from system_clock to steady_clock
    auto system_now = std::chrono::system_clock::now();
    auto steady_now = Clock::now();
    auto duration_from_now = nxt - system_now;
    auto steady_target = steady_now + duration_from_now;
    
    ScheduledTask st;
    st.func = t;
    st.next_run = steady_target;
    st.interval = std::chrono::hours(24);
    st.recurring = true;
    st.cron_expr = cron;
    st.dependencies = d;
    return create_task(std::move(st));
}
TaskHandle Scheduler::schedule_every(Duration i, Task t,
                                    const std::vector<TaskHandle>& d) {
    ScheduledTask st;
    st.func = t;
    st.next_run = Clock::now() + i;
    st.interval = i;
    st.recurring = true;
    st.dependencies = d;
    return create_task(std::move(st));
}



void Scheduler::wait_for(TaskHandle h) {
    auto it = impl_->id2idx.find(h.id);
    if (it == impl_->id2idx.end()) return;
    impl_->tasks[it->second].future().wait();
}
template<class T>
T Scheduler::get_result(TaskHandle h) {
    auto it = impl_->id2idx.find(h.id);
    if (it == impl_->id2idx.end()) throw std::runtime_error("bad handle");
    auto f = impl_->tasks[it->second].future().get();
    if (!f) throw std::runtime_error("no result");
    return std::any_cast<T>(*f);
}

}  // namespace tf