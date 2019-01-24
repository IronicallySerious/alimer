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

add_subdirectory(fmt)
add_subdirectory(spdlog)

# SPIRV-Cross
add_subdirectory(spirv-cross)

# Vulkan libs
if (ALIMER_VULKAN)
    # volk
    add_library(volk STATIC ${CMAKE_CURRENT_SOURCE_DIR}/volk/volk.c)
    if(MSVC)
        target_compile_options(volk PRIVATE /W4 /WX)
    else()
        target_compile_options(volk PRIVATE -Wall -Wextra -Werror)
        target_link_libraries(volk PRIVATE dl)
    endif()
    target_include_directories(volk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

    if (MSVC)
        set_property(TARGET "volk" PROPERTY FOLDER "third_party")
    endif ()

    # vkmemalloc
    add_library(vkmemalloc INTERFACE)
    target_include_directories(vkmemalloc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vkmemalloc/src)
endif ()

if (ALIMER_SDL2)
    add_subdirectory (libsdl2)
endif ()

if (ALIMER_OPENGL)
    add_subdirectory(glad)
    set_property(TARGET "glad" PROPERTY FOLDER "third_party")
endif ()

# CLI11
add_subdirectory(CLI11)
# ImGui
add_subdirectory(imgui)

if (ALIMER_SCRIPTING_LUA)
    add_subdirectory(lua)
    set_property(TARGET "liblua" PROPERTY FOLDER "third_party")
endif ()

vs_group_subdirectory_targets(${CMAKE_CURRENT_SOURCE_DIR} third_party)
