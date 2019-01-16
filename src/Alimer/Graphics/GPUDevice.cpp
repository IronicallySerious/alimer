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

#include "Graphics/GPUDevice.h"
#include "Graphics/SwapChain.h"
#include "Graphics/Texture.h"
#include "Graphics/Sampler.h"
#include "Graphics/DeviceBackend.h"
#include "Core/Log.h"

#if defined(ALIMER_D3D11)
#   include "Graphics/D3D11/DeviceD3D11.h"
#endif

#if defined(ALIMER_D3D12)
//#   include "Graphics/D3D12/D3D12Graphics.h"
#endif

#if defined(ALIMER_OPENGL)
#   include "Graphics/OpenGL/DeviceGL.h"
#endif

#if defined(ALIMER_VULKAN)
//#   include "Graphics/Vulkan/VulkanGraphicsDevice.h"
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
    GPUDevice::GPUDevice(GraphicsBackend preferredBackend, bool validation)
        : _frameIndex(0)
    {
        GraphicsBackend backend = preferredBackend;
        if (backend == GraphicsBackend::Default)
        {
            backend = GetDefaultPlatformBackend();
        }

        if (!IsBackendSupported(backend))
        {
            ALIMER_LOGERROR("Backend {} is not supported", EnumToString(backend));
            return;
        }

        switch (backend)
        {
        case GraphicsBackend::Empty:
            break;
        case GraphicsBackend::Vulkan:
            break;
        case GraphicsBackend::D3D11:
#if defined(ALIMER_D3D11)
            _impl = new DeviceD3D11(validation);
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
#if defined(ALIMER_OPENGL)
            _impl = new DeviceGL(validation);
            ALIMER_LOGINFO("D3D12 backend created with success.");
#else
            ALIMER_LOGERROR("D3D12 backend is not supported.");
#endif
            break;
        default:
            ALIMER_UNREACHABLE();
        }

        // Create immediate command buffer.
        _immediateCommandContext = new CommandContext(this, _impl->GetDefaultCommandBuffer());
        AddSubsystem(this);
    }

    GPUDevice::~GPUDevice()
    {
        Finalize();
        RemoveSubsystem(this);
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
#if defined(ALIMER_OPENGL)
            return DeviceGL::IsSupported();
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

#if defined(ALIMER_OPENGL)
            if (DeviceGL::IsSupported())
            {
                backends.insert(GraphicsBackend::OpenGL);
            }
#endif

#if defined(ALIMER_D3D11)
            if (DeviceD3D11::IsSupported())
            {
                backends.insert(GraphicsBackend::D3D11);
            }
#endif


#if defined(ALIMER_D3D12)
            //if (D3D12Graphics::IsSupported())
            //{
            //    backends.insert(GraphicsBackend::D3D12);
            //}
#endif

#if defined(ALIMER_VULKAN)
            //if (DeviceVk::IsSupported())
            //{
            //    backends.insert(GraphicsBackend::Vulkan);
            //}
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

    bool GPUDevice::WaitIdle()
    {
        return _impl->WaitIdle();
    }

    GraphicsBackend GPUDevice::GetBackend() const
    {
        return _impl->GetBackend();
    }

    const GPULimits& GPUDevice::GetLimits() const
    {
        return _impl->GetLimits();
    }

    const GraphicsDeviceFeatures& GPUDevice::GetFeatures() const
    {
        return _impl->GetFeatures();
    }

    uint64_t GPUDevice::Frame()
    {
        _impl->Tick();
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
        //newContext->SetName(name);
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

        //Shader::RegisterObject();
        Texture::RegisterObject();
        Sampler::RegisterObject();
    }

#if TODO
    SwapChain* GPUDevice::CreateSwapChain(const SwapChainDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);

        if (descriptor->nativeWindow == nullptr)
        {
            ALIMER_LOGCRITICAL("Cannot create swap chain with null window handle.");
        }

        if (descriptor->width == 0 || descriptor->height == 0)
        {
            ALIMER_LOGCRITICAL("Cannot create swap chain with 0 width or height.");
        }

        return CreateSwapChainImpl(descriptor);
    }

    Texture* GPUDevice::CreateTexture1D(uint32_t width, uint32_t mipLevels, uint32_t arraySize, PixelFormat format, TextureUsage usage, const void* initialData)
    {
        TextureDescriptor descriptor = {};
        descriptor.width = width;;
        descriptor.height = 1;
        descriptor.depth = 1;
        descriptor.arraySize = arraySize;
        descriptor.mipLevels = mipLevels;
        descriptor.samples = SampleCount::Count1;
        descriptor.type = TextureType::Type1D;
        descriptor.format = format;
        descriptor.usage = usage;
        return CreateTexture(&descriptor, initialData);
    }

    Texture* GPUDevice::CreateTexture2D(
        uint32_t width, uint32_t height,
        uint32_t mipLevels, uint32_t arraySize,
        PixelFormat format, TextureUsage usage,
        SampleCount samples, const void* initialData)
    {
        TextureDescriptor descriptor = {};
        descriptor.width = width;;
        descriptor.height = height;
        descriptor.depth = 1;
        descriptor.arraySize = arraySize;
        descriptor.mipLevels = mipLevels;
        descriptor.samples = samples;
        descriptor.type = TextureType::Type2D;
        descriptor.format = format;
        descriptor.usage = usage;
        return CreateTexture(&descriptor, initialData);
    }

    Texture* GPUDevice::CreateTextureCube(uint32_t size, uint32_t mipLevels, uint32_t arraySize, PixelFormat format, TextureUsage usage, const void* initialData)
    {
        TextureDescriptor descriptor = {};
        descriptor.width = size;;
        descriptor.height = size;
        descriptor.depth = 1;
        descriptor.arraySize = arraySize;
        descriptor.mipLevels = mipLevels;
        descriptor.samples = SampleCount::Count1;
        descriptor.type = TextureType::Type3D;
        descriptor.format = format;
        descriptor.usage = usage;
        return CreateTexture(&descriptor, initialData);
    }

    Texture* GPUDevice::CreateTexture3D(uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, PixelFormat format, TextureUsage usage, const void* initialData)
    {
        TextureDescriptor descriptor = {};
        descriptor.width = width;;
        descriptor.height = height;
        descriptor.depth = depth;
        descriptor.arraySize = 1;
        descriptor.mipLevels = mipLevels;
        descriptor.samples = SampleCount::Count1;
        descriptor.type = TextureType::Type3D;
        descriptor.format = format;
        descriptor.usage = usage;
        return CreateTexture(&descriptor, initialData);
    }

    static bool ValidateFramebufferAttachment(const FramebufferAttachment& attachment, bool isDepthAttachment)
    {
#ifndef _DEBUG
        ALIMER_UNUSED(attachment);
        ALIMER_UNUSED(isDepthAttachment);
        return true;
#else
        if (attachment.texture == nullptr)
        {
            ALIMER_LOGERROR("Framebuffer attachment error : texture is null.");
            return false;
        }

        if (attachment.level >= attachment.texture->GetMipLevels())
        {
            ALIMER_LOGERROR("Framebuffer attachment error : mipLevel out of bound.");
            return false;
        }

        /*if (attachment.layerCount != RemainingArrayLayers)
        {
            if (attachment.layerCount == 0)
            {
                ALIMER_LOGERROR("Error when attaching texture to framebuffer : Requested to attach zero array slices");
                return false;
            }

            if (attachment.texture->GetTextureType() == TextureType::Type3D)
            {
                if (attachment.baseArrayLayer + attachment.layerCount > attachment.texture->GetDepth())
                {
                    ALIMER_LOGERROR("Error when attaching texture to framebuffer : Requested depth index is out of bound.");
                    return false;
                }
            }
            else
            {
                if (attachment.baseArrayLayer + attachment.layerCount > attachment.texture->GetArrayLayers())
                {
                    ALIMER_LOGERROR("Error when attaching texture to framebuffer : Requested array index is out of bound.");
                    return false;
                }
            }
        }*/

        if (isDepthAttachment)
        {
            if (IsDepthStencilFormat(attachment.texture->GetFormat()) == false)
            {
                ALIMER_LOGERROR("Error when attaching texture to framebuffer : Attaching to depth-stencil target, but resource has color format.");
                return false;
            }

            if (!any(attachment.texture->GetUsage() & TextureUsage::RenderTarget))
            {
                ALIMER_LOGERROR("Error when attaching texture to FBO: Attaching to depth-stencil target, the texture has no RenderTarget usage flag.");
                return false;
            }
        }
        else
        {
            if (IsDepthStencilFormat(attachment.texture->GetFormat()))
            {
                ALIMER_LOGERROR("Error when attaching texture to FBO: Attaching to color target, but resource has depth-stencil format.");
                return false;
            }

            if (!any(attachment.texture->GetUsage() & TextureUsage::RenderTarget))
            {
                ALIMER_LOGERROR("Error when attaching texture to FBO: Attaching to color target, the texture has no RenderTarget usage flag.");
                return false;
            }
        }

        return true;
#endif
    }

    Framebuffer* GPUDevice::CreateFramebuffer(const FramebufferDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);

        for (uint32_t i = 0; i < MaxColorAttachments; ++i)
        {
            if (descriptor->colorAttachments[i].texture == nullptr)
            {
                continue;
            }

            if (!ValidateFramebufferAttachment(descriptor->colorAttachments[i], false))
            {
                return nullptr;
            }
        }

        if (descriptor->depthStencilAttachment.texture != nullptr)
        {
            if (!ValidateFramebufferAttachment(descriptor->depthStencilAttachment, true))
            {
                return nullptr;
            }
        }

        return CreateFramebufferImpl(descriptor);
    }

    Buffer* GPUDevice::CreateBuffer(const BufferDescriptor* descriptor, const void* initialData)
    {
        ALIMER_ASSERT(descriptor);
        return CreateBufferImpl(descriptor, initialData);
    }

    Shader* GPUDevice::CreateShader(const ShaderDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);
        return CreateShaderImpl(descriptor);
    }
#endif // TODO

}
