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

#include "../Graphics/Backend.h"
#include "../Graphics/GPUResource.h"

namespace alimer
{
    /// Defines a Sampler class.
    class ALIMER_API Sampler final : public GPUResource
    {
        friend class GraphicsDevice;

    public:
        /// Constructor.
        Sampler(GraphicsDevice* device, const SamplerDescriptor* descriptor);

        /// Destructor.
        ~Sampler();

        void Destroy() override;

        /// Get the addressing mode for the U texcoord.
        SamplerAddressMode GetAddressModeU() const { return _descriptor.addressModeU; }

        /// Get the addressing mode for the V texcoord.
        SamplerAddressMode GetAddressModeV() const { return _descriptor.addressModeV; }

        /// Get the addressing mode for the W texcoord.
        SamplerAddressMode GetAddressModeW() const { return _descriptor.addressModeW; }

        /// Get the sampler description.
        const SamplerDescriptor &GetDescriptor() const { return _descriptor; }

    private:
        /// Create backend instance.
        bool Create();

        VgpuSampler _handle = 0;

        SamplerDescriptor _descriptor{};
    };
}
