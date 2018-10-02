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
cmake_minimum_required (VERSION 3.1)
project(shaderc)
set(TARGET libshaderc)

file (GLOB_RECURSE HEADER_FILES src/*.h)
file (GLOB_RECURSE SOURCE_FILES src/*.cpp)

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

add_library(${TARGET} STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${TARGET} PUBLIC src)
target_link_libraries(${TARGET} glslang SPIRV)

# this must match options enabled in glslang's CMakeLists.txt
target_compile_options(${TARGET} PRIVATE -DAMD_EXTENSIONS -DNV_EXTENSIONS )

# Set folder
set_target_properties(${TARGET} PROPERTIES FOLDER "Tools")

# ==================================================================================================
# Binary
# ==================================================================================================
set(TARGET shaderc)

set(SOURCE_FILES main.cpp)

# Define the target.
add_executable(${TARGET} ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(${TARGET} libshaderc CLI11)

set_target_properties(${TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")
set_target_properties(${TARGET} PROPERTIES FOLDER "Tools")

install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
