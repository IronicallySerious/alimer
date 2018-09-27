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

#include "D3D11GpuAdapter.h"
#include "../../Base/String.h"
using namespace Microsoft::WRL;

namespace Alimer
{
    D3D11GpuAdapter::D3D11GpuAdapter(IDXGIAdapter1* adapter)
        : _adapter(adapter)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        switch (desc.VendorId)
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

        case 0x1414:
            _vendor = GpuVendor::Warp;
            break;

        default:
            _vendor = GpuVendor::Unknown;
            break;
        }

        _vendorID = desc.VendorId;
        _deviceID = desc.DeviceId;
        _deviceName.SetUTF8FromWChar(desc.Description);
    }

    D3D11GpuAdapter::~D3D11GpuAdapter()
    {
        SafeRelease(_adapter, "IDXGIAdapter1");
    }
}
