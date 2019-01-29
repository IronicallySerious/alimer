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

set (TARGET_NAME Alimer)

# Generate the AlimerConfig.h file
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/foundation/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/foundation/version.h)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/AlimerConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/AlimerConfig.h)
if (ALIMER_INSTALL)
    install (FILES ${CMAKE_CURRENT_BINARY_DIR}/foundation/version.h DESTINATION ${DEST_BASE_INCLUDE_DIR}/Alimer/foundation)
    install (FILES ${CMAKE_CURRENT_BINARY_DIR}/AlimerConfig.h DESTINATION ${DEST_BASE_INCLUDE_DIR}/Alimer)
endif ()

# Add source files from subdirectories
define_engine_source_files (NORECURSE . foundation Base Core Debug IO Math Graphics Input Audio Application)
define_engine_source_files (Resource Serialization Scene Renderer)
define_engine_source_files (NORECURSE UI)

if (ALIMER_NETWORK)
    define_engine_source_files(Network)
endif ()

if (ALIMER_DESKTOP)
    define_engine_source_files(Application/glfw)
elseif (ALIMER_SDL2)
    define_engine_source_files(Application/SDL2)
elseif (ALIMER_UWP)
	define_engine_source_files(Application/UWP)
endif ()

if (ALIMER_WINDOWS OR ALIMER_UWP)
    define_engine_source_files(Audio/XAudio2)
endif()

if (ALIMER_D3D11 OR ALIMER_D3D12)
    define_engine_source_files(Graphics/D3D)
endif ()

if (ALIMER_D3D11)
    define_engine_source_files(Graphics/D3D11)
endif ()

# Temporary disable d3d12
#if (ALIMER_D3D12)
#    define_engine_source_files(Graphics/D3D12)
#endif ()

if (ALIMER_OPENGL)
    define_engine_source_files(Graphics/opengl)
endif ()

if (ALIMER_VULKAN)
    define_engine_source_files(Graphics/vulkan)

    #set(GLSLANG_HEADER_FILES
	#    ${ALIMER_THIRD_PARTY_DIR}/glslang/StandAlone/DirStackFileIncluder.h
	#    ${ALIMER_THIRD_PARTY_DIR}/glslang/StandAlone/ResourceLimits.h
	#    ${ALIMER_THIRD_PARTY_DIR}/glslang/StandAlone/Worklist.h
    #)

    #set(GLSLANG_SOURCE_FILES
	#    ${ALIMER_THIRD_PARTY_DIR}/glslang/StandAlone/ResourceLimits.cpp
    #)

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
add_alimer_library(${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(${TARGET_NAME}
    PUBLIC
	    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

if (ALIMER_OPENGL AND ALIMER_DESKTOP)
    target_include_directories(${TARGET_NAME} PRIVATE ${ALIMER_THIRD_PARTY_DIR}/khronos)
endif ()

# Link dependencies
set (THIRD_PARTY_DEPENDENCIES fmt spdlog spirv_cross ImGui)

if (ALIMER_DESKTOP)
    list (APPEND THIRD_PARTY_DEPENDENCIES glfw)
elseif (ALIMER_SDL2)
    list (APPEND THIRD_PARTY_DEPENDENCIES SDL2)
endif ()

if (ALIMER_SCRIPTING_LUA)
    list (APPEND THIRD_PARTY_DEPENDENCIES liblua)
endif ()

if (ALIMER_VULKAN)
    list (APPEND THIRD_PARTY_DEPENDENCIES volk vkmemalloc)
endif ()

foreach (dep ${THIRD_PARTY_DEPENDENCIES})
    if (TARGET "${dep}")
        target_link_libraries(${TARGET_NAME} PUBLIC ${dep})
    endif ()
endforeach()

target_compile_definitions(${TARGET_NAME} PRIVATE -DALIMER_COMPILE)
target_compile_definitions(${TARGET_NAME} PRIVATE ALIMER_DEFAULT_ASSETS_DIRECTORY=\"${CMAKE_CURRENT_SOURCE_DIR}/../../assets\")

if (EMSCRIPTEN OR ANDROID OR IOS)
    target_compile_definitions(${TARGET_NAME} PUBLIC -DALIMER_SIMD_DISABLED)
endif ()

if (ALIMER_BUILD_SHARED OR ALIMER_CSHARP OR EMSCRIPTEN)
	target_compile_definitions(${TARGET_NAME} PRIVATE ALIMER_SHARED_LIBRARY)
endif ()

if (ALIMER_SHARED OR ANDROID)
    set_target_properties(${TARGET_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif ()

if (ALIMER_CSHARP)
    target_compile_definitions(${TARGET_NAME} PRIVATE -DALIMER_CSHARP)
endif ()

if (ALIMER_DESKTOP)
    target_compile_definitions(${TARGET_NAME} PRIVATE -DALIMER_GLFW)
elseif (ALIMER_SDL2)
    target_compile_definitions(${TARGET_NAME} PRIVATE -DALIMER_SDL2)
endif ()

if (NOT CMAKE_CROSS_COMPILING AND ALIMER_TOOLS)
    target_compile_definitions(${TARGET_NAME} PRIVATE -DALIMER_TOOLS)
endif ()

# Add OS-specific library dependencies
if (ALIMER_WINDOWS)
    target_link_libraries (${TARGET_NAME} PUBLIC winmm ws2_32)
elseif (ALIMER_UWP)
    target_link_libraries (${TARGET_NAME} PUBLIC d3d12 d3d11 dxgi d3dcompiler)
else ()
    target_link_libraries (${TARGET_NAME} PUBLIC dl)
    if (ALIMER_ANDROID)
        target_link_libraries (${TARGET_NAME} PUBLIC log android)
    elseif (ALIMER_DESKTOP)
	    target_link_libraries (${TARGET_NAME} PUBLIC m pthread)
        if (NOT APPLE)
            target_link_libraries (${TARGET_NAME} PUBLIC rt uuid)
        endif ()
    endif ()

    if (ALIMER_OPENGL)
        if (ANDROID)
            target_link_libraries(${TARGET_NAME} PUBLIC EGL GLESv3)
        else()
            target_link_libraries (${TARGET_NAME} PUBLIC GL)
            target_compile_definitions(${TARGET_NAME} PRIVATE -DALIMER_TOOLS)
        endif ()

        if (NOT ALIMER_DESKTOP)
            target_compile_definitions(${TARGET_NAME} PRIVATE -DALIMER_OPENGLES)
        endif ()
    endif ()
endif ()

# Suppress some MSVC warnings
if (MSVC)
    # = conversion possible loss of data -- for STB image
    target_compile_options(${TARGET_NAME} PRIVATE /wd4100)
endif()

# SDK installation
if (ALIMER_INSTALL)
    install(TARGETS ${TARGET_NAME}
	    EXPORT ${TARGET_NAME}
	    LIBRARY DESTINATION lib
	    RUNTIME DESTINATION bin
	    ARCHIVE DESTINATION lib
    )
endif ()
