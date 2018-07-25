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

#include "Alimer.h"
using namespace Alimer;

namespace Alimer
{
    struct VertexColor
    {
        Vector3 position;
        Color color;
    };

    struct VertexColorTexture
    {
        Vector3 position;
        Color color;
        Vector2 textureCoordinate;
    };

    class TriangleExample
    {
    public:
        void Initialize(Graphics* graphics)
        {
            VertexColor triangleVertices[] =
            {
                { vec3(0.0f, 0.5f, 0.0f), Color::Red },
                { vec3(0.5f, -0.5f, 0.0f), Color::Lime },
                { vec3(-0.5f, -0.5f, 0.0f), Color::Blue }
            };


            std::vector<VertexElement> vertexDeclaration;
            vertexDeclaration.emplace_back(VertexFormat::Float3, VertexElementSemantic::POSITION);
            vertexDeclaration.emplace_back(VertexFormat::Float4, VertexElementSemantic::COLOR);

            _vertexBuffer = new VertexBuffer(graphics);
            _vertexBuffer->Define(3, vertexDeclaration, ResourceUsage::Immutable, triangleVertices);

            // Create shader.
            _shader = graphics->CreateShader("assets://shaders/color.vert", "assets://shaders/color.frag");

            _camera.viewMatrix = Matrix4x4::Identity;
            _camera.projectionMatrix = Matrix4x4::Identity;
            GpuBufferDescription uboBufferDesc = {};
            uboBufferDesc.usage = BufferUsage::Uniform;
            uboBufferDesc.elementSize = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = new GpuBuffer(graphics, uboBufferDesc, &_camera);
        }

        void Render(CommandBuffer* commandBuffer)
        {
            commandBuffer->BeginRenderPass(nullptr, Color(0.0f, 0.2f, 0.4f, 1.0f));
            commandBuffer->SetShader(_shader.Get());
            commandBuffer->SetVertexBuffer(0, _vertexBuffer.Get());
            commandBuffer->SetUniformBuffer(0, 0, _perCameraUboBuffer.Get());
            commandBuffer->Draw(PrimitiveTopology::Triangles, 3);
            commandBuffer->EndRenderPass();
        }

    private:
        SharedPtr<VertexBuffer> _vertexBuffer;
        SharedPtr<Shader> _shader;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;

        struct PerCameraCBuffer
        {
            mat4 viewMatrix;
            mat4 projectionMatrix;
        };

        PerCameraCBuffer _camera;
    };

    class QuadExample
    {
    public:
        void Initialize(Graphics* graphics)
        {
            VertexColor triangleVertices[] =
            {
                { Vector3(-0.5f, 0.5f, 0.0f), Color(0.0f, 0.0f, 1.0f, 1.0f) },
                { Vector3(0.5f, 0.5f, 0.0f), Color(1.0f, 0.0f, 0.0f, 1.0f) },
                { Vector3(0.5f, -0.5f, 0.0f), Color(0.0f, 1.0f, 0.0f, 1.0f) },
                { Vector3(-0.5f, -0.5f, 0.0f), Color(1.0f, 1.0f, 0.0f, 1.0f) },
            };

            std::vector<VertexElement> vertexDeclaration;
            vertexDeclaration.emplace_back(VertexFormat::Float3, VertexElementSemantic::POSITION);
            vertexDeclaration.emplace_back(VertexFormat::Float4, VertexElementSemantic::COLOR);
            _vertexBuffer = new VertexBuffer(graphics);
            _vertexBuffer->Define(4, vertexDeclaration, ResourceUsage::Immutable, triangleVertices);

            // Create index buffer.
            const uint16_t indices[] = {
                0, 1, 2, 0, 2, 3
            };
            _indexBuffer = new IndexBuffer(graphics);
            _indexBuffer->Define(6, IndexType::UInt16, ResourceUsage::Immutable, indices);

            _shader = graphics->CreateShader("assets://shaders/color.vert", "assets://shaders/color.frag");

            _camera.viewMatrix = Matrix4x4::Identity;
            _camera.projectionMatrix = Matrix4x4::Identity;
            GpuBufferDescription uboBufferDesc = {};
            uboBufferDesc.usage = BufferUsage::Uniform;
            uboBufferDesc.elementSize = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = new GpuBuffer(graphics, uboBufferDesc, &_camera);
        }

        void Render(CommandBuffer* commandBuffer)
        {
            commandBuffer->BeginRenderPass(nullptr, Color(0.0f, 0.2f, 0.4f, 1.0f));
            commandBuffer->SetShader(_shader.Get());
            commandBuffer->SetVertexBuffer(0, _vertexBuffer.Get());
            commandBuffer->SetIndexBuffer(_indexBuffer.Get());
            commandBuffer->SetUniformBuffer(0, 0, _perCameraUboBuffer.Get());
            commandBuffer->DrawIndexed(PrimitiveTopology::Triangles, 6);
            commandBuffer->EndRenderPass();
        }

    private:
        SharedPtr<VertexBuffer> _vertexBuffer;
        SharedPtr<IndexBuffer> _indexBuffer;
        SharedPtr<Shader> _shader;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;

        struct PerCameraCBuffer
        {
            mat4 viewMatrix;
            mat4 projectionMatrix;
        };

        PerCameraCBuffer _camera;
    };

    class CubeExample
    {
    public:
        void Initialize(Graphics* graphics, float aspectRatio)
        {
            // A box has six faces, each one pointing in a different direction.
            const int FaceCount = 6;

            static const Vector3 faceNormals[FaceCount] =
            {
                Vector3(0,  0,  1),
                Vector3(0,  0, -1),
                Vector3(1,  0,  0),
                Vector3(-1,  0,  0),
                Vector3(0,  1,  0),
                Vector3(0, -1,  0),
            };

            std::vector<VertexColor> vertices;
            std::vector<uint16_t> indices;

            Vector3 tsize(1.0f, 1.0f, 1.0f);
            tsize = tsize / 2.0f;

            // Create each face in turn.
            for (int i = 0; i < FaceCount; i++)
            {
                Vector3 normal = faceNormals[i];

                // Get two vectors perpendicular both to the face normal and to each other.
                Vector3 basis = (i >= 4) ? Vector3::UnitZ : Vector3::UnitY;

                Vector3 side1 = Vector3::Cross(normal, basis);
                Vector3 side2 = Vector3::Cross(normal, side1);

                // Six indices (two triangles) per face.
                size_t vbase = vertices.size();
                indices.push_back(static_cast<uint16_t>(vbase + 0));
                indices.push_back(static_cast<uint16_t>(vbase + 1));
                indices.push_back(static_cast<uint16_t>(vbase + 2));

                indices.push_back(static_cast<uint16_t>(vbase + 0));
                indices.push_back(static_cast<uint16_t>(vbase + 2));
                indices.push_back(static_cast<uint16_t>(vbase + 3));

                // Four vertices per face.
                // (normal - side1 - side2) * tsize // normal // t0
                vertices.push_back({ Vector3::Multiply(Vector3::Subtract(Vector3::Subtract(normal, side1), side2), tsize), Color(1.0f, 0.0f, 0.0f) });

                // (normal - side1 + side2) * tsize // normal // t1
                vertices.push_back({ Vector3::Multiply(Vector3::Add(Vector3::Subtract(normal, side1), side2), tsize), Color(0.0f, 1.0f, 0.0f) });

                // (normal + side1 + side2) * tsize // normal // t2
                vertices.push_back({ Vector3::Multiply(Vector3::Add(normal, Vector3::Add(side1, side2)), tsize), Color(0.0f, 0.0f, 1.0f) });

                // (normal + side1 - side2) * tsize // normal // t3
                vertices.push_back({ Vector3::Multiply(Vector3::Subtract(Vector3::Add(normal, side1), side2), tsize), Color(1.0f, 0.0f, 1.0f) });
            }

            std::vector<VertexElement> vertexDeclaration;
            vertexDeclaration.emplace_back(VertexFormat::Float3, VertexElementSemantic::POSITION);
            vertexDeclaration.emplace_back(VertexFormat::Float4, VertexElementSemantic::COLOR);
            _vertexBuffer = new VertexBuffer(graphics);
            _vertexBuffer->Define(static_cast<uint32_t>(vertices.size()), vertexDeclaration, ResourceUsage::Immutable, vertices.data());

            _indexBuffer = new IndexBuffer(graphics);
            _indexBuffer->Define(static_cast<uint32_t>(indices.size()), IndexType::UInt16, ResourceUsage::Immutable, indices.data());

            _shader = graphics->CreateShader("assets://shaders/color.vert", "assets://shaders/color.frag");

            _camera.viewMatrix = Matrix4x4::CreateLookAt(Vector3(0, 0, 5), Vector3::Zero, Vector3::UnitY);
            _camera.projectionMatrix = Matrix4x4::CreatePerspectiveFieldOfView(M_PIDIV4, aspectRatio, 0.1f, 100);
            GpuBufferDescription uboBufferDesc = {};
            uboBufferDesc.usage = BufferUsage::Uniform;
            uboBufferDesc.elementSize = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = new GpuBuffer(graphics, uboBufferDesc, &_camera);
        }

        void Render(CommandBuffer* commandBuffer)
        {
            commandBuffer->BeginRenderPass(nullptr, Color(0.0f, 0.2f, 0.4f, 1.0f));
            commandBuffer->SetShader(_shader.Get());
            commandBuffer->SetVertexBuffer(0, _vertexBuffer.Get());
            commandBuffer->SetIndexBuffer(_indexBuffer.Get());
            commandBuffer->SetUniformBuffer(0, 0, _perCameraUboBuffer.Get());
            commandBuffer->DrawIndexed(PrimitiveTopology::Triangles, 6);
            commandBuffer->EndRenderPass();
        }

    private:
        SharedPtr<VertexBuffer> _vertexBuffer;
        SharedPtr<IndexBuffer> _indexBuffer;
        SharedPtr<Shader> _shader;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;

        struct PerCameraCBuffer
        {
            mat4 viewMatrix;
            mat4 projectionMatrix;
        };

        PerCameraCBuffer _camera;
    };

    class TexturedCubeExample
    {
    public:
        void Initialize(Graphics* graphics, float aspectRatio)
        {
            // A box has six faces, each one pointing in a different direction.
            const int FaceCount = 6;

            static const Vector3 faceNormals[FaceCount] =
            {
                Vector3(0,  0,  1),
                Vector3(0,  0, -1),
                Vector3(1,  0,  0),
                Vector3(-1,  0,  0),
                Vector3(0,  1,  0),
                Vector3(0, -1,  0),
            };

            static const Vector2 textureCoordinates[4] =
            {
                Vector2::UnitX,
                Vector2::One,
                Vector2::UnitY,
                Vector2::Zero,
            };

            std::vector<VertexColorTexture> vertices;
            std::vector<uint16_t> indices;

            Vector3 tsize(1.0f, 1.0f, 1.0f);
            tsize = tsize / 2.0f;

            // Create each face in turn.
            for (int i = 0; i < FaceCount; i++)
            {
                Vector3 normal = faceNormals[i];

                // Get two vectors perpendicular both to the face normal and to each other.
                Vector3 basis = (i >= 4) ? Vector3::UnitZ : Vector3::UnitY;

                Vector3 side1 = Vector3::Cross(normal, basis);
                Vector3 side2 = Vector3::Cross(normal, side1);

                // Six indices (two triangles) per face.
                size_t vbase = vertices.size();
                indices.push_back(static_cast<uint16_t>(vbase + 0));
                indices.push_back(static_cast<uint16_t>(vbase + 1));
                indices.push_back(static_cast<uint16_t>(vbase + 2));

                indices.push_back(static_cast<uint16_t>(vbase + 0));
                indices.push_back(static_cast<uint16_t>(vbase + 2));
                indices.push_back(static_cast<uint16_t>(vbase + 3));

                // Four vertices per face.
                // (normal - side1 - side2) * tsize // normal // t0
                vertices.push_back({ Vector3::Multiply(Vector3::Subtract(Vector3::Subtract(normal, side1), side2), tsize), Color(1.0f, 0.0f, 0.0f), textureCoordinates[0] });

                // (normal - side1 + side2) * tsize // normal // t1
                vertices.push_back({ Vector3::Multiply(Vector3::Add(Vector3::Subtract(normal, side1), side2), tsize), Color(0.0f, 1.0f, 0.0f), textureCoordinates[1] });

                // (normal + side1 + side2) * tsize // normal // t2
                vertices.push_back({ Vector3::Multiply(Vector3::Add(normal, Vector3::Add(side1, side2)), tsize), Color(0.0f, 0.0f, 1.0f), textureCoordinates[2] });

                // (normal + side1 - side2) * tsize // normal // t3
                vertices.push_back({ Vector3::Multiply(Vector3::Subtract(Vector3::Add(normal, side1), side2), tsize), Color(1.0f, 0.0f, 1.0f), textureCoordinates[3] });
            }

            std::vector<VertexElement> vertexDeclaration;
            vertexDeclaration.emplace_back(VertexFormat::Float3, VertexElementSemantic::POSITION);
            vertexDeclaration.emplace_back(VertexFormat::Float4, VertexElementSemantic::COLOR);
            vertexDeclaration.emplace_back(VertexFormat::Float2, VertexElementSemantic::TEXCOORD);

            _vertexBuffer = new VertexBuffer(graphics);
            _vertexBuffer->Define(static_cast<uint32_t>(vertices.size()), vertexDeclaration, ResourceUsage::Immutable, vertices.data());

            _indexBuffer = new IndexBuffer(graphics);
            _indexBuffer->Define(static_cast<uint32_t>(indices.size()), IndexType::UInt16, ResourceUsage::Immutable, indices.data());

            _shader = graphics->CreateShader("assets://shaders/sprite.vert", "assets://shaders/sprite.frag");

            _camera.viewMatrix = Matrix4x4::CreateLookAt(Vector3(0, 0, 5), Vector3::Zero, Vector3::UnitY);
            _camera.projectionMatrix = Matrix4x4::CreatePerspectiveFieldOfView(M_PIDIV4, aspectRatio, 0.1f, 100);
            GpuBufferDescription uboBufferDesc = {};
            uboBufferDesc.usage = BufferUsage::Uniform;
            uboBufferDesc.elementSize = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = new GpuBuffer(graphics, uboBufferDesc, &_camera);

            // Create checkerboard texture.
            ImageLevel initial = {};
            static const uint32_t checkerboard[] = {
                0xffffffffu, 0xffffffffu, 0xff000000u, 0xff000000u,
                0xffffffffu, 0xffffffffu, 0xff000000u, 0xff000000u,
                0xff000000u, 0xff000000u, 0xffffffffu, 0xffffffffu,
                0xff000000u, 0xff000000u, 0xffffffffu, 0xffffffffu,
            };
            initial.data = checkerboard;

            TextureDescription textureDesc = {};
            textureDesc.width = textureDesc.height = 4;
            _texture = graphics->CreateTexture(textureDesc, &initial);
        }

        void Render(CommandBuffer* commandBuffer)
        {
            commandBuffer->BeginRenderPass(nullptr, Color(0.0f, 0.2f, 0.4f, 1.0f));
            commandBuffer->SetShader(_shader.Get());
            commandBuffer->SetVertexBuffer(0, _vertexBuffer.Get());
            commandBuffer->SetIndexBuffer(_indexBuffer.Get());
            commandBuffer->SetUniformBuffer(0, 0, _perCameraUboBuffer.Get());
            commandBuffer->SetTexture(0, _texture.Get(), ShaderStage::Fragment);
            commandBuffer->DrawIndexed(PrimitiveTopology::Triangles, 6);
            commandBuffer->EndRenderPass();
        }

    private:
        SharedPtr<VertexBuffer> _vertexBuffer;
        SharedPtr<IndexBuffer> _indexBuffer;
        SharedPtr<Shader> _shader;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;
        SharedPtr<Texture> _texture;

        struct PerCameraCBuffer
        {
            mat4 viewMatrix;
            mat4 projectionMatrix;
        };

        PerCameraCBuffer _camera;
    };

    class RuntimeApplication final : public Application
    {
    public:
        RuntimeApplication();
        ~RuntimeApplication() override = default;

    private:
        void Initialize() override;
        void OnRenderFrame(double frameTime, double elapsedTime) override;

    private:
        TriangleExample _triangleExample;
        QuadExample _quadExample;
        CubeExample _cubeExample;
        TexturedCubeExample _texturedCubeExample;
    };

    RuntimeApplication::RuntimeApplication()
    {
        _settings.graphicsDeviceType = GraphicsDeviceType::Direct3D11;
        //_settings.graphicsDeviceType = GraphicsDeviceType::Vulkan;
    }

    void RuntimeApplication::Initialize()
    {
        // _triangleExample.Initialize(_graphics);
        //_quadExample.Initialize(_graphics);
        //_cubeExample.Initialize(_graphics, _window->GetAspectRatio());
        _texturedCubeExample.Initialize(_graphics, _window->GetAspectRatio());

        // Create scene
        //_scene = new Scene();
       //auto triangleEntity = _scene->CreateEntity();
       // triangleEntity->AddComponent<TransformComponent>();
       // triangleEntity->AddComponent<RenderableComponent>()->renderable = new TriangleRenderable();
    }

    void RuntimeApplication::OnRenderFrame(double frameTime, double elapsedTime)
    {
        auto commandBuffer = _graphics->RequestCommandBuffer();
        //_triangleExample.Render(commandBuffer);
        //_quadExample.Render(commandBuffer);
        //_cubeExample.Render(commandBuffer);
        _texturedCubeExample.Render(commandBuffer);

        // Submit command buffer.
        _graphics->Submit(commandBuffer);
    }
}

ALIMER_APPLICATION(Alimer::RuntimeApplication);
