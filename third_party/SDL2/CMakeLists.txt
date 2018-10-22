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

cmake_minimum_required(VERSION 3.1)

project (SDL2 C)

set (TARGET SDL2)
set (HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set (SRC_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/src)

# General header files
file(GLOB HEADER_FILES ${HEADER_DIR}/*.h)

# General source files
file(GLOB SOURCE_FILES
    ${SRC_DIR}/*.c
    ${SRC_DIR}/atomic/*.c
    ${SRC_DIR}/audio/*.c
    ${SRC_DIR}/audio/disk/*.c
    ${SRC_DIR}/audio/dsp/*.c
    ${SRC_DIR}/audio/dummy/*.c
    ${SRC_DIR}/cpuinfo/*.c
    ${SRC_DIR}/events/*.c
    ${SRC_DIR}/file/*.c
    ${SRC_DIR}/joystick/*.c
    ${SRC_DIR}/joystick/steam/*.c
    ${SRC_DIR}/haptic/*.c
    ${SRC_DIR}/libm/*.c
    ${SRC_DIR}/power/*.c
    ${SRC_DIR}/stdlib/*.c
    ${SRC_DIR}/thread/*.c
    ${SRC_DIR}/timer/*.c
    ${SRC_DIR}/video/*.c
    ${SRC_DIR}/video/yuv2rgb/*.c
    ${SRC_DIR}/video/dummy/*.c
)

# Windows specific files
if (WIN32)
    file(GLOB HEADER_FILES_WIN32
        ${SRC_DIR}/core/windows/*.h
        ${SRC_DIR}/audio/winmm/*.h
        ${SRC_DIR}/audio/wasapi/*.h
        ${SRC_DIR}/thread/windows/*.h
        ${SRC_DIR}/joystick/windows/*.h
        ${SRC_DIR}/haptic/windows/*.h
        ${SRC_DIR}/video/windows/*.h
    )

    file(GLOB SOURCE_FILES_WIN32
        ${SRC_DIR}/core/windows/*.c
        ${SRC_DIR}/video/windows/*.c
        ${SRC_DIR}/thread/windows/*.c
        ${SRC_DIR}/thread/generic/SDL_syscond.c # Weird but this is how they do it on windows.
        ${SRC_DIR}/power/windows/*.c
        ${SRC_DIR}/filesystem/windows/*.c
        ${SRC_DIR}/timer/windows/*.c
        ${SRC_DIR}/joystick/windows/*.c
        ${SRC_DIR}/haptic/windows/*.c
        ${SRC_DIR}/loadso/windows/*.c
        ${SRC_DIR}/audio/winmm/*.c
        ${SRC_DIR}/audio/wasapi/*.c
    )

    set(HEADER_FILES ${HEADER_FILES} ${HEADER_FILES_WIN32})
    set(SOURCE_FILES ${SOURCE_FILES} ${SOURCE_FILES_WIN32})
    list(APPEND EXTRA_LIBS user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 shell32)
endif()

# we're building a library
add_library(${TARGET} ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(${TARGET} PUBLIC ${HEADER_DIR})
target_compile_definitions(${TARGET} PUBLIC -DSDL_MAIN_HANDLED=1 -DSDL_RENDER_DISABLED=1)
target_link_libraries(${TARGET} ${EXTRA_LIBS})
