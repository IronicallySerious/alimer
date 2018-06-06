file (GLOB_RECURSE HEADER_FILES *.h *.hpp)
file (GLOB_RECURSE SOURCE_FILES *.c *.cpp)

# Define the target.
add_library(VulkanD3D12 SHARED ${SOURCE_FILES} ${HEADER_FILES})
if (PLATFORM_UWP)
    target_link_libraries (VulkanD3D12 d3d12.lib dxgi.lib d3dcompiler.lib)
endif ()

target_include_directories(VulkanD3D12 
	PRIVATE 
	$<BUILD_INTERFACE:${ALIMER_THIRD_PARTY_DIR}>
)
