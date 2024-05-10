cmake_minimum_required(VERSION 3.1)

project(async_structures)

add_compile_options(-Werror -Wall)

add_executable(run_tests main.cpp mcsp.cpp set.cpp)
