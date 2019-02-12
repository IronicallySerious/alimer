cmake_minimum_required(VERSION 3.0)
project(vk_mem_alloc)

add_library(vk_mem_alloc STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/vk_mem_alloc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vk_mem_alloc.cpp
)

target_include_directories(vk_mem_alloc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${ALIMER_THIRD_PARTY_DIR}/khronos)
