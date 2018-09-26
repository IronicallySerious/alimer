cmake_minimum_required(VERSION 3.5)

# Source environment
if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
    execute_process(COMMAND cmd /c set OUTPUT_VARIABLE ENVIRONMENT)
else ()
    execute_process(COMMAND env OUTPUT_VARIABLE ENVIRONMENT)
endif ()

# Configure CMake global variables
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

# Define CMake options
include (AlimerOptions)
include (CMakeDependentOption)
option (ALIMER_CSHARP "Enable C# support" OFF)
option (ALIMER_USE_DEBUG_INFO   "Enable debug information in all configurations." ON)

# Setup global per-platform compiler/linker options
if( PLATFORM_WINDOWS OR PLATFORM_UWP )
    add_definitions(-D_UNICODE)
	add_compile_options(-D_CRT_NONSTDC_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE)
    add_compile_options($<$<CONFIG:DEBUG>:-D_SECURE_SCL_THROWS=0> $<$<CONFIG:DEBUG>:-D_SILENCE_DEPRECATION_OF_SECURE_SCL_THROWS>)
    add_compile_options(-D_HAS_ITERATOR_DEBUGGING=$<CONFIG:DEBUG> -D_SECURE_SCL=$<CONFIG:DEBUG>)
    #add_compile_options(-D_HAS_EXCEPTIONS=0)
    add_compile_options(-D_USE_MATH_DEFINES=1)

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
	if( PLATFORM_WINDOWS )
        add_compile_options($<$<CONFIG:DEBUG>:/MTd> $<$<NOT:$<CONFIG:DEBUG>>:/MT>)
    elseif ( PLATFORM_UWP )
        add_compile_options($<$<CONFIG:DEBUG>:/MDd> $<$<NOT:$<CONFIG:DEBUG>>:/MD>)
    endif()

    # Disable runtime checks.
	STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
	STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

    # Disable specific link libraries
	if ( PLATFORM_WINDOWS )
        # ucm_add_linker_flags(EXE STATIC SHARED /NODEFAULTLIB:"MSVCRT.lib")
    endif()

	# Use security checks only in debug
	if ( PLATFORM_UWP )
		add_compile_options($<$<CONFIG:DEBUG>:/sdl> $<$<NOT:$<CONFIG:DEBUG>>:/sdl->)
	else()
		add_compile_options($<$<CONFIG:DEBUG>:/GS> $<$<NOT:$<CONFIG:DEBUG>>:/GS->)
	endif()

	# Enable function-level linking
	add_compile_options(/Gy)

	# Enable SIMD
	if( PLATFORM_WINDOWS )
		if( PLATFORM_64BIT )
			if( PLATFORM_USE_AVX )
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
	if( (PLATFORM_WINDOWS OR PLATFORM_UWP) AND (NOT MSVC_VERSION LESS 1900) )
		add_compile_options(/wd4838 /wd4312 /wd4477 /wd4244 /wd4091 /wd4311 /wd4302 /wd4476 /wd4474)
        add_compile_options(/wd4309)	# truncation of constant value
	endif()

	# Force specific warnings as errors
	add_compile_options(/we4101)

	# Treat all other warnings as errors
    # add_compile_options(/WX)

	if ( PLATFORM_UWP )
	    # Consume Windows Runtime
		add_compile_options(/ZW)
		# C++ exceptions
		add_compile_options(/EHsc)
    endif()

elseif (PLATFORM_WEB)
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

# Initialize the Dev configuration from release configuration.
set(CMAKE_C_FLAGS_DEV "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEV "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_STATIC_LINKER_FLAGS_DEV "${CMAKE_STATIC_LINKER_FLAGS_RELEASE}")
set(CMAKE_SHARED_LINKER_FLAGS_DEV "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")
set(CMAKE_MODULE_LINKER_FLAGS_DEV "${CMAKE_MODULE_LINKER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_DEV "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

set(DEBUG_COMPILE_OPTIONS "-DALIMER_DEV=1")
add_compile_options("$<$<CONFIG:Debug>:${DEBUG_COMPILE_OPTIONS}>")

set(DEV_COMPILE_OPTIONS "-DALIMER_DEV=1" )
add_compile_options("$<$<CONFIG:Dev>:${DEV_COMPILE_OPTIONS}>")

set(RELEASE_COMPILE_OPTIONS "-DALIMER_DEV=0")
add_compile_options("$<$<CONFIG:Release>:${RELEASE_COMPILE_OPTIONS}>")

# Setup SDK install destinations
if (WIN32)
    set (SCRIPT_EXT .bat)
else ()
    set (SCRIPT_EXT .sh)
endif ()

set (DEST_BASE_INCLUDE_DIR include)
set (DEST_INCLUDE_DIR ${DEST_BASE_INCLUDE_DIR}/Alimer)
set (DEST_ARCHIVE_DIR lib)
set (DEST_BIN_DIR bin)
set (DEST_TOOLS_DIR ${DEST_BIN_DIR})
set (DEST_SHARE_DIR share)
set (DEST_ASSETS_DIR ${DEST_BIN_DIR})
set (DEST_THIRDPARTY_HEADERS_DIR ${DEST_INCLUDE_DIR}/ThirdParty)

if (ANDROID)
    set (DEST_LIBRARY_DIR ${DEST_ARCHIVE_DIR})
else ()
    set (DEST_LIBRARY_DIR bin)
endif ()
