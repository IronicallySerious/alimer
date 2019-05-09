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

if (WIN32)
    #set (VGPU_RENDERER D3D11 CACHE STRING "Select renderer: D3D11 | GL")
    set (VGPU_RENDERER GL CACHE STRING "Select renderer: D3D11 | GL")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "WindowsStore") # UWP
    set (VGPU_RENDERER D3D11 CACHE STRING "Select renderer: D3D11")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Durango") #  XboxOne
    set (VGPU_RENDERER D3D11 CACHE STRING "Select renderer: D3D11")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    # Use Metal on IOS and macOS platforms
    set (VGPU_RENDERER Metal CACHE STRING "Use Metal renderer" FORCE)
else ()
    set (VGPU_RENDERER GL CACHE STRING "Use OpenGL renderer" FORCE)
endif ()
string(TOUPPER "${VGPU_RENDERER}" VGPU_RENDERER)

set(VGPU_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/vgpu.h ${CMAKE_CURRENT_SOURCE_DIR}/vgpu.c)

if("${VGPU_RENDERER}" STREQUAL "GL")
    set (VGPU_SOURCES ${VGPU_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/vgpu_gl.c
    )
endif()

add_library(vgpu STATIC ${VGPU_SOURCES})
target_include_directories(vgpu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

if("${VGPU_RENDERER}" STREQUAL "GL")
    target_link_libraries(vgpu PUBLIC glad)
    if (EMSCRIPTEN)
        target_compile_definitions(vgpu PUBLIC -DVGPU_WEBGL)
    elseif(ANDROID)
        target_compile_definitions(vgpu PUBLIC -DVGPU_GLES)
    else ()
        target_compile_definitions(vgpu PUBLIC -DVGPU_GL)
    endif ()
endif ()
