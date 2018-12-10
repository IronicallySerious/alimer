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

# Compile built-in shaders
if (ALIMER_TOOLS)
    set(SHADERS_SRCS
        shaders/imgui.vert
        shaders/imgui.frag
    )

    set (SHADER_BINS)
    set (SHADER_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/shaders_h")

    foreach (shader_src ${SHADERS_SRCS})
        get_filename_component(local_name "${shader_src}" NAME)
        get_filename_component(full_name "${shader_src}" ABSOLUTE)
        set (output_path "${SHADER_OUTPUT_PATH}/${local_name}.h")

        # For now compile to spv
        add_custom_command(
                OUTPUT ${output_path}
                COMMAND shaderc -i ${full_name} -o ${output_path} -f header
                MAIN_DEPENDENCY ${shader_src}
                DEPENDS ${shader_src} shaderc
                COMMENT "Compiling shader ${shader_src} to ${output_path}"
        )

        list(APPEND SHADER_BINS ${output_path})
    endforeach()
endif()

set (TARGET_NAME libAlimer)
set (ALIMER_NATIVE_INCLUDE_DIRS ${SPIRV_TOOLS_INCLUDE_DIR} ${STB_INCLUDE_DIR})

# Generate the AlimerConfig.h file
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/AlimerConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/AlimerConfig.h)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/AlimerConfig.h DESTINATION ${DEST_BASE_INCLUDE_DIR}/Alimer)

# Add source files from subdirectories
define_engine_source_files (NORECURSE . Base Core Debug IO Math Graphics Input Audio Application)
define_engine_source_files (Resource Serialization Scene Renderer)
define_engine_source_files (NORECURSE UI)

if (ALIMER_NETWORK)
    define_engine_source_files(Network)
endif ()

if (ALIMER_WINDOWS OR ALIMER_LINUX OR ALIMER_OSX)
    define_engine_source_files(Application/SDL2)
    define_engine_source_files(Input/SDL2)
elseif (ALIMER_UWP)
	define_engine_source_files(Application/UWP)
	define_engine_source_files(Input/UWP)
endif ()

if (ALIMER_WINDOWS OR ALIMER_UWP)
    define_engine_source_files(Audio/XAudio2)
endif()

if (ALIMER_D3D11 OR ALIMER_D3D12)
    define_engine_source_files(Graphics/D3D)
endif ()

if (ALIMER_D3D12)
    define_engine_source_files(Graphics/D3D12)
endif ()

if (ALIMER_GL)
    define_engine_source_files(Graphics/OpenGL)
    list (APPEND ALIMER_NATIVE_INCLUDE_DIRS ${ALIMER_THIRD_PARTY_DIR}/khronos)
endif ()

if (ALIMER_VULKAN)
    # define_engine_source_files(Graphics/Vulkan)

    #set(GLSLANG_HEADER_FILES
	#    ${ALIMER_THIRD_PARTY_DIR}/glslang/StandAlone/DirStackFileIncluder.h
	#    ${ALIMER_THIRD_PARTY_DIR}/glslang/StandAlone/ResourceLimits.h
	#    ${ALIMER_THIRD_PARTY_DIR}/glslang/StandAlone/Worklist.h
    #)

    #set(GLSLANG_SOURCE_FILES
	#    ${ALIMER_THIRD_PARTY_DIR}/glslang/StandAlone/ResourceLimits.cpp
    #)

    #list (APPEND HEADER_FILES ${GLSLANG_HEADER_FILES})
    #list (APPEND SOURCE_FILES ${GLSLANG_SOURCE_FILES})
endif ()

if (ALIMER_CSHARP)
    define_engine_source_files(CSharp)
endif ()

if (MSVC)
    list (APPEND SOURCE_FILES ${ALIMER_ROOT_DIR}/script/visualizers/alimer.natvis)
endif ()

# Group source code in VS solution
group_sources()

# Define the library target.
add_alimer_library(${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_BINS})

target_include_directories(${TARGET_NAME}
    PUBLIC
	    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

target_include_directories(${TARGET_NAME}
    PRIVATE
    ${ALIMER_THIRD_PARTY_DIR}
    ${ALIMER_NATIVE_INCLUDE_DIRS}
    )

# Link dependencies
set (THIRD_PARTY_DEPENDENCIES fmt spirv_cross)
if (ALIMER_WINDOWS OR ALIMER_LINUX OR ALIMER_OSX)
    list (APPEND THIRD_PARTY_DEPENDENCIES SDL2)
endif ()

# if (NOT ALIMER_DISABLE_LUA)
#     list (APPEND THIRD_PARTY_DEPENDENCIES liblua)
# endif()

if (ALIMER_VULKAN)
    list (APPEND THIRD_PARTY_DEPENDENCIES volk vkmemalloc)
endif ()

foreach (dep ${THIRD_PARTY_DEPENDENCIES})
    if (TARGET "${dep}")
        target_link_libraries(${TARGET_NAME} ${dep})
    endif ()
endforeach()

target_compile_definitions(${TARGET_NAME} PRIVATE ALIMER_DEFAULT_ASSETS_DIRECTORY=\"${CMAKE_CURRENT_SOURCE_DIR}/../../assets\")

if (ALIMER_SHARED OR ALIMER_CSHARP OR EMSCRIPTEN)
	target_compile_definitions(${TARGET_NAME} PRIVATE -DALIMER_EXPORTS INTERFACE -DALIMER_IMPORTS)
else ()
    target_compile_definitions(${TARGET_NAME} PRIVATE -DALIMER_STATIC)
endif ()

if (ALIMER_CSHARP)
    target_compile_definitions(${TARGET_NAME} PRIVATE -DALIMER_CSHARP)
endif ()

# Add OS-specific library dependencies
if (ALIMER_WINDOWS)
    target_link_libraries (${TARGET_NAME} winmm ws2_32)
elseif (ALIMER_UWP)
    target_link_libraries (${TARGET_NAME} PRIVATE d3d12 d3d11 dxgi d3dcompiler)
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

# Suppress some MSVC warnings
if (MSVC)
    # = conversion possible loss of data -- for STB image
    target_compile_options(${TARGET_NAME} PRIVATE /wd4100)
endif()

# SDK installation
if (NOT ALIMER_SKIP_INSTALL)
    install(TARGETS ${TARGET_NAME}
	    EXPORT ${TARGET_NAME}
	    LIBRARY DESTINATION lib
	    RUNTIME DESTINATION bin
	    ARCHIVE DESTINATION lib
    )
endif ()
