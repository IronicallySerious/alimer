cmake_minimum_required(VERSION 3.5)

# Source environment
if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
    execute_process(COMMAND cmd /c set OUTPUT_VARIABLE ENVIRONMENT)
else ()
    execute_process(COMMAND env OUTPUT_VARIABLE ENVIRONMENT)
endif ()

# Detect host.
if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    set (ALIMER_HOST_WINDOWS 1)
    set (ALIMER_HOST_NAME "Windows")
elseif (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Darwin")
    set (ALIMER_HOST_OSX 1)
    set (ALIMER_HOST_NAME "OSX")
elseif (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    set (ALIMER_HOST_LINUX 1)
    set (ALIMER_HOST_NAME "Linux")
else()
    message(WARNING "Host system not recognized, setting to 'Linux'")
    set (ALIMER_HOST_LINUX 1)
    set (ALIMER_HOST_NAME "Linux")
endif()

 # Detect compiler
 if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
     set (ALIMER_CLANG 1)
     set (ALIMER_COMPILER_NAME "Clang (ALIMER_CLANG)")
     set (CLANG ON)
     set (GNU ON)
 elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
     set (ALIMER_GCC 1)
     set (ALIMER_COMPILER_NAME "GCC (ALIMER_GCC)")
     set (GCC ON)
     set (GNU ON)
 elseif (MSVC)
     set (ALIMER_MSVC 1)
     set (ALIMER_COMPILER_NAME "VisualStudio (ALIMER_MSVC)")
 else()
    message(FATAL_ERROR "Cannot detect compiler")
 endif()

# Set platform variables
if ( "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" )
	set (ALIMER_WINDOWS ON)
	set (ALIMER_PLATFORM_NAME "Windows")
elseif ( "${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore" )
	set (ALIMER_UWP ON)
	set (ALIMER_PLATFORM_NAME "UWP")
    set (ALIMER_UWP_VERSION_MIN 10.0.16299.0)
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Durango" )
	set (ALIMER_XBOX_ONE 1)
    set (VALIMER_PLATFORM_NAME "XboxOne")
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Orbis" )
	set (ALIMER_PS4 1)
	set (ALIMER_PLATFORM_NAME "PS4")
elseif( "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin" )
    if( IOS )
        set (ALIMER_IOS ON)
		set (ALIMER_PLATFORM_NAME "iOS")
	else()
		set (ALIMER_OSX ON)
		set (ALIMER_PLATFORM_NAME "macOS")
	endif()
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	set (ALIMER_LINUX ON)
	set (ALIMER_PLATFORM_NAME "Linux")
elseif( ANDROID )
    set (ALIMER_ANDROID ON)
    set (ALIMER_PLATFORM_NAME "Android")
elseif( EMSCRIPTEN )
    set (ALIMER_EMSCRIPTEN ON)
	set (ALIMER_PLATFORM_NAME "Web")
else()
	message(FATAL_ERROR "Unknown platform ${CMAKE_SYSTEM_NAME}!")
endif ()

if (ALIMER_WINDOWS OR ALIMER_LINUX OR ALIMER_OSX)
	set (ALIMER_DESKTOP ON)
endif ()

# Detect target architecture
if( ((ALIMER_WINDOWS OR ALIMER_UWP OR ALIMER_OSX) AND CMAKE_CL_64) OR (ALIMER_IOS AND CMAKE_OSX_ARCHITECTURES MATCHES "arm64") OR ALIMER_XBOX_ONE OR ALIMER_PS4 OR ALIMER_LINUX )
	set (ALIMER_64BIT 1)
endif()

if( ALIMER_WINDOWS OR ALIMER_OSX OR ALIMER_LINUX OR ALIMER_XBOX_ONE OR ALIMER_PS4 OR ALIMER_EMSCRIPTEN OR ALIMER_UWP )
	if (ALIMER_64BIT)
		set (ALIMER_ARCH_NAME "x64")
	else()
		set (ALIMER_ARCH_NAME "x86")
	endif()
elseif( ALIMER_IOS OR ALIMER_ANDROID )
	if (ALIMER_64BIT)
		set (ALIMER_ARCH_NAME "arm64")
	else()
		set (ALIMER_ARCH_NAME "arm")
	endif()
else()
	message(FATAL_ERROR "Unknown platform architecture!")
endif()

# Define CMake options
include (CMakeDependentOption)
macro (alimer_option NAME DESCRIPTION)
    if (NOT ${NAME}_DEFAULT)
        set (${NAME}_DEFAULT ${ALIMER_ENABLE_ALL})
    endif ()
    if (NOT DEFINED ${NAME})
        option(${NAME} "${DESCRIPTION}" ${${NAME}_DEFAULT})
    endif ()
endmacro ()

option (ALIMER_POSITION_INDEPENDENT "Position independent" ON)
option (ALIMER_SANITIZE "Sanitize address, threads and other compiler options" OFF)
option (ALIMER_SKIP_INSTALL "Skip installation" ${ALIMER_SKIP_INSTALL})
option (ALIMER_SHARED "Enable shared library build" OFF)
option (ALIMER_USE_DEBUG_INFO   "Enable debug information in all configurations." ON)

# Threads are still experimental on emscripten.
if (NOT ALIMER_EMSCRIPTEN OR ALIMER_ENABLE_ALL)
    set (ALIMER_THREADING_DEFAULT ON)
else ()
    set (ALIMER_THREADING_DEFAULT OFF)
endif ()

# Graphics backends
if (WIN32)
    set (ALIMER_RENDERER_DEFAULT Vulkan)
elseif (ALIMER_UWP)
    set (ALIMER_RENDERER_DEFAULT D3D11)
else ()
    set (ALIMER_RENDERER_DEFAULT OpenGL)
endif ()

set (ALIMER_RENDERER ${ALIMER_RENDERER_DEFAULT} CACHE STRING "Select renderer: Vulkan | D3D12 | D3D11 | OpenGL")
set (ALIMER_RENDERER_NAME ${ALIMER_RENDERER})
string(TOUPPER "${ALIMER_RENDERER}" ALIMER_RENDERER)
set (ALIMER_${ALIMER_RENDERER} ON)

# Tools
if (ALIMER_DESKTOP)
    set (ALIMER_TOOLS_DEFAULT ON)
else ()
    set (ALIMER_TOOLS_DEFAULT OFF)
endif()

option (ALIMER_ENABLE_ALL "Enables all optional subsystems by default" OFF)

alimer_option (ALIMER_CSHARP "Enable C# support")
alimer_option (ALIMER_THREADING "Enable multithreading")
alimer_option (ALIMER_TOOLS "Enable Tools")


# Setup global per-platform compiler/linker options
if( MSVC )
    # Enable full optimization in dev/release
    add_compile_options($<$<CONFIG:DEBUG>:/Od> $<$<NOT:$<CONFIG:DEBUG>>:/Ox>)

    # Inline function expansion
    add_compile_options(/Ob2)

    # Enable intrinsic functions in dev/release
    add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:/Oi>)

    # Favor fast code
	add_compile_options(/Ot)

	# Enable fiber-safe optimizations in dev/release
	add_compile_options($<$<NOT:$<CONFIG:DEBUG>>:/GT>)

	# Enable string pooling
    add_compile_options(/GF)

    # Select static/dynamic runtime library
	if( ALIMER_WINDOWS )
        add_compile_options($<$<CONFIG:DEBUG>:/MTd> $<$<NOT:$<CONFIG:DEBUG>>:/MT>)
    elseif ( ALIMER_UWP )
        add_compile_options($<$<CONFIG:DEBUG>:/MDd> $<$<NOT:$<CONFIG:DEBUG>>:/MD>)
    endif()

    # Disable runtime checks.
	STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
	STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

    # Disable specific link libraries
	if ( ALIMER_WINDOWS )
        # ucm_add_linker_flags(EXE STATIC SHARED /NODEFAULTLIB:"MSVCRT.lib")
    endif()

	# Use security checks only in debug
	if ( ALIMER_UWP )
		add_compile_options($<$<CONFIG:DEBUG>:/sdl> $<$<NOT:$<CONFIG:DEBUG>>:/sdl->)
	else()
		add_compile_options($<$<CONFIG:DEBUG>:/GS> $<$<NOT:$<CONFIG:DEBUG>>:/GS->)
	endif()

	# Enable function-level linking
	add_compile_options(/Gy)

	# Enable SIMD
	if( ALIMER_WINDOWS )
		if( ALIMER_64BIT )
			if( ALIMER_USE_AVX )
				add_compile_options(/arch:AVX -DAVX)
			endif()
		else()
			add_compile_options(/arch:SSE2)
		endif()
	endif()

    # Use fast floating point model
    add_compile_options(/fp:fast)

    # Enable multi-processor compilation
	add_compile_options(/MP)

	# Set warning level 3
	add_compile_options(/W3)
    # ucm_add_flags(/W4 CONFIG Debug)

	# Disable specific warnings
	add_compile_options(/wd4127 /wd4351 /wd4005)

	# Disable specific warnings for MSVC14 and above
	if( (ALIMER_WINDOWS OR ALIMER_UWP) AND (NOT MSVC_VERSION LESS 1900) )
		add_compile_options(/wd4838 /wd4312 /wd4477 /wd4244 /wd4091 /wd4311 /wd4302 /wd4476 /wd4474)
        add_compile_options(/wd4309)	# truncation of constant value
	endif()

	# Force specific warnings as errors
	add_compile_options(/we4101)

	# Treat all other warnings as errors
    # add_compile_options(/WX)

	if ( ALIMER_UWP )
	    # Consume Windows Runtime
		add_compile_options(/ZW)
		# C++ exceptions
		add_compile_options(/EHsc)
    endif()

elseif (ALIMER_EMSCRIPTEN)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-warn-absolute-paths -Wno-unknown-warning-option")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-warn-absolute-paths -Wno-unknown-warning-option")
    if (ALIMER_THREADING)
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_PTHREADS=1")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_PTHREADS=1")
    endif ()
    set (CMAKE_C_FLAGS_RELEASE "-Oz -DNDEBUG")
    set (CMAKE_CXX_FLAGS_RELEASE "-Oz -DNDEBUG")
else ()
    # Use fast floating point model
    add_compile_options(-ffast-math)
endif ()

# Setup SDK install destinations
set (DEST_BASE_INCLUDE_DIR include)
set (DEST_INCLUDE_DIR ${DEST_BASE_INCLUDE_DIR}/Alimer)
set (DEST_SHARE_DIR share)
set (DEST_ASSETS_DIR ${DEST_BIN_DIR})

# Functions and macros
function(alimer_setup_common_properties target)
    if (MSVC)
        target_compile_definitions(${target} PRIVATE _SCL_SECURE_NO_WARNINGS _CRT_SECURE_NO_WARNINGS NOMINMAX)
        target_compile_definitions(${target} PRIVATE _UNICODE)
    elseif (CMAKE_COMPILER_IS_GNUCXX OR (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang"))
    endif()

    # Define 32 versus 64 bit architecture
    if( ALIMER_64BIT )
        target_compile_definitions(${target} PRIVATE ALIMER_64BIT)
    else()
        target_compile_definitions(${target} PRIVATE ALIMER_32BIT)
    endif()

    if (ALIMER_UWP)
        set_target_properties(${target} PROPERTIES VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION "${ALIMER_UWP_VERSION_MIN}")
    endif()

endfunction()

function(add_alimer_library target)
    if (ALIMER_SHARED)
        add_library(${target} SHARED ${ARGN})
    else()
        add_library(${target} STATIC ${ARGN})
    endif()

    alimer_setup_common_properties(${target})

    if (ALIMER_POSITION_INDEPENDENT)
        set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    else()
        set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
    endif()

endfunction()

function (add_alimer_executable target)
    if (NOT ALIMER_WIN32_CONSOLE)
        set (TARGET_TYPE WIN32)
    endif ()

    if (ALIMER_ANDROID)
        add_library(${target} SHARED ${ARGN})
    else ()
        add_executable (${target} ${TARGET_TYPE} ${ARGN})
    endif ()

    alimer_setup_common_properties(${target})

    # Link to alimer library
    target_link_libraries (${target} libAlimer)
endfunction ()

function (add_alimer_plugin target)
    if (NOT ALIMER_STATIC_PLUGIN)
        add_library(${target} SHARED ${ARGN})
    else ()
    	add_library(${target} STATIC ${ARGN})
    endif ()

    alimer_setup_common_properties(${target})

    if (ALIMER_POSITION_INDEPENDENT)
        set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    else()
        set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
    endif()

     # Link to alimer library
     target_link_libraries (${target} libAlimer)
endfunction ()
