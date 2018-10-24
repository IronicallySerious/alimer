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

#include "../Graphics/GraphicsDeviceFeatures.h"

namespace Alimer
{
    GraphicsDeviceFeatures::GraphicsDeviceFeatures()
    {
        Reset();
    }

    void GraphicsDeviceFeatures::Reset()
    {
        _vendorID = 0;
        _vendor = GpuVendor::Unknown;
        _deviceID = 0;
        _deviceName.clear();
        _multithreading = false;
        _maxColorAttachments = 1;
    }

    void GraphicsDeviceFeatures::SetVendorId(uint32_t vendorID)
    {
        _vendorID = vendorID;
        switch (vendorID)
        {
        case 0x13B5:
            _vendor = GpuVendor::Arm;
            break;
        case 0x10DE:
            _vendor = GpuVendor::Nvidia;
            break;
        case 0x1002:
        case 0x1022:
            _vendor = GpuVendor::Amd;
            break;
        case 0x8086:
            _vendor = GpuVendor::Intel;
            break;
        default:
            _vendor = GpuVendor::Unknown;
            break;
        }
    }

    void GraphicsDeviceFeatures::SetDeviceId(uint32_t deviceID)
    {
        _deviceID = deviceID;
    }

    void GraphicsDeviceFeatures::SetDeviceName(const std::string& deviceName)
    {
        _deviceName = deviceName;
    }
}
