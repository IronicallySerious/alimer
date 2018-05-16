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
project (Alimer)

# Enable common build options
if (NOT DEFINED CMAKE_CXX_STANDARD)
	set (CMAKE_CXX_STANDARD 11)
endif ()
# Workarounds
if (EMSCRIPTEN OR APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++${CMAKE_CXX_STANDARD}")
endif ()
if (APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif ()

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

# Include Alimer cmake module.
include (Alimer)

# Set alimer sdk path.
set (ALIMER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (ALIMER_SDK_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Source/Alimer)
set (ALIMER_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Source/ThirdParty)
set (ALIMER_ASSETS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/assets)

# Source
add_subdirectory (Source)

# Set VS Startup project.
if(CMAKE_VERSION VERSION_GREATER  "3.6")
	if (ALIMER_TOOLS)
		set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "AlimerStudio" )
	else ()
		set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "Alimer" )
	endif ()
endif()
