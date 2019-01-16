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

#include "DeviceGL.h"

namespace alimer
{
    bool DeviceGL::IsSupported()
    {
        static bool availableCheck = false;
        static bool isAvailable = false;

        if (availableCheck)
            return isAvailable;

        availableCheck = true;
        isAvailable = true;
        return true;
    }

    DeviceGL::DeviceGL(bool validation)
        : DeviceBackend(GraphicsBackend::OpenGL, validation)
    {
        InitializeCaps();

        // Create immediate command buffer.
        //_immediateCommandContext = new CommandContextGL(this);
    }

    DeviceGL::~DeviceGL()
    {
    }

    bool DeviceGL::WaitIdle()
    {
        return true;
    }

    void DeviceGL::Tick()
    {

    }

    void DeviceGL::InitializeCaps()
    {
        //_features.SetVendorId(desc.VendorId);
        //_features.SetDeviceId(desc.DeviceId);
        //_features.SetDeviceName(String(desc.Description));
        _features.SetMultithreading(false);

        _limits.maxColorAttachments = 8u;
        _limits.maxBindGroups = 4u;
        _limits.minUniformBufferOffsetAlignment = 16;
    }

    GPUSwapChain* DeviceGL::CreateSwapChain(const SwapChainDescriptor* descriptor)
    {
        return nullptr;
    }

    GPUTexture* DeviceGL::CreateTexture(const TextureDescriptor* descriptor, void* nativeTexture, const void* initialData)
    {
        return nullptr;
    }

    /*Framebuffer* DeviceGL::CreateFramebufferImpl(const FramebufferDescriptor* descriptor)
    {
        return nullptr;
    }

    Buffer* DeviceGL::CreateBufferImpl(const BufferDescriptor* descriptor, const void* initialData)
    {
        return nullptr;
    }*/

    GPUSampler* DeviceGL::CreateSampler(const SamplerDescriptor* descriptor)
    {
        return nullptr;
    }

    /*Shader* DeviceGL::CreateShaderImpl(const ShaderDescriptor* descriptor)
    {
        return nullptr;
    }*/
}
