//
// Copyright (c) 2018 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../../Core/Log.h"
#include "../../Core/Window.h"
#include "VulkanGraphics.h"
using namespace std;

namespace Alimer
{
	/*
	* A layer can expose extensions, keep track of those
	* extensions here.
	*/
	struct LayerProperties {
		VkLayerProperties properties;
		std::vector<VkExtensionProperties> instance_extensions;
		std::vector<VkExtensionProperties> device_extensions;
	};

	static inline VkResult InitGlobalExtensionProperties(LayerProperties &layer_props)
	{
		VkExtensionProperties *instanceExtensions;
		uint32_t instanceExtensionCount;
		VkResult res;
		char *layer_name = nullptr;

		layer_name = layer_props.properties.layerName;

		do {
			res = vkEnumerateInstanceExtensionProperties(layer_name, &instanceExtensionCount, nullptr);
			if (res) return res;

			if (instanceExtensionCount == 0) 
				return VK_SUCCESS;

			layer_props.instance_extensions.resize(instanceExtensionCount);
			instanceExtensions = layer_props.instance_extensions.data();
			res = vkEnumerateInstanceExtensionProperties(layer_name, &instanceExtensionCount, instanceExtensions);
		} while (res == VK_INCOMPLETE);

		return res;
	}

	static inline VkResult InitGlobalLayerProperties(vector<LayerProperties>& instance_layer_properties)
	{
		uint32_t instance_layer_count;
		VkLayerProperties *vk_props = NULL;
		VkResult res;

		/*
		* It's possible, though very rare, that the number of
		* instance layers could change. For example, installing something
		* could include new layers that the loader would pick up
		* between the initial query for the count and the
		* request for VkLayerProperties. The loader indicates that
		* by returning a VK_INCOMPLETE status and will update the
		* the count parameter.
		* The count parameter will be updated with the number of
		* entries loaded into the data pointer - in case the number
		* of layers went down or is smaller than the size given.
		*/
		do {
			res = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
			if (res) return res;

			if (instance_layer_count == 0) {
				return VK_SUCCESS;
			}

			vk_props = (VkLayerProperties *)realloc(vk_props, instance_layer_count * sizeof(VkLayerProperties));

			res = vkEnumerateInstanceLayerProperties(&instance_layer_count, vk_props);
		} while (res == VK_INCOMPLETE);

		/*
		* Now gather the extension list for each instance layer.
		*/
		for (uint32_t i = 0; i < instance_layer_count; i++) {
			LayerProperties layer_props;
			layer_props.properties = vk_props[i];
			res = InitGlobalExtensionProperties(layer_props);
			if (res) return res;
			instance_layer_properties.push_back(layer_props);
		}
		free(vk_props);

		return res;
	}

	VkBool32 CheckLayers(const vector<LayerProperties> &layer_props, const vector<const char *> &layer_names)
	{
		size_t checkCount = layer_names.size();
		size_t layerCount = layer_props.size();
		for (size_t i = 0; i < checkCount; i++) {
			VkBool32 found = 0;
			for (size_t j = 0; j < layerCount; j++) {
				if (!strcmp(layer_names[i], layer_props[j].properties.layerName)) {
					found = 1;
				}
			}
			if (!found) {
				ALIMER_LOGDEBUG("[Vulkan] - Cannot find layer: %s", layer_names[i]);
				return 0;
			}
		}
		return 1;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(
		VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
		uint64_t object, size_t location, int32_t messageCode,
		const char *pLayerPrefix, const char *pMessage, void *pUserData)
	{
		(void)objectType;
		(void)object;
		(void)location;
		(void)pUserData;

		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		{
			ALIMER_LOGERROR("[Vulkan] - [%s] Code %d : %s", pLayerPrefix, messageCode, pMessage);
		}
		else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		{
			ALIMER_LOGWARN("[Vulkan] - [%s] Code %d : %s", pLayerPrefix, messageCode, pMessage);
		}
		else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		{
			ALIMER_LOGERROR("[Vulkan] - PERFORMANCE WARNING: [%s] Code %d : %s", pLayerPrefix, messageCode, pMessage);
		}
		else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		{
			ALIMER_LOGINFO("[Vulkan] - [%s] Code %d : %s", pLayerPrefix, messageCode, pMessage);
		}
		else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		{
			ALIMER_LOGDEBUG("[%s] Code %d : %s", pLayerPrefix, messageCode, pMessage);
		}
		else
		{
			ALIMER_LOGINFO("%s: %s", pLayerPrefix, pMessage);
		}

		return VK_FALSE;
	}

	bool VulkanGraphics::IsSupported()
	{
		static bool availableCheck = false;
		static bool isAvailable = false;

		if (availableCheck)
			return isAvailable;

		availableCheck = true;
		VkResult vkRes = volkInitialize();
		if (vkRes != VK_SUCCESS)
		{
			isAvailable = false;
			ALIMER_LOGERROR("Failed to initialize Vulkan");
			return false;
		}

		isAvailable = true;
		return true;
	}

	VulkanGraphics::VulkanGraphics(bool validation, const string& applicationName)
		: Graphics(GraphicsDeviceType::Vulkan)
	{
		vector<LayerProperties> instanceLayerProperties;
		VkResult result = InitGlobalLayerProperties(instanceLayerProperties);

		uint32_t apiVersion = VK_API_VERSION_1_0;
		// Determine if the new instance version command is available
		if (vkEnumerateInstanceVersion != nullptr)
		{
			uint32_t checkApiVersion = 0;
			if (vkEnumerateInstanceVersion(&checkApiVersion) == VK_SUCCESS)
			{
				// Translate the version into major/minor for easier comparison
				uint32_t loader_major_version = VK_VERSION_MAJOR(checkApiVersion);
				uint32_t loader_minor_version = VK_VERSION_MINOR(checkApiVersion);
				ALIMER_LOGDEBUG("Loader/Runtime support detected for Vulkan %d.%d", loader_major_version, loader_minor_version);
				apiVersion = checkApiVersion;
			}
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = applicationName.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Alimer";
		//appInfo.engineVersion = VK_MAKE_VERSION(ALIMER_VERSION_MAJOR, ALIMER_VERSION_MINOR, ALIMER_VERSION_PATCH);
		appInfo.engineVersion = VK_MAKE_VERSION(0, 9, 0);
		appInfo.apiVersion = apiVersion;

		vector<const char*> instanceExtensionNames = { VK_KHR_SURFACE_EXTENSION_NAME };

		// Enable surface extensions depending on os.
#if ALIMER_PLATFORM_WINDOWS
		instanceExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif ALIMER_PLATFORM_LINUX
		instanceExtensionNames.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif ALIMER_PLATFORM_APPLE_OSX
		instanceExtensionNames.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif ALIMER_PLATFORM_APPLE_IOS || ALIMER_PLATFORM_APPLE_TV
		instanceExtensionNames.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif ALIMER_PLATFORM_ANDROID
		instanceExtensionNames.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#else
		instanceExtensionNames.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif

		vector<const char*> instanceLayerNames;
		if (validation)
		{
			instanceExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

			instanceLayerNames.push_back("VK_LAYER_LUNARG_standard_validation");
			if (!CheckLayers(instanceLayerProperties, instanceLayerNames))
			{
				/* If standard validation is not present, search instead for the
				* individual layers that make it up, in the correct order.
				*/
				instanceLayerNames.clear();
				instanceLayerNames.push_back("VK_LAYER_GOOGLE_threading");
				instanceLayerNames.push_back("VK_LAYER_LUNARG_parameter_validation");
				instanceLayerNames.push_back("VK_LAYER_LUNARG_object_tracker");
				instanceLayerNames.push_back("VK_LAYER_LUNARG_core_validation");
				instanceLayerNames.push_back("VK_LAYER_GOOGLE_unique_objects");

				if (!CheckLayers(instanceLayerProperties, instanceLayerNames))
				{
					instanceLayerNames.clear();
					ALIMER_LOGWARN("[Vulkan] - Set the environment variable VK_LAYER_PATH to point to the location of your layers");
				}
			}
		}

		VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		instanceCreateInfo.pNext = nullptr;
		instanceCreateInfo.pApplicationInfo = &appInfo;
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayerNames.size());
		instanceCreateInfo.ppEnabledLayerNames = instanceLayerNames.data();
		instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensionNames.size());
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensionNames.data();

		result = vkCreateInstance(&instanceCreateInfo, nullptr, &_vkInstance);
		vkThrowIfFailed(result);

		// Now load vk symbols.
		volkLoadInstance(_vkInstance);

		// Enumerate physical devices.
		uint32_t gpuCount = 0;
		// Get number of available physical devices
		vkThrowIfFailed(vkEnumeratePhysicalDevices(_vkInstance, &gpuCount, nullptr));
		if (gpuCount > 0)
		{
			// Enumerate physical devices.
			std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
			vkEnumeratePhysicalDevices(_vkInstance, &gpuCount, physicalDevices.data());

			//for (uint32_t i = 0; i < gpuCount; ++i)
			//{
			//	_physicalDevice.Push(new VulkanPhysicalDevice(physicalDevices[i]));
			//}

			_vkPhysicalDevice = physicalDevices.front();
		}

		// Setup debug callback
		if (validation)
		{
			VkDebugReportCallbackCreateInfoEXT debugCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
			debugCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
			debugCreateInfo.pfnCallback = VkDebugCallback;
			debugCreateInfo.pUserData = nullptr;
			result = vkCreateDebugReportCallbackEXT(_vkInstance, &debugCreateInfo, nullptr, &_vkDebugCallback);
		}
	}

	VulkanGraphics::~VulkanGraphics()
	{
		WaitIdle();
		Finalize();
	}

	void VulkanGraphics::Finalize()
	{
		// Release all GPU objects
		Graphics::Finalize();

		// Destroy logical device.
		if (_device != VK_NULL_HANDLE)
		{
			vkDestroyDevice(_device, nullptr);
			_device = VK_NULL_HANDLE;
		}
	}

	bool VulkanGraphics::WaitIdle()
	{
		VkResult result = vkDeviceWaitIdle(_device);
		if (result < VK_SUCCESS)
			return false;

		return true;
	}

	bool VulkanGraphics::Initialize(std::shared_ptr<Window> window)
	{
		return Graphics::Initialize(window);
	}

	std::shared_ptr<Texture> VulkanGraphics::AcquireNextImage()
	{
		return nullptr;
	}

	bool VulkanGraphics::Present()
	{
		return true;
	}

	CommandBufferPtr VulkanGraphics::CreateCommandBuffer()
	{
		return nullptr;
	}

	GpuBufferPtr VulkanGraphics::CreateBuffer(BufferUsage usage, uint32_t elementCount, uint32_t elementSize, const void* initialData)
	{
		return nullptr;
	}

	PipelineLayoutPtr VulkanGraphics::CreatePipelineLayout()
	{
		return nullptr;
	}

	std::shared_ptr<Shader> VulkanGraphics::CreateShader(const std::string& name)
	{
		return nullptr;
	}

	std::shared_ptr<Shader> VulkanGraphics::CreateShader(const ShaderBytecode& vertex, const ShaderBytecode& fragment)
	{
		return nullptr;
	}

	PipelineStatePtr VulkanGraphics::CreateRenderPipelineState(const RenderPipelineDescriptor& descriptor)
	{
		return nullptr;
	}

	bool VulkanGraphics::PrepareDraw(PrimitiveTopology topology)
	{
		return true;
	}
}
