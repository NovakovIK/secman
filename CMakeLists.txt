cmake_minimum_required(VERSION 3.9)
project(secman)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_FILES main.cpp PriorityQueue.hpp Scheduler.cpp Scheduler.hpp Task.cpp Task.hpp)
add_executable(secman ${SOURCE_FILES})


find_package (Threads)
target_link_libraries (secman ${CMAKE_THREAD_LIBS_INIT})