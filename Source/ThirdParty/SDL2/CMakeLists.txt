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

# General source files
file(GLOB SRCS
    ${SRC_DIR}/*.c
    ${SRC_DIR}/atomic/*.c
    ${SRC_DIR}/audio/*.c
    ${SRC_DIR}/audio/disk/*.c
    ${SRC_DIR}/audio/dsp/*.c
    ${SRC_DIR}/audio/dummy/*.c
    ${SRC_DIR}/cpuinfo/*.c
    ${SRC_DIR}/dynapi/*.c
    ${SRC_DIR}/events/*.c
    ${SRC_DIR}/file/*.c
    ${SRC_DIR}/joystick/*.c
    ${SRC_DIR}/joystick/steam/*.c
    ${SRC_DIR}/haptic/*.c
    ${SRC_DIR}/libm/*.c
    ${SRC_DIR}/power/*.c
    ${SRC_DIR}/render/*.c
    ${SRC_DIR}/render/*/*.c
    ${SRC_DIR}/stdlib/*.c
    ${SRC_DIR}/thread/*.c
    ${SRC_DIR}/timer/*.c
    ${SRC_DIR}/video/*.c
    ${SRC_DIR}/video/yuv2rgb/*.c
    ${SRC_DIR}/video/dummy/*.c
)

add_definitions(-D_THREAD_SAFE)

# we're building a library
add_library(${TARGET} ${SRCS})

target_include_directories(${TARGET} PUBLIC ${HEADER_DIR})
target_link_libraries(${TARGET} ${EXTRA_LIBS})
