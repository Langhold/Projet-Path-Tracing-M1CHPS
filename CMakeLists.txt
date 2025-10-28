cmake_minimum_required(VERSION 3.15)

project(ppm LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(SOURCES src/image.c src/image.h)

add_executable(ppm src/ppm.c ${SOURCES})

target_compile_options(ppm PRIVATE -O3 -Wall)