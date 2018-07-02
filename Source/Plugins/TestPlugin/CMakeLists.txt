cmake_minimum_required (VERSION 3.2.3)

# Suppress CMake automatic regeneration, since we don't know if CMake is available
set(CMAKE_SUPPRESS_REGENERATION true)

# Define target name
set (TARGET_NAME TestPlugin)

set (PLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SOURCE_FILES 
    ${PLUGIN_DIR}/*.h 
    ${PLUGIN_DIR}/*.cpp
)

# Include engine plugin sdk files
include_directories(${ALIMER_SDK_SOURCE_PATH})

add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})