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
		file (${_action} _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${path}/*.h)
		list (REMOVE_ITEM _files
			Alimer.h
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
define_engine_source_files (NORECURSE . Core Graphics)

if (PLATFORM_WINDOWS)
	define_engine_source_files(Core/Windows)
elseif (PLATFORM_UWP)
	define_engine_source_files(Application/UWP)
endif ()

if (ALIMER_D3D12)
	define_engine_source_files(Graphics/D3D12)
endif (ALIMER_D3D12)

# Group source code in VS solution
group_sources()

# Define the library target.
add_library(libAlimer ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(libAlimer 
	PUBLIC  
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>        # For parent projects to find auto-generated engine headers
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>        # For parent projects to find engine headers
	$<INSTALL_INTERFACE:${DEST_BASE_INCLUDE_DIR}>
	PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

# Add any variables starting with Alimer_ as project defines
get_cmake_property(__cmake_variables VARIABLES)
foreach (var ${__cmake_variables})
    if ("${var}" MATCHES "^ALIMER_")
        if (${${var}})
            target_compile_definitions(libAlimer PRIVATE -D${var})
        endif ()
    endif ()
endforeach()

if (ALIMER_VULKAN)
	target_compile_definitions(libAlimer PRIVATE ALIMER_VULKAN=1)
endif (ALIMER_VULKAN)

# Add OS-specific library dependencies
if (PLATFORM_WINDOWS)
	target_compile_definitions (libAlimer PRIVATE -D_UNICODE)
elseif (PLATFORM_UWP)
	set_target_properties(libAlimer PROPERTIES VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION "${PLATFORM_UWP_VERSION_MIN}")
	target_compile_definitions (libAlimer PRIVATE -D_UNICODE)
	target_link_libraries (libAlimer mincore d3d12 d3d11 dxgi dxguid)
else ()
	target_link_libraries (libAlimer pthread)
endif ()

# SDK installation
install(TARGETS libAlimer
    EXPORT Alimer
    LIBRARY DESTINATION ${DEST_LIBRARY_DIR}
    RUNTIME DESTINATION ${DEST_BIN_DIR}
    ARCHIVE DESTINATION ${DEST_ARCHIVE_DIR}
)
