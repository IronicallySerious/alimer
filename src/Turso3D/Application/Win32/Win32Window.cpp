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

#include "../../Base/WString.h"
#include "../../Debug/Log.h"
#include "../../Math/Math.h"
#include "../Window.h"
#include "../Input.h"

#ifndef NOMINMAX
#    define NOMINMAX 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN 1
#endif
#include <Windows.h>

#include "../../Debug/DebugNew.h"

namespace Turso3D
{
    const wchar_t Window::AppWindowClass[] = L"Turso3DWindow";

    void Window::SetTitle(const String& newTitle)
    {
        title = newTitle;
        if (_handle) {
            SetWindowTextW(_handle, WString(title).CString());
        }
    }

    bool Window::SetSize(const IntVector2& size_, bool fullscreen_, bool resizable_)
    {
        inResize = true;
        IntVector2 position = savedPosition;

        if (!fullscreen_)
        {
            windowStyle = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
            if (resizable_)
                windowStyle |= WS_THICKFRAME | WS_MAXIMIZEBOX;

            // Return to desktop resolution if was fullscreen
            if (fullscreen)
                SetDisplayMode(0, 0);
        }
        else
        {
            // When switching to fullscreen, save last windowed mode position
            if (!fullscreen)
                savedPosition = Position();

            windowStyle = WS_POPUP | WS_VISIBLE;
            position = IntVector2::ZERO;
            /// \todo Handle failure to set mode
            SetDisplayMode(size_.x, size_.y);
        }

        DWORD windowExStyle = WS_EX_APPWINDOW;

        RECT rect = { 0, 0, size_.x, size_.y };
        AdjustWindowRectEx(&rect, windowStyle, FALSE, windowExStyle);

        if (!_handle)
        {
            _handle = CreateWindowExW(
                windowExStyle,
                AppWindowClass,
                WString(title).CString(),
                windowStyle,
                position.x, position.y,
                rect.right - rect.left,
                rect.bottom - rect.top,
                0, 0,
                GetModuleHandleW(nullptr),
                nullptr);

            if (!_handle)
            {
                TURSO3D_LOGERROR("Failed to create window");
                inResize = false;
                return false;
            }

            // Enable touch input if available
            RegisterTouchWindow(_handle, TWF_FINETOUCH | TWF_WANTPALM);

            minimized = false;
            focus = false;

            SetWindowLongPtrW(_handle, GWLP_USERDATA, (LONG_PTR)this);
            ShowWindow(_handle, SW_SHOW);
        }
        else
        {
            SetWindowLongW(_handle, GWL_STYLE, windowStyle);

            if (!fullscreen_ && (savedPosition.x == M_MIN_INT || savedPosition.y == M_MIN_INT))
            {
                WINDOWPLACEMENT placement;
                placement.length = sizeof(placement);
                GetWindowPlacement(_handle, &placement);
                position = IntVector2(placement.rcNormalPosition.left, placement.rcNormalPosition.top);
            }

            SetWindowPos(_handle, NULL, position.x, position.y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
            ShowWindow(_handle, SW_SHOW);
        }

        fullscreen = fullscreen_;
        resizable = resizable_;
        inResize = false;

        IntVector2 newSize = ClientRectSize();
        if (newSize != size)
        {
            size = newSize;
            resizeEvent.size = newSize;
            SendEvent(resizeEvent);
        }

        UpdateMouseVisible();
        UpdateMousePosition();

        return true;
    }

    void Window::SetPosition(const IntVector2& position)
    {
        if (_handle) {
            SetWindowPos(_handle, nullptr, position.x, position.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
    }

    void Window::SetMouseVisible(bool enable)
    {
        if (enable != mouseVisible)
        {
            mouseVisible = enable;
            UpdateMouseVisible();
        }
    }

    void Window::SetMousePosition(const IntVector2& position)
    {
        if (_handle)
        {
            mousePosition = position;
            POINT screenPosition;
            screenPosition.x = position.x;
            screenPosition.y = position.y;
            ClientToScreen(_handle, &screenPosition);
            SetCursorPos(screenPosition.x, screenPosition.y);
        }
    }

    void Window::Close()
    {
        if (_handle)
        {
            // Return to desktop resolution if was fullscreen, else save last windowed mode position
            if (fullscreen)
                SetDisplayMode(0, 0);
            else
                savedPosition = Position();

            DestroyWindow(_handle);
            _handle = nullptr;
        }
    }

    void Window::Minimize()
    {
        if (_handle)
            ShowWindow(_handle, SW_MINIMIZE);
    }

    void Window::Maximize()
    {
        if (_handle)
            ShowWindow(_handle, SW_MAXIMIZE);
    }

    void Window::Restore()
    {
        if (_handle)
            ShowWindow(_handle, SW_RESTORE);
    }

    IntVector2 Window::Position() const
    {
        if (_handle)
        {
            RECT rect;
            GetWindowRect(_handle, &rect);
            return IntVector2(rect.left, rect.top);
        }

        return IntVector2::ZERO;
    }

    bool Window::OnWindowMessage(unsigned msg, unsigned wParam, unsigned lParam)
    {
        Input* input = Subsystem<Input>();
        bool handled = false;

        // Skip emulated mouse events that are caused by touch
        bool emulatedMouse = (GetMessageExtraInfo() & 0xffffff00) == 0xff515700;

        switch (msg)
        {
        case WM_DESTROY:
            _handle = nullptr;
            PostQuitMessage(0);
            break;

        case WM_CLOSE:
            SendEvent(closeRequestEvent);
            handled = true;
            break;

        case WM_ACTIVATE:
        {
            bool newFocus = LOWORD(wParam) != WA_INACTIVE;
            if (newFocus != focus)
            {
                focus = newFocus;
                if (focus)
                {
                    SendEvent(gainFocusEvent);
                    if (input)
                        input->OnGainFocus();
                    if (minimized)
                        Restore();

                    // If fullscreen, automatically restore mouse focus
                    if (fullscreen)
                        UpdateMouseVisible();
                }
                else
                {
                    SendEvent(loseFocusEvent);
                    if (input)
                        input->OnLoseFocus();

                    // If fullscreen, minimize on focus loss
                    if (fullscreen)
                        ShowWindow(_handle, SW_MINIMIZE);

                    // Stop mouse cursor hiding & clipping
                    UpdateMouseVisible();
                }
            }
        }
        break;

        case WM_SIZE:
        {
            bool newMinimized = (wParam == SIZE_MINIMIZED);
            if (newMinimized != minimized)
            {
                minimized = newMinimized;
                if (minimized)
                {
                    // If is fullscreen, restore desktop resolution
                    if (fullscreen)
                        SetDisplayMode(0, 0);

                    SendEvent(minimizeEvent);
                }
                else
                {
                    // If should be fullscreen, restore mode now
                    if (fullscreen)
                        SetDisplayMode(size.x, size.y);

                    SendEvent(restoreEvent);
                }
            }

            if (!minimized && !inResize)
            {
                IntVector2 newSize = ClientRectSize();
                if (newSize != size)
                {
                    size = newSize;
                    resizeEvent.size = newSize;
                    SendEvent(resizeEvent);
                }
            }

            // If mouse is currently hidden, update the clip region
            if (!mouseVisibleInternal)
                UpdateMouseClipping();
        }
        break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (input)
                input->OnKey(wParam, (lParam >> 16) & 0xff, true);
            handled = (msg == WM_KEYDOWN);
            break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (input)
                input->OnKey(wParam, (lParam >> 16) & 0xff, false);
            handled = (msg == WM_KEYUP);
            break;

        case WM_CHAR:
            if (input)
                input->OnChar(wParam);
            handled = true;
            break;

        case WM_MOUSEMOVE:
            if (input && !emulatedMouse)
            {
                IntVector2 newPosition;
                newPosition.x = (int)(short)LOWORD(lParam);
                newPosition.y = (int)(short)HIWORD(lParam);

                // Do not transmit mouse move when mouse should be hidden, but is not due to no input focus
                if (mouseVisibleInternal == mouseVisible)
                {
                    IntVector2 delta = newPosition - mousePosition;
                    input->OnMouseMove(newPosition, delta);
                    // Recenter in hidden mouse cursor mode to allow endless relative motion
                    if (!mouseVisibleInternal && delta != IntVector2::ZERO)
                        SetMousePosition(Size() / 2);
                    else
                        mousePosition = newPosition;
                }
                else
                    mousePosition = newPosition;
            }
            handled = true;
            break;

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            if (input && !emulatedMouse)
            {
                MouseButton button = (msg == WM_LBUTTONDOWN) ? MouseButton::Left : (msg == WM_MBUTTONDOWN) ? MouseButton::Middle : MouseButton::Right;
                input->OnMouseButton(button, true);
                // Make sure we track the button release even if mouse moves outside the window
                SetCapture(_handle);
                // Re-establish mouse cursor hiding & clipping
                if (!mouseVisible && mouseVisibleInternal)
                    UpdateMouseVisible();
            }
            handled = true;
            break;

        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            if (input && !emulatedMouse)
            {
                MouseButton button = (msg == WM_LBUTTONUP) ? MouseButton::Left : (msg == WM_MBUTTONUP) ? MouseButton::Middle : MouseButton::Right;
                input->OnMouseButton(button, false);
                // End capture when there are no more mouse buttons held down
                if (!input->MouseButtons()) {
                    ReleaseCapture();
                }
            }
            handled = true;
            break;

        case WM_TOUCH:
            if (input && LOWORD(wParam))
            {
                UINT num_touches = LOWORD(wParam);
                Vector<TOUCHINPUT> touches(num_touches);

                if (GetTouchInputInfo((HTOUCHINPUT)lParam, num_touches, touches.Data(), sizeof(TOUCHINPUT)))
                {
                    for (auto it = touches.Begin(); it != touches.End(); ++it)
                    {
                        // Translate touch points inside window
                        POINT point;
                        point.x = it->x / 100;
                        point.y = it->y / 100;
                        ScreenToClient(_handle, &point);
                        IntVector2 position(point.x, point.y);

                        if (it->dwFlags & (TOUCHEVENTF_DOWN || TOUCHEVENTF_UP))
                            input->OnTouch(it->dwID, true, position, 1.0f);
                        else if (it->dwFlags & TOUCHEVENTF_UP)
                            input->OnTouch(it->dwID, false, position, 1.0f);

                        // Mouse cursor will move to the position of the touch on next move, so move beforehand
                        // to prevent erratic jumps in hidden mouse mode
                        if (!mouseVisibleInternal)
                            mousePosition = position;
                    }
                }
            }

            CloseTouchInputHandle((HTOUCHINPUT)lParam);
            handled = true;
            break;

        case WM_SYSCOMMAND:
            switch ((wParam & 0xFFF0))
            {
            case SC_SCREENSAVE:
            case SC_MONITORPOWER:
            {
                const bool isScreenSaverEnabled = true;
                if (!isScreenSaverEnabled)
                {
                    // Disable screensaver
                    return true;
                }
                break;
            }
            // Prevent system bell sound from Alt key combinations
            case SC_KEYMENU:
                return true;
                break;
            }

            break;

        case WM_DPICHANGED:
        {
            UINT dpiX = LOWORD(wParam);
            UINT dpiY = HIWORD(wParam);
        }
        break;
        }

        return handled;
    }

    IntVector2 Window::ClientRectSize() const
    {
        if (_handle)
        {
            RECT rect;
            GetClientRect(_handle, &rect);
            return IntVector2(rect.right, rect.bottom);
        }

        return IntVector2::ZERO;
    }

    void Window::SetDisplayMode(int width, int height)
    {
        if (width && height)
        {
            DEVMODE screenMode;
            screenMode.dmSize = sizeof screenMode;
            screenMode.dmPelsWidth = width;
            screenMode.dmPelsHeight = height;
            screenMode.dmBitsPerPel = 32;
            screenMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
            ChangeDisplaySettings(&screenMode, CDS_FULLSCREEN);
        }
        else
            ChangeDisplaySettings(nullptr, CDS_FULLSCREEN);
    }

    void Window::UpdateMouseVisible()
    {
        if (!_handle)
            return;

        // When the window is unfocused, mouse should never be hidden
        bool newMouseVisible = HasFocus() ? mouseVisible : true;
        if (newMouseVisible != mouseVisibleInternal)
        {
            ShowCursor(newMouseVisible ? TRUE : FALSE);
            mouseVisibleInternal = newMouseVisible;
        }

        UpdateMouseClipping();
    }

    void Window::UpdateMouseClipping()
    {
        if (mouseVisibleInternal)
            ClipCursor(nullptr);
        else
        {
            RECT mouseRect;
            POINT point;
            IntVector2 windowSize = Size();

            point.x = point.y = 0;
            ClientToScreen(_handle, &point);
            mouseRect.left = point.x;
            mouseRect.top = point.y;
            mouseRect.right = point.x + windowSize.x;
            mouseRect.bottom = point.y + windowSize.y;
            ClipCursor(&mouseRect);
        }
    }

    void Window::UpdateMousePosition()
    {
        POINT screenPosition;
        GetCursorPos(&screenPosition);
        ScreenToClient(_handle, &screenPosition);
        mousePosition.x = screenPosition.x;
        mousePosition.y = screenPosition.y;
    }
}
