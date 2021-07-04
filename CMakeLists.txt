cmake_minimum_required(VERSION 3.1)
project(fsm_test)

set (CMAKE_CXX_STANDARD 11)
add_compile_options(-Wall)
set(sources "example.cpp")

add_executable(player ${sources})
