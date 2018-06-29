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

file (GLOB_RECURSE HEADER_FILES *.h *.hpp)
file (GLOB_RECURSE SOURCE_FILES *.c *.cpp)

set(GLSLANG_HEADER_FILES
	${ALIMER_THIRD_PARTY_DIR}/glslang/StandAlone/DirStackFileIncluder.h
	${ALIMER_THIRD_PARTY_DIR}/glslang/StandAlone/ResourceLimits.h
	${ALIMER_THIRD_PARTY_DIR}/glslang/StandAlone/Worklist.h
)

set(GLSLANG_SOURCE_FILES
	${ALIMER_THIRD_PARTY_DIR}/glslang/StandAlone/ResourceLimits.cpp
)

list (APPEND HEADER_FILES ${GLSLANG_HEADER_FILES})
list (APPEND SOURCE_FILES ${GLSLANG_SOURCE_FILES})

# Define the target.
add_executable(shaderc ${SOURCE_FILES} ${HEADER_FILES})

set_target_properties(shaderc PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(shaderc PRIVATE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	$<BUILD_INTERFACE:${ALIMER_THIRD_PARTY_DIR}>
)

target_link_libraries(shaderc glslang SPIRV)

set_target_properties(shaderc PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(shaderc PROPERTIES FOLDER "Tools")

install(TARGETS shaderc RUNTIME DESTINATION ${DEST_TOOLS_DIR})
