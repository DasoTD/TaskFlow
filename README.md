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

## Demos & Examples

TaskFlow includes several comprehensive demos showcasing real-world parallel programming scenarios:

### 📊 **Built-in Examples**

```bash
# Basic DAG dependency example
./build/Release/dag_demo.exe

# Multi-stage data processing pipeline
./build/Release/data_pipeline.exe
```

### 🚀 **Advanced Demos**

#### **Data Analysis Demo** (`/demos/data_analysis/`)
Real-time data processing and analysis:
- **Simple Analysis**: Statistical computations with parallel processing
- **Streaming Processor**: Continuous data stream processing with TaskFlow

```bash
cd demos/data_analysis
mkdir build && cd build
cmake .. && cmake --build . --config Release
./simple_data_analysis.exe
./streaming_processor.exe
```

#### **vcpkg TaskFlow Demo** (`/demos/vcpkg_taskflow_demo/`)
Professional examples using official TaskFlow from vcpkg:

- **📈 Data Analysis Pipeline**: Parallel statistical analysis (mean, variance, min/max)
- **🤖 ML Pipeline**: Concurrent model training with ensemble creation  
- **🌐 IoT Stream Processing**: Real-time sensor data processing with alerts

```bash
cd demos/vcpkg_taskflow_demo
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg.cmake
cmake --build build --config Release

# Run the demos
./build/Release/vcpkg_data_analysis.exe
./build/Release/vcpkg_ml_pipeline.exe  
./build/Release/vcpkg_iot_stream.exe
```

**Performance Results:**
- Data Analysis: **4x speedup** (32ms → 8ms)
- ML Pipeline: **3.6x speedup** (2000ms → 549ms)  
- IoT Processing: **4x speedup** (125ms → 31ms)

### 🎯 **What Each Demo Teaches**

| Demo | Concept | Real-World Use Case |
|------|---------|-------------------|
| `dag_demo.cpp` | Task Dependencies | Build systems, workflow automation |
| `data_pipeline.cpp` | Multi-stage Processing | ETL pipelines, data transformation |
| `simple_data_analysis.cpp` | Parallel Computation | Scientific computing, analytics |
| `streaming_processor.cpp` | Real-time Processing | Live data feeds, monitoring |
| `vcpkg_data_analysis.cpp` | Statistical Analysis | Financial modeling, research |
| `vcpkg_ml_pipeline.cpp` | Model Training | Machine learning, AI pipelines |
| `vcpkg_iot_stream.cpp` | Sensor Processing | Smart cities, industrial IoT |

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