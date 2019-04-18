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

#include "Application.h"
#include "Input.h"
#include "Window.h"
#include "Resource/ResourceCache.h"
#include "Graphics/Graphics.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/ConstantBuffer.h"
#include "Graphics/Shader.h"
#include "../Math/Matrix4.h"
#include "../Debug/Log.h"
#include "../Debug/DebugNew.h"

namespace Turso3D
{
    Application::Application()
    {
        // Register self as a subsystem
        RegisterSubsystem(this);

        // Create input subsystem.
        input = new Input();

        RegisterResourceLibrary();
        RegisterGraphicsLibrary();
        //RegisterRendererLibrary();

        PlatformConstruct();
    }

    Application::~Application()
    {
        PlatformShutdown();
        RemoveSubsystem(this);
    }

    int Application::Run()
    {
        if (_running)
        {
            TURSO3D_LOGERROR("Cannot run Application while it is already running");
            return EXIT_FAILURE;
        }

        _running = true;
        return PlatformRun();
    }

    void Application::Exit()
    {

    }

    AutoPtr<VertexBuffer> s_vertexBuffer;
    AutoPtr<IndexBuffer> s_indexBuffer;
    AutoPtr<ConstantBuffer> s_constantBuffer;

    AutoPtr<Shader> s_vertexShader;
    AutoPtr<Shader> s_fragmentShader;

    bool Application::InitializeBeforeRun()
    {
        graphics = new Graphics();
        graphics->RenderWindow()->SetTitle("Turso3D");
        if (!graphics->SetMode(IntVector2(800, 600), false, true)) {
            return false;
        }

        SubscribeToEvent(graphics->RenderWindow()->closeRequestEvent, &Application::HandleWindowCloseRequest);

        float vertexData[] = {
            // Position             // Color
            -1.0f, -1.0f, -1.0f,   1.0f, 0.0f, 0.0f, 1.0f,
             1.0f, -1.0f, -1.0f,   1.0f, 0.0f, 0.0f, 1.0f,
             1.0f,  1.0f, -1.0f,   1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,   1.0f, 0.0f, 0.0f, 1.0f,

            -1.0f, -1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
             1.0f, -1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
             1.0f,  1.0,  1.0,   0.0, 1.0, 0.0, 1.0,
            -1.0f,  1.0,  1.0,   0.0, 1.0, 0.0, 1.0,

            -1.0f, -1.0, -1.0,   0.0, 0.0, 1.0, 1.0,
            -1.0f,  1.0, -1.0,   0.0, 0.0, 1.0, 1.0,
            -1.0f,  1.0,  1.0,   0.0, 0.0, 1.0, 1.0,
            -1.0f, -1.0,  1.0,   0.0, 0.0, 1.0, 1.0,

            1.0f, -1.0, -1.0,   1.0, 0.5, 0.0, 1.0,
            1.0f,  1.0, -1.0,   1.0, 0.5, 0.0, 1.0,
            1.0f,  1.0,  1.0,   1.0, 0.5, 0.0, 1.0,
            1.0f, -1.0,  1.0,   1.0, 0.5, 0.0, 1.0,

            -1.0f, -1.0, -1.0,   0.0, 0.5, 1.0, 1.0,
            -1.0f, -1.0,  1.0,   0.0, 0.5, 1.0, 1.0,
             1.0f, -1.0,  1.0,   0.0, 0.5, 1.0, 1.0,
             1.0f, -1.0, -1.0,   0.0, 0.5, 1.0, 1.0,

            -1.0f,  1.0, -1.0,   1.0, 0.0, 0.5, 1.0,
            -1.0f,  1.0,  1.0,   1.0, 0.0, 0.5, 1.0,
             1.0f,  1.0,  1.0,   1.0, 0.0, 0.5, 1.0,
             1.0f,  1.0, -1.0,   1.0, 0.0, 0.5, 1.0
        };

        Vector<VertexElement> vertexDeclaration;
        vertexDeclaration.Push(VertexElement(ELEM_VECTOR3, SEM_POSITION));
        vertexDeclaration.Push(VertexElement(ELEM_VECTOR4, SEM_COLOR));
        s_vertexBuffer = new VertexBuffer();
        s_vertexBuffer->Define(ResourceUsage::Immutable, 24, vertexDeclaration, false, vertexData);

        uint16_t indexData[] = {
            0, 1, 2,  0, 2, 3,
            6, 5, 4,  7, 6, 4,
            8, 9, 10,  8, 10, 11,
            14, 13, 12,  15, 14, 12,
            16, 17, 18,  16, 18, 19,
            22, 21, 20, 23, 22, 20 };
        s_indexBuffer = new IndexBuffer();
        s_indexBuffer->Define(ResourceUsage::Immutable, _countof(indexData), IndexType::UInt16, false, indexData);

        Matrix4 worldViewProjection = Matrix4::IDENTITY;

        Constant pc(ELEM_MATRIX4, "worldViewProjection");
        s_constantBuffer = new ConstantBuffer();
        s_constantBuffer->Define(1, &pc);
        s_constantBuffer->SetConstant("worldViewProjection", worldViewProjection);
        s_constantBuffer->Apply();

        String vsCode =
#ifndef TURSO3D_OPENGL
            "cbuffer params : register(b0)\n"
            "{\n"
            "    float4x4 worldViewProjection;\n"
            "}\n"
            "struct VOut\n"
            "{\n"
            "    float4 position : SV_POSITION;\n"
            "    float4 color : COLOR0;\n"
            "};\n"
            "\n"
            "VOut main(float4 position : POSITION, float4 color : COLOR)\n"
            "{\n"
            "    VOut output;\n"
            "    output.position = mul(worldViewProjection, position);\n"
            "    output.color = color;\n"
            "    return output;\n"
            "}";
#else
            "#version 150\n"
            "\n"
            "in vec3 position;\n"
            "in vec2 texCoord;\n"
            "in vec3 texCoord1; // objectPosition\n"
            "\n"
            "out vec2 vTexCoord;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    gl_Position = vec4(position + texCoord1, 1.0);\n"
            "    vTexCoord = texCoord;\n"
            "}\n";
#endif

        s_vertexShader = new Shader();
        s_vertexShader->SetName("Test.vs");
        s_vertexShader->Define(SHADER_VS, vsCode);

        String psCode =
#ifndef TURSO3D_OPENGL
            "float4 main(float4 position : SV_POSITION, float4 color : COLOR0) : SV_TARGET\n"
            "{\n"
            "    return color;\n"
            "}\n";
#else
            "#version 150\n"
            "\n"
            "layout(std140) uniform ConstantBuffer0\n"
            "{\n"
            "    vec4 color;\n"
            "};\n"
            "\n"
            "uniform sampler2D Texture0;\n"
            "in vec2 vTexCoord;\n"
            "out vec4 fragColor;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    fragColor = color * texture(Texture0, vTexCoord);\n"
            "}\n";
#endif

        s_fragmentShader = new Shader();
        s_fragmentShader->SetName("Test.ps");
        s_fragmentShader->Define(SHADER_PS, psCode);
        return true;
    }

    void Application::HandleWindowCloseRequest(Event&)
    {
        graphics->RenderWindow()->Close();
    }

    void Application::Tick()
    {
        input->Update();

        graphics->Clear(CLEAR_COLOR | CLEAR_DEPTH, Color(0.5f, 0.5f, 0.5f));
        graphics->SetVertexBuffer(0, s_vertexBuffer.Get());
        graphics->SetIndexBuffer(s_indexBuffer.Get());
        graphics->SetShaders(s_vertexShader->CreateVariation(), s_fragmentShader->CreateVariation());
        graphics->SetDepthState(CompareFunction::LessEqual, true);
        graphics->SetColorState(BLEND_MODE_REPLACE);
        graphics->SetRasterizerState(CULL_BACK, FILL_SOLID);
        graphics->SetConstantBuffer(SHADER_VS, 0, s_constantBuffer.Get());
        //graphics->Draw(TRIANGLE_LIST, 0, 3);
        graphics->DrawIndexed(TRIANGLE_LIST, 0, 36, 0);

        // Present to window.
        graphics->Present();
    }
}
