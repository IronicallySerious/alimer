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

#include "UI/UI.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
// #include "imgui/imgui_freetype.h"

namespace Alimer
{
    UI::UI(const uvec2& size)
    {
        _imContext = ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
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
        io.SetClipboardTextFn = [](void* userData, const char* text) { SDL_SetClipboardText(text); };
        io.GetClipboardTextFn = [](void* userData) -> const char* { return SDL_GetClipboardText(); };
        */
        io.IniFilename = nullptr;
        //io.ImeWindowHandle = gApplication().GetMainWindow().GetHandle();
        io.UserData = this;

        if (io.Fonts->Fonts.empty())
        {
            io.Fonts->AddFontDefault();
            UpdateFontTexture();
        }

        io.DisplaySize = ImVec2(
            (float)size.x,
            (float)size.y);

        ImGui::NewFrame();
        ImGui::EndFrame();
    }

    UI::~UI()
    {
        ImGui::EndFrame();
        ImGui::Shutdown(_imContext);
        ImGui::DestroyContext(_imContext);
    }

    void UI::UpdateFontTexture()
    {
        ImGuiIO& io = ImGui::GetIO();

        // font texture for imgui's default font
        unsigned char* fontPixels;
        int fontWidth, fontHeight;
        //ImGuiFreeType::BuildFontAtlas(io.Fonts, ImGuiFreeType::ForceAutoHint);
        io.Fonts->GetTexDataAsRGBA32(&fontPixels, &fontWidth, &fontHeight);

        //auto saveStream = FileSystem::Get().Open("assets://Font.bmp", StreamMode::WriteOnly);
        //Image image;
        //image.Define(uvec2(fontWidth, fontHeight), PixelFormat::RGBA8UNorm);
        //image.SetData(fontPixels);
        //image.Save(saveStream.Get(), ImageFormat::Bmp);

        if (_fontTexture.IsNull())
        {
            ImageLevel imageData;
            imageData.rowPitch = fontWidth * 4;
            imageData.data = fontPixels;

            TextureDescriptor descriptor = {};
            descriptor.width = fontWidth;
            descriptor.height = fontHeight;
            descriptor.format = PixelFormat::RGBA8UNorm;
            //_fontTexture = _graphicsDevice->CreateTexture(&descriptor, &imageData);
        }

        // Store our identifier
        io.Fonts->TexID = (void*)_fontTexture.Get();
        io.Fonts->ClearTexData();
    }
}
