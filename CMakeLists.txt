#
# Copyright (c) 2018 Amer Koleci and contributors.
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

# Set project name
project (Alimer LANGUAGES CXX C)

# Alimer Version
set (PRODUCT_NAME "Alimer")
set (PRODUCT_COMPANY "Amer Koleci")
set (PRODUCT_COPYRIGHT "Copyright (C) 22018 ${PRODUCT_COMPANY}")
set (PRODUCT_VERSION_MAJOR "0")
set (PRODUCT_VERSION_MINOR "9")
set (PRODUCT_VERSION_REVISION "0")

# Get the current working branch
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Configure CMake global variables
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_POSITION_INDEPENDENT_CODE ON)
set (CMAKE_INSTALL_MESSAGE LAZY)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set (CMAKE_PDB_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/libs")
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/lib")

if( ALIMER_WINDOWS )
	set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD 1)
endif()

# Enable solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Define standard configurations
if ( CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_CONFIGURATION_TYPES MATCHES "Debug;Dev;Release" )
	set (CMAKE_CONFIGURATION_TYPES "Debug;Dev;Release" CACHE STRING "List of supported configurations." FORCE)
endif()

# Set default configuration to Release for single-configuration generators
if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE Release)
endif ()

# Set CMake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)

# Include Alimer module.
include (Alimer)

# Include Alimer packaging module.
include (AlimerPackaging)

# Set alimer sdk path.
set (ALIMER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (ALIMER_SDK_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Source/Alimer)
set (ALIMER_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set (ALIMER_ASSETS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/assets)

# Workarounds
if (EMSCRIPTEN OR APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++${CMAKE_CXX_STANDARD}")
endif ()

if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif ()

if (MSVC)
    # In DEBUG, avoid generating a PDB file which seems to mess with incremental compilation.
    # Instead generate debug info directly inside obj files.
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Z7")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")

    set(CMAKE_C_FLAGS_DEV "${CMAKE_C_FLAGS_DEV} /Z7")
    set(CMAKE_CXX_FLAGS_DEV "${CMAKE_CXX_FLAGS_DEV} /Z7")

    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
endif ()

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
add_subdirectory(Source)

if (NOT ALIMER_SKIP_INSTALL)
    # Install CMake modules and toolchains provided by and for Alimer
    install (DIRECTORY ${CMAKE_SOURCE_DIR}/CMake/ DESTINATION ${DEST_SHARE_DIR}/CMake)    # Note: the trailing slash is significant
endif()

# Set VS Startup project.
if(CMAKE_VERSION VERSION_GREATER "3.6")
    if (ALIMER_TOOLS)
        set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "AlimerStudio" )
    else ()
        set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "Alimer" )
    endif ()
endif()

# Print current build configuration
message (STATUS "Build Configuration:")
message (STATUS "  Host            ${ALIMER_HOST_NAME}")
message (STATUS "  C++ Compiler    ${ALIMER_COMPILER_NAME}")
message (STATUS "  Platform        ${ALIMER_PLATFORM_NAME}")
message (STATUS "  Architecture    ${ALIMER_ARCH_NAME}")

if (ALIMER_SHARED)
    message(STATUS "  Library         SHARED")
else()
    message(STATUS "  Library         STATIC")
endif()

message(STATUS "  Threading       ${ALIMER_THREADING}")
message(STATUS "  GL              ${ALIMER_GL}")
message(STATUS "  Vulkan          ${ALIMER_VULKAN}")
message(STATUS "  CSharp          ${ALIMER_CSHARP}")
message(STATUS "  Tools           ${ALIMER_TOOLS}")
