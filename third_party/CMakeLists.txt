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

# GLFW
if(NOT (EMSCRIPTEN OR ANDROID))
    set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set (GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set (GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set (GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    #set (GLFW_USE_WAYLAND ${VGPU_USE_WAYLAND} CACHE BOOL "" FORCE)
    add_subdirectory(glfw)
    set_property(TARGET glfw PROPERTY FOLDER "third_party")
    unset(LIB_SUFFIX CACHE)
endif ()

if (ALIMER_OPENGL)
    add_subdirectory(glad)
    set_property(TARGET glad PROPERTY FOLDER "third_party")
endif ()

if (ALIMER_VULKAN)
    # vulkan
    add_library(vulkan INTERFACE)
    target_include_directories(vulkan INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(vulkan INTERFACE VK_NO_PROTOTYPES)

    if (ANDROID)
	    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_ANDROID_KHR)
    elseif (WIN32)
	    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_WIN32_KHR)
    elseif (APPLE)
	    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_MACOS_MVK)
    elseif (UNIX)
	    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_XCB_KHR)
    endif()

    # volk
    set (VOLK_FILES "volk/volk.c" "volk/volk.h")
    add_library(volk STATIC ${VOLK_FILES})
    target_link_libraries(volk PUBLIC vulkan)
    target_include_directories(volk PUBLIC "volk")
    set_property(TARGET volk PROPERTY FOLDER "third_party")

    # vma
    set (VMA_FILES "vk_mem_alloc/vk_mem_alloc.cpp" "vk_mem_alloc/vk_mem_alloc.h")
    add_library(vma STATIC ${VMA_FILES})
    target_link_libraries(vma PUBLIC vulkan)
    target_include_directories(vma PUBLIC "vk_mem_alloc")
    set_property(TARGET vma PROPERTY FOLDER "third_party")
endif ()

add_subdirectory(vgpu)
set_property(TARGET vgpu PROPERTY FOLDER "third_party")
