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

cmake_minimum_required(VERSION 3.5)

if (NOT WIN32 OR ALIMER_CSHARP_MONO)
    find_package(Mono REQUIRED)

    find_program(CSC csc PATHS /Library/Frameworks/Mono.framework/Versions/Current/bin ${MONO_PATH}/bin)
    if (NOT CSC)
        message(FATAL_ERROR "csc could not be found.")
    endif ()

    find_program(MSBUILD msbuild PATHS /Library/Frameworks/Mono.framework/Versions/Current/bin ${MONO_PATH}/bin)
    if (NOT MSBUILD)
        message(FATAL_ERROR "MSBuild could not be found.")
    endif ()

    macro(create_mono_compiler_settings)
	    set(MONO_LANGUAGE_VERSION 7)
        if("${BUILD_PLATFORM}" STREQUAL "Win32")
            set(MONO_CPU_PLATFORM x86)
            set(MONO_PREPROCESSOR_DEFINE WIN32)
        elseif("${BUILD_PLATFORM}" STREQUAL "Win64")
            set(MONO_CPU_PLATFORM anycpu)
            set(MONO_PREPROCESSOR_DEFINE WIN64)
        endif()
	    set(MONO_PREPROCESSOR_DEFINE ${MONO_PREPROCESSOR_DEFINE}$<SEMICOLON>$<UPPER_CASE:$<CONFIG>>)
    endmacro()

    option (ALIMER_CSHARP_WITH_CSC "Build C# with Roslyn Compiler" ON)

    if(OPTION_BUILD_CSHARP_WITH_CSC)
	    create_mono_compiler_settings()
	    message(STATUS "C# Mono platform ${MONO_CPU_PLATFORM}")
	    message(STATUS "C# Mono preprocessor define ${MONO_PREPROCESSOR_DEFINE}")
	    message(STATUS "C# Language Version : ${MONO_LANGUAGE_VERSION}")
    endif()
endif ()

if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    # Workaround for some cases where csc has issues when invoked by CMake.
    set (TERM_WORKAROUND env TERM=xterm)
endif ()

# Add commont macro
macro (add_target_csharp TARGET PROJECT_FILE LIBRARY SOLUTION_FOLDER)
    if (NOT ALIMER_CSHARP)
        return ()
    endif ()

    if (WIN32 AND NOT ALIMER_CSHARP_MONO)
        include_external_msproject(${TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_FILE})
    else()
        set (MSBUILD_COMMON_PARAMETERS /p:BuildDir="${CMAKE_BINARY_DIR}/" /p:Platform=${CSHARP_PLATFORM} /p:Configuration=$<CONFIG> /consoleloggerparameters:ErrorsOnly)

        if (ALIMER_CSHARP_MONO)
            list (APPEND MSBUILD_COMMON_PARAMETERS /p:TargetFramework=net45)
        endif ()

        add_custom_target(${TARGET}
            COMMAND ${TERM_WORKAROUND} ${MSBUILD} ${PROJECT_FILE} ${MSBUILD_COMMON_PARAMETERS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${ARGN}
        )
        set_target_properties(${TARGET} PROPERTIES EXCLUDE_FROM_ALL OFF)
    endif()

    set_target_properties(${TARGET} PROPERTIES FOLDER "${SOLUTION_FOLDER}")

    if (NOT ALIMER_SKIP_INSTALL)
        if (MSVC)
            set (NET_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
        else ()
            set (NET_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
        endif ()

        if (${LIBRARY})
            install (FILES ${NET_OUTPUT_DIRECTORY}/${TARGET}.dll DESTINATION bin)
        else()
            install (FILES ${NET_OUTPUT_DIRECTORY}/${TARGET}.exe DESTINATION bin)
        endif ()
    endif ()

endmacro()

# Alimer C# bindings
add_subdirectory(AlimerSharp)

# Editor
add_subdirectory(Alimer.Studio)
