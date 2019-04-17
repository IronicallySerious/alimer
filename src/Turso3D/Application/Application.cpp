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
#include "Graphics/Shader.h"
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
    }

    Application::~Application()
    {
        RemoveSubsystem(this);
    }

    int Application::Run()
    {
        graphics = new Graphics();
        graphics->RenderWindow()->SetTitle("Turso3D");
        if (!graphics->SetMode(IntVector2(800, 600), false, true)) {
            return EXIT_FAILURE;
        }

        SubscribeToEvent(graphics->RenderWindow()->closeRequestEvent, &Application::HandleWindowCloseRequest);

        float vertexData[] = {
            // Position             // Color
            0.0f, 0.5f, 0.0f,       1.0f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.0f,      0.0f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.0f,     0.0f, 0.0f, 1.0f, 1.0f
        };

        Vector<VertexElement> vertexDeclaration;
        vertexDeclaration.Push(VertexElement(ELEM_VECTOR3, SEM_POSITION));
        vertexDeclaration.Push(VertexElement(ELEM_VECTOR4, SEM_COLOR));
        AutoPtr<VertexBuffer> vb = new VertexBuffer();
        vb->Define(ResourceUsage::Immutable, 3, vertexDeclaration, false, vertexData);

        String vsCode =
#ifndef TURSO3D_OPENGL
            "struct VOut\n"
            "{\n"
            "    float4 position : SV_POSITION;\n"
            "    float4 color : COLOR0;\n"
            "};\n"
            "\n"
            "VOut main(float4 position : POSITION, float4 color : COLOR)\n"
            "{\n"
            "    VOut output;\n"
            "    output.position = position;\n"
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

        AutoPtr<Shader> vs = new Shader();
        vs->SetName("Test.vs");
        vs->Define(SHADER_VS, vsCode);
        ShaderVariation* vsv = vs->CreateVariation();

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

        AutoPtr<Shader> ps = new Shader();
        ps->SetName("Test.ps");
        ps->Define(SHADER_PS, psCode);
        ShaderVariation* psv = ps->CreateVariation();

        while (graphics->RenderWindow()->IsOpen())
        {
            input->Update();

            graphics->Clear(CLEAR_COLOR | CLEAR_DEPTH, Color(0.0f, 0.0f, 0.5f));
            graphics->SetVertexBuffer(0, vb);
            graphics->SetShaders(vsv, psv);
            graphics->SetDepthState(CMP_LESS_EQUAL, true);
            graphics->SetColorState(BLEND_MODE_REPLACE);
            graphics->SetRasterizerState(CULL_BACK, FILL_SOLID);
            graphics->Draw(TRIANGLE_LIST, 0, 3);

            // Present to window.
            graphics->Present();
        }

        return EXIT_SUCCESS;
    }

    void Application::HandleWindowCloseRequest(Event&)
    {
        graphics->RenderWindow()->Close();
    }
}
