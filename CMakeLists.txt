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
    src/benchmarks.c
)

target_include_directories(LibPathTracing PUBLIC ${PROJECT_SOURCE_DIR}/includes)



if(APPLE)
	target_link_libraries(LibPathTracing
		m
		MPI::MPI_C
		${OpenMP_LIBRARIES}
	)
	target_compile_options(LibPathTracing PRIVATE
    -Wall -Wextra -lc
	)
else()
	target_link_libraries(LibPathTracing
		m
		MPI::MPI_C
		OpenMP::OpenMP_C
	)
	target_compile_options(LibPathTracing PRIVATE
    -Wall -Wextra -lc -fopenmp
	)
endif()

# --------------------------------------------------
# EXECUTABLE
# --------------------------------------------------

add_executable(ppm src/ppm.c)

target_link_libraries(ppm PRIVATE LibPathTracing)


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
