cmake_minimum_required(VERSION 3.24)
project(ex2)

set(CMAKE_CXX_STANDARD 17)

add_executable(ex2 uthreads.cpp demo_itimer.c demo_jmp.c demo_singInt_handler.c uthreads.h Thread.cpp Thread.h)
