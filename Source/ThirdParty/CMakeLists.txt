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

add_subdirectory (STB)
add_subdirectory (SPIRV-Cross)
add_subdirectory (entt)

if (ALIMER_SDL)
    add_subdirectory(SDL2)
endif ()

if (NOT ALIMER_DISABLE_SHADER_COMPILER)
    option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
    set(LIB_TYPE STATIC)
    set(ENABLE_GLSLANG_INSTALL OFF)
    set (ENABLE_HLSL OFF CACHE BOOL "Enables HLSL input support")
    set (ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "Enable glslang binaries")
	set (SKIP_GLSLANG_INSTALL ON CACHE BOOL "Skip glslang installation")
    add_subdirectory (glslang EXCLUDE_FROM_ALL)
endif ()

if (PLATFORM_DESKTOP)
    add_subdirectory(imgui)
endif (PLATFORM_DESKTOP)

vs_group_subdirectory_targets(${CMAKE_CURRENT_SOURCE_DIR} ThirdParty)
