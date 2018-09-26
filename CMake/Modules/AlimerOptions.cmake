cmake_minimum_required(VERSION 3.1)

include(AlimerPlatforms)
include(AlimerMacros)

option (ALIMER_ENABLE_ALL "Enables all optional subsystems by default" OFF)
option (ALIMER_SHARED "Enable shared library build" OFF)

# Determine library type
if (ALIMER_SHARED)
    set (ALIMER_LIBRARY_TYPE SHARED)
else ()
    set (ALIMER_LIBRARY_TYPE STATIC)
endif ()

# Threads are still experimental on emscripten.
if (NOT EMSCRIPTEN OR ALIMER_ENABLE_ALL)
    set (ALIMER_THREADING_DEFAULT ON)
else ()
    set (ALIMER_THREADING_DEFAULT OFF)
endif ()

macro (alimer_option NAME DESCRIPTION)
    if (NOT ${NAME}_DEFAULT)
        set (${NAME}_DEFAULT ${ALIMER_ENABLE_ALL})
    endif ()
    if (NOT DEFINED ${NAME})
        option(${NAME} "${DESCRIPTION}" ${${NAME}_DEFAULT})
    endif ()
endmacro ()

# Graphics backends
if (NOT PLATFORM_UWP)
    set (ALIMER_GL_DEFAULT ON)
else ()
    set (ALIMER_GL_DEFAULT OFF)
endif ()

if (PLATFORM_WINDOWS OR PLATFORM_LINUX OR PLATFORM_ANDROID)
    set (ALIMER_VULKAN_DEFAULT ON)
else ()
    set (ALIMER_VULKAN_DEFAULT OFF)
endif ()

if (PLATFORM_WINDOWS OR PLATFORM_UWP)
    set (ALIMER_D3D11_DEFAULT ON)
else ()
    set (ALIMER_D3D11_DEFAULT OFF)
endif ()

#if (PLATFORM_WINDOWS OR PLATFORM_UWP)
#	set (ALIMER_D3D12_DEFAULT ON)
#else ()
    set (ALIMER_D3D12_DEFAULT OFF)
#endif ()

# Tools
if (PLATFORM_DESKTOP)
    set (ALIMER_TOOLS_DEFAULT ON)
else ()
    set (ALIMER_TOOLS_DEFAULT OFF)
endif()

alimer_option (ALIMER_THREADING "Enable multithreading")
alimer_option (ALIMER_GL "Enable OpenGL backend")
alimer_option (ALIMER_VULKAN "Enable Vulkan backend")
alimer_option (ALIMER_D3D11 "Enable D3D11 backend")
alimer_option (ALIMER_D3D12 "Enable D3D12 backend")
alimer_option (ALIMER_TOOLS "Enable Tools")
