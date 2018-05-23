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

#include "InputWindows.h"
#include "../../Core/Log.h"

namespace Alimer
{
	InputWindows::InputWindows()
	{
		for (uint32_t i = 0; i < static_cast<unsigned>(CursorType::Count); ++i)
		{
			switch (static_cast<CursorType>(i))
			{
				case CursorType::Arrow:
					_cursors[i] = LoadCursorW(nullptr, IDC_ARROW);
					break;

				case CursorType::Cross:
					_cursors[i] = LoadCursorW(nullptr, IDC_CROSS);
					break;

				case CursorType::Hand:
					_cursors[i] = LoadCursorW(nullptr, IDC_HAND);
					break;
			}
		}

		_cursor = _cursors[static_cast<unsigned>(CursorType::Arrow)];
	}

	InputWindows::~InputWindows()
	{
	}

	bool InputWindows::IsCursorVisible() const
	{
		return _cursorVisible;
	}

	void InputWindows::SetCursorVisible(bool visible)
	{
		_cursorVisible = visible;
		UpdateCursor();
	}

	void InputWindows::UpdateCursor()
	{
		if (_cursorVisible)
		{
			while (::ShowCursor(TRUE) < 0);
			::SetCursor(_cursor);
		}
		else
		{
			while (::ShowCursor(FALSE) >= 0);
		}
	}
}
