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

if( _ALIMER_GUARD_ )
	return()
endif()
set (_ALIMER_GUARD_ 1)

# Detect host.
if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    set (ALIMER_HOST_WINDOWS 1)
    set (ALIMER_HOST_NAME "Windows")
elseif (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin")
    set (ALIMER_HOST_OSX 1)
    set (ALIMER_HOST_NAME "OSX")
elseif (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    set (ALIMER_HOST_LINUX 1)
    set (ALIMER_HOST_NAME "Linux")
else()
    message(WARNING "Host system not recognized, setting to 'Linux'")
    set (ALIMER_HOST_LINUX 1)
    set (ALIMER_HOST_NAME "Linux")
endif()

# Detect compiler
if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    set (ALIMER_CLANG 1)
    set (ALIMER_COMPILER_NAME "Clang (ALIMER_CLANG)")
    set (CLANG ON)
    set (GNU ON)
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    set (ALIMER_GCC 1)
    set (ALIMER_COMPILER_NAME "GCC (ALIMER_GCC)")
    set (GCC ON)
    set (GNU ON)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
    set (ALIMER_CLANG 1)
    set (ALIMER_COMPILER_NAME "MSVC Clang (ALIMER_CLANG)")
    set (CLANG ON)
    set (GNU ON)
    set (CLANG_CL ON)
elseif (MSVC)
    set (ALIMER_MSVC 1)
    set (ALIMER_COMPILER_NAME "VisualStudio (ALIMER_MSVC)")
else()
    message(FATAL_ERROR "Cannot detect compiler")
endif()

# Set platform variables
if ( "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" )
	set (ALIMER_WINDOWS ON)
	set (ALIMER_PLATFORM_NAME "Windows")
elseif ( "${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore" )
	set (ALIMER_UWP ON)
	set (ALIMER_PLATFORM_NAME "UWP")
    set (ALIMER_UWP_VERSION_MIN 10.0.16299.0)
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Durango" )
	set (ALIMER_XBOX_ONE 1)
    set (ALIMER_PLATFORM_NAME "XboxOne")
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Orbis" )
	set (ALIMER_PS4 1)
	set (ALIMER_PLATFORM_NAME "PS4")
elseif( "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin" )
    if( IOS )
        set (ALIMER_IOS ON)
		set (ALIMER_PLATFORM_NAME "iOS")
	else()
		set (ALIMER_OSX ON)
		set (ALIMER_PLATFORM_NAME "macOS")
	endif()
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	set (ALIMER_LINUX ON)
	set (ALIMER_PLATFORM_NAME "Linux")
elseif( ANDROID )
    set (ALIMER_ANDROID ON)
    set (ALIMER_PLATFORM_NAME "Android")
elseif( EMSCRIPTEN )
    set (ALIMER_WEB ON)
	set (ALIMER_PLATFORM_NAME "Web")
else()
	message(FATAL_ERROR "Unknown platform ${CMAKE_SYSTEM_NAME}!")
endif ()

if (ALIMER_WINDOWS OR ALIMER_LINUX OR ALIMER_OSX)
	set (ALIMER_DESKTOP ON)
endif ()

# Detect target architecture
if( ((ALIMER_WINDOWS OR ALIMER_UWP OR ALIMER_OSX) AND CMAKE_CL_64) OR (ALIMER_IOS AND CMAKE_OSX_ARCHITECTURES MATCHES "arm64") OR ALIMER_XBOX_ONE OR ALIMER_PS4 OR ALIMER_LINUX )
	set (ALIMER_64BIT 1)
endif()

if( ALIMER_WINDOWS OR ALIMER_OSX OR ALIMER_LINUX OR ALIMER_XBOX_ONE OR ALIMER_PS4 OR ALIMER_WEB OR ALIMER_UWP )
	if (ALIMER_64BIT)
		set (ALIMER_ARCH_NAME "x64")
	else()
		set (ALIMER_ARCH_NAME "x86")
	endif()
elseif( ALIMER_IOS OR ALIMER_ANDROID )
	if (ALIMER_64BIT)
		set (ALIMER_ARCH_NAME "arm64")
	else()
		set (ALIMER_ARCH_NAME "arm")
	endif()
else()
	message(FATAL_ERROR "Unknown platform architecture!")
endif()

# Include options and settings
include (AlimerOptions)
include (AlimerUtils)

# Select static/dynamic runtime library
if( MSVC AND ALIMER_WINDOWS )
    replace_compile_flags("/MDd" "/MTd" debug)
    replace_compile_flags("/MD" "/MT" dev release)
endif()
