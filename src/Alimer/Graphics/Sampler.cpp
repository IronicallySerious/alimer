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

#include "../Graphics/Sampler.h"
#include "../Graphics/GraphicsDevice.h"

namespace alimer
{
    Sampler::Sampler(GraphicsDevice* device, const SamplerDescriptor* descriptor)
        : GPUResource(device, Type::Sampler)
    {
        memcpy(&_descriptor, descriptor, sizeof(SamplerDescriptor));
    }

    Sampler::~Sampler()
    {
        Destroy();
    }

    void Sampler::Destroy()
    {
        if (_handle != nullptr)
        {
            //_device->DestroySampler(_handle);
            vgpuDestroySampler(_handle);
            _handle = nullptr;
        }
    }

    bool Sampler::Create()
    {
        VgpuSamplerDescriptor descriptor = {};
        descriptor.addressModeU = (VgpuSamplerAddressMode)_descriptor.addressModeU;
        descriptor.addressModeV = (VgpuSamplerAddressMode)_descriptor.addressModeV;
        descriptor.addressModeW = (VgpuSamplerAddressMode)_descriptor.addressModeW;
        descriptor.magFilter = (VgpuSamplerMinMagFilter)_descriptor.magFilter;
        descriptor.minFilter = (VgpuSamplerMinMagFilter)_descriptor.minFilter;
        descriptor.mipmapFilter = (VgpuSamplerMipFilter)_descriptor.mipmapFilter;
        descriptor.maxAnisotropy = _descriptor.maxAnisotropy;
        descriptor.compareOp = (VgpuCompareOp)_descriptor.compareOp;
        descriptor.borderColor = (VgpuSamplerBorderColor) _descriptor.borderColor;
        descriptor.lodMinClamp = _descriptor.lodMinClamp;
        descriptor.lodMaxClamp = _descriptor.lodMaxClamp;

        if (vgpuCreateSampler(&descriptor, &_handle) != VGPU_SUCCESS)
        {
            ALIMER_LOGERROR("Vulkan: Failed to create sampler");
            return false;
        }

        return true;
    }
}
