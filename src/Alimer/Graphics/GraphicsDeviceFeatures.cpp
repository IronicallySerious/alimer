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

#include "../Graphics/GraphicsDeviceFeatures.h"

namespace alimer
{
    GraphicsDeviceFeatures::GraphicsDeviceFeatures()
    {
    }

    void GraphicsDeviceFeatures::SetVendorId(uint32_t vendorId)
    {
        _vendorId = vendorId;

        // Find vendor
        switch (vendorId)
        {
        case 0x13B5:
            _vendor = GpuVendor::ARM;
            break;
        case 0x10DE:
            _vendor = GpuVendor::NVIDIA;
            break;
        case 0x1002:
        case 0x1022:
            _vendor = GpuVendor::AMD;
            break;
        case 0x163C:
        case 0x8086:
            _vendor = GpuVendor::INTEL;
            break;
        default:
            _vendor = GpuVendor::Unknown;
            break;
        }
    }
}
