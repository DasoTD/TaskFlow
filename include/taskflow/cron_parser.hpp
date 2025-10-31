#pragma once
#include <string>
#include <chrono>

namespace tf {

struct CronSchedule {
    int minute = -1, hour = -1, day_of_month = -1, month = -1, day_of_week = -1;
    bool valid = false;
};

CronSchedule parse_cron(const std::string& expr);
std::chrono::system_clock::time_point next_cron_time(const CronSchedule& s);

}  // namespace tf