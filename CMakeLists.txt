cmake_minimum_required(VERSION 3.15)

project(ppm LANGUAGES C)

set(CMAKE_C_STANDARD 11)

# --------------------------------------------------
# MPI
# --------------------------------------------------

find_package(MPI REQUIRED)

# --------------------------------------------------
# OpenMP
# --------------------------------------------------

find_package(OpenMP)

if(OpenMP_C_FOUND)

    message(STATUS "OpenMP found")

else()

    if(APPLE)

        message(STATUS "Using Homebrew libomp")

        execute_process(
            COMMAND brew --prefix libomp
            OUTPUT_VARIABLE LIBOMP_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        add_compile_options(-Xpreprocessor -fopenmp)
        include_directories(${LIBOMP_PREFIX}/include)
        link_directories(${LIBOMP_PREFIX}/lib)


    endif()

        set(OpenMP_LIBRARIES omp)
endif()

# --------------------------------------------------
# LIBRARY
# --------------------------------------------------

add_library(LibPathTracing
    src/image.c
    src/ray.c
    src/scene.c
    src/tools.c
    src/vector.c
    src/light.c

    src/simd/fSimd.c
    src/simd/fHit.c
    src/simd/fColor.c
    src/simd/fImage.c
    src/simd/fRay.c
    src/simd/fScene.c
    src/simd/fLight.c
)


set(COMMON_COMPILE_FLAGS
    $<$<CONFIG:Debug>:-Wall -Wextra -g -O0 -fno-inline -fno-omit-frame-pointer>
    $<$<CONFIG:Release>:-O3 -march=native -Wno-all>
)

set(SIMD_COMPILE_FLAGS -Werror -Wall -Wextra -O3 -march=native -g3)

set_source_files_properties(src/simd/fSimd.c PROPERTIES
    COMPILE_OPTIONS "${SIMD_COMPILE_FLAGS}"
)

target_include_directories(LibPathTracing PUBLIC ${PROJECT_SOURCE_DIR}/includes)

target_link_libraries(LibPathTracing m sleef)

add_executable(ppm src/ppm.c)
add_executable(simd main.c)

target_link_libraries(ppm PRIVATE LibPathTracing MPI::MPI_C)
target_link_libraries(simd PRIVATE LibPathTracing)

target_compile_options(ppm PRIVATE ${COMMON_COMPILE_FLAGS})
target_compile_options(simd PRIVATE ${SIMD_COMPILE_FLAGS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(ppm PRIVATE -Wall -Wextra -g -DDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(ppm PRIVATE -Wall -Wextra -O3 -march=native)
endif()



# --------------------------------------------------
# TESTS
# --------------------------------------------------

add_executable(test_runner
               test/test_runner.c
               test/test_vector.c
               test/test_ray.c
               test/test_tools.c
               test/test_scene.c
)
target_compile_options(test_runner PRIVATE ${COMMON_COMPILE_FLAGS})

include(FetchContent)
FetchContent_Declare(
    unity
    GIT_REPOSITORY  https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG         v2.6.1
    GIT_SHALLOW TRUE
)

FetchContent_GetProperties(unity)
if(NOT unity_POPULATED)
    FetchContent_MakeAvailable(unity)
endif()

target_link_libraries(test_runner PRIVATE unity LibPathTracing)

add_custom_target(test test_runner
    DEPENDS $<TARGET_FILE:test_runner>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running unit tests..."
)

message(STATUS "Build type = ${CMAKE_BUILD_TYPE}")
