include(ucm)

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
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
    set (ALIMER_CLANG 1)
    set (ALIMER_COMPILER_NAME "MSVC Clang (ALIMER_CLANG)")
    set (CLANG ON)
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
if (ALIMER_WINDOWS OR ALIMER_LINUX OR ALIMER_ANDROID)
    set (ALIMER_VULKAN_DEFAULT ON)
else ()
    set (ALIMER_VULKAN_DEFAULT OFF)
endif ()

# Tools
if (ALIMER_DESKTOP)
    set (ALIMER_TOOLS_DEFAULT ON)
else ()
    set (ALIMER_TOOLS_DEFAULT OFF)
endif()

option (ALIMER_ENABLE_ALL "Enables all optional subsystems by default" OFF)


if (NOT CMAKE_CROSS_COMPILING AND ALIMER_DESKTOP)
    set (ALIMER_CSHARP_DEFAULT ON)
else ()
    set (ALIMER_CSHARP_DEFAULT OFF)
endif ()

alimer_option (ALIMER_THREADING "Enable multithreading")
alimer_option (ALIMER_GL "Enable OpenGL backend")
alimer_option (ALIMER_VULKAN "Enable Vulkan backend")
alimer_option (ALIMER_TOOLS "Enable Tools")
alimer_option (ALIMER_CSHARP "Enable C# support")

set (ALIMER_FLAGS "")
set (ALIMER_DEFS "")
set (ALIMER_INTERNAL_FLAGS "")
set (ALIMER_INTERNAL_DEFS "")
set (ALIMER_GENERATED_FLAGS "")

# Select static/dynamic runtime library
if( ALIMER_WINDOWS )
    ucm_set_runtime(STATIC)
elseif( ALIMER_UWP OR ALIMER_XBOX_ONE )
    ucm_set_runtime(DYNAMIC)
endif()

# Setup global per-platform compiler/linker options
if( MSVC )
	# Disable specific warnings
	add_compile_options(/wd4127 /wd4351 /wd4005)

	# Disable specific warnings for MSVC14 and above
	if( (ALIMER_WINDOWS OR ALIMER_UWP) AND (NOT MSVC_VERSION LESS 1900) )
		add_compile_options(/wd4838 /wd4312 /wd4477 /wd4244 /wd4091 /wd4311 /wd4302 /wd4476 /wd4474)
        add_compile_options(/wd4309)	# truncation of constant value
	endif()

	# Force specific warnings as errors
	add_compile_options(/we4101)

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

# Functions and macros
function(alimer_setup_common_properties target)
    if (MSVC)
        target_compile_definitions(${target} PRIVATE _SCL_SECURE_NO_WARNINGS _CRT_SECURE_NO_WARNINGS NOMINMAX)
        target_compile_definitions(${target} PRIVATE _UNICODE)

        # Enable full optimization in Dev/Release
        target_compile_options(${target} PRIVATE $<$<CONFIG:DEBUG>:/Od> $<$<NOT:$<CONFIG:DEBUG>>:/Ox>)

        # Inline function expansion
        target_compile_options(${target} PRIVATE /Ob2)

         # Enable intrinsic functions in dev/release
	    target_compile_options(${target} PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:/Oi>)

        # Favor fast code
        target_compile_options(${target} PRIVATE /Ot)

        # Enable fiber-safe optimizations in dev/release
	    target_compile_options(${target} PRIVATE $<$<NOT:$<CONFIG:DEBUG>>:/GT>)

	    # Enable string pooling
	    target_compile_options(${target} PRIVATE /GF)

        target_compile_options(${target} PRIVATE /EHs /bigobj)

	    # Use security checks only in debug
	    if ( ALIMER_UWP )
            target_compile_options(${target} PRIVATE $<$<CONFIG:DEBUG>:/sdl> $<$<NOT:$<CONFIG:DEBUG>>:/sdl->)
	        # Consume Windows Runtime
		    target_compile_options(${target} PRIVATE /ZW)
		    # C++ exceptions
		    target_compile_options(${target} PRIVATE /EHsc)
        else()
            target_compile_options(${target} PRIVATE $<$<CONFIG:DEBUG>:/GS> $<$<NOT:$<CONFIG:DEBUG>>:/GS->)
        endif()

        # Enable function-level linking
        target_compile_options(${target} PRIVATE /Gy)

        # Use fast floating point model
        target_compile_options(${target} PRIVATE /fp:fast)

        # Set warning level 3 in non debug, otherwise level 4 and warning as errors
        target_compile_options(${target} PRIVATE
            $<$<NOT:$<CONFIG:DEBUG>>:/W3>
            $<$<CONFIG:Debug>:/W4>
            $<$<CONFIG:Debug>:/WX>
        )

        # Enable multi-processor compilation
        target_compile_options(${target} PRIVATE /MP)

        # Enable SIMD
        if( ALIMER_WINDOWS )
            if( ALIMER_64BIT )
                if( ALIMER_USE_AVX )
                    target_compile_options(${target} PRIVATE /arch:AVX)
                    target_compile_definitions(${target} PRIVATE AVX)
                endif()
            else()
                target_compile_options(${target} PRIVATE /arch:SSE2)
            endif()
        endif()

    elseif (CMAKE_COMPILER_IS_GNUCXX OR (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang"))
        #
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
    if (ALIMER_SHARED OR ALIMER_CSHARP)
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

function(alimer_external_target folder target)
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS ${ALIMER_FLAGS})
    set_property(TARGET ${target} APPEND PROPERTY COMPILE_DEFINITIONS ${ALIMER_DEFS})
    set_property(TARGET ${target} PROPERTY FOLDER "${folder}")
endfunction()
