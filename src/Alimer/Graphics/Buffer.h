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

#include "../Base/Ptr.h"
#include "../Base/String.h"
#include "../Graphics/GPUResource.h"

namespace alimer
{
    struct GPUBuffer;

	/// Defines a GPU Buffer class.
	class ALIMER_API Buffer : public GPUResource
	{
        ALIMER_OBJECT(Buffer, GPUResource);

	protected:
        /// Constructor.
        Buffer();

    public:
        /// Desturctor
        ~Buffer() override;

        void Destroy() override;

        /// Replace entire buffer data in synchronous way.
        bool SetSubData(const void* pData);

        /// Replace buffer data in synchronous way.
        bool SetSubData(uint32_t offset, uint32_t size, const void* pData);

        /// Get the buffer usage flags.
        BufferUsage GetUsage() const { return _descriptor.usage; }

        /// Get single element size.
        uint32_t GetElementSize() const { return _descriptor.stride; }

        /// Get size in bytes of the buffer.
        uint64_t GetSize() const { return _descriptor.size; }

        /// Get the texture description.
        const BufferDescriptor& GetDescriptor() const { return _descriptor; }

        /// Get the GPUBuffer.
        GPUBuffer* GetGPUBuffer() const { return _buffer; }

    protected:
        bool CreateGPUBuffer(bool useShadowData, const void* data);

        GPUBuffer* _buffer = nullptr;
        BufferDescriptor _descriptor{};
        /// CPU-side shadow data.
        AutoArrayPtr<uint8_t> _shadowData;
	};

    /// Defines a vertex buffer.
    class VertexBuffer final : public Buffer
    {
    public:
        /// Constructor.
        VertexBuffer();

        bool Define(uint32_t vertexCount, const PODVector<VertexElement>& elements, bool useShadowData, const void* data = nullptr);
        bool Define(uint32_t vertexCount, uint32_t elementsCount, const VertexElement* elements, bool useShadowData, const void* data = nullptr);

        /// Return number of vertex elements.
        uint32_t GetElementsCount() const { return _elements.Size(); }

        /// Return vertex elements.
        const PODVector<VertexElement>& GetElements() const { return _elements; }

    private:
        PODVector<VertexElement> _elements;
    };

    /// Defines a index buffer.
    class IndexBuffer final : public Buffer
    {
    public:
        /// Constructor.
        IndexBuffer();

        bool Define(uint32_t indexCount, IndexType indexType, bool useShadowData, const void* data = nullptr);

        /// Return number of indices.
        uint32_t GetIndexCount() const { return _indexCount; }
        /// Return the type of index.
        IndexType GetIndexType() const { return _indexType; }

    private:
        /// Number of indices.
        uint32_t _indexCount = 0;
        /// Type of index.
        IndexType _indexType = IndexType::UInt16;
    };
}
