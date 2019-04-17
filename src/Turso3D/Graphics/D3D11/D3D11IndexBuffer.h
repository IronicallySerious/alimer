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

#include "../../Base/AutoPtr.h"
#include "../GPUObject.h"
#include "../GraphicsDefs.h"

namespace Turso3D
{
    /// GPU buffer for index data.
    class TURSO3D_API IndexBuffer : public RefCounted, public GPUObject
    {
    public:
        /// Construct.
        IndexBuffer();
        /// Destruct.
        ~IndexBuffer();

        /// Release the index buffer and CPU shadow data.
        void Release() override;

        /// Define buffer. Immutable buffers must specify initial data here.  Return true on success.
        bool Define(ResourceUsage usage, size_t numIndices, size_t indexSize, bool useShadowData, const void* data = nullptr);
        /// Redefine buffer data either completely or partially. Not supported for immutable buffers. Return true on success.
        bool SetData(size_t firstIndex, size_t numIndices, const void* data);

        /// Return CPU-side shadow data if exists.
        unsigned char* ShadowData() const { return shadowData.Get(); }
        /// Return number of indices.
        size_t NumIndices() const { return numIndices; }
        /// Return size of index in bytes.
        size_t IndexSize() const { return indexSize; }
        /// Return resource usage type.
        ResourceUsage Usage() const { return usage; }
        /// Return whether is dynamic.
        bool IsDynamic() const { return usage == ResourceUsage::Dynamic; }
        /// Return whether is immutable.
        bool IsImmutable() const { return usage == ResourceUsage::Immutable; }

        /// Return the D3D11 buffer. Used internally and should not be called by portable application code.
        void* D3DBuffer() const { return buffer; }

    private:
        /// Create the GPU-side index buffer. Return true on success.
        bool Create(const void* data);

        /// D3D11 buffer.
        void* buffer;
        /// CPU-side shadow data.
        AutoArrayPtr<unsigned char> shadowData;
        /// Number of indices.
        size_t numIndices;
        /// Size of index in bytes.
        size_t indexSize;
        /// Resource usage type.
        ResourceUsage usage = ResourceUsage::Default;
    };

}
