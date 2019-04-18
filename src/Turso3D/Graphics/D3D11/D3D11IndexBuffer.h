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
#include "../Buffer.h"

namespace Turso3D
{
    /// GPU buffer for index data.
    class TURSO3D_API IndexBuffer : public Buffer
    {
    public:
        /// Construct.
        IndexBuffer();
        /// Destruct.
        ~IndexBuffer();

        /// Define buffer. Immutable buffers must specify initial data here.  Return true on success.
        bool Define(ResourceUsage usage, uint32_t indexCount, IndexType indexType, bool useShadowData = false, const void* data = nullptr);
        /// Redefine buffer data either completely or partially. Not supported for immutable buffers. Return true on success.
        bool SetData(const void* data, uint32_t indexStart, uint32_t indexCount);

        /// Return number of indices.
        uint32_t NumIndices() const { return _indexCount; }
        /// Get the index type.
        IndexType GetIndexType() const { return _indexType; }
        /// Return size of index in bytes.
        uint32_t IndexSize() const { return _indexType == IndexType::UInt32 ? 4u : 2u; }

    private:
        /// Number of indices.
        uint32_t _indexCount = 0;
        /// Size of index in bytes.
        IndexType _indexType = IndexType::UInt16;
        /// Resource usage type.
        ResourceUsage usage = ResourceUsage::Default;
    };

}
