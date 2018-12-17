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
        Color4 color;
    };

    struct VertexColorTexture
    {
        vec3 position;
        Color4 color;
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
        void Initialize(GPUDevice* device, ResourceManager& resources)
        {
            ALIMER_UNUSED(resources);

            VertexColor triangleVertices[] =
            {
                { vec3(0.0f, 0.5f, 0.0f), Color4::Red },
                { vec3(0.5f, -0.5f, 0.0f), Color4::Lime },
                { vec3(-0.5f, -0.5f, 0.0f), Color4::Blue }
            };

            //PODVector<VertexElement> vertexElement;
            //vertexElement.Push(VertexElement(VertexFormat::Float3, VertexElementSemantic::Position));
            //vertexElement.Push(VertexElement(VertexFormat::Float4, VertexElementSemantic::Color0));

            _vertexBuffer = device->CreateBuffer(BufferUsage::Vertex, 3, sizeof(VertexColor), triangleVertices);

            /*BufferDescriptor uboBufferDesc = {};
            uboBufferDesc.resourceUsage = ResourceUsage::Dynamic;
            uboBufferDesc.usage = BufferUsage::Uniform;
            uboBufferDesc.size = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = graphics->CreateBuffer(&uboBufferDesc, &_camera);

            // Define pipeline now.
            RenderPipelineDescriptor renderPipelineDesc = {};
            */

            // Shaders
            //auto texture = resources.Load<Texture>("textures/test.png");
            //auto vertexShader = resources.Load<ShaderModule>("shaders/color.vert");
            //auto fragmentShader = resources.Load<ShaderModule>("shaders/color.frag");
            //_shader = new Shader();
            //_shader->Define(vertexShader.Get(), fragmentShader.Get());
        }

        void Render(GPUDevice* device)
        {
            CommandContext& context = device->GetImmediateContext();
            Color4 clearColor(0.0f, 0.2f, 0.4f, 1.0f);
            context.BeginDefaultRenderPass(clearColor);
            //context.Draw(3, 0);
            context.EndRenderPass();
            context.Flush();
        }

    private:
        SharedPtr<Buffer> _vertexBuffer;
        //GpuBuffer _perCameraUboBuffer;

        SharedPtr<ShaderModule> _vertexShader;
        SharedPtr<ShaderModule> _fragmentShader;
        //SharedPtr<Shader> _shader;
        PerCameraCBuffer _camera{};
    };

#if TODO
    class QuadExample
    {
    public:
        void Initialize(ResourceManager& resources)
        {
            ALIMER_UNUSED(resources);

            /*VertexColor triangleVertices[] =
            {
                { vec3(-0.5f, 0.5f, 0.0f), Color4(0.0f, 0.0f, 1.0f, 1.0f) },
                { vec3(0.5f, 0.5f, 0.0f), Color4(1.0f, 0.0f, 0.0f, 1.0f) },
                { vec3(0.5f, -0.5f, 0.0f), Color4(0.0f, 1.0f, 0.0f, 1.0f) },
                { vec3(-0.5f, -0.5f, 0.0f), Color4(1.0f, 1.0f, 0.0f, 1.0f) },
            };

            _vertexBuffer.Define(
                BufferUsage::Vertex,
                sizeof(triangleVertices),
                sizeof(VertexColor),
                triangleVertices);

            // Create index buffer.
            const uint16_t indices[] = {
                0, 1, 2, 0, 2, 3
            };

            _indexBuffer.Define(
                BufferUsage::Index,
                sizeof(indices),
                sizeof(uint16_t),
                indices);

            BufferDescriptor uboBufferDesc = {};
            uboBufferDesc.resourceUsage = ResourceUsage::Dynamic;
            uboBufferDesc.usage = BufferUsage::Uniform;
            uboBufferDesc.size = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = graphics->CreateBuffer(&uboBufferDesc, &_camera);

            // Define pipeline now.
            RenderPipelineDescriptor renderPipelineDesc = {};

            // Shaders
            _shader = new Shader();
            _shader->Define(ShaderStage::Vertex, resources.ReadBytes("shaders/color.vert.spv"));
            _shader->Define(ShaderStage::Fragment, resources.ReadBytes("shaders/color.frag.spv"));
            _shader->Finalize();

            renderPipelineDesc.shader = _shader;
            renderPipelineDesc.vertexDescriptor.layouts[0].stride = sizeof(VertexColor);
            renderPipelineDesc.vertexDescriptor.attributes[0].format = VertexFormat::Float3;
            renderPipelineDesc.vertexDescriptor.attributes[1].format = VertexFormat::Float4;
            _pipeline.Define(&renderPipelineDesc);*/
        }

        void Render()
        {
            //agpuCmdSetShader(_shader->GetHandle());
            //agpuCmdSetVertexBuffer(0, _vertexBuffer.GetHandle(), 0, AGPU_VERTEX_INPUT_RATE_VERTEX);
            //agpuCmdSetIndexBuffer(_indexBuffer.GetHandle(), 0, AGPU_INDEX_TYPE_UINT16);
            //agpuCmdDrawIndexed(6, 0, 0);

            //context->SetShader(_shader.Get());
            //context->SetVertexBuffer(_vertexBuffer.Get(), 0, 0);
            //context->SetIndexBuffer(_indexBuffer.Get(), 0);
            //context->SetBuffer(_perCameraUboBuffer.Get(), 0, 0);

            //context->DrawIndexed(PrimitiveTopology::Triangles, 6, 0, 0);
        }

    private:
        //GpuBuffer _vertexBuffer;
        //GpuBuffer _indexBuffer;
        //SharedPtr<GpuBuffer> _perCameraUboBuffer;
        // PerCameraCBuffer _camera;

        SharedPtr<Shader> _shader;
        Pipeline _pipeline;
    };


    class CubeExample
    {
    public:
        void Initialize(float aspectRatio)
        {
            _mesh = Mesh::CreateCube(graphics);

            // Uniform buffer
            //_camera.viewMatrix = Matrix4x4::CreateLookAt(vec3(0, 0, 5), vec3::zero(), vec3::unit_y());
            //_camera.projectionMatrix = Matrix4x4::CreatePerspectiveFieldOfView(M_PIDIV4, aspectRatio, 0.1f, 100);

            BufferDescriptor uboBufferDesc = {};
            uboBufferDesc.resourceUsage = ResourceUsage::Dynamic;
            uboBufferDesc.usage = BufferUsage::Uniform;
            uboBufferDesc.stride = sizeof(PerCameraCBuffer);
            uboBufferDesc.size = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = graphics->CreateBuffer(&uboBufferDesc, &_camera);

            // Per draw ubo.
            uboBufferDesc.stride = sizeof(PerVertexData);
            uboBufferDesc.size = sizeof(PerVertexData);
            _perDrawUboBuffer = graphics->CreateBuffer(&uboBufferDesc, &_perDrawData);

            // Shader program
            auto vertexShader = graphics->CreateShaderModule("assets://shaders/color.vert");
            auto fragmentShader = graphics->CreateShaderModule("assets://shaders/color.frag");
            _program = graphics->CreateShaderProgram(vertexShader, fragmentShader);
        }

        void Render(double deltaTime)
        {
            float time = static_cast<float>(deltaTime);
            //_perDrawData.worldMatrix = Matrix4x4::CreateRotationX(time) * Matrix4x4::CreateRotationY(time * 2) * Matrix4x4::CreateRotationZ(time * .7f);
            //_perDrawUboBuffer->SetSubData(&_perDrawData);

            // Bind shader program.
            context->SetShaderProgram(_program.Get());
            context->BindBuffer(_perCameraUboBuffer.Get(), 0, 0);
            context->BindBuffer(_perDrawUboBuffer.Get(), 0, 1);

            // Draw mesh.
            _mesh->Draw(context);
        }

    private:
        SharedPtr<Mesh> _mesh;
        SharedPtr<ShaderProgram> _program;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;
        SharedPtr<GpuBuffer> _perDrawUboBuffer;

        PerCameraCBuffer _camera;
        PerVertexData _perDrawData;
    };

    class TexturedCubeExample
    {
    public:
        void Initialize(float aspectRatio)
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

        void Render()
        {
            context->SetShader(_shader.Get());
            context->SetVertexBuffer(0, _vertexBuffer.Get());
            context->SetIndexBuffer(_indexBuffer.Get());
            context->SetUniformBuffer(0, 0, _perCameraUboBuffer.Get());
            context->SetTexture(0, _texture.Get(), ShaderStage::Fragment);
            context->DrawIndexed(PrimitiveTopology::Triangles, 6);
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
        //QuadExample _quadExample;
        //CubeExample _cubeExample;
        //TexturedCubeExample _texturedCubeExample;
    };

    RuntimeApplication::RuntimeApplication()
    {
    }

    void RuntimeApplication::Initialize()
    {
        _triangleExample.Initialize(_gpuDevice.Get(), _resources);
        //_quadExample.Initialize(_resources);
        //_cubeExample.Initialize(_graphicsDevice.Get(), _window->GetAspectRatio());
        //_texturedCubeExample.Initialize(_graphicsDevice.Get(), _window->getAspectRatio());

        // Create triangle scene
        //auto triangleEntity = _entities.CreateEntity();
        //triangleEntity->AddComponent<TransformComponent>();
        //auto renderable = triangleEntity->AddComponent<RenderableComponent>();
        //auto mesh = new Mesh(_graphics.Get());
        //renderable->renderable = MakeDerivedHandle<Renderable, MeshRenderable>(mesh);
        //triangleEntity->AddComponent<RenderableComponent>();
    }

    void RuntimeApplication::OnRenderFrame(double frameTime, double elapsedTime)
    {
        ALIMER_UNUSED(frameTime);
        ALIMER_UNUSED(elapsedTime);

        _triangleExample.Render(_gpuDevice.Get());
        //_quadExample.Render(context);
        //_cubeExample.Render(commandBuffer, elapsedTime);
        //_texturedCubeExample.Render(commandBuffer);
    }
}

int main(int argc, char** argv)
{
    using namespace Alimer;

    RuntimeApplication app;
    int returnCode = app.Run(argc, argv);
    return returnCode;
}

#if ALIMER_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    ALIMER_UNUSED(hInstance);
    ALIMER_UNUSED(hPrevInstance);
    ALIMER_UNUSED(lpCmdLine);
    ALIMER_UNUSED(nCmdShow);
    int argc = __argc;
    char** argv = __argv;
    return main(argc, argv);
}
#endif
