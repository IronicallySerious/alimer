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

if (ALIMER_THIRDPARTY_SHARED_LIBS)
    set (THIRDPARTY_LIB_TYPE SHARED)
else ()
    set (THIRDPARTY_LIB_TYPE STATIC)
endif ()

add_subdirectory (STB)
add_subdirectory (fmt EXCLUDE_FROM_ALL)
add_subdirectory (SPIRV-Cross)

if (ALIMER_GLFW)
    set (GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set (GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set (GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    #set (BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
    set (GLFW_USE_WAYLAND ${USE_WAYLAND} CACHE BOOL "" FORCE)
    if (WITH_PIC OR SHARED)
        set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    endif()

    add_subdirectory(glfw)
    include_directories(glfw/include)
endif ()

if (ALIMER_TOOLS)
    set (ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "Enable glslang binaries")
	set (SKIP_GLSLANG_INSTALL ON CACHE BOOL "Skip glslang installation")
    add_subdirectory (glslang EXCLUDE_FROM_ALL)
endif ()

vs_group_subdirectory_targets(${CMAKE_CURRENT_SOURCE_DIR} ThirdParty)
