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

# Compile SPIRV GLSL (https://gist.github.com/vlsh/a0d191701cb48f157b05be7f74d79396)
#if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
#	set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
#else()
#	set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin32/glslangValidator.exe")
#endif()

#file(GLOB_RECURSE GLSL_SOURCE_FILES
#	"${ALIMER_ASSETS_PATH}/shaders/*.vert"
#	"${ALIMER_ASSETS_PATH}/shaders/*.frag"
#	)

#foreach(GLSL ${GLSL_SOURCE_FILES})
#	get_filename_component(FILE_NAME ${GLSL} NAME)
#	set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
#	add_custom_command(
#		OUTPUT ${SPIRV}
#		COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
#		COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
#		DEPENDS ${GLSL})
#	list(APPEND SPIRV_BINARY_FILES ${SPIRV})
#endforeach()

#add_custom_target(
#	AlimerShaders 
#	DEPENDS ${SPIRV_BINARY_FILES}
#	)

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
		list (REMOVE_ITEM _files
			Graphics/GraphicsImpl.h
		)
		list (APPEND HEADER_FILES ${_files})
		# Install them
		install (FILES ${_files} DESTINATION ${DEST_BASE_INCLUDE_DIR}/Alimer/${path})
		# Get source files
		file (${_action} _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.cpp)
		list (APPEND SOURCE_FILES ${_files})
	endforeach ()
endmacro()

# Add source files from subdirectories
define_engine_source_files (NORECURSE . Core Util IO Math Graphics Input Audio Application)
define_engine_source_files (Resource Serialization Scene)

if (PLATFORM_WINDOWS)
	define_engine_source_files(Application/Windows)
	define_engine_source_files(Input/Windows)
	define_engine_source_files(Audio/WASAPI)
elseif (PLATFORM_UWP)
	define_engine_source_files(Application/UWP)
	define_engine_source_files(Audio/WASAPI)
endif ()

if (ALIMER_VULKAN)
	define_engine_source_files(Graphics/Vulkan)
	# volk
	list (APPEND SOURCE_FILES ${ALIMER_THIRD_PARTY_DIR}/volk/volk.c)
endif (ALIMER_VULKAN)

if (ALIMER_D3D12)
	define_engine_source_files(Graphics/D3D12)
endif (ALIMER_D3D12)

# SPIRV-Cross
set(SPIRV_CROSS_HEADER_FILES
	${ALIMER_THIRD_PARTY_DIR}/SPIRV-Cross/GLSL.std.450.h
	${ALIMER_THIRD_PARTY_DIR}/SPIRV-Cross/spirv_common.hpp
	${ALIMER_THIRD_PARTY_DIR}/SPIRV-Cross/spirv_cfg.hpp
	${ALIMER_THIRD_PARTY_DIR}/SPIRV-Cross/spirv_cross.hpp
	${ALIMER_THIRD_PARTY_DIR}/SPIRV-Cross/spirv.hpp
)

set(SPIRV_CROSS_SOURCE_FILES
	${ALIMER_THIRD_PARTY_DIR}/SPIRV-Cross/spirv_cfg.cpp
	${ALIMER_THIRD_PARTY_DIR}/SPIRV-Cross/spirv_cross.cpp
)

set(NEED_SPIRV_CROSS_GLSL OFF)

if (ALIMER_D3D11 OR ALIMER_D3D12)
	list(APPEND SPIRV_CROSS_HEADER_FILES
		${ALIMER_THIRD_PARTY_DIR}/SPIRV-Cross/spirv_hlsl.hpp
	)

	list(APPEND SPIRV_CROSS_SOURCE_FILES
		${ALIMER_THIRD_PARTY_DIR}/SPIRV-Cross/spirv_hlsl.cpp
	)
	set(NEED_SPIRV_CROSS_GLSL ON)
endif()

if (ALIMER_VULKAN OR ALIMER_OPENGL OR NEED_SPIRV_CROSS_GLSL)
	list(APPEND SPIRV_CROSS_HEADER_FILES
		${ALIMER_THIRD_PARTY_DIR}/SPIRV-Cross/spirv_glsl.hpp
	)
	list(APPEND SPIRV_CROSS_SOURCE_FILES
		${ALIMER_THIRD_PARTY_DIR}/SPIRV-Cross/spirv_glsl.cpp
	)
endif()

list (APPEND HEADER_FILES ${SPIRV_CROSS_HEADER_FILES})
list (APPEND SOURCE_FILES ${SPIRV_CROSS_SOURCE_FILES})
install (FILES ${SPIRV_CROSS_HEADER_FILES} DESTINATION ${DEST_INCLUDE_DIR}/ThirdParty/SPIRV-Cross)

# Group source code in VS solution
group_sources()

# Define the library target.
add_library(libAlimer ${ALIMER_LIBRARY_TYPE} ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(libAlimer 
	PUBLIC  
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${DEST_BASE_INCLUDE_DIR}>
	PRIVATE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${ALIMER_THIRD_PARTY_DIR}>
    $<BUILD_INTERFACE:${ALIMER_THIRD_PARTY_DIR}/glfw/include>
)

if (BUILD_SHARED_LIBS)
	target_compile_definitions(libAlimer PRIVATE -DALIMER_SHARED=1)
endif ()

if (ALIMER_D3D12)
	target_compile_definitions(libAlimer PRIVATE -DALIMER_D3D12)
endif (ALIMER_D3D12)

if (ALIMER_VULKAN)
	target_compile_definitions(libAlimer PRIVATE -DALIMER_VULKAN)
	if (PLATFORM_WINDOWS)
		target_compile_definitions (libAlimer PRIVATE -DVK_USE_PLATFORM_WIN32_KHR)
	elseif (PLATFORM_LINUX)
		if(ALIMER_LINUX_WAYLAND)
			target_compile_definitions (libAlimer PRIVATE -DVK_USE_PLATFORM_WAYLAND_KHR)
		else()
			target_compile_definitions (libAlimer PRIVATE -DVK_USE_PLATFORM_XCB_KHR)
		endif()
	elseif (PLATFORM_ANDROID)
		target_compile_definitions (libAlimer PRIVATE -DVK_USE_PLATFORM_ANDROID_KHR)
	endif()

endif (ALIMER_VULKAN)

if (ALIMER_SHADER_COMPILER)
	target_include_directories(libAlimer PRIVATE ${ALIMER_THIRD_PARTY_DIR}/glslang)
	target_compile_definitions(libAlimer PRIVATE -DALIMER_SHADER_COMPILER)
	target_link_libraries (libAlimer glslang SPIRV)
endif (ALIMER_SHADER_COMPILER)

# Add OS-specific library dependencies
if (PLATFORM_WINDOWS)
	target_compile_definitions (libAlimer PRIVATE -D_UNICODE -DNOMINMAX)
elseif (PLATFORM_UWP)
	set_target_properties(libAlimer PROPERTIES VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION "${PLATFORM_UWP_VERSION_MIN}")
	target_compile_definitions (libAlimer PRIVATE -D_UNICODE -DNOMINMAX)
	target_link_libraries (libAlimer mincore d3d12.lib dxgi.lib d3dcompiler.lib)
else ()
	target_link_libraries (libAlimer pthread)
endif ()

if (ALIMER_CSHARP)
	target_compile_definitions(libAlimer PRIVATE -DALIMER_CSHARP)
endif (ALIMER_CSHARP)

# add_dependencies(libAlimer AlimerShaders)

# add_custom_command(TARGET libAlimer POST_BUILD
# 	COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:libAlimer>/shaders/"
# 	COMMAND ${CMAKE_COMMAND} -E copy_directory
# 		"${PROJECT_BINARY_DIR}/shaders"
# 		"$<TARGET_FILE_DIR:libAlimer>/shaders"
# 		)

# SDK installation
install(TARGETS libAlimer
	EXPORT Alimer
	LIBRARY DESTINATION ${DEST_LIBRARY_DIR}
	RUNTIME DESTINATION ${DEST_BIN_DIR}
	ARCHIVE DESTINATION ${DEST_ARCHIVE_DIR}
)
