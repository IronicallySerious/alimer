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

#include <vector>
#include "../Resource/Resource.h"
#include "../Graphics/Buffer.h"
#include "../Graphics/Types.h"

namespace alimer
{
    class Buffer;
    class CommandBuffer;

    enum class MeshAttribute : unsigned
    {
        Position = 0,
        UV = 1,
        Normal = 2,
        Tangent = 3,
        BoneIndex = 4,
        BoneWeights = 5,
        VertexColor = 6,
        Count,
        None
    };

    struct MeshAttributeLayout
    {
        VertexFormat format = VertexFormat::Unknown;
        uint32_t offset = 0;
    };

    /// Defines a Mesh.
    class ALIMER_API Mesh final : public Resource
    {
        ALIMER_OBJECT(Mesh, Resource);

    public:
        Mesh();
        ~Mesh() override;

        bool Define(const std::vector<vec3>& positions, const std::vector<Color4>& colors, const std::vector<uint16_t>& indices);

        void SetVertexData(const void* vertexData, uint32_t vertexStart = 0, uint32_t vertexCount = 0);

        void Draw(CommandBuffer& commandBuffer, uint32_t instanceCount = 1);

        uint32_t GetIndexCount() { return _indexCount; }
        uint32_t GetVertexCount() { return _vertexCount; }

        Buffer* GetVertexBuffer() const { return _vertexBuffer.Get(); }
        Buffer* GetIndexBuffer() const { return _indexBuffer.Get(); }

        static Mesh* CreateCube(float size = 1.0f);
        static Mesh* CreateBox(const vec3& size = vec3(1.0f));

    private:
        MeshAttributeLayout _attributes[ecast(MeshAttribute::Count)];

        SharedPtr<Buffer> _vertexBuffer;
        SharedPtr<Buffer> _indexBuffer;

        uint32_t _vertexCount = 0;
        uint32_t _vertexStride = 0;
        uint32_t _indexCount = 0;
        uint32_t _indexStride = 2;
    };
}
