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

#include "../UI/Gui.h"
#include "../Debug/Log.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/Shader.h"
#include "shaders_h/imgui.vert.h"
#include "shaders_h/imgui.frag.h"

namespace Alimer
{
    Gui::Gui()
    {
        /*RenderPipelineDescriptor renderPipelineDesc = {};

        // Shaders
        renderPipelineDesc.vertex = new Shader();
        renderPipelineDesc.vertex->Define(ShaderStage::Vertex, sizeof(imgui_vs), imgui_vs);

        renderPipelineDesc.fragment = new Shader();
        renderPipelineDesc.fragment->Define(ShaderStage::Fragment, sizeof(imgui_fs), imgui_fs);

        renderPipelineDesc.vertexDescriptor.layouts[0].stride = 28;
        renderPipelineDesc.vertexDescriptor.attributes[0].format = VertexFormat::Float3;
        renderPipelineDesc.vertexDescriptor.attributes[1].format = VertexFormat::Float4;
        _pipeline = new Pipeline();
        _pipeline->Define(&renderPipelineDesc);*/

        AddSubsystem(this);
    }

    Gui::~Gui()
    {
        RemoveSubsystem(this);
    }
}
