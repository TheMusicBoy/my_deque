cmake_minimum_required(VERSION 3.15)

project(Deque)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)

option(BUILD_TESTS "" OFF)

if (BUILD_TESTS)
    message("Building tests...")
    add_subdirectory(test)
endif()