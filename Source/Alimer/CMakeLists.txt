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

macro (define_engine_source_files)
	cmake_parse_arguments(DEFINE_SRC_FILES "NORECURSE" "" "" ${ARGN} )
	if (DEFINE_SRC_FILES_NORECURSE)
		set (_action GLOB)
	else ()
		set (_action GLOB_RECURSE)
	endif ()
	foreach (path ${DEFINE_SRC_FILES_UNPARSED_ARGUMENTS})
		# Get header files
		file (${_action} _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.h ${path}/*.inl)
        
		list (APPEND HEADER_FILES ${_files})

        # Remove unsupported files for install.
        list (REMOVE_ITEM _files
            Graphics/GraphicsImpl.h
        )

		# Install them
		install (FILES ${_files} DESTINATION ${DEST_BASE_INCLUDE_DIR}/Alimer/${path})

		# Get source files
		file (${_action} _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.cpp)
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

set (TARGET_NAME libAlimer)

# Generate the AlimerVersion.h file
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/AlimerVersion.h.in ${CMAKE_CURRENT_BINARY_DIR}/AlimerVersion.h)

# Add source files from subdirectories
define_engine_source_files (NORECURSE . Base Core IO Math Graphics Input Audio Application)
define_engine_source_files (Resource Serialization Scene Renderer)
define_engine_source_files (NORECURSE experimental)

if (ALIMER_WINDOWS)
	define_engine_source_files(Application/Windows)
	define_engine_source_files(Input/Windows)
	define_engine_source_files(Audio/WASAPI)
elseif (ALIMER_UWP)
	define_engine_source_files(Application/UWP)
	define_engine_source_files(Audio/WASAPI)
endif ()

if (ALIMER_WINDOWS OR ALIMER_UWP)
    define_engine_source_files(IO/Windows)
endif()

if (ALIMER_GL)
    define_engine_source_files(Graphics/OpenGL)
endif (ALIMER_GL)

if (ALIMER_VULKAN)
    define_engine_source_files(Graphics/Vulkan)
endif (ALIMER_VULKAN)

if (ALIMER_D3D11 OR ALIMER_D3D12)
    define_engine_source_files(Graphics/D3D)
endif()

if (ALIMER_D3D11)
	define_engine_source_files(Graphics/D3D11)
endif ()

if (ALIMER_D3D12)
	define_engine_source_files(Graphics/D3D12)
endif ()

# Group source code in VS solution
group_sources()

# Define the library target.
add_alimer_library(libAlimer ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(libAlimer
    PUBLIC
	    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	    $<INSTALL_INTERFACE:${DEST_BASE_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${DEST_THIRDPARTY_HEADERS_DIR}>
	PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${ALIMER_THIRD_PARTY_DIR}>
        $<BUILD_INTERFACE:${ALIMER_THIRD_PARTY_DIR}>/khronos
)

# Link dependencies
set (THIRD_PARTY_DEPENDENCIES STB spirv_cross)
if (ALIMER_CAN_COMPILE)
    list (APPEND THIRD_PARTY_DEPENDENCIES glslang SPIRV)
endif ()

foreach (dep ${THIRD_PARTY_DEPENDENCIES})
    if (TARGET "${dep}")
        target_link_libraries(libAlimer ${dep})
    endif ()
endforeach()

target_compile_definitions(libAlimer PRIVATE ALIMER_DEFAULT_ASSETS_DIRECTORY=\"${CMAKE_CURRENT_SOURCE_DIR}/../../assets\")
if (ALIMER_D3D11)
    target_compile_definitions(libAlimer PRIVATE -DALIMER_D3D11)
endif()

if (ALIMER_D3D12)
    target_compile_definitions(libAlimer PRIVATE -DALIMER_D3D12)
endif()

if (ALIMER_THREADING)
    target_compile_definitions(libAlimer PRIVATE -DALIMER_THREADING=1)
endif ()

if (ALIMER_SHARED OR EMSCRIPTEN)
	target_compile_definitions(libAlimer PRIVATE -DALIMER_SHARED INTERFACE -DALIMER_IMPORTS)
endif ()

if (ALIMER_GL)
	target_compile_definitions(libAlimer PRIVATE -DALIMER_GL)
endif (ALIMER_GL)

if (ALIMER_VULKAN)
	target_compile_definitions(libAlimer PRIVATE -DALIMER_VULKAN)
endif ()

# Add OS-specific library dependencies
if (ALIMER_WINDOWS)
    # Link universal crt libs (https://blogs.msdn.microsoft.com/vcblog/2015/03/03/introducing-the-universal-crt/)
    # To avoid SDL2 linking issues.
    if (ALIMER_WINDOWS_CRT)
        target_link_libraries(${TARGET_NAME} debug libcmtd libvcruntimed libucrtd)
        target_link_libraries(${TARGET_NAME} optimized libcmt libvcruntime libucrt)
    endif ()

    target_link_libraries (${TARGET_NAME} winmm ws2_32)
elseif (ALIMER_UWP)
    if (ALIMER_D3D11 OR ALIMER_D3D12)
        target_link_libraries (${TARGET_NAME} PRIVATE dxgi d3dcompiler)
    endif ()

    if (ALIMER_D3D11)
        target_link_libraries (${TARGET_NAME} PRIVATE d3d11)
    endif()

    if (ALIMER_D3D12)
        target_link_libraries (${TARGET_NAME} PRIVATE d3d12)
    endif ()

elseif (ALIMER_ANDROID)
    if (${ANDROID_PLATFORM_LEVEL} LESS 12)
        message(FATAL_ERROR "OpenGLES 2 is not supported before API level 11 (currently using ${ANDROID_PLATFORM_LEVEL}).")
        return()
    elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
        add_definitions("-DDYNAMIC_ES3")
        set(GL3STUB_SRC gl3stub.c)
        set(OPENGL_LIB GLESv2)
    else ()
        set(OPENGL_LIB GLESv3)
    endif ()

    target_link_libraries (${TARGET_NAME} ${OPENGL_LIB} android EGL log m)
else ()
	target_link_libraries (${TARGET_NAME} pthread)
endif ()

if (ALIMER_CSHARP)
	target_compile_definitions(${TARGET_NAME} PRIVATE -DALIMER_CSHARP)
endif ()

# SDK installation
if (NOT ALIMER_SKIP_INSTALL)
    install(TARGETS ${TARGET_NAME}
	    EXPORT ${TARGET_NAME}
	    LIBRARY DESTINATION lib
	    RUNTIME DESTINATION bin
	    ARCHIVE DESTINATION lib
    )
endif ()
