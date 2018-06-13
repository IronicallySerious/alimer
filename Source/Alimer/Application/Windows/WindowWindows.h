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

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "../Window.h"

namespace Alimer
{
	class OleDropTarget;

	/// Win32 OS window implementation.
	class WindowWindows final : public Window
	{
	public:
		WindowWindows(const std::string& title, uint32_t width, uint32_t height, bool fullscreen);
		~WindowWindows() override;
		void Destroy();
		void Activate(bool focused);

		void Show();
		void Close();
		LRESULT OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam);

		inline HINSTANCE GetHInstance() const { return _hInstance; }
		inline HWND GetHandle() const { return _hwnd; }

	private:
		void InitAfterCreation();

		HINSTANCE _hInstance = nullptr;
		HWND _hwnd = nullptr;
		HMONITOR _monitor = nullptr;
		bool _visible = false;
		bool _focused = false;
		int _showCommand = SW_SHOW;
		OleDropTarget* _dropTarget;
		HCURSOR _cursor;
	};
}
