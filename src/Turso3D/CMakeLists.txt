#
# Alimer is based on the Turso3D codebase.
# Copyright (c) 2018-2019 Amer Koleci and contributors.
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
project (Turso3D)

option (TURSO3D_SHARED "Build Turso3D as a shared library" FALSE)
option (TURSO3D_LOGGING "Enable logging macros" TRUE)
option (TURSO3D_PROFILING "Enable performance profiling" TRUE)

# Macro to add files from a directory grouped to the solution file
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
            install (FILES ${_files} DESTINATION include/alimer/${path})
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

# Set definitions
if (TURSO3D_SHARED)
    add_definitions (-DTURSO3D_EXPORTS)
    set (TURSO3D_LIB_TYPE SHARED)
else ()
    set (TURSO3D_LIB_TYPE STATIC)
endif ()

if (WIN32 AND NOT TURSO3D_OPENGL)
    set (TURSO3D_D3D11 TRUE)
endif ()

# Generate the Turso3DConfig.h file
configure_file (Turso3DConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/Turso3DConfig.h)

define_engine_source_files (Base Debug Thread IO Object Math Resource)
define_engine_source_files (NORECURSE . Graphics Application)
define_engine_source_files (Scene Renderer)

if (TURSO3D_D3D11)
    define_engine_source_files (Graphics/D3D11)
endif ()
if (TURSO3D_OPENGL)
    define_engine_source_files (Graphics/GL)
    #add_engine_directory (ThirdParty/FlextGL)
endif ()

if (WIN32)
    define_engine_source_files (Application/Win32)
endif ()

# Group source code in VS solution
group_sources()

# Define the Turso3D library target
add_library (${PROJECT_NAME} ${TURSO3D_LIB_TYPE} ${HEADER_FILES} ${SOURCE_FILES} $<$<CXX_COMPILER_ID:MSVC>:${ALIMER_ROOT_DIR}/script/visualizers/alimer.natvis>)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
endif ()

target_link_libraries(${PROJECT_NAME} stb)

# Add rendering API specific includes and library dependencies
if (TURSO3D_D3D11)
    target_link_libraries (${PROJECT_NAME} d3d11 dxgi dxguid)
endif ()

if (TURSO3D_OPENGL)
    #include_directories (ThirdParty/FlextGL)
    #find_package (OpenGL REQUIRED)
    #target_link_libraries (${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
endif ()

# Add OS-specific library dependencies
if (WIN32)
    target_link_libraries (${PROJECT_NAME} winmm)
else ()
    target_link_libraries (${PROJECT_NAME} pthread)
endif ()
