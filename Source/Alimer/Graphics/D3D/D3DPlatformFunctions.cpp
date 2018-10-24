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

#include "../../AlimerConfig.h"

#if ALIMER_COMPILE_D3D11 || ALIMER_COMPILE_D3D12
#include "D3DPlatformFunctions.h"

namespace Alimer
{
    D3DPlatformFunctions::D3DPlatformFunctions()
    {

    }

    D3DPlatformFunctions::~D3DPlatformFunctions()
    {
        if (_d3d12Lib)
        {
            FreeLibrary(_d3d12Lib);
            _d3d12Lib = nullptr;
        }

        if (_d3dCompilerLib)
        {
            FreeLibrary(_d3dCompilerLib);
            _d3dCompilerLib = nullptr;
        }
    }

    bool D3DPlatformFunctions::LoadFunctions(bool loadD3D12)
    {
        _d3dCompilerLib = LoadLibraryW(L"d3dcompiler_47.dll");
        _d3d12Lib = LoadLibraryW(L"d3d12.dll");

        d3dCompile = reinterpret_cast<pD3DCompile>(GetProcAddress(_d3dCompilerLib, "D3DCompile"));
        if (d3dCompile == nullptr)
            return false;

        return true;
    }
}
#endif /* ALIMER_COMPILE_D3D11 || ALIMER_COMPILE_D3D12 */
