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

#include "graphics/GraphicsDeviceFactory.h"

#if defined(_DURANGO) || defined(_XBOX_ONE)
#   define ALIMER_D3D12_DYNAMIC_LIB 0
#elif defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
#   define ALIMER_D3D12_DYNAMIC_LIB 0
#elif defined(_WIN64) || defined(_WIN32)
#   define ALIMER_D3D12_DYNAMIC_LIB 1
#endif

#include <wrl/client.h>

#if defined(NTDDI_WIN10_RS2)
#    include <dxgi1_6.h>
#elif defined(NTDDI_WIN10_RS2)
#    include <dxgi1_5.h>
#else
#   include <dxgi1_4.h>
#endif
#include <d3d12.h>

namespace alimer
{
    /// D3D12 backend
    class ALIMER_API GraphicsDeviceFactoryD3D12 final : public GraphicsDeviceFactory
    {
    public:
        GraphicsDeviceFactoryD3D12(bool validation);
        /// Destructor.
        ~GraphicsDeviceFactoryD3D12() override;

    private:
    };
}
