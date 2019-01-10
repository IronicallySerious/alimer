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

cmake_minimum_required(VERSION 3.5)

if( _ALIMER_SETTINGS_ )
	return()
endif()

set (_ALIMER_SETTINGS_ 1)

include (Alimer)

# Source environment
if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
    execute_process(COMMAND cmd /c set OUTPUT_VARIABLE ENVIRONMENT)
else ()
    execute_process(COMMAND env OUTPUT_VARIABLE ENVIRONMENT)
endif ()
string (REGEX REPLACE "=[^\n]*\n?" ";" ENVIRONMENT "${ENVIRONMENT}")
set (IMPORT_ALIMER_VARIABLES_FROM_ENV BUILD_SHARED_LIBS SWIG_EXECUTABLE SWIG_DIR)
foreach(key ${ENVIRONMENT})
    list (FIND IMPORT_ALIMER_VARIABLES_FROM_ENV ${key} _index)
    if ("${key}" MATCHES "^(ALIMER_|CMAKE_|ANDROID_).+" OR ${_index} GREATER -1)
        if (NOT DEFINED ${key})
            set (${key} $ENV{${key}} CACHE STRING "" FORCE)
        endif ()
    endif ()
endforeach()

include (CMakeDependentOption)
option (ALIMER_ENABLE_ALL "Enables all optional subsystems by default" OFF)

# Enable features user has chosen
foreach(FEATURE in ${ALIMER_FEATURES})
    set (ALIMER_${FEATURE} ON CACHE BOOL "" FORCE)
endforeach()

if (ALIMER_WINDOWS OR ALIMER_LINUX OR ALIMER_ANDROID)
    set (ALIMER_VULKAN_DEFAULT ON)
endif ()

macro (alimer_option NAME DESCRIPTION)
    if (NOT ${NAME}_DEFAULT)
        set (${NAME}_DEFAULT ${ALIMER_ENABLE_ALL})
    endif ()
    if (NOT DEFINED ${NAME})
        option(${NAME} "${DESCRIPTION}" ${${NAME}_DEFAULT})
    endif ()
endmacro ()

# Setup options
option (ALIMER_PROFILING "Enable performance profiling" TRUE)
option (ALIMER_EXCEPTIONS "Enable exceptions support" OFF)

if (ALIMER_ANDROID OR ALIMER_IOS OR ALIMER_WEB)
    set (ALIMER_SIMD OFF CACHE INTERNAL "Enable SIMD support" FORCE)
    set (ALIMER_BUNDLE ON CACHE INTERNAL "Bundle mode" FORCE)
    set (ALIMER_TOOLS OFF CACHE INTERNAL "Build tools and editors" FORCE)
    set (ALIMER_HOT_LOADING OFF CACHE INTERNAL "Enable hot-loading of assets and plugins" FORCE)

    if (ALIMER_WEB)
        set (EMSCRIPTEN_MEM_INIT_METHOD 1 CACHE STRING "emscripten: how to represent initial memory content (0..2)")
        set (EMSCRIPTEN_MEMORY_LIMIT 128 CACHE NUMBER "Memory limit in megabytes. Set to 0 for dynamic growth.")
        set (EMSCRIPTEN_USE_WASM ON CACHE BOOL "Enable WebAssembly support for Web platform.")
        set (EMSCRIPTEN_MEMORY_GROWTH OFF CACHE BOOL "Allow memory growth. Disables some optimizations.")

        set (ALIMER_WEBGL2 ON CACHE BOOL "Enable WebGL 2.0 support for Web platform.")
        set (ALIMER_WEB_FILESYSTEM OFF CACHE BOOL  "emscripten: enable FS module" OFF)
    endif ()
else()
    option (ALIMER_SIMD "Enable SIMD support" ON)
    option (ALIMER_BUNDLE "Bundle all plugins and runtime together and make a single binary" OFF)
    option (ALIMER_TOOLS "Build tools and editors" ${ALIMER_DESKTOP})
    option (ALIMER_HOT_LOADING "Enable hot-loading of assets and plugins" ${ALIMER_DESKTOP})
endif ()

if (ALIMER_WEB)
    set (ALIMER_THREADING OFF CACHE INTERNAL "Disable threading on WEB" FORCE)
    set (ALIMER_NETWORK OFF CACHE INTERNAL "Disable network on WEB" FORCE)
else ()
    option (ALIMER_THREADING "Enable threading support" TRUE)
    option (ALIMER_NETWORK "Enable network support" TRUE)
endif ()

option (ALIMER_SKIP_INSTALL "Skip installation" ${ALIMER_SKIP_INSTALL})
set (ALIMER_SHARED OFF CACHE BOOL "Enable shared library build")
set (ALIMER_USE_DEBUG_INFO ON CACHE BOOL "Enable debug information in all configurations.")

# Graphics backends
if (ALIMER_WINDOWS OR ALIMER_UWP OR ALIMER_XBOX_ONE)
    set (ALIMER_D3D11 ON CACHE BOOL "Enable Direct3D11 backend.")
    set (ALIMER_D3D12 ON CACHE BOOL "Enable Direct3D12 backend.")
else ()
    set (ALIMER_D3D11 OFF CACHE INTERNAL "Direct3D11 not supported" FORCE)
    set (ALIMER_D3D12 OFF CACHE INTERNAL "Direct3D11 not supported" FORCE)
endif ()

if (ALIMER_WINDOWS OR ALIMER_LINUX OR ALIMER_ANDROID)
    set (ALIMER_OPENGL ON CACHE BOOL "Enable OpenGL backend.")
    set (ALIMER_VULKAN ON CACHE BOOL "Enable Vulkan backend.")
else ()
    set (ALIMER_OPENGL OFF CACHE INTERNAL "OpenGL not supported" FORCE)
    set (ALIMER_VULKAN OFF CACHE INTERNAL "Vulkan not supported" FORCE)
endif ()

alimer_option (ALIMER_CSHARP "Enable C# support")
alimer_option (ALIMER_CSHARP_MONO "Use mono for C# support")

if (ALIMER_CSHARP)
    set (ALIMER_SHARED ON CACHE INTERNAL "Vulkan not supported" FORCE)
endif ()
