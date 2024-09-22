cmake_minimum_required(VERSION 3.22.1)

set(PROJECT_NAME StochasticWarp)
project(${PROJECT_NAME})

# include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/eigen.cmake")
# message(STATUS "Found Eigen3 include directories: ${EIGEN3_INCLUDE_DIRS}")

include($ENV{DEVKIT_LOCATION}/cmake/pluginEntry.cmake)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/eigen3)
file(GLOB_RECURSE CORE_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp)

set(SOURCE_FILES
    ${CORE_SOURCE_FILES}
    src/StochasticWarp.cpp
)

set(LIBRARIES
    OpenMaya Foundation
)

build_plugin()