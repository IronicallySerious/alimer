//
// Alimer is based on the Turso3D codebase.
// Copyright (c) 2018-2019 Amer Koleci and contributors.
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

#include "Turso3DPlatform.h"

/// %Turso3D engine namespace.
namespace Turso3D
{
}

#ifdef _MSC_VER
#   pragma warning(disable:4481)
#endif

// Shared library exports
/* #undef TURSO3D_SHARED */
#if defined(_WIN32) && defined(TURSO3D_SHARED)
#   ifdef _MSC_VER
#       pragma warning(disable:4251)
#   endif
#   ifdef TURSO3D_EXPORTS
#       define TURSO3D_API __declspec(dllexport)
#   else
#       define TURSO3D_API __declspec(dllimport)
#   endif
#else
#   define TURSO3D_API
#endif

// Turso3D build configuration
#define TURSO3D_LOGGING
#define TURSO3D_PROFILING
#define TURSO3D_D3D11
/* #undef TURSO3D_OPENGL */
