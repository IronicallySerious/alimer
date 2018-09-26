cmake_minimum_required (VERSION 3.2.3)

# Define target name
set (TARGET TestPlugin)

set (PLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB SOURCE_FILES
    ${PLUGIN_DIR}/*.h
    ${PLUGIN_DIR}/*.cpp
)

add_alimer_plugin(${TARGET} ${SOURCE_FILES})
