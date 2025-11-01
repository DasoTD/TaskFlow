#include <taskflow/scheduler.hpp>
#include <benchmark/benchmark.h>

using namespace std::chrono_literals;

static void BM_Chain(benchmark::State& st) {
    tf::Scheduler s; s.start();
    for (auto _ : st) {
        tf::TaskHandle prev{};
        for (int i = 0; i < 100; ++i) {
            prev = s.schedule_once(tf::Clock::now() + 1us, []{}, prev.is_valid() ? std::vector{prev} : std::vector<tf::TaskHandle>{});
        }
        s.wait_for(prev);
    }
    s.stop();
}
BENCHMARK(BM_Chain);