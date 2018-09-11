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

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_C_STANDARD 99)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set project name
project (Alimer LANGUAGES CXX C)
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

# Include cotire first.
include (cotire)

# Include ucm.
include (ucm)

# Include Alimer cmake module.
include (Alimer)

# Enable Unity build
set(UCM_UNITY_BUILD ON)

# Set alimer sdk path.
set (ALIMER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (ALIMER_SDK_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Source/Alimer)
set (ALIMER_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Source/ThirdParty)
set (ALIMER_ASSETS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/assets)

# Source
add_subdirectory (Source)

include(AlimerPackaging)

# Install CMake modules and toolchains provided by and for Alimer
install (DIRECTORY ${CMAKE_SOURCE_DIR}/CMake/ DESTINATION ${DEST_SHARE_DIR}/CMake)    # Note: the trailing slash is significant

# Set VS Startup project.
if(CMAKE_VERSION VERSION_GREATER "3.6")
    if (ALIMER_TOOLS)
        set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "AlimerStudio" )
    else ()
        set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "Alimer" )
    endif ()
endif()

# Print current build configuration
message(STATUS "Alimer Configuration:")
message(STATUS "  Platform        ${PLATFORM_NAME}")
message(STATUS "  Architecture    ${ARCH_NAME}")

message(STATUS "  Library         ${ALIMER_LIBRARY_TYPE}")
message(STATUS "  Threading       ${ALIMER_THREADING}")
message(STATUS "  GL              ${ALIMER_GL}")
message(STATUS "  D3D11           ${ALIMER_D3D11}")
message(STATUS "  D3D12           ${ALIMER_D3D12}")
message(STATUS "  Vulkan          ${ALIMER_VULKAN}")
message(STATUS "  SDL2            ${ALIMER_SDL}")
message(STATUS "  CSharp          ${ALIMER_CSHARP}")
message(STATUS "  Tools           ${ALIMER_TOOLS}")
