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
#include "../Core/Log.h"
#include "../Graphics/GPUDevice.h"
#include "../Graphics/Shader.h"
//#include <ImGuizmo/ImGuizmo.h>
#include <imgui.h>
#include <imgui_internal.h>
//#include <imgui_freetype.h>

namespace Alimer
{
    Gui::Gui()
    {
        IMGUI_CHECKVERSION();
        _imContext = ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.UserData = this;
        /*io.KeyMap[ImGuiKey_Tab] = SCANCODE_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = SCANCODE_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = SCANCODE_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = SCANCODE_UP;
        io.KeyMap[ImGuiKey_DownArrow] = SCANCODE_DOWN;
        io.KeyMap[ImGuiKey_Home] = SCANCODE_HOME;
        io.KeyMap[ImGuiKey_End] = SCANCODE_END;
        io.KeyMap[ImGuiKey_Delete] = SCANCODE_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = SCANCODE_BACKSPACE;
        io.KeyMap[ImGuiKey_Enter] = SCANCODE_RETURN;
        io.KeyMap[ImGuiKey_Escape] = SCANCODE_ESCAPE;
        io.KeyMap[ImGuiKey_A] = SCANCODE_A;
        io.KeyMap[ImGuiKey_C] = SCANCODE_C;
        io.KeyMap[ImGuiKey_V] = SCANCODE_V;
        io.KeyMap[ImGuiKey_X] = SCANCODE_X;
        io.KeyMap[ImGuiKey_Y] = SCANCODE_Y;
        io.KeyMap[ImGuiKey_Z] = SCANCODE_Z;
        io.KeyMap[ImGuiKey_PageUp] = SCANCODE_PAGEUP;
        io.KeyMap[ImGuiKey_PageDown] = SCANCODE_DOWN;
        io.KeyMap[ImGuiKey_Space] = SCANCODE_SPACE;
        io.SetClipboardTextFn = [](void* userData, const char* text) { SDL_SetClipboardText(text); };
        io.GetClipboardTextFn = [](void* userData) -> const char* { return SDL_GetClipboardText(); };*/

        ApplyStyleDefault(true, 1.0f);

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
        //ImGui::EndFrame();
        ImGui::Shutdown(_imContext);
        ImGui::DestroyContext(_imContext);
        RemoveSubsystem(this);
    }

    void Gui::ApplyStyleDefault(bool darkStyle, float alpha)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScrollbarSize = 10.f;
        if (darkStyle)
            ImGui::StyleColorsDark(&style);
        else
            ImGui::StyleColorsLight(&style);
        style.Alpha = alpha;
        style.FrameRounding = 3.0f;
        //style.ScaleAllSizes(GetFontScale());
    }
}
