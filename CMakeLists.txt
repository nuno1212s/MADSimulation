cmake_minimum_required(VERSION 3.16)
project(MADSim)

set(CMAKE_CXX_STANDARD 14)

link_libraries(pthread)

add_executable(MADSim main.cpp simfuncs.cpp simfuncs.h simfuncsasync.cpp simfuncsasync.h)