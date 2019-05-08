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

define_engine_source_files (foundation)
define_engine_source_files (NORECURSE core)

if (WIN32)
    define_engine_source_files(core/windows)
endif ()

# Group source code in VS solution
group_sources()

if (ALIMER_BUILD_SHARED)
    add_library(alimer SHARED ${SOURCE_FILES} ${HEADER_FILES})
else()
    if (WIN32 AND NOT ALIMER_WIN32_FORCE_MAIN)
        add_executable(alimer WIN32 ${SOURCE_FILES} ${HEADER_FILES})
    else ()
        add_executable(alimer ${SOURCE_FILES} ${HEADER_FILES})
    endif ()
endif()

target_include_directories(alimer
    PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

# Libraries
if (WIN32)
    set_target_properties(alimer PROPERTIES COMPILE_FLAGS "/wd4244")
    if (ALIMER_WIN32_FORCE_MAIN)
        set_target_properties(alimer PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
        set_target_properties(alimer PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
        target_compile_definitions(alimer PRIVATE ALIMER_WIN32_FORCE_MAIN)
    endif()

    target_compile_definitions(alimer PRIVATE UNICODE _UNICODE)
    target_compile_definitions(alimer PRIVATE _SCL_SECURE_NO_WARNINGS _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)

    if (MSVC_VERSION VERSION_LESS 1900)
        target_compile_definitions(alimer PRIVATE -Dinline=__inline -Dsnprintf=_snprintf)
    endif()
elseif (APPLE)
elseif (EMSCRIPTEN)
    target_link_libraries(alimer PRIVATE ${ALIMER_EMSCRIPTEN_FLAGS})
elseif (ANDROID)
    target_link_libraries(alimer PRIVATE log EGL GLESv3)
    target_include_directories(alimer PRIVATE ${ANDROID_NDK}/sources/android/cpufeatures)
    target_include_directories(alimer PRIVATE ${ANDROID_NDK}/sources/android/native_app_glue/)
else ()
    target_link_libraries(alimer PRIVATE glfw)
endif ()

if (ALIMER_VULKAN)
    target_compile_definitions(alimer PRIVATE -DVGPU_VK)
    target_link_libraries(alimer PRIVATE
        volk
        vma
    )
elseif (ALIMER_D3D11)
    target_compile_definitions(alimer PRIVATE -DVGPU_D3D11)
    target_link_libraries(alimer PRIVATE
        d3d11
        dxgi
    )
endif ()
