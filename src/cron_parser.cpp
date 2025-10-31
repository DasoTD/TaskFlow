#include <taskflow/cron_parser.hpp>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <vector>

namespace tf {

CronSchedule parse_cron(const std::string& expr) {
    CronSchedule s;
    std::istringstream iss(expr);
    std::vector<std::string> f;
    std::string t;
    while (std::getline(iss, t, ' ')) f.push_back(t);
    if (f.size() != 5) return s;

    auto parse = [](const std::string& v, int min, int max) -> int {
        if (v == "*") return -1;
        if (v.find("*/") == 0) { int step = std::stoi(v.substr(2)); return (step >= min && step <= max) ? step : -1; }
        return std::stoi(v);
    };

    try {
        s.minute = parse(f[0],0,59);
        s.hour   = parse(f[1],0,23);
        s.day_of_month = parse(f[2],1,31);
        s.month = parse(f[3],1,12);
        s.day_of_week = parse(f[4],0,6);
        s.valid = true;
    } catch(...) { s.valid = false; }
    return s;
}

std::chrono::system_clock::time_point next_cron_time(const CronSchedule& s) {
    auto now = std::chrono::system_clock::now();
    auto tp = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&tp);
    tm.tm_sec = 0; ++tm.tm_min;
    if (tm.tm_min >= 60) { tm.tm_min = 0; ++tm.tm_hour; }
    if (s.minute >= 0) tm.tm_min = s.minute;
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

}  // namespace tf