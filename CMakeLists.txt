cmake_minimum_required(VERSION 3.9)
project(secman)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_FILES main.cpp tread_pool.hpp interruptable_sleep.hpp scheduler.hpp cron.hpp argparse.hpp cron.cpp scheduler.cpp interruptable_sleep.cpp tread_pool.cpp)
add_executable(secman ${SOURCE_FILES})


find_package (Threads)
target_link_libraries (secman ${CMAKE_THREAD_LIBS_INIT})