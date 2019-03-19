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

cmake_minimum_required(VERSION 3.5)
project (alimer)

macro (define_engine_source_files)
    cmake_parse_arguments(DEFINE_SRC_FILES "NORECURSE" "" "" ${ARGN} )
    if (DEFINE_SRC_FILES_NORECURSE)
        set (_action GLOB)
    else ()
        set (_action GLOB_RECURSE)
    endif ()
    foreach (path ${DEFINE_SRC_FILES_UNPARSED_ARGUMENTS})
        # Get header files
        file (${_action} _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.h)
        list (APPEND HEADER_FILES ${_files})
        # Install them
        if (ALIMER_INSTALL)
            install (FILES ${_files} DESTINATION ${DEST_INCLUDE_DIR}/${path})
        endif ()
        # Get source files
        file (${_action} _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}  ${path}/*.c ${path}/*.cpp)
        list (APPEND SOURCE_FILES ${_files})
    endforeach ()
endmacro()

# Groups sources into subfolders.
macro(group_sources)
    file (GLOB_RECURSE children LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/**)
    foreach (child ${children})
        if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
            string(REPLACE "/" "\\" groupname "${child}")
            file (GLOB files LIST_DIRECTORIES false ${CMAKE_CURRENT_SOURCE_DIR}/${child}/*)
            source_group(${groupname} FILES ${files})
        endif ()
    endforeach ()
endmacro()

define_engine_source_files (foundation core)
define_engine_source_files (engine)
define_engine_source_files (NORECURSE graphics)

if (ALIMER_OPENGL)
    define_engine_source_files(graphics/opengl)
endif ()

if (ALIMER_D3D12)
    define_engine_source_files(graphics/d3d12)
endif ()

# Group source code in VS solution
group_sources()

set (TARGET_NAME alimer_new)

if (ALIMER_BUILD_SHARED)
    add_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} $<$<CXX_COMPILER_ID:MSVC>:${ALIMER_ROOT_DIR}/script/visualizers/alimer.natvis>)
    target_compile_definitions(${TARGET_NAME} PRIVATE ALIMER_BUILD_SHARED_LIBRARY)
else()
    add_library(${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES} $<$<CXX_COMPILER_ID:MSVC>:${ALIMER_ROOT_DIR}/script/visualizers/alimer.natvis>)
endif ()

target_include_directories(${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)



if (ALIMER_DESKTOP)
    target_compile_definitions(${TARGET_NAME} PRIVATE ALIMER_GLFW)
    target_link_libraries(${TARGET_NAME} PRIVATE glfw)
elseif (ANDROID)
    target_link_libraries(${TARGET_NAME} PRIVATE dl log android)
    
    # target_link_libraries(${TARGET_NAME} PRIVATE GLESv3)
    target_include_directories(${TARGET_NAME} PRIVATE ${ANDROID_NDK}/sources/android/cpufeatures)
    target_include_directories(${TARGET_NAME} PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue/)
elseif (EMSCRIPTEN)
endif ()

# OpenGL
if (ALIMER_OPENGL)
    target_compile_definitions(${TARGET_NAME} PRIVATE ALIMER_OPENGL)
    if (WIN32)
        target_link_libraries(${TARGET_NAME} PRIVATE opengl32)
    elseif (ANDROID)
        target_link_libraries(${TARGET_NAME} PRIVATE EGL GLESv2)
    endif()

    if (NOT EMSCRIPTEN)
        target_link_libraries(${TARGET_NAME} PRIVATE glad)
    endif ()
endif ()

# D3D12
if (ALIMER_D3D12)
    target_compile_definitions(${TARGET_NAME} PRIVATE ALIMER_D3D12)
endif ()
