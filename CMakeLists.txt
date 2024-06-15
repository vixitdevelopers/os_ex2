cmake_minimum_required(VERSION 3.24)
project(ex2)

set(CMAKE_CXX_STANDARD 17)

add_executable(ex2 uthreads.cpp uthreads.h Scheduler.h Scheduler.cpp Thread.h Thread.cpp test2_two_thread.cpp)

