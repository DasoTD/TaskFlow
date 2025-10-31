cmake_minimum_required(VERSION 3.21)
project(TaskFlow VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options for vcpkg compatibility
option(TASKFLOW_BUILD_EXAMPLES "Build TaskFlow examples" ON)
option(TASKFLOW_BUILD_TESTS "Build TaskFlow tests" ON)
option(TASKFLOW_BUILD_BENCHMARKS "Build TaskFlow benchmarks" ON)

# ---------- dependencies ----------
if(TASKFLOW_BUILD_TESTS OR TASKFLOW_BUILD_BENCHMARKS)
    include(FetchContent)
    
    if(TASKFLOW_BUILD_TESTS)
        FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
        FetchContent_MakeAvailable(googletest)
    endif()
    
    if(TASKFLOW_BUILD_BENCHMARKS)
        FetchContent_Declare(benchmark URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip)
        set(BENCHMARK_ENABLE_TESTING OFF)
        FetchContent_MakeAvailable(benchmark)
    endif()
endif()

# ---------- library ----------
add_library(taskflow src/thread_pool.cpp src/cron_parser.cpp src/scheduler.cpp)
target_include_directories(taskflow PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# ---------- examples ----------
if(TASKFLOW_BUILD_EXAMPLES)
    add_executable(demo examples/dag_demo.cpp)
    target_link_libraries(demo taskflow)

    add_executable(data_pipeline examples/data_pipeline.cpp)
    target_link_libraries(data_pipeline taskflow)
endif()

# ---------- tests ----------
if(TASKFLOW_BUILD_TESTS)
    enable_testing()
    add_executable(test_dag tests/tests_dag.cpp)
    target_link_libraries(test_dag taskflow gtest_main)
    add_test(NAME DAGTest COMMAND test_dag)

    add_executable(simple_test tests/simple_test.cpp)
    target_link_libraries(simple_test gtest)
    add_test(NAME SimpleTest COMMAND simple_test)
endif()

# ---------- benchmarks ----------
if(TASKFLOW_BUILD_BENCHMARKS)
    add_executable(bench_dag benchmarks/bench_dag.cpp)
    target_link_libraries(bench_dag taskflow benchmark::benchmark)
endif()

# ---------- install ----------
include(GNUInstallDirs)
install(TARGETS taskflow
        EXPORT taskflow-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY include/taskflow DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT taskflow-targets
        FILE taskflow-targets.cmake
        NAMESPACE taskflow::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/taskflow)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/taskflow-config-version.cmake"
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion)

install(FILES
        cmake/taskflow-config.cmake
        "${CMAKE_CURRENT_BINARY_DIR}/taskflow-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/taskflow)