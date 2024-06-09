cmake_minimum_required(VERSION 3.24)
project(ex2)

set(CMAKE_CXX_STANDARD 17)

add_executable(ex2 uthreads.cpp uthreads.h Thread.cpp Thread.h main.cpp)
