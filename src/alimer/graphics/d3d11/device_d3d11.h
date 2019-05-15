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

#include "graphics/graphics_device.h"

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <wrl/client.h>

#define D3D11_NO_HELPERS
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <stdexcept>

namespace alimer
{
    /// D3D11 graphics device.
    class ALIMER_API DeviceD3D11 final : public GraphicsDevice
    {
    public:
        DeviceD3D11();

        /// Destructor.
        ~DeviceD3D11() override;

    private:
        D3D_FEATURE_LEVEL                               _featureLevel;
        Microsoft::WRL::ComPtr<ID3D11Device1>           _d3dDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    _d3dContext;
    };

    namespace d3d11
    {
        inline void ThrowIfFailed(HRESULT hr)
        {
            if (FAILED(hr))
            {
                // Set a breakpoint on this line to catch DirectX API errors
                throw std::exception();
            }
        }
    }
} 
