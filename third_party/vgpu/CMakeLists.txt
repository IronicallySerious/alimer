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
project (vgpu)

set(VGPU_HEADER_FILES ${CMAKE_CURRENT_LIST_DIR}/vgpu.h)
set(VGPU_SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/vgpu.cpp)

if (ALIMER_D3D12)
    list (APPEND VGPU_SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/vgpu_d3d12.cpp)
elseif (ALIMER_D3D11)
    list (APPEND VGPU_SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/vgpu_d3d11.cpp)
elseif (ALIMER_VULKAN)
    list (APPEND VGPU_SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/vgpu_vk.cpp)
    # volk
    list (APPEND VGPU_SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/libs/volk/volk.c)
endif ()

add_library (vgpu STATIC ${VGPU_HEADER_FILES} ${VGPU_SOURCE_FILES})
target_include_directories(vgpu PRIVATE ${CMAKE_CURRENT_LIST_DIR}/libs)

if (ALIMER_D3D12)
elseif (ALIMER_D3D11)
elseif (ALIMER_VULKAN)
    if(NOT MSVC)
        target_link_libraries(vgpu PRIVATE dl)
    endif ()
endif ()