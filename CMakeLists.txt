#
# Copyright (c) 2017-2019 Amer Koleci and contributors.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

# Set CMake minimum version
cmake_minimum_required (VERSION 3.5)
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_C_STANDARD 99)
project (Alimer LANGUAGES CXX C)

# Alimer Version
set (PRODUCT_NAME "Alimer")
set (PRODUCT_COMPANY "Amer Koleci")
set (PRODUCT_COPYRIGHT "Copyright (C) 22018 ${PRODUCT_COMPANY}")
set (ALIMER_VERSION_MAJOR 0)
set (ALIMER_VERSION_MINOR 1)
set (ALIMER_VERSION_PATCH 0)
set (ALIMER_VERSION ${ALIMER_VERSION_MAJOR}.${ALIMER_VERSION_MINOR}.${ALIMER_VERSION_PATCH})

# Get the current working branch
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE ALIMER_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE ALIMER_GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Output paths
set (ALIMER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (ALIMER_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
mark_as_advanced(ALIMER_ROOT_DIR)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ALIMER_BUILD_DIR}/bin")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${ALIMER_BUILD_DIR}/lib")
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${ALIMER_BUILD_DIR}/lib")

# Enable solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Define standard configurations
if ( CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_CONFIGURATION_TYPES MATCHES "Debug;Dev;Release" )
	set (CMAKE_CONFIGURATION_TYPES "Debug;Dev;Release" CACHE STRING "List of supported configurations." FORCE)
endif()

# Set CMake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# Include Alimer module.
include (Alimer)

# Set platform variables
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set (LINUX ON CACHE BOOL "" FORCE)
endif ()

# Options
option (ALIMER_BUILD_SHARED "Build as a shared library." OFF)
option (ALIMER_SCRIPTING_LUA "Enable Lua scripting support" ON)
option (ALIMER_CSHARP "Enable C# support" OFF)
option (ALIMER_CSHARP_MONO "Use mono for C# support" OFF)
option (ALIMER_INSTALL "Generate installation target" ON)

if (EMSCRIPTEN)
    set (ALIMER_THREADING OFF CACHE INTERNAL "Disable threading on WEB" FORCE)
    set (ALIMER_NETWORK OFF CACHE INTERNAL "Disable network on WEB" FORCE)
else ()
    option (ALIMER_THREADING "Enable threading support" TRUE)
    option (ALIMER_NETWORK "Enable network support" TRUE)
endif ()

# Graphics
if (ALIMER_WINDOWS)
    #set (ALIMER_RENDERER D3D11 CACHE STRING "Select renderer: D3D11 | D3D12 | Vulkan | OpenGL")
    set (ALIMER_RENDERER Vulkan CACHE STRING "Select renderer: D3D11 | D3D12 | Vulkan | OpenGL")
elseif (ALIMER_UWP OR ALIMER_XBOX_ONE)
    set (ALIMER_RENDERER D3D11 CACHE STRING "Select renderer: D3D11 | D3D12")
elseif (LINUX OR ANDROID)
    set (ALIMER_RENDERER OpenGL CACHE STRING "Select renderer: Vulkan | OpenGL")
else ()
    set (ALIMER_RENDERER OpenGL CACHE STRING "Select renderer: Vulkan | OpenGL")
endif ()
string(TOUPPER "${ALIMER_RENDERER}" ALIMER_RENDERER)
set (ALIMER_${ALIMER_RENDERER} ON)

if (ALIMER_CSHARP)
    set (ALIMER_BUILD_SHARED ON CACHE INTERNAL "Force shared library build" FORCE)
endif ()

if (ANDROID OR ALIMER_BUILD_SHARED)
    set (CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Set alimer sdk path.
set (ALIMER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (ALIMER_SDK_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Source/Alimer)
set (ALIMER_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set (ALIMER_ASSETS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/assets)

# Initialize the Dev configuration from release configuration.
set(CMAKE_C_FLAGS_DEV "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEV "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_STATIC_LINKER_FLAGS_DEV "${CMAKE_STATIC_LINKER_FLAGS_RELEASE}")
set(CMAKE_SHARED_LINKER_FLAGS_DEV "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")
set(CMAKE_MODULE_LINKER_FLAGS_DEV "${CMAKE_MODULE_LINKER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_DEV "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

set(DEBUG_COMPILE_OPTIONS "-DALIMER_DEV=1")
add_compile_options("$<$<CONFIG:Debug>:${DEBUG_COMPILE_OPTIONS}>")

set(DEV_COMPILE_OPTIONS "-DALIMER_DEV=1" )
add_compile_options("$<$<CONFIG:Dev>:${DEV_COMPILE_OPTIONS}>")

set(RELEASE_COMPILE_OPTIONS "-DALIMER_DEV=0")
add_compile_options("$<$<CONFIG:Release>:${RELEASE_COMPILE_OPTIONS}>")

# third_party
add_subdirectory(third_party)

# Source
add_subdirectory(src)

if (ALIMER_INSTALL)
    # Install CMake modules and toolchains provided by and for Alimer
    install (DIRECTORY ${CMAKE_SOURCE_DIR}/CMake/ DESTINATION ${DEST_SHARE_DIR}/CMake)    # Note: the trailing slash is significant

    # Install data files (assets)
    file (GLOB ASSET_DIRS ${Alimer_SOURCE_DIR}/assets ${CMAKE_SOURCE_DIR}/assets)
    file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    if (NOT ANDROID)
        foreach (ASSET_DIR ${ASSET_DIRS})
            # get_filename_component (NAME ${ASSET_DIR} NAME)
            install (DIRECTORY ${ASSET_DIR} DESTINATION bin)
        endforeach ()
    endif ()
endif()

# Set VS Startup project.
if(CMAKE_VERSION VERSION_GREATER "3.6")
    if (ALIMER_TOOLS)
        set_property (DIRECTORY PROPERTY VS_STARTUP_PROJECT "AlimerStudio")
    else ()
        set_property (DIRECTORY PROPERTY VS_STARTUP_PROJECT "AlimerPlayer")
    endif ()
endif()

# Print current build configuration
message (STATUS "Build Configuration:")
message (STATUS "  Host            ${ALIMER_HOST_NAME}")
message (STATUS "  C++ Compiler    ${ALIMER_COMPILER_NAME}")
message (STATUS "  Platform        ${ALIMER_PLATFORM_NAME}")
message (STATUS "  Architecture    ${ALIMER_ARCH_NAME}")

if (ALIMER_BUILD_SHARED)
    message(STATUS "  Library         SHARED")
else()
    message(STATUS "  Library         STATIC")
endif()

message(STATUS "  Threading       ${ALIMER_THREADING}")
message(STATUS "  Renderer        ${ALIMER_RENDERER}")
message(STATUS "  Tools           ${ALIMER_TOOLS}")
message(STATUS "  Lua             ${ALIMER_SCRIPTING_LUA}")
message(STATUS "  CSharp          ${ALIMER_CSHARP}")
