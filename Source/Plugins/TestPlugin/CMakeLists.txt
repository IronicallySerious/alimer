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

add_alimer_plugin(${TARGET_NAME} ${SOURCE_FILES})
