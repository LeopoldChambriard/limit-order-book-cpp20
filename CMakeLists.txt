cmake_minimum_required(VERSION 3.20)
project(limit_order_book_cpp20 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Drapeaux d'optimisation haute performance
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -march=native)
endif()

include(FetchContent)

# 1. Bibliothèque principale LOB
add_library(lob_core
    src/OrderBook.cpp
    src/MatchingEngine.cpp
)

target_include_directories(lob_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 2. Tests unitaires (GoogleTest)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(test_matching tests/test_matching.cpp)
target_link_libraries(test_matching PRIVATE lob_core GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(test_matching)

# 3. Benchmarks de latence (Google Benchmark)
FetchContent_Declare(
    googlebenchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(bench_lob benchmarks/bench_lob.cpp)
target_link_libraries(bench_lob PRIVATE lob_core benchmark::benchmark)