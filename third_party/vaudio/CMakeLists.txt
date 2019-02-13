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

cmake_minimum_required(VERSION 3.1)
project (vaudio)

set(VAUDIO_HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/vaudio/vaudio.h
    )

set(VAUDIO_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/vaudio.cpp
    )

if (WIN32)
    list (APPEND VAUDIO_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/vaudio_wasapi.cpp)
endif()

if (VAUDIO_BUILD_SHARED)
    add_library(vaudio SHARED ${VAUDIO_HEADER_FILES} ${VAUDIO_SOURCE_FILES})
    target_compile_definitions(vaudio PRIVATE VAUDIO_BUILD_SHARED_LIBRARY)
else()
    add_library (vaudio STATIC ${VAUDIO_HEADER_FILES} ${VAUDIO_SOURCE_FILES})
endif ()

target_include_directories(vaudio PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_FULL_INCLUDEDIR}>"
)

if (WIN32)
    #target_link_libraries(vaudio PRIVATE volk vk_mem_alloc)
endif ()
