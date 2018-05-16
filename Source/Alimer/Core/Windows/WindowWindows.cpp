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

#include "WindowWindows.h"
#include "EngineWindows.h"

// Dropfile support.
#include <shellapi.h>

#include <Ole2.h>
#include <oleidl.h>

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#define SIGNATURE_MASK 0x0FFFFFF00
#define MOUSEEVENTF_FROMTOUCH 0x0FF515700

namespace Alimer
{
	static uint32_t _windowCount = 0;
	static const LPCWSTR AppWindowClass = L"AlimerWindow";

	static void HandleMouseButtonEvent(UINT msg, WPARAM wParam, LPARAM lParam)
	{
#if TODO
		Vector2 position(
			static_cast<float>(GET_X_LPARAM(lParam)),
			static_cast<float>(GET_Y_LPARAM(lParam)));

		const bool pressed =
			msg == WM_LBUTTONDOWN
			|| msg == WM_RBUTTONDOWN
			|| msg == WM_MBUTTONDOWN
			|| msg == WM_XBUTTONDOWN;
		MouseButton button;

		if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP)
		{
			button = MouseButton::Left;
		}
		else if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)
		{
			button = MouseButton::Right;
		}
		else if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP)
		{
			button = MouseButton::Middle;
		}
		else if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONUP)
		{
			if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
				button = MouseButton::X1;
			else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2)
				button = MouseButton::X2;
			else
				return;
		}
		else
		{
			return;
		}

		gInput().PostMouseButtonEvent(position, button, pressed);
#endif // TODO

	}

	static void HandleMouseMotionEvent(WPARAM wParam, LPARAM lParam)
	{
#if TODO
		Vector2 position(
			static_cast<float>(GET_X_LPARAM(lParam)),
			static_cast<float>(GET_Y_LPARAM(lParam)));
		gInput().PostMouseMotionEvent(position);
#endif // TODO

	}

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg == WM_ACTIVATEAPP)
		{
			if (wParam)
			{
				engine->Resume();
			}
			else
			{
				engine->Pause();
			}

			return DefWindowProcW(hwnd, msg, wParam, lParam);
		}

		auto* window = reinterpret_cast<Alimer::WindowWindows*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		if (!window)
			return DefWindowProcW(hwnd, msg, wParam, lParam);

		switch (msg)
		{
			case WM_CLOSE:
			{
				window->Close();
				return 0;
			}

			case WM_DESTROY:
			{
				_windowCount--;
				if (_windowCount == 0)
					PostQuitMessage(0);
				break;
			}

			case WM_ACTIVATE:
			{
				bool newFocus = LOWORD(wParam) != WA_INACTIVE;
				window->Activate(newFocus);
			}
			break;

			case WM_SETCURSOR:
			{
				if (LOWORD(lParam) == HTCLIENT)
				{
					//static_cast<Win32Input*>(Input::GetInstance())->UpdateCursor();
					return TRUE;
				}
				break;
			}

			case WM_SYSCOMMAND:
			{
				switch (wParam)
				{
					case SC_SCREENSAVE:
					case SC_MONITORPOWER:
					{
						//if (!IsScreenSaverEnabled())
						{
							// Disable screensaver
							return 0;
						}
						break;
					}
					// Disable accessing menu using alt key
					case SC_KEYMENU:
						return 0;
				}
				break;
			}

			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_XBUTTONDOWN:
			case WM_XBUTTONUP:
			{
				LONG_PTR extraInfo = GetMessageExtraInfo();

				// don't handle mouse event that came from touch
				if ((extraInfo & SIGNATURE_MASK) != MOUSEEVENTF_FROMTOUCH)
				{
					HandleMouseButtonEvent(msg, wParam, lParam);

					if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONUP)
						return TRUE;
				}

				return 0;
			}

			case WM_MOUSEMOVE:
			{
				LPARAM extraInfo = GetMessageExtraInfo();

				// don't handle mouse event that came from touch
				if ((extraInfo & SIGNATURE_MASK) != MOUSEEVENTF_FROMTOUCH)
				{
					HandleMouseMotionEvent(wParam, lParam);
				}

				return 0;
			}

			case WM_DROPFILES:
			{
				HDROP drop = (HDROP)wParam;
				UINT count = DragQueryFileW(drop, 0xFFFFFFFF, NULL, 0);
				for (UINT i = 0; i < count; ++i)
				{
				}

				DragFinish(drop);
				return 0;
			}
		}

		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}

	// OleDropTarget.
	// https://blogs.msdn.microsoft.com/oldnewthing/20100503-00/?p=14183
	class OleDropTarget final : public IDropTarget
	{
	public:
		// IUnknown interface
		HRESULT __stdcall QueryInterface(REFIID riid, void ** ppvObject) override
		{
			if (riid == IID_IUnknown || riid == IID_IDropTarget)
			{
				*ppvObject = static_cast<IUnknown*>(this);
				AddRef();
				return S_OK;
			}
			*ppvObject = nullptr;
			return E_NOINTERFACE;
		}

		ULONG __stdcall AddRef(void) override
		{
			return InterlockedIncrement(&_oleRefCount);
		}

		ULONG __stdcall Release(void) override
		{
			LONG cRef = InterlockedDecrement(&_oleRefCount);
			if (cRef == 0) delete this;
			return cRef;
		}

		// IDropTarget interface
		HRESULT __stdcall DragEnter(__RPC__in_opt IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, __RPC__inout DWORD *pdwEffect) override
		{
			*pdwEffect &= DROPEFFECT_COPY;
			return S_OK;
		}

		HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, __RPC__inout DWORD *pdwEffect) override
		{
			*pdwEffect &= DROPEFFECT_COPY;
			return S_OK;
		}

		HRESULT __stdcall DragLeave(void) override
		{
			return S_OK;
		}

		HRESULT __stdcall Drop(__RPC__in_opt IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, __RPC__inout DWORD *pdwEffect) override
		{
			*pdwEffect &= DROPEFFECT_COPY;
			return S_OK;
		}

	private:
		LONG _oleRefCount = 0;
	};

	WindowWindows::WindowWindows(HINSTANCE hInstance)
		: _dropTarget(new OleDropTarget())
	{
		_hInstance = hInstance;
		if (_hInstance)
		{
			// If not hInstance sent, use main module one.
			_hInstance = GetModuleHandleW(nullptr);
		}

		if (_windowCount == 0)
		{
			WNDCLASSEX wc;
			wc.cbSize = sizeof(WNDCLASSEX);
			wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wc.lpfnWndProc = WndProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = _hInstance;
			wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
			wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
			wc.hIconSm = nullptr;
			wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));;
			wc.lpszMenuName = nullptr;  // No default menu
			wc.lpszClassName = AppWindowClass;

			if (!::RegisterClassExW(&wc))
			{
				//ALIMER_LOGERROR("[Win32] - Failed to register window class.");
				return;
			}

			// Initialize OLE for Drag/Drop support.
			HRESULT result = OleInitialize(nullptr);
			if (result != S_OK && result != S_FALSE)
			{
				//ALIMER_LOGERROR("[Win32] - OleInitialize failed.");
			}
		}

		_windowStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPSIBLINGS | WS_BORDER | WS_DLGFRAME | WS_THICKFRAME | WS_GROUP | WS_TABSTOP;
		_windowExStyle = WS_EX_APPWINDOW;

		if (_resizable)
			_windowStyle |= WS_SIZEBOX | WS_MAXIMIZEBOX;

		int x = CW_USEDEFAULT;
		int y = CW_USEDEFAULT;

		RECT windowRect = { 0, 0, static_cast<LONG>(_width), static_cast<LONG>(_height) };
		AdjustWindowRectEx(&windowRect, _windowStyle, FALSE, _windowExStyle);

		int width = CW_USEDEFAULT;
		int height = CW_USEDEFAULT;
		if (_width > 0)
			width = windowRect.right - windowRect.left;
		if (_height > 0)
			height = windowRect.bottom - windowRect.top;

		wchar_t titleBuffer[256] = L"";

		if (!_title.empty()
			&& MultiByteToWideChar(CP_UTF8, 0, _title.c_str(), -1, titleBuffer, 256) == 0)
		{
			//ALIMER_LOGERROR("[Win32] - Failed to convert UTF-8 to wide char");
			return;
		}

		_hwnd =
			CreateWindowExW(
				_windowExStyle,
				AppWindowClass,
				titleBuffer,
				_windowStyle,
				x, y, width, height,
				nullptr,
				nullptr,
				_hInstance,
				nullptr);

		if (!_hwnd)
		{
			//ALIMER_LOGERROR("[Win32] - Failed to create window");
			return;
		}

		//_created = true;
		_windowCount++;
		_hwnd = _hwnd;
		//_platformData.hdc = GetDC(_hwnd);

		InitAfterCreation();
		Show();
		::SetWindowLongPtrW(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}

	WindowWindows::~WindowWindows()
	{
		_dropTarget = nullptr;
	}

	void WindowWindows::InitAfterCreation()
	{
		_monitor = MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST);

		RECT windowRect;
		GetClientRect(_hwnd, &windowRect);
		_width = static_cast<uint32_t>(windowRect.right - windowRect.left);
		_height = static_cast<uint32_t>(windowRect.bottom - windowRect.top);

		if (RegisterTouchWindow(_hwnd, 0) == FALSE)
		{
			//ALIMER_LOGWARN("[Win32] - Failed to enable touch for window");
		}

		// Enable dropping files.
		DragAcceptFiles(_hwnd, TRUE);

		if (RegisterDragDrop(_hwnd, _dropTarget) != S_OK)
		{
			//ALIMER_LOGWARN("[Win32] - Failed to register drag and drop for window");
		}
	}

	void WindowWindows::Show()
	{
		if (_visible)
			return;

		_visible = true;
		::ShowWindow(_hwnd, _showCommand);
	}

	void WindowWindows::Close()
	{
		const bool shouldCancel = false;
		if (!shouldCancel)
		{
			Destroy();
			//ShouldClose = true;
			//SendEvent(closeRequestEvent);
		}
	}

	void WindowWindows::Destroy()
	{
		if (IsWindow(_hwnd))
		{
			HRESULT result = RevokeDragDrop(_hwnd);
			_dropTarget = nullptr;
		}

		HWND destroyHandle = _hwnd;
		_hwnd = nullptr;
		::DestroyWindow(destroyHandle);
	}

	void WindowWindows::Activate(bool focused)
	{
		if (_focused == focused)
			return;

		_focused = focused;
	}
}
