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

#include "SamplerD3D11.h"
#include "DeviceD3D11.h"
#include "Graphics/D3D/D3DConvert.h"
#include "D3D11Convert.h"
#include "Core/Log.h"
using namespace Microsoft::WRL;

namespace alimer
{
    SamplerD3D11::SamplerD3D11(DeviceD3D11* device, const SamplerDescriptor* descriptor)
        : Sampler(device, descriptor)
    {
        auto &cache = device->GetCache();
        _samplerState = cache.GetSamplerState(descriptor);
    }

    SamplerD3D11::~SamplerD3D11()
    {
        Destroy();
    }

    void SamplerD3D11::Destroy()
    {
        // State is destroyed by cache.
    }
}
