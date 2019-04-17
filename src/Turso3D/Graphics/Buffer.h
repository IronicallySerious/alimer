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

#include "../Graphics/GPUObject.h"
#include "../Graphics/GraphicsDefs.h"

#ifdef TURSO3D_D3D11
struct ID3D11Buffer;
#endif

namespace Turso3D
{
    /// GPU base buffer class.
    class TURSO3D_API Buffer : public RefCounted, public GPUObject
    {
    protected:
        /// Constructor.
        Buffer(BufferType bufferType_);

    public:
        /// Destructor.
        ~Buffer();

        /// Release the vertex buffer and CPU shadow data.
        void Release() override;

        /// Return resource usage type.
        ResourceUsage Usage() const { return usage; }
        /// Return whether is dynamic.
        bool IsDynamic() const { return usage == ResourceUsage::Dynamic; }
        /// Return whether is immutable.
        bool IsImmutable() const { return usage == ResourceUsage::Immutable; }

#ifdef TURSO3D_D3D11
        /// Return the D3D11 buffer. Used internally and should not be called by portable application code.
        ID3D11Buffer* GetHandle() const { return _handle; }
#endif

    protected:
        /// Create the GPU-side buffer. Return true on success.
        bool Create(const void* data);

    protected:
#ifdef TURSO3D_D3D11
        ID3D11Buffer* _handle = nullptr;
#endif
        /// Type of buffer.
        BufferType bufferType;

        /// Resource usage type.
        ResourceUsage usage = ResourceUsage::Default;

        /// Size in bytes
        uint32_t sizeInBytes = 0;
    };
}
