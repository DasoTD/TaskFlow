#include <taskflow/scheduler.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <map>

using namespace std::chrono_literals;

struct ProcessingTask {
    std::string name;
    std::string input_file;
    std::string output_file;
    std::chrono::milliseconds duration;
    std::vector<std::string> dependencies;
};

int main() {
    // Example data processing pipeline
    std::vector<ProcessingTask> pipeline = {
        {"extract_data", "raw_data.csv", "extracted.csv", 1000ms, {}},
        {"clean_data", "extracted.csv", "cleaned.csv", 800ms, {"extract_data"}},
        {"analyze_data", "cleaned.csv", "analysis.json", 1500ms, {"clean_data"}},
        {"generate_report", "analysis.json", "report.pdf", 1200ms, {"analyze_data"}},
        {"backup_results", "", "backup.zip", 500ms, {"generate_report"}},
        {"notify_users", "", "", 200ms, {"backup_results"}}
    };

    tf::Scheduler scheduler;
    scheduler.start();

    std::map<std::string, tf::TaskHandle> task_handles;
    
    std::cout << "Starting data processing pipeline...\n\n";

    for (const auto& task_def : pipeline) {
        // Convert dependency names to handles
        std::vector<tf::TaskHandle> deps;
        for (const auto& dep_name : task_def.dependencies) {
            if (task_handles.find(dep_name) != task_handles.end()) {
                deps.push_back(task_handles[dep_name]);
            }
        }

        // Create the task
        auto handle = scheduler.schedule_once(
            tf::Clock::now() + 100ms, 
            [name = task_def.name, input = task_def.input_file, output = task_def.output_file, duration = task_def.duration]() {
                std::cout << "[START] " << name;
                if (!input.empty()) std::cout << " (input: " << input << ")";
                if (!output.empty()) std::cout << " (output: " << output << ")";
                std::cout << std::endl;
                
                // Simulate processing time
                std::this_thread::sleep_for(duration);
                
                std::cout << "[DONE]  " << name << " (took " << duration.count() << "ms)" << std::endl;
            },
            deps
        );

        task_handles[task_def.name] = handle;
    }

    // Wait for the final task to complete
    if (task_handles.find("notify_users") != task_handles.end()) {
        scheduler.wait_for(task_handles["notify_users"]);
    }

    std::cout << "\nPipeline completed successfully!" << std::endl;
    scheduler.stop();
    return 0;
}