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

#include "Graphics.h"
#include "ShaderCompiler.h"
#include "../IO/FileSystem.h"
#include "../IO/Path.h"
#include "../Core/Log.h"

#if ALIMER_D3D12
#include "Graphics/D3D12/D3D12Graphics.h"
#endif

#include "volk/volk.h"

namespace Alimer
{
    Alimer::Graphics* graphics = nullptr;

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

    static VkResult InitGlobalLayerProperties(std::vector<LayerProperties>& instance_layer_properties)
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

    VkBool32 CheckLayers(
        const std::vector<LayerProperties> &layer_props,
        const std::vector<const char *> &layer_names)
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

    Graphics::Graphics(bool validation, const std::string& applicationName)
        : _validation(validation)
    {
#ifdef _WIN32
        HMODULE vulkanD3D12Module = LoadLibraryA("VulkanD3D12.dll");
        if (!vulkanD3D12Module)
        {
            return;
        }

        vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(vulkanD3D12Module, "vkGetInstanceProcAddr");
#endif

        volkInitializeCustom(vkGetInstanceProcAddr);
        /*VkResult vkRes = volkInitialize();
        if (vkRes != VK_SUCCESS)
        {
        isAvailable = false;
        ALIMER_LOGERROR("Failed to initialize Vulkan");
        return false;
        }*/

        std::vector<LayerProperties> instanceLayerProperties;
        VkResult result = InitGlobalLayerProperties(instanceLayerProperties);

        uint32_t apiVersion = VK_API_VERSION_1_0;
        // Determine if the new instance version command is available
        if (vkEnumerateInstanceVersion != nullptr)
        {
            uint32_t checkApiVersion = 0;
            if (vkEnumerateInstanceVersion(&checkApiVersion) == VK_SUCCESS)
            {
                // Translate the version into major/minor for easier comparison
                ALIMER_LOGDEBUG("Loader/Runtime support detected for Vulkan %d.%d.%d",
                    VK_VERSION_MAJOR(checkApiVersion),
                    VK_VERSION_MINOR(checkApiVersion),
                    VK_VERSION_PATCH(checkApiVersion));

                apiVersion = checkApiVersion;
            }
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = applicationName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Alimer Engine";
        //appInfo.engineVersion = VK_MAKE_VERSION(ALIMER_VERSION_MAJOR, ALIMER_VERSION_MINOR, ALIMER_VERSION_PATCH);
        appInfo.engineVersion = VK_MAKE_VERSION(0, 9, 0);
        appInfo.apiVersion = apiVersion;

        std::vector<const char*> instanceExtensionNames = { VK_KHR_SURFACE_EXTENSION_NAME };

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

        std::vector<const char*> instanceLayerNames;
        bool hasValidationLayer = false;
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
                else
                {
                    hasValidationLayer = true;
                }
            }
            else
            {
                hasValidationLayer = true;
            }
        }

        VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        instanceCreateInfo.pNext = nullptr;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayerNames.size());
        instanceCreateInfo.ppEnabledLayerNames = instanceLayerNames.data();
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensionNames.size());
        instanceCreateInfo.ppEnabledExtensionNames = instanceExtensionNames.data();

        result = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);
        //vkThrowIfFailed(result);

        // Now load vk symbols.
        volkLoadInstance(_instance);

        // Setup debug callback
        if (validation && hasValidationLayer)
        {
            VkDebugReportCallbackCreateInfoEXT debugCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
            debugCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
            debugCreateInfo.pfnCallback = VkDebugCallback;
            debugCreateInfo.pUserData = nullptr;
            result = vkCreateDebugReportCallbackEXT(_instance, &debugCreateInfo, nullptr, &_debugCallback);
        }

        graphics = this;
    }

    Graphics::~Graphics()
    {
        Finalize();
        graphics = nullptr;
    }

    void Graphics::Finalize()
    {

    }

    bool Graphics::Initialize(const SharedPtr<Window>& window)
    {
        _window = window;
        return true;
    }

    bool Graphics::WaitIdle()
    {
        return true;
    }

    SharedPtr<Texture> Graphics::AcquireNextImage()
    {
        return SharedPtr<Texture>();
    }

    bool Graphics::Present()
    {
        return true;
    }

    SharedPtr<CommandBuffer> Graphics::GetCommandBuffer()
    {
        return SharedPtr<CommandBuffer>();
    }

    GpuBufferPtr Graphics::CreateBuffer(BufferUsage usage, uint32_t elementCount, uint32_t elementSize, const void* initialData)
    {
        return nullptr;
    }

    PipelineLayoutPtr Graphics::CreatePipelineLayout()
    {
        return nullptr;
    }

    SharedPtr<Shader> Graphics::CreateShader(const std::string& vertexShaderFile, const std::string& fragmentShaderFile)
    {
        std::string baseShaderUrl = "assets://shaders/";

        // Lookup for GLSL shader.
        if (!FileSystem::Get().FileExists(baseShaderUrl + vertexShaderFile + ".glsl"))
        {
            ALIMER_LOGERROR("GLSL shader does not exists '%s'", vertexShaderFile.c_str());
            return nullptr;
        }

        if (!FileSystem::Get().FileExists(baseShaderUrl + fragmentShaderFile + ".glsl"))
        {
            ALIMER_LOGERROR("GLSL shader does not exists '%s'", fragmentShaderFile.c_str());
            return nullptr;
        }

        // Compile GLSL.
        std::string errorLog;
        std::vector<uint32_t> vertexByteCode = ShaderCompiler::Compile(baseShaderUrl + vertexShaderFile + ".glsl", errorLog);
        std::vector<uint32_t> fragmentByteCode = ShaderCompiler::Compile(baseShaderUrl + fragmentShaderFile + ".glsl", errorLog);

        ShaderStageDescription vertex = {};
        vertex.byteCode = vertexByteCode.data();
        vertex.byteCodeSize = vertexByteCode.size();
        vertex.entryPoint = "main";

        ShaderStageDescription fragment = {};
        fragment.byteCode = fragmentByteCode.data();
        fragment.byteCodeSize = fragmentByteCode.size();
        fragment.entryPoint = "main";

        return CreateShader(vertex, fragment);
    }

    SharedPtr<Shader> Graphics::CreateComputeShader(const ShaderStageDescription& desc)
    {
        return nullptr;
    }

    SharedPtr<Shader> Graphics::CreateShader(
        const ShaderStageDescription& vertex,
        const ShaderStageDescription& fragment)
    {
        return nullptr;
    }

    PipelineStatePtr Graphics::CreateRenderPipelineState(const RenderPipelineDescriptor& descriptor)
    {
        return nullptr;
    }
}
