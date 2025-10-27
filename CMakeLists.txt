cmake_minimum_required(VERSION 3.15)

project(ppm LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_executable(ppm src/ppm.c)

target_compile_options(ppm PRIVATE -O0 -Wall)