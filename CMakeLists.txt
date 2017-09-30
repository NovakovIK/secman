cmake_minimum_required(VERSION 3.9)
project(daemon)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp)
add_executable(daemon ${SOURCE_FILES})