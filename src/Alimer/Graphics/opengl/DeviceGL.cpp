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
    bool GPUDeviceGL::IsSupported()
    {
        static bool availableCheck = false;
        static bool isAvailable = false;

        if (availableCheck)
            return isAvailable;

        availableCheck = true;
        isAvailable = true;
        return true;
    }

    GPUDeviceGL::GPUDeviceGL(bool validation, bool headless)
        : GraphicsDevice(GraphicsBackend::OpenGL, validation, headless)
    {
        InitializeCaps();

        // Create immediate command buffer.
        //_immediateCommandContext = new CommandContextGL(this);

        //OnAfterCreated();
    }

    GPUDeviceGL::~GPUDeviceGL()
    {
    }

    void GPUDeviceGL::InitializeCaps()
    {
        //_features.SetVendorId(desc.VendorId);
        //_features.SetDeviceId(desc.DeviceId);
        //_features.SetDeviceName(String(desc.Description));
        _features.SetMultithreading(false);

        _features.SetMaxColorAttachments(8u);
        _features.SetMaxBindGroups(4u);
        _features.SetMinUniformBufferOffsetAlignment(16);
    }

    void GPUDeviceGL::WaitIdle()
    {

    }

    /*bool GPUDeviceGL::SetMode(const SwapChainHandle* handle, const SwapChainDescriptor* descriptor)
    {
        return false;
    }

    bool GPUDeviceGL::BeginFrame()
    {
        return false;
    }

    void GPUDeviceGL::EndFrame()
    {

    }

    GPUTexture* GPUDeviceGL::CreateTexture(const TextureDescriptor* descriptor, void* nativeTexture, const void* pInitData)
    {
        return nullptr;
    }

    GPUSampler* GPUDeviceGL::CreateSampler(const SamplerDescriptor* descriptor) 
    {
        return nullptr;
    }

    GPUBuffer* GPUDeviceGL::CreateBuffer(const BufferDescriptor* descriptor, const void* pInitData)
    {
        return nullptr;
    }*/
}
