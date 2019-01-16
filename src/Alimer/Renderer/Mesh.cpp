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

#include "../Renderer/Mesh.h"
#include "../Graphics/GPUDevice.h"

namespace alimer
{
    Mesh::Mesh()
    {
        
    }

    Mesh::~Mesh()
    {
    }

    bool Mesh::Define(const PODVector<vec3>& positions, const PODVector<Color4>& colors, const PODVector<uint16_t>& indices)
    {
        //auto graphics = Object::GetSubsystem<Graphics>();
        _vertexBuffer.Reset();
        _indexBuffer.Reset();

        _vertexCount = positions.Size();
        _indexCount = indices.Size();

        // Create vertex buffer.
        _vertexStride = sizeof(vec3) + sizeof(Color4);

        struct VertexColor
        {
            vec3 position;
            Color4 color;
        };

        std::vector<VertexColor> vertices;

        for (uint32_t i = 0; i < _vertexCount; ++i)
        {
            vertices.push_back({ positions[i], colors[i] });
        }

        /*BufferDescriptor vertexBufferDesc = {};
        vertexBufferDesc.resourceUsage = ResourceUsage::Default;
        vertexBufferDesc.usage = BufferUsage::Vertex;
        vertexBufferDesc.size = positions.size() * _vertexStride;
        vertexBufferDesc.stride = _vertexStride;
        //_vertexBuffer = _graphicsDevice->CreateBuffer( &vertexBufferDesc, vertices.data());

        // Create index buffer.
        if (indices.size())
        {
            BufferDescriptor indexBufferDesc = {};
            indexBufferDesc.resourceUsage = ResourceUsage::Default;
            indexBufferDesc.usage = BufferUsage::Index;
            indexBufferDesc.size = indices.size() * sizeof(uint16_t);
            indexBufferDesc.stride = sizeof(uint16_t);
            //_indexBuffer = _graphicsDevice->CreateBuffer(&indexBufferDesc, indices.data());
        }*/

        return true;
    }

    void Mesh::SetVertexData(const void* vertexData, uint32_t vertexStart, uint32_t vertexCount)
    {
        if (vertexStart == 0
            && vertexCount == 0)
        {
            _vertexBuffer->SetSubData(0, _vertexStride * _vertexCount, vertexData);
        }
        else
        {
            if (vertexCount == 0)
            {
                vertexCount = _vertexCount - vertexStart;
            }

            _vertexBuffer->SetSubData(vertexStart * _vertexStride, vertexCount * _vertexStride, vertexData);
        }
    }

    void Mesh::Draw(CommandContext& context, uint32_t instanceCount)
    {
        ALIMER_ASSERT(instanceCount >= 1);

        /*context->BindVertexBuffer(_vertexBuffer, 0);

        // TODO: Add VertexInputFormat
        //context->SetVertexInputFormat();

        if (_indexBuffer)
        {
            context->BindIndexBuffer(_indexBuffer, 0, _indexType);
            context->DrawIndexed(PrimitiveTopology::Triangles, _indexCount, instanceCount, 0u);
        }
        else
        {
            context->Draw(PrimitiveTopology::Triangles, _vertexCount, instanceCount, 0u, 0u);
        }*/
    }

    Mesh* Mesh::CreateCube(float size)
    {
        return CreateBox(vec3(size));
    }

    Mesh* Mesh::CreateBox(const vec3& size)
    {
        // A box has six faces, each one pointing in a different direction.
        const int FaceCount = 6;

        static const vec3 faceNormals[FaceCount] =
        {
            vec3(0,  0,  1),
            vec3(0,  0, -1),
            vec3(1,  0,  0),
            vec3(-1,  0,  0),
            vec3(0,  1,  0),
            vec3(0, -1,  0),
        };

        PODVector<vec3> positions;
        PODVector<Color4> colors;
        PODVector<uint16_t> indices;

        vec3 tsize = size / 2.0f;

        // Create each face in turn.
        for (int i = 0; i < FaceCount; i++)
        {
            vec3 normal = faceNormals[i];

            // Get two vectors perpendicular both to the face normal and to each other.
            vec3 basis = (i >= 4) ? vec3::unit_z() : vec3::unit_y();

            vec3 side1 = cross(normal, basis);
            vec3 side2 = cross(normal, side1);

            // Six indices (two triangles) per face.
            size_t vbase = positions.Size();
            indices.Push(static_cast<uint16_t>(vbase + 0));
            indices.Push(static_cast<uint16_t>(vbase + 1));
            indices.Push(static_cast<uint16_t>(vbase + 2));

            indices.Push(static_cast<uint16_t>(vbase + 0));
            indices.Push(static_cast<uint16_t>(vbase + 2));
            indices.Push(static_cast<uint16_t>(vbase + 3));

            // Four vertices per face.
            // (normal - side1 - side2) * tsize // normal // t0
            positions.Push((normal - side1 - side2) * tsize);
            colors.Push(Color4(1.0f, 0.0f, 0.0f));

            // (normal - side1 + side2) * tsize // normal // t1
            positions.Push((normal - side1 + side2) * tsize);
            colors.Push(Color4(0.0f, 1.0f, 0.0f));

            // (normal + side1 + side2) * tsize // normal // t2
            positions.Push((normal + side1 + side2) * tsize);
            colors.Push(Color4(0.0f, 0.0f, 1.0f));

            // (normal + side1 - side2) * tsize // normal // t3
            positions.Push((normal + side1 - side2) * tsize);
            colors.Push(Color4(1.0f, 0.0f, 1.0f));
        }

        Mesh* newMesh = new Mesh();
        newMesh->Define(positions, colors, indices);
        return newMesh;
    }
}
