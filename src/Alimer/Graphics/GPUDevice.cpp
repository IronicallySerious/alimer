//
// Copyright (c) 2017-2019 Amer Koleci and contributors.
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

#include "../Graphics/GPUDevice.h"
#include "../Graphics/GPUDeviceImpl.h"
#include "../Core/Log.h"

#if defined(ALIMER_D3D11)
#   include "../Graphics/D3D11/DeviceD3D11.h"
#endif

#if ALIMER_COMPILE_VULKAN
#   include "../Graphics/Vulkan/VulkanGraphicsDevice.h"
#endif

#if defined(ALIMER_D3D12)
#include "../Graphics/D3D12/D3D12Graphics.h"
#endif

#if defined(_WIN32)
#include <windows.h>

// Prefer the high-performance GPU on switchable GPU systems
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif /* defined(_WIN32) */

namespace alimer
{
    GPUDevice::GPUDevice(GPUDeviceImpl* impl, bool validation)
        : _impl(impl)
        , _backend(impl->GetBackend())
        , _validation(validation)
        , _frameIndex(0)
    {
        _immediateCommandContext = new CommandContext(this, _impl->GetDefaultCommandBuffer());
        AddSubsystem(this);
    }

    GPUDevice::~GPUDevice()
    {
        Finalize();
        RemoveSubsystem(this);
    }

    GPUDevice* GPUDevice::Create(GraphicsBackend preferredBackend, bool validation)
    {
        GraphicsBackend backend = preferredBackend;
        if (backend == GraphicsBackend::Default)
        {
            backend = GetDefaultPlatformBackend();
        }

        if (!IsBackendSupported(backend))
        {
            ALIMER_LOGERROR("Backend {} is not supported", EnumToString(backend));
            return nullptr;
        }

        GPUDeviceImpl* device = nullptr;
        switch (backend)
        {
        case GraphicsBackend::Empty:
            break;
        case GraphicsBackend::Vulkan:
            break;
        case GraphicsBackend::D3D11:
#if defined(ALIMER_D3D11)
            device = new DeviceD3D11(validation);
            ALIMER_LOGINFO("D3D11 backend created with success.");
#else
            ALIMER_LOGERROR("D3D11 backend is not supported.");
#endif
            break;
        case GraphicsBackend::D3D12:
#if defined(ALIMER_D3D12)
            //device = new D3D12Graphics(validation);
            ALIMER_LOGINFO("D3D12 backend created with success.");
#else
            ALIMER_LOGERROR("D3D12 backend is not supported.");
#endif
            break;
        case GraphicsBackend::Metal:
            break;
        case GraphicsBackend::OpenGL:
            break;
        default:
            ALIMER_UNREACHABLE();
        }

        return new GPUDevice(device, validation);
    }


    void GPUDevice::Shutdown()
    {
    }

    void GPUDevice::Finalize()
    {
        // Destroy undestroyed resources.
        if (_gpuResources.Size())
        {
            std::lock_guard<std::mutex> lock(_gpuResourceMutex);
            for (auto it = _gpuResources.Begin(); it != _gpuResources.End(); ++it)
            {
                (*it)->Destroy();
            }

            _gpuResources.Clear();
        }

        SafeDelete(_immediateCommandContext);
    }

    bool GPUDevice::IsBackendSupported(GraphicsBackend backend)
    {
        if (backend == GraphicsBackend::Default)
        {
            backend = GetDefaultPlatformBackend();
        }

        switch (backend)
        {
        case GraphicsBackend::Empty:
            return true;
        case GraphicsBackend::Vulkan:
#if AGPU_VULKAN
            return true; // VulkanGraphicsDevice::IsSupported();
#else
            return false;
#endif
        case GraphicsBackend::D3D11:
#if defined(ALIMER_D3D11)
            return DeviceD3D11::IsSupported();
#else
            return false;
#endif

        case GraphicsBackend::D3D12:
#if defined(ALIMER_D3D12)
            return false;
            //return D3D12Graphics::IsSupported();
#else
            return false;
#endif

        case GraphicsBackend::OpenGL:
#if AGPU_OPENGL
            return true;
#else
            return false;
#endif
        default:
            return false;
        }
        }

    std::set<GraphicsBackend> GPUDevice::GetAvailableBackends()
    {
        static std::set<GraphicsBackend> backends;

        if (backends.empty())
        {
            backends.insert(GraphicsBackend::Empty);

#if ALIMER_COMPILE_OPENGL
            backends.insert(GraphicsBackend::OpenGL);
#endif

#if defined(ALIMER_D3D11)
            if (DeviceD3D11::IsSupported())
            {
                backends.insert(GraphicsBackend::D3D11);
            }
#endif


#if defined(ALIMER_D3D12)
            /*if (D3D12Graphics::IsSupported())
            {
                backends.insert(GraphicsBackend::D3D12);
            }*/
#endif

#if ALIMER_COMPILE_VULKAN
            if (VulkanGraphicsDevice::IsSupported())
            {
                backends.insert(GraphicsBackend::Vulkan);
            }
#endif
        }

        return backends;
    }

    GraphicsBackend GPUDevice::GetDefaultPlatformBackend()
    {
#if ALIMER_PLATFORM_WINDOWS || ALIMER_PLATFORM_UWP || ALIMER_PLATFORM_XBOX_ONE
        if (IsBackendSupported(GraphicsBackend::D3D12))
        {
            return GraphicsBackend::D3D12;
        }

        return GraphicsBackend::D3D11;
#elif ALIMER_PLATFORM_LINUX || ALIMER_PLATFORM_ANDROID
        return GraphicsBackend::OpenGL;
#elif ALIMER_PLATFORM_MACOS || ALIMER_PLATFORM_IOS || ALIMER_PLATFORM_TVOS
        return GraphicsBackend::Metal;
#else
        return GraphicsBackend::OpenGL;
#endif
    }

    const GPULimits& GPUDevice::GetLimits() const
    {
        return _impl->GetLimits();
    }

    const GraphicsDeviceFeatures& GPUDevice::GetFeatures() const
    {
        return _impl->GetFeatures();
    }

    bool GPUDevice::WaitIdle()
    {
        return _impl->WaitIdle();
    }

    uint64_t GPUDevice::Frame()
    {
        OnFrame();
        return ++_frameIndex;
    }

    void GPUDevice::TrackResource(GPUResource* resource)
    {
        std::unique_lock<std::mutex> lock(_gpuResourceMutex);
        _gpuResources.Push(resource);
    }

    void GPUDevice::UntrackResource(GPUResource* resource)
    {
        std::unique_lock<std::mutex> lock(_gpuResourceMutex);
        _gpuResources.Remove(resource);
    }

    CommandContext& GPUDevice::Begin(const String& name)
    {
        CommandContext* newContext = nullptr; // AllocateContext();
        newContext->SetName(name);
        if (!name.IsEmpty())
        {
            //GpuProfiler::BeginBlock(name, newContext);
        }

        return *newContext;
    }

    void GPUDevice::NotifyValidationError(const char* message)
    {
        // TODO: Add callback.
        ALIMER_UNUSED(message);
    }

    void GPUDevice::RegisterObject()
    {
        static bool registered = false;
        if (registered)
            return;
        registered = true;

        ShaderModule::RegisterObject();
        Shader::RegisterObject();
        Texture::RegisterObject();
        Sampler::RegisterObject();
    }

    Texture* GPUDevice::CreateTexture1D(uint32_t width, uint32_t mipLevels, uint32_t arrayLayers, PixelFormat format, TextureUsage usage, const void* initialData)
    {

#if defined(ALIMER_DEV)
        if (format == PixelFormat::Unknown)
        {
            ALIMER_LOGCRITICAL("Invalid texture usage");
        }

        if (usage == TextureUsage::None)
        {
            ALIMER_LOGCRITICAL("Invalid texture usage");
        }

        if (width == 0
            || arrayLayers == 0
            || mipLevels == 0)
        {
            ALIMER_LOGCRITICAL("Cannot create an empty texture");
        }
#endif // defined(ALIMER_DEV)

        return nullptr;
        //return CreateTextureCore(TextureType::Type1D, width, 1, 1, mipLevels, arrayLayers, format, usage, SampleCount::Count1, initialData);
    }

    
    Texture* GPUDevice::CreateTextureCube(uint32_t size, uint32_t mipLevels, uint32_t arrayLayers, PixelFormat format, TextureUsage usage, const void* initialData)
    {
#if defined(ALIMER_DEV)
        if (format == PixelFormat::Unknown)
        {
            ALIMER_LOGCRITICAL("Invalid texture usage");
        }

        if (usage == TextureUsage::None)
        {
            ALIMER_LOGCRITICAL("Invalid texture usage");
        }

        if (size == 0
            || arrayLayers == 0
            || mipLevels == 0)
        {
            ALIMER_LOGCRITICAL("Cannot create an empty texture");
        }
#endif // defined(ALIMER_DEV)

        return nullptr;
        //return CreateTextureCore(TextureType::TypeCube, size, size, 1, mipLevels, arrayLayers, format, usage, SampleCount::Count1, initialData);
    }

    Texture* GPUDevice::CreateTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, PixelFormat format, TextureUsage usage, const void* initialData)
    {
#if defined(ALIMER_DEV)
        if (format == PixelFormat::Unknown)
        {
            ALIMER_LOGCRITICAL("Invalid texture usage");
        }

        if (usage == TextureUsage::None)
        {
            ALIMER_LOGCRITICAL("Invalid texture usage");
        }

        if (width == 0
            || height == 0
            || depth == 0
            || mipLevels == 0)
        {
            ALIMER_LOGCRITICAL("Cannot create an empty texture");
        }
#endif // defined(ALIMER_DEV)

        return nullptr;
        //return CreateTextureCore(TextureType::Type3D, width, height, depth, mipLevels, 1, format, usage, SampleCount::Count1, initialData);
    }
}
