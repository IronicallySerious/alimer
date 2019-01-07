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

add_subdirectory(fmt)
add_subdirectory(spdlog)

# SPIRV-Cross
add_subdirectory(spirv-cross)

# if (NOT ALIMER_DISABLE_LUA)
    # add_subdirectory(Lua)
# endif()

# Vulkan libs
if (ALIMER_VULKAN)
    # volk
    add_subdirectory(volk)

    if (ALIMER_WINDOWS)
        target_compile_definitions(volk PUBLIC -DVK_USE_PLATFORM_WIN32_KHR)
    endif()

    # vkmemalloc
    add_library(vkmemalloc INTERFACE)
    target_sources(vkmemalloc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vkmemalloc/src/vk_mem_alloc.h)
    target_include_directories(vkmemalloc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vkmemalloc/src)
endif ()

# STB
set(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/stb)

if (ALIMER_GLFW)
    set (GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set (GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set (GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set (BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
    set( GLFW_USE_WAYLAND ${ALIMER_GLFW_WAYLAND} CACHE BOOL "" FORCE)
    add_subdirectory(glfw)
endif ()

if (ALIMER_OPENGL)
    add_subdirectory(glad)
    set_property(TARGET "glad" PROPERTY FOLDER "third_party")
endif ()

# CLI11
add_subdirectory(CLI11)
# ImGui
add_subdirectory(imgui)

vs_group_subdirectory_targets(${CMAKE_CURRENT_SOURCE_DIR} third_party)
