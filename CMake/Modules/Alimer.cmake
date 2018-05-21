include (CMakeSettings)

# Setup global per-platform compiler/linker options
if( PLATFORM_WINDOWS OR PLATFORM_UWP )
    add_compile_options(-D_CRT_NONSTDC_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE)
    add_compile_options($<$<CONFIG:DEBUG>:-D_SECURE_SCL_THROWS=0> $<$<CONFIG:DEBUG>:-D_SILENCE_DEPRECATION_OF_SECURE_SCL_THROWS>)
    add_compile_options(-D_HAS_ITERATOR_DEBUGGING=$<CONFIG:DEBUG> -D_SECURE_SCL=$<CONFIG:DEBUG>)
    add_compile_options(-D_HAS_EXCEPTIONS=0)

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
	elseif( PLATFORM_UWP )
		add_compile_options($<$<CONFIG:DEBUG>:/MDd> $<$<NOT:$<CONFIG:DEBUG>>:/MD>)
	endif()

	# Disable specific link libraries
	if ( PLATFORM_WINDOWS )
		add_linker_flags(/NODEFAULTLIB:"MSVCRT.lib")
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

	# Disable run-time type information (RTTI)
	replace_compile_flags("/GR" "/GR-")

	# Enable multi-processor compilation for Visual Studio 2012 and above
	add_compile_options(/MP)

	# Set warning level 3
	add_compile_options(/W3)

	# Disable specific warnings
	add_compile_options(/wd4351 /wd4005)

	# Disable specific warnings for MSVC14 and above
	if( (PLATFORM_WINDOWS OR PLATFORM_UWP) AND (NOT MSVC_VERSION LESS 1900) )
		add_compile_options(/wd4838 /wd4312 /wd4477 /wd4244 /wd4091 /wd4311 /wd4302 /wd4476 /wd4474)
		add_compile_options(/wd4309)	# truncation of constant value
	endif()

	# Force specific warnings as errors
	add_compile_options(/we4101)

	# Treat all other warnings as errors
	add_compile_options(/WX)

	if ( PLATFORM_UWP )
	    # Consume Windows Runtime
		add_compile_options(/ZW)
		# C++ exceptions
		add_compile_options(/EHsc)
	endif()

	# Clean-up linker flags case since VS IDE doesn't recognize them properly
	replace_linker_flags("/debug" "/DEBUG" debug)
	replace_linker_flags("/machine:x64" "/MACHINE:X64")
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
set (DEST_ARCHIVE_DIR lib)
set (DEST_BIN_DIR bin)
set (DEST_SHARE_DIR share)


if (ANDROID)
    set (DEST_LIBRARY_DIR ${DEST_ARCHIVE_DIR})
else ()
    set (DEST_LIBRARY_DIR bin)
endif ()