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

# Always included libraries
foreach (TARGET stb freetype fmt)
    add_subdirectory (${TARGET})

    if (TARGET target_name)
        set_property(TARGET ${target_name} PROPERTY FOLDER "third_party")
    endif ()
endforeach ()

# SPIRV-Cross
add_subdirectory(spirv-cross)

if (ALIMER_GLFW)
    # glfw
    set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set (GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set (GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set (GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    #set (GLFW_USE_WAYLAND ${VGPU_USE_WAYLAND} CACHE BOOL "" FORCE)
    add_subdirectory(glfw)
endif ()

# lua
add_subdirectory(lua)

# sokol_gfx
add_library(sokol_gfx INTERFACE)
target_include_directories(sokol_gfx INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/sokol)

# sokol_app
add_library(sokol_app INTERFACE)
target_include_directories(sokol_app INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/sokol)

if (ALIMER_VULKAN)
    # vulkan
    add_library(vulkan INTERFACE)
    target_include_directories(vulkan INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(vulkan INTERFACE VK_NO_PROTOTYPES)

    if(ANDROID)
	    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_ANDROID_KHR)
    elseif(WIN32)
	    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_WIN32_KHR)
    elseif(APPLE)
	    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_MACOS_MVK)
    elseif(UNIX)
	    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_XCB_KHR)
    endif()

    # vma
    add_library(vma INTERFACE)
    target_include_directories(vma INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/vk_mem_alloc")
    target_link_libraries(vma INTERFACE vulkan)

    # volk
    set (VOLK_FILES "volk/volk.c" "volk/volk.h")
    add_library(volk STATIC ${VOLK_FILES})
    target_link_libraries(volk PUBLIC vulkan)

    if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
        target_compile_definitions(volk PRIVATE VK_USE_PLATFORM_WIN32_KHR VK_KHR_win32_surface)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_compile_definitions(volk PRIVATE VK_USE_PLATFORM_XCB_KHR VK_KHR_xcb_surface)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
        target_compile_definitions(volk PRIVATE VK_USE_PLATFORM_ANDROID_KHR VK_KHR_android_surface)
    else()
        message(FATAL_ERROR "Volk can't be cofigured for " ${CMAKE_SYSTEM_NAME})
    endif()

    target_include_directories(volk PUBLIC "volk")
    set_property(TARGET volk PROPERTY FOLDER "third_party")

    # glslang
    option(ENABLE_SPVREMAPPER OFF)
    option(ENABLE_AMD_EXTENSIONS OFF)
    option(ENABLE_GLSLANG_BINARIES OFF)
    option(ENABLE_NV_EXTENSIONS OFF)
    option(ENABLE_HLSL OFF)
    option(ENABLE_OPT OFF)
    option(BUILD_TESTING OFF)

    add_subdirectory(glslang)

    if (NOT TARGET glslang-default-resource-limits)
        add_library(glslang-default-resource-limits glslang/StandAlone/ResourceLimits.cpp)
        set_property(TARGET glslang-default-resource-limits PROPERTY FOLDER "third_party")

        target_include_directories(glslang-default-resource-limits PUBLIC glslang/StandAlone)
    endif()

    if(NOT MSVC)
        target_compile_options(glslang PRIVATE
            "-Wno-logical-op-parentheses"
            "-Wno-unused-parameter")

        target_compile_options(SPIRV PRIVATE
            "-Wno-logical-op-parentheses"
            "-Wno-unused-parameter")
    endif()

    set_property(TARGET glslang PROPERTY FOLDER "third_party")
    set_property(TARGET OGLCompiler PROPERTY FOLDER "third_party")
    set_property(TARGET OSDependent PROPERTY FOLDER "third_party")
    set_property(TARGET SPIRV PROPERTY FOLDER "third_party")
endif ()

# CLI11
add_subdirectory(CLI11)
# ImGui
add_subdirectory(imgui)

vs_group_subdirectory_targets(${CMAKE_CURRENT_SOURCE_DIR} third_party)
