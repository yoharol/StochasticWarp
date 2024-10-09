cmake_minimum_required(VERSION 3.22.1)

set(PROJECT_NAME StochasticWarp)
project(${PROJECT_NAME})

function(set_high_optimization_flags target)
    if (MSVC)
        target_compile_options(${target} PRIVATE /Ox /GL)
        target_link_options(${target} PRIVATE /LTCG)  # Link-time code generation
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${target} PRIVATE -O3 -flto -march=native)
        target_link_options(${target} PRIVATE -flto)  # Link-time optimization
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE -O3 -flto -march=native)
        target_link_options(${target} PRIVATE -flto)  # Link-time optimization
    endif()
endfunction()

find_package(OpenMP REQUIRED)

if(OpenMP_CXX_FOUND)
    message(STATUS "Found OpenMP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
    message(FATAL_ERROR "OpenMP not found.")
endif()

include($ENV{DEVKIT_LOCATION}/cmake/pluginEntry.cmake)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/eigen3)
file(GLOB_RECURSE CORE_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp)

set(SOURCE_FILES
    ${CORE_SOURCE_FILES}
    src/StochasticWarp.cpp
)

set(LIBRARIES
    OpenMaya OpenMayaAnim Foundation
)

build_plugin()