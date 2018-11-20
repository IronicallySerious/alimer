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

option (OPTION_BUILD_CSHARP_WITH_CSC "Build C# with Roslyn Compiler" ON)

# Add commont macro
macro (add_target_csharp TARGET FOLDER SOLUTION_FOLDER)
    if (NOT ALIMER_CSHARP)
        return ()
    endif ()

    if (WIN32 AND NOT ALIMER_CSHARP_MONO)
        include_external_msproject(${TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/${FOLDER}/${TARGET}.csproj)
    else()

    endif()

    set_property(TARGET ${TARGET} PROPERTY FOLDER "${SOLUTION_FOLDER}")

    if (NOT ALIMER_SKIP_INSTALL)
        if (MSVC)
            set (NET_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
        else ()
            set (NET_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
        endif ()

        install (FILES
            ${NET_OUTPUT_DIRECTORY}/${TARGET}.dll
            DESTINATION bin
        )
    endif ()

endmacro()

# Alimer C# bindings
add_target_csharp(AlimerSharp "AlimerSharp" "CSharp")

# Editor
add_target_csharp(Alimer.Studio "Alimer.Studio" "CSharp")
