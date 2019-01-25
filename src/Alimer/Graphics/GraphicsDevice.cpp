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

#include "Graphics/PhysicalDevice.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/SwapChain.h"
#include "Graphics/Texture.h"
#include "Graphics/Sampler.h"
#include "Core/Log.h"

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
    static GraphicsDevice* __graphicsInstance = nullptr;
    
    GraphicsDevice::GraphicsDevice(bool validation, bool headless)
        : _validation(validation)
        , _headless(headless)
        , _initialized(false)
    {
        if (!IsSupported())
        {
            ALIMER_LOGCRITICAL("Graphics backend is not supported");
        }

        PlatformConstruct();
    }

    GraphicsDevice::~GraphicsDevice()
    {
        WaitIdle();

        // Destroy undestroyed resources.
        SafeDelete(_pointSampler);
        SafeDelete(_linearSampler);
        SafeDelete(_mainSwapChain);

        if (_gpuResources.Size())
        {
            std::lock_guard<std::mutex> lock(_gpuResourceMutex);
            for (auto it = _gpuResources.Begin(); it != _gpuResources.End(); ++it)
            {
                (*it)->Destroy();
            }

            _gpuResources.Clear();
        }

        _renderContext.Reset();

        // Destroy adapters
        for(uint32_t i = 0; i < _physicalDevices.Size(); ++i)
        {
            SafeDelete(_physicalDevices[i]);
        }
        _physicalDevices.Clear();

        // Destroy backend.
        PlatformDestroy();

        RemoveSubsystem(this);
    }

    bool GraphicsDevice::Initialize(const GraphicsDeviceDescriptor* descriptor)
    {
        if (_initialized)
        {
            return true;
        }

        if (!PlatformInitialize(descriptor))
        {
            return false;
        }

        _initialized = true;
        return true;
    }

    void GraphicsDevice::OnAfterCreated()
    {
        SamplerDescriptor descriptor = {};
        _pointSampler = CreateSampler(&descriptor);

        descriptor.magFilter = SamplerMinMagFilter::Linear;
        descriptor.minFilter = SamplerMinMagFilter::Linear;
        descriptor.mipmapFilter = SamplerMipFilter::Linear;
        _linearSampler = CreateSampler(&descriptor);
    }

    bool GraphicsDevice::BeginFrame()
    {
        if (_inBeginFrame)
        {
            ALIMER_LOGCRITICAL("Cannot nest BeginFrame calls, call EndFrame first.");
        }

        //if (!BeginFrameImpl())
        //{
        //    ALIMER_LOGCRITICAL("Failed to begin rendering frame.");
        //}

        _inBeginFrame = true;
        return true;
    }

    uint64_t GraphicsDevice::EndFrame()
    {
        if (!_inBeginFrame)
        {
            ALIMER_LOGCRITICAL("BeginFrame must be called before EndFrame.");
        }

        //EndFrameImpl();
        _inBeginFrame = false;
        return ++_frameIndex;
    }

    void GraphicsDevice::TrackResource(GPUResource* resource)
    {
        std::unique_lock<std::mutex> lock(_gpuResourceMutex);
        _gpuResources.Push(resource);
    }

    void GraphicsDevice::UntrackResource(GPUResource* resource)
    {
        std::unique_lock<std::mutex> lock(_gpuResourceMutex);
        _gpuResources.Remove(resource);
    }

    void GraphicsDevice::NotifyValidationError(const char* message)
    {
        ALIMER_UNUSED(message);
    }


    static inline TextureUsage UpdateTextureUsage(TextureUsage usage, bool hasInitData, uint32_t mipLevels)
    {
        if ((mipLevels != 0) || (hasInitData == false))
        {
            return usage;
        }

        usage |= TextureUsage::RenderTarget;
        return  usage;
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

    Texture* GraphicsDevice::Create1DTexture(uint32_t width, PixelFormat format, uint32_t arraySize, uint32_t mipLevels, TextureUsage textureUsage, const void* pInitData)
    {
        ALIMER_ASSERT_MSG(width >= 1, "Width must be greather than 0.");
        ALIMER_ASSERT_MSG(arraySize >= 1 && arraySize <= 2048, "Array size must be between 1 and 2048.");
        ALIMER_ASSERT_MSG(format != PixelFormat::Unknown, "Invalid pixel format.");

        TextureDescriptor descriptor = {};
        descriptor.width = width;
        descriptor.height = 1;
        descriptor.depth = 1;
        descriptor.arraySize = arraySize;
        descriptor.mipLevels = mipLevels;
        if (descriptor.mipLevels == 0)
        {
            descriptor.mipLevels = bitScanReverse(width) + 1;
        }

        descriptor.samples = SampleCount::Count1;
        descriptor.type = TextureType::Type1D;
        descriptor.format = format;
        descriptor.usage = UpdateTextureUsage(textureUsage, pInitData != nullptr, mipLevels);
        return nullptr;
        //return CreateTextureImpl(&descriptor, nullptr, pInitData);
    }

    Texture* GraphicsDevice::Create2DTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t arraySize, uint32_t mipLevels, TextureUsage textureUsage, const void* pInitData)
    {
        ALIMER_ASSERT_MSG(width >= 1, "Width must be greather than 0.");
        ALIMER_ASSERT_MSG(height >= 1, "Height must be greather than 0.");
        ALIMER_ASSERT_MSG(arraySize >= 1 && arraySize <= 2048, "Array size must be between 1 and 2048.");
        ALIMER_ASSERT_MSG(format != PixelFormat::Unknown, "Invalid pixel format.");

        TextureDescriptor descriptor = {};
        descriptor.width = width;
        descriptor.height = height;
        descriptor.depth = 1;
        descriptor.arraySize = arraySize;
        descriptor.mipLevels = mipLevels;
        if (descriptor.mipLevels == 0)
        {
            uint32_t dims = width | height;
            descriptor.mipLevels = bitScanReverse(dims) + 1;
        }

        descriptor.samples = SampleCount::Count1;
        descriptor.type = TextureType::Type2D;
        descriptor.format = format;
        descriptor.usage = UpdateTextureUsage(textureUsage, pInitData != nullptr, mipLevels);
        return nullptr;
       // return CreateTextureImpl(&descriptor, nullptr, pInitData);
    }

    Texture* GraphicsDevice::Create2DMultisampleTexture(uint32_t width, uint32_t height, PixelFormat format, SampleCount samples, uint32_t arraySize, TextureUsage textureUsage, const void* pInitData)
    {
        ALIMER_ASSERT_MSG(width >= 1, "Width must be greather than 0.");
        ALIMER_ASSERT_MSG(height >= 1, "Height must be greather than 0.");
        ALIMER_ASSERT_MSG(arraySize >= 1 && arraySize <= 2048, "Array size must be between 1 and 2048.");
        ALIMER_ASSERT_MSG(format != PixelFormat::Unknown, "Invalid pixel format.");

        TextureDescriptor descriptor = {};
        descriptor.width = width;
        descriptor.height = height;
        descriptor.depth = 1;
        descriptor.arraySize = arraySize;
        descriptor.mipLevels = 1;
        descriptor.samples = samples;
        descriptor.type = TextureType::Type2D;
        descriptor.format = format;
        descriptor.usage = textureUsage;
        return nullptr;
        //return CreateTextureImpl(&descriptor, nullptr, pInitData);
    }

    Texture* GraphicsDevice::Create3DTexture(uint32_t width, uint32_t height, uint32_t depth, PixelFormat format, uint32_t mipLevels, TextureUsage textureUsage, const void* pInitData)
    {
        ALIMER_ASSERT_MSG(width >= 1, "Width must be greather than 0.");
        ALIMER_ASSERT_MSG(height >= 1, "Height must be greather than 0.");
        ALIMER_ASSERT_MSG(depth >= 1, "Depth must be greather than 0.");
        ALIMER_ASSERT_MSG(format != PixelFormat::Unknown, "Invalid pixel format.");

        TextureDescriptor descriptor = {};
        descriptor.width = width;
        descriptor.height = height;
        descriptor.depth = depth;
        descriptor.arraySize = 1;
        descriptor.mipLevels = mipLevels;
        if (descriptor.mipLevels == 0)
        {
            uint32_t dims = width | height;
            descriptor.mipLevels = bitScanReverse(dims) + 1;
        }
        descriptor.samples = SampleCount::Count1;
        descriptor.type = TextureType::Type3D;
        descriptor.format = format;
        descriptor.usage = UpdateTextureUsage(textureUsage, pInitData != nullptr, mipLevels);
        return nullptr;
        //return CreateTextureImpl(&descriptor, nullptr, pInitData);
    }

    Texture* GraphicsDevice::CreateCubeTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t arraySize, uint32_t mipLevels, TextureUsage textureUsage, const void* pInitData)
    {
        ALIMER_ASSERT_MSG(width >= 1, "Width must be greather than 0.");
        ALIMER_ASSERT_MSG(height >= 1, "Height must be greather than 0.");
        ALIMER_ASSERT_MSG(arraySize >= 1 && arraySize <= 2048, "Array size must be between 1 and 2048.");
        ALIMER_ASSERT_MSG(format != PixelFormat::Unknown, "Invalid pixel format.");

        TextureDescriptor descriptor = {};
        descriptor.width = width;
        descriptor.height = height;
        descriptor.depth = 1;
        descriptor.arraySize = arraySize;
        descriptor.mipLevels = mipLevels;
        if (descriptor.mipLevels == 0)
        {
            uint32_t dims = width | height;
            descriptor.mipLevels = bitScanReverse(dims) + 1;
        }
        descriptor.samples = SampleCount::Count1;
        descriptor.type = TextureType::TypeCube;
        descriptor.format = format;
        descriptor.usage = UpdateTextureUsage(textureUsage, pInitData != nullptr, mipLevels);
        return nullptr;
        //return CreateTextureImpl(&descriptor, nullptr, pInitData);
    }

    Framebuffer* GraphicsDevice::CreateFramebuffer(const FramebufferDescriptor* descriptor)
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

        return nullptr;
        //return CreateFramebufferImpl(descriptor);
    }

    Buffer* GraphicsDevice::CreateBuffer(const BufferDescriptor* descriptor, const void* pInitData)
    {
        ALIMER_ASSERT(descriptor);

        if (descriptor->size == 0)
        {
            ALIMER_LOGERROR("Cannot create empty buffer.");
            return false;
        }

        if (descriptor->usage == BufferUsage::None)
        {
            ALIMER_LOGERROR("Invalid buffer usage.");
            return false;
        }
        return nullptr;
        //return CreateBufferImpl(descriptor, pInitData);
    }

    Sampler* GraphicsDevice::CreateSampler(const SamplerDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);
        return nullptr; 
        //return CreateSamplerImpl(descriptor);
    }

    Shader* GraphicsDevice::CreateShader(const ShaderDescriptor* descriptor)
    {
        ALIMER_ASSERT(descriptor);
        return nullptr; 
        // return CreateShaderImpl(descriptor);
    }
}
