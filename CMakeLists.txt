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

# Set CMake minimum version
cmake_minimum_required (VERSION 3.5)
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)
set (CMAKE_C_STANDARD 99)
project (Alimer LANGUAGES CXX C)

# Alimer Version
set (PRODUCT_NAME "Alimer")
set (PRODUCT_COMPANY "Amer Koleci")
set (PRODUCT_COPYRIGHT "Copyright (C) 22018 ${PRODUCT_COMPANY}")
set (PROJECT_VERSION_MAJOR 0)
set (PROJECT_VERSION_MINOR 1)
set (PROJECT_VERSION_PATCH 0)
set (PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# Get the current working branch
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE ALIMER_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest abbreviated commit hash of the working branch
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE ALIMER_GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Output paths
set (ALIMER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (ALIMER_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
mark_as_advanced(ALIMER_ROOT_DIR)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ALIMER_BUILD_DIR}/bin")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${ALIMER_BUILD_DIR}/lib")
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${ALIMER_BUILD_DIR}/lib")

# Enable solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set CMake modules search path
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# Include Alimer module.
include (Alimer)

# Set platform variables
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set (LINUX ON CACHE BOOL "" FORCE)
endif ()

# Options
option (ALIMER_BUILD_SHARED "Build as a shared library." OFF)
option (ALIMER_INSTALL "Generate installation target" ON)
option (ALIMER_LOGGING "Enable logging macros" ON)
option (ALIMER_PROFILING "Enable performance profiling" ON)
option (ALIMER_PLUGINS "Enable plugins support" ON)

if (EMSCRIPTEN)

else ()
    option (ALIMER_THREADING "Enable threading support" TRUE)
    option (ALIMER_NETWORK "Enable network support" TRUE)
endif ()

# Graphics Backend
if (WIN32)
    #set (ALIMER_RENDERER D3D11 CACHE STRING "Select renderer: D3D11 | OpenGL")
    set (ALIMER_RENDERER OpenGL CACHE STRING "Select renderer: D3D11 | OpenGL")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "WindowsStore") # UWP
    set (ALIMER_RENDERER D3D11 CACHE STRING "Select renderer: D3D11")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Durango") #  XboxOne
    set (ALIMER_RENDERER D3D11 CACHE STRING "Select renderer: D3D11")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    # Use Metal on IOS and macOS platforms
    set (ALIMER_RENDERER Metal CACHE STRING "Use Metal renderer" FORCE)
else ()
    set (ALIMER_RENDERER OpenGL CACHE STRING "Use OpenGL renderer" FORCE)
endif ()
string(TOUPPER "${ALIMER_RENDERER}" ALIMER_RENDERER)
set (ALIMER_${ALIMER_RENDERER} ON)

if (EMSCRIPTEN OR ANDROID OR IOS)
    set (ALIMER_TOOLS OFF CACHE INTERNAL "Build tools and editors" FORCE)
else()
    option (ALIMER_TOOLS "Build tools and editors" ${ALIMER_DESKTOP})
endif ()

if (ANDROID OR ALIMER_BUILD_SHARED)
    set (CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Set alimer sdk path.
set (ALIMER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (ALIMER_SDK_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Source/Alimer)
set (ALIMER_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set (ALIMER_ASSETS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/assets)

# Setup
if (EMSCRIPTEN)
    string(CONCAT ALIMER_EMSCRIPTEN_FLAGS
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
        # "--shell-file \"${CMAKE_CURRENT_SOURCE_DIR}/src/resources/alimer.html\""
    )
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ALIMER_EMSCRIPTEN_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ALIMER_EMSCRIPTEN_FLAGS}")
    set (CMAKE_EXECUTABLE_SUFFIX ".html")
    set (ALIMER_THREADING OFF CACHE INTERNAL "Disable threading on WEB" FORCE)
    set (ALIMER_NETWORK OFF CACHE INTERNAL "Disable network on WEB" FORCE)
elseif(ANDROID)
    set (ALIMER_BUILD_SHARED CACHE INTERNAL "Build as shared library on Android" FORCE)
elseif(UNIX)
    if(APPLE)
        set(CMAKE_MACOSX_RPATH 1)
    endif()
    find_package(PkgConfig)
endif ()

# third_party
add_subdirectory(third_party)

# Source
add_subdirectory(src)

if (ALIMER_INSTALL)
    # README.md and license
    install (FILES
        ${ALIMER_ROOT_DIR}/README.md
        ${ALIMER_ROOT_DIR}/LICENSE
        DESTINATION .
    )

    # Install CMake modules and toolchains provided by and for Alimer
    install (DIRECTORY ${CMAKE_SOURCE_DIR}/CMake/ DESTINATION share/cmake)    # Note: the trailing slash is significant

    # Install data files (assets)
    file (GLOB ASSET_DIRS ${ALIMER_ROOT_DIR}/assets ${CMAKE_SOURCE_DIR}/assets)
    file (MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    if (NOT ANDROID)
        foreach (ASSET_DIR ${ASSET_DIRS})
            # get_filename_component (NAME ${ASSET_DIR} NAME)
            install (DIRECTORY ${ASSET_DIR} DESTINATION bin)
        endforeach ()
    endif ()
endif()

# Set VS Startup project.
if(CMAKE_VERSION VERSION_GREATER "3.6")
    if (NOT ALIMER_BUILD_SHARED)
        set_property (DIRECTORY PROPERTY VS_STARTUP_PROJECT "alimer")
    endif ()
endif()

# Print current build configuration
message (STATUS "Build Configuration:")
message (STATUS "  Host            ${ALIMER_HOST_NAME}")
message (STATUS "  C++ Compiler    ${ALIMER_COMPILER_NAME}")
message (STATUS "  Platform        ${ALIMER_PLATFORM_NAME}")
message (STATUS "  Architecture    ${ALIMER_ARCH_NAME}")

if (ALIMER_BUILD_SHARED)
    message(STATUS "  Library         SHARED")
else()
    message(STATUS "  Library         STATIC")
endif()

message(STATUS "  Threading       ${ALIMER_THREADING}")
message(STATUS "  Renderer        ${ALIMER_RENDERER}")
message(STATUS "  Tools           ${ALIMER_TOOLS}")
message(STATUS "  Plugins         ${ALIMER_PLUGINS}")

# Packaging
set (ALIMER_DESCRIPTION "Alimer is a free lightweight, cross-platform 2D and 3D game engine implemented in C++14 and released under the MIT license. Greatly inspired by Urho3D (https://github.com/urho3d/Urho3D).")
set (CPACK_PACKAGE_NAME "alimer")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY ${ALIMER_DESCRIPTION})
set (CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set (CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set (CPACK_PACKAGE_DESCRIPTION_FILE "${ALIMER_ROOT_DIR}/README.md")
set (CPACK_RESOURCE_FILE_WELCOME "${ALIMER_ROOT_DIR}/README.md")
set (CPACK_RESOURCE_FILE_LICENSE "${ALIMER_ROOT_DIR}/LICENSE")
set (CPACK_PACKAGE_FILE_NAME "alimer-${PROJECT_VERSION}$ENV{ALIMER_PACKAGE_SUFFIX}")
set (CPACK_GENERATOR "ZIP;TGZ") # Remove this, if you want the NSIS installer on Windows
include(CPack)
