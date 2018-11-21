if( _ALIMER_OPTIONS_GUARD_ )
	return()
endif()
set (_ALIMER_OPTIONS_GUARD_ 1)

include (Alimer)

# Source environment
if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
    execute_process(COMMAND cmd /c set OUTPUT_VARIABLE ENVIRONMENT)
else ()
    execute_process(COMMAND env OUTPUT_VARIABLE ENVIRONMENT)
endif ()
string(REGEX REPLACE "=[^\n]*\n?" ";" ENVIRONMENT "${ENVIRONMENT}")
set(IMPORT_ALIMER_VARIABLES_FROM_ENV BUILD_SHARED_LIBS SWIG_EXECUTABLE SWIG_DIR)
foreach(key ${ENVIRONMENT})
    list (FIND IMPORT_ALIMER_VARIABLES_FROM_ENV ${key} _index)
    if ("${key}" MATCHES "^(ALIMER_|CMAKE_|ANDROID_).+" OR ${_index} GREATER -1)
        if (NOT DEFINED ${key})
            set (${key} $ENV{${key}} CACHE STRING "" FORCE)
        endif ()
    endif ()
endforeach()

include (CMakeDependentOption)
option (ALIMER_ENABLE_ALL "Enables all optional subsystems by default" OFF)

# Enable features user has chosen
foreach(FEATURE in ${ALIMER_FEATURES})
    set (ALIMER_${FEATURE} ON CACHE BOOL "" FORCE)
endforeach()

# Threads are still experimental on emscripten.
if (NOT EMSCRIPTEN OR ALIMER_ENABLE_ALL)
    set (ALIMER_THREADING_DEFAULT ON)
else ()
    set (ALIMER_THREADING_DEFAULT OFF)
endif ()

# Graphics backends
if (ALIMER_WINDOWS OR ALIMER_UWP)
    set (ALIMER_D3D11_DEFAULT ON)
    set (ALIMER_D3D12_DEFAULT ON)
endif ()

if (ALIMER_WINDOWS OR ALIMER_LINUX OR ALIMER_ANDROID)
    set (ALIMER_VULKAN_DEFAULT ON)
endif ()

# Tools
if (ALIMER_DESKTOP)
    set (ALIMER_TOOLS_DEFAULT ON)
endif()

if (NOT CMAKE_CROSS_COMPILING AND ALIMER_DESKTOP)
    set (ALIMER_CSHARP_DEFAULT ON)
    set (ALIMER_SHARED_DEFAULT ON)
endif ()

macro (alimer_option NAME DESCRIPTION)
    if (NOT ${NAME}_DEFAULT)
        set (${NAME}_DEFAULT ${ALIMER_ENABLE_ALL})
    endif ()
    if (NOT DEFINED ${NAME})
        option(${NAME} "${DESCRIPTION}" ${${NAME}_DEFAULT})
    endif ()
endmacro ()

alimer_option (ALIMER_POSITION_INDEPENDENT "Position independent" ON)
alimer_option (ALIMER_SANITIZE "Sanitize address, threads and other compiler options" OFF)
alimer_option (ALIMER_SKIP_INSTALL "Skip installation" ${ALIMER_SKIP_INSTALL})
alimer_option (ALIMER_SHARED "Enable shared library build" OFF)
alimer_option (ALIMER_USE_DEBUG_INFO   "Enable debug information in all configurations." ON)
alimer_option (ALIMER_LOGGING "Enable logging macros" TRUE)
alimer_option (ALIMER_PROFILING "Enable performance profiling" TRUE)
alimer_option (ALIMER_THREADING "Enable multithreading")
alimer_option (ALIMER_D3D11 "Enable Direct3D11 backend")
alimer_option (ALIMER_D3D12 "Enable Direct3D12 backend")
alimer_option (ALIMER_VULKAN "Enable Vulkan backend")
alimer_option (ALIMER_OPENGL "Enable OpenGL backend")
alimer_option (ALIMER_TOOLS "Enable Tools")
alimer_option (ALIMER_CSHARP "Enable C# support")
alimer_option (ALIMER_CSHARP_MONO "Use mono for C# support")
