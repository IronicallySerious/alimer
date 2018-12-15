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

#pragma once

#include "../Graphics/Types.h"
#include "../Graphics/GraphicsDeviceFeatures.h"

namespace Alimer
{
    struct TextureData
    {
        /// Pointer to pixel data.
        const void* data;

        /// Single row pitch.
        uint32_t rowPitch = 0;
    };

    class GPUShaderModule
    {
    protected:
        /// Constructor.
        GPUShaderModule() {}

    public:
        virtual ~GPUShaderModule() = default;
    };

    class GPUTexture
    {
    protected:
        /// Constructor.
        GPUTexture(const TextureDescriptor* descriptor)
        {
            memcpy(&_descriptor, descriptor, sizeof(TextureDescriptor));
        }

    public:
        virtual ~GPUTexture() = default;

        const TextureDescriptor& GetDescriptor() const { return _descriptor; }

    protected:
        TextureDescriptor   _descriptor{};
    };

    class GPUSampler
    {
    protected:
        /// Constructor.
        GPUSampler() {}

    public:
        virtual ~GPUSampler() = default;
    };

    struct RenderWindowDescriptor;
    class RenderWindow;
    class CommandContext;

    class GPUDevice
    {
    protected:
        /// Constructor.
        GPUDevice() {}

    public:
        virtual ~GPUDevice() = default;

        virtual bool Initialize(const RenderWindowDescriptor* mainWindowDescriptor) = 0;
        virtual bool WaitIdle() = 0;

        virtual RenderWindow* GetMainWindow() const = 0;
        virtual CommandContext* GetImmediateContext() const = 0;

        //virtual GPUShaderModule* CreateShaderModule(ShaderStage stage, const Vector<uint8_t>& bytecode) = 0;
        virtual GPUTexture* CreateTexture(const TextureDescriptor* descriptor, const TextureData* initialData) = 0;
        virtual GPUSampler* CreateSampler(const SamplerDescriptor* descriptor) = 0;

        const GraphicsDeviceFeatures& GetFeatures() const
        {
            return _features;
        }

    protected:
        GraphicsDeviceFeatures _features = {};
    };
}
