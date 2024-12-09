cmake_minimum_required(VERSION 3.16)

project(mst-bench VERSION 1.0 LANGUAGES CXX)

# for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)
FetchContent_Declare(
    micro_ut_boost
    GIT_REPOSITORY https://github.com/boost-ext/ut.git
)
FetchContent_MakeAvailable(micro_ut_boost)

file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB BENCH_FILES "${CMAKE_SOURCE_DIR}/mst-bench/*.cpp")
file(GLOB TEST_FILES "${CMAKE_SOURCE_DIR}/tests/*.cpp")

# library for computing mst
add_library(mst-lib ${SRC_FILES})
target_include_directories(mst-lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_options(mst-lib PRIVATE -g -Wall -Wextra -pedantic)

# executable with cli interface for runing stuff needed for the benchmark
add_executable(${PROJECT_NAME} ${BENCH_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} argparse mst-lib)
target_compile_options(${PROJECT_NAME} PRIVATE -g -Wall -Wextra -pedantic)

# unit tests
message(${TEST_FILES})
add_executable(tests ${TEST_FILES})
target_link_libraries(tests ut mst-lib)
target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/include)
