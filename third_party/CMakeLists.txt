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

# SPIRV-Cross
set(SPIRV_CROSS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/GLSL.std.450.h
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_common.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_cfg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_cfg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_cross.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_cross.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv.hpp
)

set(NEED_SPIRV_CROSS_GLSL OFF)
if (ALIMER_D3D11 OR ALIMER_D3D12)
	list(APPEND SPIRV_CROSS_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_hlsl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_hlsl.hpp
	)

	set(NEED_SPIRV_CROSS_GLSL ON)
endif()

if (ALIMER_METAL)
    list(APPEND SPIRV_CROSS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_msl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_msl.hpp
    )
    set(NEED_SPIRV_CROSS_GLSL ON)
endif()

if (ALIMER_VULKAN OR ALIMER_OPENGL OR NEED_SPIRV_CROSS_GLSL)
	list(APPEND SPIRV_CROSS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_glsl.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/spirv-cross/spirv_glsl.hpp
	)
endif()

add_library (spirv_cross STATIC ${SPIRV_CROSS_SOURCES})
target_compile_definitions(spirv_cross PUBLIC SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
set(SPIRV_CROSS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
alimer_external_target("third_party" spirv_cross)

# glslang
option (BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
set (LIB_TYPE STATIC)
set (ENABLE_GLSLANG_INSTALL OFF)

set (ENABLE_HLSL OFF CACHE BOOL "Enables HLSL input support")
set (ENABLE_SPVREMAPPER OFF CACHE BOOL "Enables building of SPVRemapper")
set (ENABLE_AMD_EXTENSIONS ON CACHE BOOL "Enables support of AMD-specific extensions")
set (ENABLE_NV_EXTENSIONS ON CACHE BOOL "Enables support of Nvidia-specific extensions")
set (ENABLE_OPT OFF CACHE BOOL "Enables spirv-opt capability if present")
set (ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "Enable glslang binaries")
set (SKIP_GLSLANG_INSTALL ON CACHE BOOL "Skip glslang installation")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glslang/glslang)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glslang/OGLCompilersDLL)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glslang/SPIRV)

alimer_external_target("third_party" glslang)
alimer_external_target("third_party" OGLCompiler)
alimer_external_target("third_party" OSDependent)
alimer_external_target("third_party" SPIRV)
target_compile_options(glslang PUBLIC -DAMD_EXTENSIONS -DNV_EXTENSIONS )

# Vulkan libs
if (ALIMER_VULKAN)
    # volk
    set(VOLK_CROSS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/volk/volk.h
        ${CMAKE_CURRENT_SOURCE_DIR}/volk/volk.c
    )
    add_library (volk STATIC ${VOLK_CROSS_SOURCES})
    alimer_external_target("third_party" volk)

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

if (ALIMER_DESKTOP AND ALIMER_TOOLS)
    add_subdirectory(CLI11)

    add_subdirectory(imgui)
    alimer_external_target("third_party" ImGui)
endif ()