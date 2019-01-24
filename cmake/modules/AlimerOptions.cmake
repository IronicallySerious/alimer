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

# Setup options
option (ALIMER_PROFILING "Enable performance profiling" TRUE)

if (ALIMER_ANDROID OR ALIMER_IOS OR ALIMER_WEB)
    set (ALIMER_TOOLS OFF CACHE INTERNAL "Build tools and editors" FORCE)

    if (ALIMER_WEB)
        set (EMSCRIPTEN_MEM_INIT_METHOD 1 CACHE STRING "emscripten: how to represent initial memory content (0..2)")
        set (EMSCRIPTEN_MEMORY_LIMIT 128 CACHE NUMBER "Memory limit in megabytes. Set to 0 for dynamic growth.")
        set (EMSCRIPTEN_USE_WASM ON CACHE BOOL "Enable WebAssembly support for Web platform.")
        set (EMSCRIPTEN_MEMORY_GROWTH OFF CACHE BOOL "Allow memory growth. Disables some optimizations.")

        set (ALIMER_WEBGL2 ON CACHE BOOL "Enable WebGL 2.0 support for Web platform.")
        set (ALIMER_WEB_FILESYSTEM OFF CACHE BOOL  "emscripten: enable FS module" OFF)
    endif ()
else()
    option (ALIMER_TOOLS "Build tools and editors" ${ALIMER_DESKTOP})
endif ()
