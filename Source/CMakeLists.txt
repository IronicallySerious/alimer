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

# ThirdParty
add_subdirectory(ThirdParty)

# https://github.com/google/nxt-standalone/blob/master/src/backend/CMakeLists.txt
if (ALIMER_D3D11 OR ALIMER_D3D12)
	# WIN10_SDK_PATH will be something like C:\Program Files (x86)\Windows Kits\10
	get_filename_component(WIN10_SDK_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Microsoft SDKs\\Windows\\v10.0;InstallationFolder]" ABSOLUTE CACHE)

	# TEMP_WIN10_SDK_VERSION will be something like ${CMAKE_CURRENT_SOURCE_DIR}\10.0.14393 or ${CMAKE_CURRENT_SOURCE_DIR}\10.0.14393.0
	get_filename_component(TEMP_WIN10_SDK_VERSION "[HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Microsoft SDKs\\Windows\\v10.0;ProductVersion]" ABSOLUTE CACHE)

	# strip off ${CMAKE_CURRENT_SOURCE_DIR} to get just the version number
	get_filename_component(WIN10_SDK_VERSION ${TEMP_WIN10_SDK_VERSION} NAME)
	# WIN10_SDK_VERSION will be something like 10.0.14393 or 10.0.14393.0; we need the one that matches the directory name.
	if (IS_DIRECTORY "${WIN10_SDK_PATH}/Include/${WIN10_SDK_VERSION}.0")
		set(WIN10_SDK_VERSION "${WIN10_SDK_VERSION}.0")
	endif()

	# Find the d3d11, d3d12 and dxgi include path, it will typically look something like this.
	# C:\Program Files (x86)\Windows Kits\10\Include\10.0.10586.0\um\d3d12.h
	# C:\Program Files (x86)\Windows Kits\10\Include\10.0.10586.0\shared\dxgi1_4.h
	find_path(D3D11_INCLUDE_DIR     # Set variable D3D11_INCLUDE_DIR
		d3d11_1.h                   # Find a path with d3d11_1.h
		HINTS "${WIN10_SDK_PATH}/Include/${WIN10_SDK_VERSION}/um"
		DOC "path to WIN10 SDK header files"
		HINTS
	)

	find_path(D3D12_INCLUDE_DIR     # Set variable D3D12_INCLUDE_DIR
		d3d12.h                     # Find a path with d3d12.h
		HINTS "${WIN10_SDK_PATH}/Include/${WIN10_SDK_VERSION}/um"
		DOC "path to WIN10 SDK header files"
		HINTS
	)

	find_path(DXGI_INCLUDE_DIR      # Set variable DXGI_INCLUDE_DIR
		dxgi1_4.h                   # Find a path with dxgi1_4.h
		HINTS "${WIN10_SDK_PATH}/Include/${WIN10_SDK_VERSION}/shared"
		DOC "path to WIN10 SDK header files"
		HINTS
	)

	if (CMAKE_GENERATOR MATCHES "Visual Studio.*ARM" )
		set(WIN10_SDK_LIB_PATH ${WIN10_SDK_PATH}/Lib/${WIN10_SDK_VERSION}/um/arm)
	elseif (CMAKE_GENERATOR MATCHES "Visual Studio.*ARM64" )
		set(WIN10_SDK_LIB_PATH ${WIN10_SDK_PATH}/Lib/${WIN10_SDK_VERSION}/um/arm64)
	elseif (CMAKE_GENERATOR MATCHES "Visual Studio.*Win64" )
		set(WIN10_SDK_LIB_PATH ${WIN10_SDK_PATH}/Lib/${WIN10_SDK_VERSION}/um/x64)
	else()
		set(WIN10_SDK_LIB_PATH ${WIN10_SDK_PATH}/Lib/${WIN10_SDK_VERSION}/um/x86)
	endif()

	find_library(D3D11_LIBRARY NAMES d3d11.lib HINTS ${WIN10_SDK_LIB_PATH})
	find_library(D3D12_LIBRARY NAMES d3d12.lib HINTS ${WIN10_SDK_LIB_PATH})
	find_library(DXGI_LIBRARY NAMES dxgi.lib HINTS ${WIN10_SDK_LIB_PATH})
	find_library(D3DCOMPILER_LIBRARY NAMES d3dcompiler.lib HINTS ${WIN10_SDK_LIB_PATH})

	list(APPEND D3D11_LIBRARIES
		${D3D11_LIBRARY}
		${DXGI_LIBRARY}
		${D3DCOMPILER_LIBRARY}
	)

	list(APPEND D3D12_LIBRARIES
		${D3D12_LIBRARY}
		${DXGI_LIBRARY}
		${D3DCOMPILER_LIBRARY}
	)
endif()

if (ALIMER_D3D12)
	add_subdirectory (VulkanD3D12)
endif ()

# Engine
add_subdirectory (Alimer)

if (NOT CMAKE_CROSS_COMPILING)
    add_subdirectory (Tools)
endif ()