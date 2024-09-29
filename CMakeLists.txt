cmake_minimum_required(VERSION 3.22.1)

set(PROJECT_NAME StochasticWarp)
project(${PROJECT_NAME})

# include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/eigen.cmake")
# message(STATUS "Found Eigen3 include directories: ${EIGEN3_INCLUDE_DIRS}")

include($ENV{DEVKIT_LOCATION}/cmake/pluginEntry.cmake)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/eigen3)
file(GLOB_RECURSE CORE_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp)

find_package(OpenMP REQUIRED)

if(OpenMP_CXX_FOUND)
    message(STATUS "Found OpenMP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
    message(FATAL_ERROR "OpenMP not found.")
endif()

set(SOURCE_FILES
    ${CORE_SOURCE_FILES}
    src/StochasticWarp.cpp
)

set(LIBRARIES
    OpenMaya OpenMayaAnim Foundation
)

build_plugin()