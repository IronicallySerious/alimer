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
file(GLOB HEADER_FILES 
    ${HEADER_DIR}/*.h
    ${SRC_DIR}/*.h
    ${SRC_DIR}/audio/*.h
    ${SRC_DIR}/audio/disk/*.h
    ${SRC_DIR}/audio/dsp/*.h
    ${SRC_DIR}/audio/dummy/*.h
)

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
    ${SRC_DIR}/joystick/hidapi/*.c
    ${SRC_DIR}/haptic/*.c
    ${SRC_DIR}/hidapi/*.c
    ${SRC_DIR}/libm/*.c
    ${SRC_DIR}/power/*.c
    ${SRC_DIR}/sensor/*.c
    ${SRC_DIR}/sensor/dummy/*.c
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
        ${SRC_DIR}/hidapi/windows/*.c
        ${SRC_DIR}/loadso/windows/*.c
        ${SRC_DIR}/audio/winmm/*.c
        ${SRC_DIR}/audio/wasapi/*.c
    )

    set(HEADER_FILES ${HEADER_FILES} ${HEADER_FILES_WIN32})
    set(SOURCE_FILES ${SOURCE_FILES} ${SOURCE_FILES_WIN32})
    
    list(APPEND EXTRA_LIBS dinput8)
    list(APPEND EXTRA_LIBS dxguid)
    list(APPEND EXTRA_LIBS gdi32)
    list(APPEND EXTRA_LIBS imagehlp)
    list(APPEND EXTRA_LIBS imm32)
    list(APPEND EXTRA_LIBS ole32)
    list(APPEND EXTRA_LIBS oleaut32)
    list(APPEND EXTRA_LIBS shell32)
    list(APPEND EXTRA_LIBS user32)
    list(APPEND EXTRA_LIBS uuid)
    list(APPEND EXTRA_LIBS version)
    list(APPEND EXTRA_LIBS winmm)
    list(APPEND EXTRA_LIBS setupapi)
endif()

# Linux specific files
if (LINUX)
    file(GLOB SOURCE_FILES_LINUX
            ${SRC_DIR}/core/unix/*.c
            ${SRC_DIR}/video/x11/*.c
            ${SRC_DIR}/thread/pthread/*.c
            ${SRC_DIR}/power/linux/*.c
            ${SRC_DIR}/filesystem/unix/*.c
            ${SRC_DIR}/timer/unix/*.c
            ${SRC_DIR}/joystick/linux/*.c
            ${SRC_DIR}/haptic/linux/*.c
            ${SRC_DIR}/loadso/dlopen/*.c
    )
    set(SOURCE_FILES ${SOURCE_FILES} ${SOURCE_FILES_LINUX})
endif()

# Darwin specific files
if (APPLE)
    include_directories(${SRC_DIR}/video/khronos)
    file(GLOB SOURCE_FILES_DARWIN
        ${SRC_DIR}/audio/coreaudio/*.c
        ${SRC_DIR}/file/cocoa/*.m
        ${SRC_DIR}/filesystem/cocoa/*.m
        ${SRC_DIR}/video/cocoa/*.m
        ${SRC_DIR}/joystick/darwin/*.c
        ${SRC_DIR}/haptic/darwin/*.c
        ${SRC_DIR}/power/macosx/*.c
        ${SRC_DIR}/timer/unix/*.c
        ${SRC_DIR}/thread/pthread/*.c
        ${SRC_DIR}/loadso/dlopen/*.c
        ${SRC_DIR}/render/metal/*.m
    )

    set(SOURCE_FILES ${SOURCE_FILES} ${SOURCE_FILES_DARWIN})

    # Darwin frameworks
    find_library(COCOA_LIBRARY Cocoa)
    list(APPEND EXTRA_LIBS ${COCOA_LIBRARY})

    find_library(IOKIT IOKit)
    list(APPEND EXTRA_LIBS ${IOKIT})

    find_library(FORCEFEEDBACK ForceFeedback)
    list(APPEND EXTRA_LIBS ${FORCEFEEDBACK})

    find_library(CARBON_LIBRARY Carbon)
    list(APPEND EXTRA_LIBS ${CARBON_LIBRARY})

    find_library(COREAUDIO CoreAudio)
    list(APPEND EXTRA_LIBS ${COREAUDIO})

    find_library(AUDIOUNIT AudioUnit)
    list(APPEND EXTRA_LIBS ${AUDIOUNIT})

    find_library(CORE_VIDEO_LIBRARY CoreVideo)
    list(APPEND EXTRA_LIBS ${CORE_VIDEO_LIBRARY})

    find_library(METAL_LIBRARY Metal)
    list(APPEND EXTRA_LIBS ${METAL_LIBRARY})
endif()

# we're building a library
add_library(${TARGET} ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(${TARGET} PUBLIC ${HEADER_DIR})
target_compile_definitions(${TARGET} PUBLIC -DSDL_MAIN_HANDLED=1 -DSDL_RENDER_DISABLED=1)
target_link_libraries(${TARGET} PUBLIC ${EXTRA_LIBS})

if (NOT MSVC)
    target_compile_options(${TARGET}
        PRIVATE -msse2
        PRIVATE -msse
        PRIVATE -mmmx
        PRIVATE -Wno-unused-variable
        PRIVATE -Wno-deprecated-declarations
        PRIVATE -Wno-switch
        PRIVATE -Wno-tautological-pointer-compare
        PRIVATE -Wno-parentheses-equality
    )
endif ()
