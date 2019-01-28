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

#pragma once

#include "BackendGL.h"

namespace alimer
{
    /// OpenGL gpu backend.
    class GPUDeviceGL final : public GPUDevice
    {
    public:
        /// Is backend supported?
        static bool IsSupported();

        /// Constructor.
        GPUDeviceGL(bool validation, bool headless);

        /// Destructor.
        ~GPUDeviceGL() override;

        void WaitIdle() override;
        bool SetMode(const SwapChainHandle* handle, const SwapChainDescriptor* descriptor) override;

        bool BeginFrame() override;
        void EndFrame() override;

        GPUTexture* CreateTexture(const TextureDescriptor* descriptor, void* nativeTexture, const void* pInitData) override;
        GPUSampler* CreateSampler(const SamplerDescriptor* descriptor) override;
        GPUBuffer* CreateBuffer(const BufferDescriptor* descriptor, const void* pInitData) override;
        //Framebuffer* CreateFramebufferImpl(const FramebufferDescriptor* descriptor) override;
        //Shader* CreateShaderImpl(const ShaderDescriptor* descriptor) override;

    private:
        void InitializeCaps();
    };
}
