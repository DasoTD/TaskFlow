#include <taskflow/scheduler.hpp>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

int main() {
    tf::Scheduler s;
    s.start();

    auto backup = s.schedule_once(std::chrono::steady_clock::now() + 1s, []{
        std::cout << "[1] backup\n";
    });

    auto report = s.schedule_once(std::chrono::steady_clock::now() + 2s, []{
        std::cout << "[2] report\n";
    }, {backup});

    auto notify = s.schedule_once(std::chrono::steady_clock::now() + 3s, []{
        std::cout << "[3] notify\n";
    }, {backup, report});

    s.wait_for(notify);
    s.stop();
}   