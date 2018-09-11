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
        vec3 position;
        Color color;
    };

    struct VertexColorTexture
    {
        vec3 position;
        Color color;
        vec2 textureCoordinate;
    };

    struct PerCameraCBuffer
    {
        mat4 viewMatrix;
        mat4 projectionMatrix;
    };

    struct PerVertexData
    {
        mat4 worldMatrix;
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

            std::array<VertexAttributeDescriptor, 2> vertexInputAttributs;
            // Attribute location 0: Position
            vertexInputAttributs[0].bufferIndex = 0;
            vertexInputAttributs[0].location = 0;
            vertexInputAttributs[0].format = VertexFormat::Float3;
            vertexInputAttributs[0].offset = offsetof(VertexColor, position);
            // Attribute location 1: Color
            vertexInputAttributs[1].bufferIndex = 0;
            vertexInputAttributs[1].location = 1;
            vertexInputAttributs[1].format = VertexFormat::Float4;
            vertexInputAttributs[1].offset = offsetof(VertexColor, color);

            VertexInputFormatDescriptor inputFormatDescriptor = {};
            inputFormatDescriptor.attributes = vertexInputAttributs.data();
            inputFormatDescriptor.attributesCount = 2;
            _vertexInputFormat = graphics->CreateVertexInputFormat(&inputFormatDescriptor);

            BufferDescriptor vertexBufferDesc = {};
            vertexBufferDesc.usage = BufferUsage::TransferDest | BufferUsage::Vertex;
            vertexBufferDesc.size = sizeof(triangleVertices);
            vertexBufferDesc.stride = sizeof(VertexColor);
            _vertexBuffer = graphics->CreateBuffer(MemoryFlags::GpuOnly, &vertexBufferDesc, triangleVertices);

            // Create shader program.
            auto vertexShader = graphics->CreateShaderModule("assets://shaders/color.vert");
            auto fragmentShader = graphics->CreateShaderModule("assets://shaders/color.frag");
            _program = graphics->CreateShaderProgram(vertexShader, fragmentShader);

            _camera.viewMatrix = Matrix4();
            _camera.projectionMatrix = Matrix4();
            BufferDescriptor uboBufferDesc = {};
            uboBufferDesc.usage = BufferUsage::TransferDest | BufferUsage::Uniform;
            uboBufferDesc.size = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = graphics->CreateBuffer(MemoryFlags::CpuToGpu, &uboBufferDesc, &_camera);
        }

        void Render(CommandBuffer* context)
        {
            context->BeginRenderPass(nullptr, Color(0.0f, 0.2f, 0.4f, 1.0f));
            context->SetShaderProgram(_program.Get());
            //context->SetVertexInputFormat(_vertexInputFormat.Get());
            context->BindVertexBuffer(_vertexBuffer.Get(), 0);
            context->BindBuffer(_perCameraUboBuffer.Get(), 0, 0);
            context->Draw(PrimitiveTopology::Triangles, 3);
            context->EndRenderPass();
        }

    private:
        SharedPtr<VertexInputFormat> _vertexInputFormat;
        SharedPtr<GpuBuffer> _vertexBuffer;
        SharedPtr<ShaderProgram> _program;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;

        PerCameraCBuffer _camera{};
    };

    class QuadExample
    {
    public:
        void Initialize(Graphics* graphics)
        {
            VertexColor triangleVertices[] =
            {
                { vec3(-0.5f, 0.5f, 0.0f), Color(0.0f, 0.0f, 1.0f, 1.0f) },
                { vec3(0.5f, 0.5f, 0.0f), Color(1.0f, 0.0f, 0.0f, 1.0f) },
                { vec3(0.5f, -0.5f, 0.0f), Color(0.0f, 1.0f, 0.0f, 1.0f) },
                { vec3(-0.5f, -0.5f, 0.0f), Color(1.0f, 1.0f, 0.0f, 1.0f) },
            };

            BufferDescriptor vertexBufferDesc = {};
            vertexBufferDesc.usage = BufferUsage::TransferDest | BufferUsage::Vertex;
            vertexBufferDesc.size = sizeof(triangleVertices);
            vertexBufferDesc.stride = sizeof(VertexColor);
            _vertexBuffer = graphics->CreateBuffer(MemoryFlags::GpuOnly, &vertexBufferDesc, triangleVertices);

            // Create index buffer.
            const uint16_t indices[] = {
                0, 1, 2, 0, 2, 3
            };

            BufferDescriptor indexBufferDesc = {};
            indexBufferDesc.usage = BufferUsage::TransferDest | BufferUsage::Index;
            indexBufferDesc.size = sizeof(indices);
            indexBufferDesc.stride = sizeof(uint16_t);
            _indexBuffer = graphics->CreateBuffer(MemoryFlags::GpuOnly, &indexBufferDesc, indices);

            // Create shader program.
            auto vertexShader = graphics->CreateShaderModule("assets://shaders/color.vert");
            auto fragmentShader = graphics->CreateShaderModule("assets://shaders/color.frag");
            _program = graphics->CreateShaderProgram(vertexShader, fragmentShader);

            BufferDescriptor uboBufferDesc = {};
            uboBufferDesc.usage = BufferUsage::TransferDest | BufferUsage::Uniform;
            uboBufferDesc.size = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = graphics->CreateBuffer(MemoryFlags::GpuOnly, &uboBufferDesc, &_camera);
        }

        void Render(CommandBuffer* context)
        {
            context->BeginRenderPass(nullptr, Color(0.0f, 0.2f, 0.4f, 1.0f));
            context->SetShaderProgram(_program.Get());
            context->BindVertexBuffer(_vertexBuffer.Get(), 0);
            context->BindIndexBuffer(_indexBuffer.Get(), 0, IndexType::UInt16);
            context->BindBuffer(_perCameraUboBuffer.Get(), 0, 0);
            context->DrawIndexed(PrimitiveTopology::Triangles, 6);
            context->EndRenderPass();
        }

    private:
        SharedPtr<GpuBuffer> _vertexBuffer;
        SharedPtr<GpuBuffer> _indexBuffer;
        SharedPtr<ShaderProgram> _program;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;

        PerCameraCBuffer _camera;
    };

    class CubeExample
    {
    public:
        void Initialize(Graphics* graphics, float aspectRatio)
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

            std::vector<VertexColor> vertices;
            std::vector<uint16_t> indices;

            vec3 tsize(1.0f, 1.0f, 1.0f);
            tsize = tsize / 2.0f;

            // Create each face in turn.
            for (int i = 0; i < FaceCount; i++)
            {
                vec3 normal = faceNormals[i];

                // Get two vectors perpendicular both to the face normal and to each other.
                vec3 basis = (i >= 4) ? vec3::unit_z() : vec3::unit_y();

                vec3 side1 = cross(normal, basis);
                vec3 side2 = cross(normal, side1);

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
                vertices.push_back({ (normal - side1 - side2) * tsize, Color(1.0f, 0.0f, 0.0f) });

                // (normal - side1 + side2) * tsize // normal // t1
                vertices.push_back({ (normal - side1 + side2) * tsize, Color(0.0f, 1.0f, 0.0f) });

                // (normal + side1 + side2) * tsize // normal // t2
                vertices.push_back({ (normal + side1 + side2) * tsize, Color(0.0f, 0.0f, 1.0f) });

                // (normal + side1 - side2) * tsize // normal // t3
                vertices.push_back({ (normal + side1 - side2) * tsize, Color(1.0f, 0.0f, 1.0f) });
            }

            BufferDescriptor vertexBufferDesc = {};
            vertexBufferDesc.usage = BufferUsage::TransferDest | BufferUsage::Vertex;
            vertexBufferDesc.size = vertices.size() * sizeof(VertexColor);
            vertexBufferDesc.stride = sizeof(VertexColor);
            _vertexBuffer = graphics->CreateBuffer(MemoryFlags::GpuOnly, &vertexBufferDesc, vertices.data());

            // Index
            BufferDescriptor indexBufferDesc = {};
            indexBufferDesc.usage = BufferUsage::TransferDest | BufferUsage::Index;
            indexBufferDesc.size = indices.size() * sizeof(uint16_t);
            indexBufferDesc.stride = sizeof(uint16_t);
            _indexBuffer = graphics->CreateBuffer(MemoryFlags::GpuOnly, &indexBufferDesc, indices.data());

            // Uniform buffer
            _camera.viewMatrix = Matrix4::LookAt(vec3(0, 0, 5), vec3::zero(), vec3::unit_y());
            _camera.projectionMatrix = Matrix4::Perspective(M_PIDIV4, aspectRatio, 0.1f, 100, false);

            BufferDescriptor uboBufferDesc = {};
            uboBufferDesc.usage = BufferUsage::TransferDest | BufferUsage::Uniform;
            uboBufferDesc.stride = sizeof(PerCameraCBuffer);
            uboBufferDesc.size = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = graphics->CreateBuffer(MemoryFlags::GpuOnly, &uboBufferDesc, &_camera);

            // Per draw ubo.
            uboBufferDesc.stride = sizeof(PerVertexData);
            uboBufferDesc.size = sizeof(PerVertexData);
            _perDrawUboBuffer = graphics->CreateBuffer(MemoryFlags::GpuOnly, &uboBufferDesc, &_perDrawData);

            // Shader program
            auto vertexShader = graphics->CreateShaderModule("assets://shaders/color.vert");
            auto fragmentShader = graphics->CreateShaderModule("assets://shaders/color.frag");
            _program = graphics->CreateShaderProgram(vertexShader, fragmentShader);
        }

        void Render(CommandBuffer* context)
        {
            _perDrawUboBuffer->SetSubData(&_perDrawData);

            context->BeginRenderPass(nullptr, Color(0.0f, 0.2f, 0.4f, 1.0f));
            context->SetShaderProgram(_program.Get());
            context->BindVertexBuffer(_vertexBuffer.Get(), 0);
            context->BindIndexBuffer(_indexBuffer.Get(), 0, IndexType::UInt16);
            context->BindBuffer(_perCameraUboBuffer.Get(), 0, 0);
            context->BindBuffer(_perDrawUboBuffer.Get(), 0, 1);
            context->DrawIndexed(PrimitiveTopology::Triangles, 6);
            context->EndRenderPass();
        }

    private:
        SharedPtr<GpuBuffer> _vertexBuffer;
        SharedPtr<GpuBuffer> _indexBuffer;
        SharedPtr<ShaderProgram> _program;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;
        SharedPtr<GpuBuffer> _perDrawUboBuffer;

        PerCameraCBuffer _camera;
        PerVertexData _perDrawData;
    };

#if TODO
    class TexturedCubeExample
    {
    public:
        void Initialize(Graphics* graphics, float aspectRatio)
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

            static const vec2 textureCoordinates[4] =
            {
                vec2::unit_x(),
                vec2::one(),
                vec2::unit_y(),
                vec2::zero(),
            };

            std::vector<VertexColorTexture> vertices;
            std::vector<uint16_t> indices;

            vec3 tsize(1.0f);
            tsize = tsize / 2.0f;

            // Create each face in turn.
            for (int i = 0; i < FaceCount; i++)
            {
                vec3 normal = faceNormals[i];

                // Get two vectors perpendicular both to the face normal and to each other.
                vec3 basis = (i >= 4) ? vec3::unit_z() : vec3::unit_y();

                vec3 vec3 = cross(normal, basis);
                vec3 side2 = cross(normal, side1);

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

            _vertexBuffer = new VertexBuffer();
            _vertexBuffer->Define(static_cast<uint32_t>(vertices.size()), vertexDeclaration, ResourceUsage::Immutable, vertices.data());

            _indexBuffer = graphics->CreateIndexBuffer(static_cast<uint32_t>(indices.size()), IndexType::UInt16, ResourceUsage::Immutable, indices.data());

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

            TextureDescriptor textureDesc = {};
            textureDesc.width = textureDesc.height = 4;
            _texture = graphics->CreateTexture(&textureDesc, &initial);
        }

        void Render(CommandContexzt* context)
        {
            context->BeginRenderPass(nullptr, Color(0.0f, 0.2f, 0.4f, 1.0f));
            context->SetShader(_shader.Get());
            context->SetVertexBuffer(0, _vertexBuffer.Get());
            context->SetIndexBuffer(_indexBuffer.Get());
            context->SetUniformBuffer(0, 0, _perCameraUboBuffer.Get());
            context->SetTexture(0, _texture.Get(), ShaderStage::Fragment);
            context->DrawIndexed(PrimitiveTopology::Triangles, 6);
            context->EndRenderPass();
        }

    private:
        SharedPtr<VertexBuffer> _vertexBuffer;
        SharedPtr<GpuBuffer> _indexBuffer;
        SharedPtr<Shader> _shader;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;
        SharedPtr<Texture> _texture;

        PerCameraCBuffer _camera;
    };
#endif // TODO

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
        //TexturedCubeExample _texturedCubeExample;
    };

    RuntimeApplication::RuntimeApplication()
    {
        _settings.graphicsDeviceType = GraphicsDeviceType::Direct3D11;
        //_settings.graphicsDeviceType = GraphicsDeviceType::Vulkan;
    }

    void RuntimeApplication::Initialize()
    {
        _triangleExample.Initialize(_graphics);
        //_quadExample.Initialize(_graphics);
        //_cubeExample.Initialize(_graphics, _window->GetAspectRatio());
        //_texturedCubeExample.Initialize(_graphics, _window->getAspectRatio());

        // Create scene
        //_scene = new Scene();
        //auto triangleEntity = _scene->CreateEntity();
        //triangleEntity->AddComponent<TransformComponent>();
        //triangleEntity->AddComponent<RenderableComponent>();
    }

    void RuntimeApplication::OnRenderFrame(double frameTime, double elapsedTime)
    {
        ALIMER_UNUSED(frameTime);
        ALIMER_UNUSED(elapsedTime);

        //auto commandBuffer = _graphics->GetDefaultCommandBuffer();
        //commandBuffer->Begin();
        //_triangleExample.Render(commandBuffer);
        //_quadExample.Render(commandBuffer);
        //_cubeExample.Render(commandBuffer);
        //_texturedCubeExample.Render(commandBuffer);
        //commandBuffer->End();
    }
}

ALIMER_APPLICATION(Alimer::RuntimeApplication);
