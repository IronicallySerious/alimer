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

function(add_plugin sample_name)
    if (ALIMER_BUILD_SHARED)
        if (WIN32)
            add_executable(${sample_name} WIN32 ${CMAKE_CURRENT_SOURCE_DIR}/${sample_name}/main.cpp)
        else ()
            add_executable(${sample_name} ${CMAKE_CURRENT_SOURCE_DIR}/${sample_name}/main.cpp)
        endif ()
    else ()
        add_library(${sample_name} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/${sample_name}/main.cpp)
    endif ()

    if (MSVC)
        set_target_properties(${sample_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
        set_target_properties(${sample_name} PROPERTIES FOLDER "plugins")
    endif ()
endfunction()

add_plugin(01_hello_world)
