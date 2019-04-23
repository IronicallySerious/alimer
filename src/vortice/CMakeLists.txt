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
project (vortice)

macro (define_engine_source_files)
	cmake_parse_arguments(DEFINE_SRC_FILES "NORECURSE" "" "" ${ARGN} )
	if (DEFINE_SRC_FILES_NORECURSE)
		set (_action GLOB)
	else ()
		set (_action GLOB_RECURSE)
	endif ()
	foreach (path ${DEFINE_SRC_FILES_UNPARSED_ARGUMENTS})
		# Get header files
		file (${_action} _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.hpp ${path}/*.h ${path}/*.inl)

		list (APPEND HEADER_FILES ${_files})

        # Remove unsupported files for install.
        list (REMOVE_ITEM _files Graphics/GPUDeviceImpl.h)

        if (ALIMER_INSTALL)
		    # Install them
            install (FILES ${_files} DESTINATION ${DEST_BASE_INCLUDE_DIR}/Alimer/${path})
        endif ()

		# Get source files
		file (${_action} _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.c ${path}/*.cpp)
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

option(VORTICE_BUILD_SHARED "Build as a shared library instead of an executable" OFF)

# Setup
if (EMSCRIPTEN)
    string(CONCAT LOVR_EMSCRIPTEN_FLAGS
        "-O3 "
        "-s WASM=1 "
        "-s USE_WEBGL2=1 "
        "-s GL_PREINITIALIZED_CONTEXT=1 "
        "-s USE_ZLIB=1 "
        "-s USE_PTHREADS=0 "
        "-s FULL_ES2=1 "
        "-s FULL_ES3=1 "
        "-s FORCE_FILESYSTEM=1 "
        "-s ALLOW_MEMORY_GROWTH=1 "
        "-s \"EXPORTED_FUNCTIONS=[ "
        "'_main'"
        "]\" "
        "-s \"EXTRA_EXPORTED_RUNTIME_METHODS=['getValue','setValue']\" "
        # "--js-library \"${CMAKE_CURRENT_SOURCE_DIR}/src/resources/webvr.js\" "
        # "--shell-file \"${CMAKE_CURRENT_SOURCE_DIR}/src/resources/vortice.html\""
    )
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LOVR_EMSCRIPTEN_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LOVR_EMSCRIPTEN_FLAGS}")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
elseif(ANDROID)
elseif(UNIX)
    if(APPLE)
        set(CMAKE_MACOSX_RPATH 1)
    endif()
    find_package(PkgConfig)
endif ()

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
define_engine_source_files(core)

if (VORTICE_BUILD_SHARED)
    add_library(vortice SHARED ${SOURCE_FILES} ${HEADER_FILES})
else()
    add_executable(vortice ${SOURCE_FILES} ${HEADER_FILES})
endif()

if (WIN32)
    target_sources(vortice PRIVATE
        platform/sokol.c
        platform/windows.c
    )
    target_link_libraries(vortice sokol_app)
endif ()

target_link_libraries(vortice sokol_gfx)

target_include_directories(vortice PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)
