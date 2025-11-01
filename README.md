# TaskFlow

A modern C++20 task scheduling library with DAG (Directed Acyclic Graph) dependencies, cron scheduling, and thread pool management.

## Features

- ⚡ **High Performance**: Built on a custom thread pool with minimal overhead
- 🔗 **DAG Dependencies**: Tasks can depend on other tasks completing first
- ⏰ **Cron Scheduling**: Schedule recurring tasks with cron expressions
- 🕒 **Time-based Scheduling**: Schedule tasks to run at specific times or intervals  
- 🔄 **Recurring Tasks**: Support for repeating tasks with intervals
- 🛡️ **Thread Safe**: All operations are thread-safe and lock-free where possible
- 📦 **Easy Integration**: Simple CMake integration

## Quick Start

### Installation

#### Using vcpkg (Recommended)
```bash
vcpkg install taskflow
```

#### Using CMake
```bash
git clone https://github.com/DasoTD/TaskFlow.git
cd TaskFlow
mkdir build && cd build
cmake .. -DTASKFLOW_BUILD_EXAMPLES=ON
cmake --build . --config Release
```

### Basic Usage

```cpp
#include <taskflow/taskflow.hpp>
#include <iostream>

int main() {
    tf::Taskflow taskflow("Simple Pipeline");
    tf::Executor executor;

    // Create tasks with dependencies
    auto backup = taskflow.emplace([](){ 
        std::cout << "[1] backup" << std::endl; 
    });

    auto report = taskflow.emplace([](){ 
        std::cout << "[2] report" << std::endl; 
    });

    auto notify = taskflow.emplace([](){ 
        std::cout << "[3] notify" << std::endl; 
    });

    // Define dependencies
    backup.succeed(report);     // report runs after backup
    report.succeed(notify);     // notify runs after report

    // Execute the taskflow
    executor.run(taskflow).wait();
    
    return 0;
}
```

## Testing with Data

Run the included examples to see TaskFlow in action:

```bash
# Basic DAG example
./build/Release/demo.exe

# Data processing pipeline
./build/Release/data_pipeline.exe
```

The data pipeline example shows a realistic workflow:
1. Extract data → Clean data → Analyze data → Generate report → Backup → Notify users

## vcpkg Integration

To make TaskFlow available via vcpkg:

1. **Create vcpkg port files** (already included in `/ports/taskflow/`)
2. **Add to vcpkg registry**:
   ```bash
   # Fork vcpkg repository
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   
   # Copy port files
   cp -r /path/to/TaskFlow/ports/taskflow ports/
   
   # Update versions
   ./vcpkg x-add-version taskflow
   ```

3. **Users can then install**:
   ```bash
   vcpkg install taskflow
   ```

## CMake Options

```cmake
option(TASKFLOW_BUILD_EXAMPLES "Build examples" ON)
option(TASKFLOW_BUILD_TESTS "Build tests" ON) 
option(TASKFLOW_BUILD_BENCHMARKS "Build benchmarks" ON)
```

## Performance

TaskFlow is designed for production use:
- Efficient dependency resolution
- Minimal overhead thread pool  
- Lock-free operations where possible
- Scales to hundreds of concurrent tasks

## License

MIT License - see LICENSE file for details.