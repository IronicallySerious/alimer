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

#pragma once

#include "../Window.h"
struct SDL_Window;

namespace Alimer
{
	/// SDL2 OS window implementation.
	class SDL2Window final : public Window
	{
	public:
        SDL2Window(const std::string& title, uint32_t width, uint32_t height, bool fullscreen);
		~SDL2Window() override;
		void Destroy();
		void Activate(bool focused);

        void Show() override;
        void Hide() override;
        void Minimize() override;
        void Maximize() override;
        void Restore() override;
        void Close() override;

        bool IsVisible() const override { return _visible; }
        bool IsMinimized() const override;

	private:
        void HandleResize(const uvec2& newSize);

        SDL_Window* _window = nullptr;
        bool _visible = true;
        bool _focused = false;
	};
}
