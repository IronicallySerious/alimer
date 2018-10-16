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

#include "../../Application/Window.h"
#include "../../Application/Application.h"
#include "../../Core/Log.h"

// Dropfile support.
#include <shellapi.h>
#include <VersionHelpers.h>

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
        ALIMER_UNUSED(msg);
        ALIMER_UNUSED(wParam);
        ALIMER_UNUSED(lParam);

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
        ALIMER_UNUSED(wParam);
        ALIMER_UNUSED(lParam);
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
                Application::GetInstance()->Resume();
            }
            else
            {
                Application::GetInstance()->Pause();
            }

            return DefWindowProcW(hwnd, msg, wParam, lParam);
        }

        auto* window = reinterpret_cast<Alimer::Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (!window)
            return DefWindowProcW(hwnd, msg, wParam, lParam);

        return window->HandleMessage(msg, wParam, lParam);
    }

    // OleDropTarget.
    // https://blogs.msdn.microsoft.com/oldnewthing/20100503-00/?p=14183
    class Win32OleDropTarget final : public IDropTarget
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
            ALIMER_UNUSED(pDataObj);
            ALIMER_UNUSED(grfKeyState);
            ALIMER_UNUSED(pt);
            *pdwEffect &= DROPEFFECT_COPY;
            return S_OK;
        }

        HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, __RPC__inout DWORD *pdwEffect) override
        {
            ALIMER_UNUSED(grfKeyState);
            ALIMER_UNUSED(pt);
            *pdwEffect &= DROPEFFECT_COPY;
            return S_OK;
        }

        HRESULT __stdcall DragLeave(void) override
        {
            return S_OK;
        }

        HRESULT __stdcall Drop(__RPC__in_opt IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, __RPC__inout DWORD *pdwEffect) override
        {
            ALIMER_UNUSED(pDataObj);
            ALIMER_UNUSED(grfKeyState);
            ALIMER_UNUSED(pt);
            *pdwEffect &= DROPEFFECT_COPY;
            return S_OK;
        }

    private:
        LONG _oleRefCount = 0;
    };

    void Window::PlatformConstruct()
    {
        _dropTarget = new Win32OleDropTarget();

        _hInstance = GetModuleHandleW(nullptr);

        if (_windowCount == 0)
        {
            WNDCLASSEXW wc;
            wc.cbSize = sizeof(WNDCLASSEX);
            wc.style = CS_HREDRAW | CS_VREDRAW;
            wc.lpfnWndProc = WndProc;
            wc.cbClsExtra = 0;
            wc.cbWndExtra = 0;
            wc.hInstance = _hInstance;
            wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wc.hIconSm = nullptr;
            wc.hbrBackground = nullptr;
            wc.lpszMenuName = nullptr;  // No default menu
            wc.lpszClassName = AppWindowClass;
            wc.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

            if (!::RegisterClassExW(&wc))
            {
                ALIMER_LOGERROR("[Win32] - Failed to register Win32 class.");
                return;
            }

            // Initialize OLE for Drag/Drop support.
            HRESULT result = OleInitialize(nullptr);
            if (result != S_OK && result != S_FALSE)
            {
                ALIMER_LOGERROR("[Win32] - OleInitialize failed.");
            }
        }
    }

    void Window::PlatformDestroy()
    {
        if (IsWindow(_handle))
        {
            RevokeDragDrop(_handle);
            _dropTarget = nullptr;

            HWND destroyHandle = _handle;
            _handle = nullptr;
            ::DestroyWindow(destroyHandle);
        }
    }

    bool Window::PlatformCreate()
    {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        const bool resizable = any(_flags & WindowFlags::Resizable);
        bool fullscreen = any(_flags & WindowFlags::Fullscreen);

        if (fullscreen)
        {
            DEVMODEW dmScreenSettings;
            memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
            dmScreenSettings.dmSize = sizeof(dmScreenSettings);
            dmScreenSettings.dmPelsWidth = screenWidth;
            dmScreenSettings.dmPelsHeight = screenHeight;
            dmScreenSettings.dmBitsPerPel = 32;
            dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
            if ((_size.x != (uint32_t)screenWidth) && (_size.y != (uint32_t)screenHeight))
            {
                if (ChangeDisplaySettingsW(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
                {
                    if (MessageBoxW(nullptr, L"Fullscreen Mode not supported!\n Switch to window mode?", L"Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
                    {
                        fullscreen = false;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
        }

        DWORD dwExStyle;
        DWORD dwStyle;

        if (fullscreen)
        {
            dwExStyle = WS_EX_APPWINDOW;
            dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        }
        else
        {
            dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
            dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

            if (resizable)
                dwStyle |= WS_SIZEBOX | WS_MAXIMIZEBOX;
        }

        RECT windowRect;
        windowRect.left = 0L;
        windowRect.top = 0L;
        windowRect.right = fullscreen ? (long)screenWidth : (long)_size.x;
        windowRect.bottom = fullscreen ? (long)screenHeight : (long)_size.y;
        AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

        wchar_t titleBuffer[256] = L"";

        if (!_title.empty()
            && MultiByteToWideChar(CP_UTF8, 0, _title.c_str(), -1, titleBuffer, 256) == 0)
        {
            ALIMER_LOGERROR("[Win32] - Failed to convert UTF-8 to wide char");
            return false;
        }

        int x = 0;
        int y = 0;
        if (!fullscreen)
        {
            x = (screenWidth - windowRect.right) / 2;
            y = (screenHeight - windowRect.bottom) / 2;
        }

        _handle = CreateWindowExW(
            dwExStyle,
            AppWindowClass,
            titleBuffer,
            dwStyle,
            x,
            y,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr,
            nullptr,
            _hInstance,
            nullptr);

        if (!_handle)
        {
            ALIMER_LOGERROR("[Win32] - Failed to create window");
            return false;
        }

        _valid = true;
        _windowCount++;
        InitAfterCreation();
        ShowWindow(_handle, _showCommand);
        SetWindowLongPtrW(_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        return true;
    }

    void Window::InitAfterCreation()
    {
        _monitor = MonitorFromWindow(_handle, MONITOR_DEFAULTTONEAREST);

        RECT windowRect;
        GetClientRect(_handle, &windowRect);
        _size.x = static_cast<uint32_t>(windowRect.right - windowRect.left);
        _size.y = static_cast<uint32_t>(windowRect.bottom - windowRect.top);

        if (RegisterTouchWindow(_handle, 0) == FALSE)
        {
            ALIMER_LOGWARN("[Win32] - Failed to enable touch for window");
        }

        // Enable dropping files.
        DragAcceptFiles(_handle, TRUE);

        if (RegisterDragDrop(_handle, _dropTarget) != S_OK)
        {
            ALIMER_LOGWARN("[Win32] - Failed to register drag and drop for window");
        }
    }

    bool Window::IsMinimized() const
    {
        return !!::IsIconic(_handle);
    }

    void Window::Show()
    {
        if (_visible)
            return;

        _visible = true;
        if (IsWindow(_handle))
        {
            ::ShowWindow(_handle, _showCommand);
        }
        else
        {
            _showCommand = SW_SHOW;
        }
    }

    void Window::Hide()
    {
        if (!_visible)
            return;

        _visible = false;
        if (IsWindow(_handle))
        {
            ::ShowWindow(_handle, SW_HIDE);
        }
        else
        {
            _showCommand = SW_HIDE;
        }
    }

    void Window::Minimize()
    {
        if (IsWindow(_handle))
        {
            ShowWindow(_handle, SW_MINIMIZE);
        }
        else
        {
            _showCommand = SW_MINIMIZE;
        }
    }

    void Window::Maximize()
    {
        if (IsWindow(_handle))
        {
            ShowWindow(_handle, SW_MAXIMIZE);
        }
        else
        {
            _showCommand = SW_MAXIMIZE;
        }
    }

    void Window::Restore()
    {
        if (IsWindow(_handle))
        {
            ShowWindow(_handle, SW_RESTORE);
        }
        else
        {
            _showCommand = SW_RESTORE;
        }
    }

    void Window::Close()
    {
        const bool shouldCancel = false;
        if (!shouldCancel)
        {
            PlatformDestroy();
            //ShouldClose = true;
            //SendEvent(closeRequestEvent);
        }
    }

    void Window::SetTitle(const std::string& newTitle)
    {
        _title = newTitle;
        if (IsWindow(_handle))
        {
            wchar_t titleBuffer[256] = L"";

            if (!newTitle.empty() && MultiByteToWideChar(CP_UTF8, 0, newTitle.c_str(), -1, titleBuffer, 256) == 0)
            {
                ALIMER_LOGERROR("[Win32] - Failed to convert UTF-8 to wide char");
            }

            SetWindowTextW(_handle, titleBuffer);
        }
    }

    LRESULT Window::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_CLOSE:
        {
            Close();
            return 0;
        }

        case WM_DESTROY:
        {
            _valid = false;
            _windowCount--;
            if (_windowCount == 0)
            {
                PostQuitMessage(0);
            }

            return 0;
        }

        case WM_ACTIVATE:
        {
            bool newFocus = LOWORD(wParam) != WA_INACTIVE;
            _focused = newFocus;
        }
        break;

        /*case WM_SETCURSOR:
        {
            if (LOWORD(lParam) == HTCLIENT)
            {
                //static_cast<Input*>(Input::GetInstance())->UpdateCursor();
                return TRUE;
            }
            break;
        }*/

        case WM_SIZE:
        {
            switch (wParam)
            {
            case SIZE_MINIMIZED:
                Application::GetInstance()->Pause();
                break;
            case SIZE_RESTORED:
                Application::GetInstance()->Resume();
                HandleResize(wParam, lParam);
                break;
            case SIZE_MAXIMIZED:
                HandleResize(wParam, lParam);
                break;
            }
            return 0;
        }

        break;

        case WM_ERASEBKGND:
            return 1;

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
        break;

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

        return DefWindowProcW(_handle, msg, wParam, lParam);
    }

    void Window::HandleResize(WPARAM wParam, LPARAM lParam)
    {
        ALIMER_UNUSED(wParam);
        _monitor = MonitorFromWindow(_handle, MONITOR_DEFAULTTONEAREST);

        _size.x = static_cast<uint32_t>(LOWORD(lParam));
        _size.y = static_cast<uint32_t>(HIWORD(lParam));
        resizeEvent.size = _size;
        SendEvent(resizeEvent);
    }
}
