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

set(SPIRV_CROSS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/GLSL.std.450.h
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_common.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cfg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cfg.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_parser.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross_parsed_ir.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/spirv_cross_parsed_ir.cpp
)

set(NEED_SPIRV_CROSS_GLSL OFF)
if (ALIMER_WINDOWS OR ALIMER_UWP)
	list(APPEND SPIRV_CROSS_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/spirv_hlsl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/spirv_hlsl.hpp
	)

	set(NEED_SPIRV_CROSS_GLSL ON)
endif()

if (ALIMER_METAL)
    list(APPEND SPIRV_CROSS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/spirv_msl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/spirv_msl.hpp
    )
    set(NEED_SPIRV_CROSS_GLSL ON)
endif()

if (ALIMER_VULKAN OR ALIMER_OPENGL OR NEED_SPIRV_CROSS_GLSL)
	list(APPEND SPIRV_CROSS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/spirv_glsl.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/spirv_glsl.hpp
	)
endif()

add_library (spirv_cross STATIC ${SPIRV_CROSS_SOURCES})
target_compile_definitions(spirv_cross PUBLIC SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
set(SPIRV_CROSS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
alimer_external_target("third_party" spirv_cross)
