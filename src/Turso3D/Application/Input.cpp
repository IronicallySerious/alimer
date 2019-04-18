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

#include "Input.h"
#include "Window.h"
#include "../Debug/DebugNew.h"

namespace Turso3D
{
    Input::Input()
        : mouseButtons(0)
        , mouseButtonsPressed(0)
    {
        RegisterSubsystem(this);
    }

    Input::~Input()
    {
        RemoveSubsystem(this);
    }

    void Input::Update()
    {
        // Clear accumulated input from last frame
        mouseButtonsPressed = 0;
        mouseMove = IntVector2::ZERO;
        keyPressed.Clear();
        rawKeyPress.Clear();
        for (auto it = touches.Begin(); it != touches.End(); ++it)
        {
            it->delta = IntVector2::ZERO;
        }
    }

    bool Input::IsKeyDown(unsigned keyCode) const
    {
        auto it = keyDown.Find(keyCode);
        return it != keyDown.End() ? it->second : false;
    }

    bool Input::IsKeyDownRaw(unsigned rawKeyCode) const
    {
        auto it = rawKeyDown.Find(rawKeyCode);
        return it != rawKeyDown.End() ? it->second : false;
    }

    bool Input::IsKeyPress(unsigned keyCode) const
    {
        auto it = keyPressed.Find(keyCode);
        return it != keyPressed.End() ? it->second : false;
    }

    bool Input::IsKeyPressRaw(unsigned rawKeyCode) const
    {
        auto it = rawKeyPress.Find(rawKeyCode);
        return it != rawKeyPress.End() ? it->second : false;
    }

    const IntVector2& Input::GetMousePosition() const
    {
        Window* window = Subsystem<Window>();
        return window ? window->MousePosition() : IntVector2::ZERO;
    }

    const IntVector2& Input::GetMousePosition(const Window& relativeTo) const
    {
        return relativeTo.MousePosition();
    }

    bool Input::IsMouseButtonDown(unsigned button) const
    {
        return (mouseButtons & (1 << button)) != 0;
    }

    bool Input::IsMouseButtonPress(unsigned button) const
    {
        return (mouseButtonsPressed & (1 << button)) != 0;
    }

    const Touch* Input::FindTouch(unsigned id) const
    {
        for (auto it = touches.Begin(); it != touches.End(); ++it)
        {
            if (it->id == id)
                return it.ptr;
        }

        return nullptr;
    }

    void Input::OnKey(unsigned keyCode, unsigned rawKeyCode, bool pressed)
    {
        bool wasDown = IsKeyDown(keyCode);

        keyDown[keyCode] = pressed;
        rawKeyDown[rawKeyCode] = pressed;
        if (pressed)
        {
            keyPressed[keyCode] = true;
            rawKeyPress[rawKeyCode] = true;
        }

        keyEvent.keyCode = keyCode;
        keyEvent.rawKeyCode = rawKeyCode;
        keyEvent.pressed = pressed;
        keyEvent.repeat = wasDown;
        SendEvent(keyEvent);
    }

    void Input::OnChar(unsigned unicodeChar)
    {
        charInputEvent.unicodeChar = unicodeChar;
        SendEvent(charInputEvent);
    }

    void Input::OnMouseMove(const IntVector2& position, const IntVector2& delta)
    {
        mouseMove += delta;

        mouseMoveEvent.position = position;
        mouseMoveEvent.delta = delta;
        SendEvent(mouseMoveEvent);
    }

    void Input::OnMouseButton(MouseButton button, bool pressed)
    {
        uint32_t bit = 1 << static_cast<uint32_t>(button);

        if (pressed) {
            mouseButtons |= bit;
            mouseButtonsPressed |= bit;
        }
        else {
            mouseButtons &= ~bit;
        }

        mouseButtonEvent.button = button;
        mouseButtonEvent.buttons = mouseButtons;
        mouseButtonEvent.pressed = pressed;
        mouseButtonEvent.position = GetMousePosition();
        SendEvent(mouseButtonEvent);
    }

    void Input::OnTouch(uint32_t internalId, bool pressed, const IntVector2& position, float pressure)
    {
        if (pressed)
        {
            bool found = false;

            // Ongoing touch
            for (auto it = touches.Begin(); it != touches.End(); ++it)
            {
                if (it->internalId == internalId)
                {
                    found = true;
                    it->lastDelta = position - it->position;

                    if (it->lastDelta != IntVector2::ZERO || pressure != it->pressure)
                    {
                        it->delta += it->lastDelta;
                        it->position = position;
                        it->pressure = pressure;

                        touchMoveEvent.id = it->id;
                        touchMoveEvent.position = it->position;
                        touchMoveEvent.delta = it->lastDelta;
                        touchMoveEvent.pressure = it->pressure;
                        SendEvent(touchMoveEvent);
                    }

                    break;
                }
            }

            // New touch
            if (!found)
            {
                // Use the first gap in current touches, or insert to the end if no gaps
                size_t insertIndex = touches.Size();
                unsigned newId = (unsigned)touches.Size();

                for (size_t i = 0; i < touches.Size(); ++i)
                {
                    if (touches[i].id > i)
                    {
                        insertIndex = i;
                        newId = i ? touches[i - 1].id + 1 : 0;
                        break;
                    }
                }

                Touch newTouch;
                newTouch.id = newId;
                newTouch.internalId = internalId;
                newTouch.position = position;
                newTouch.pressure = pressure;
                touches.Insert(insertIndex, newTouch);

                touchBeginEvent.id = newTouch.id;
                touchBeginEvent.position = newTouch.position;
                touchBeginEvent.pressure = newTouch.pressure;
                SendEvent(touchBeginEvent);
            }
        }
        else
        {
            // End touch
            for (auto it = touches.Begin(); it != touches.End(); ++it)
            {
                if (it->internalId == internalId)
                {
                    it->position = position;
                    it->pressure = pressure;

                    touchEndEvent.id = it->id;
                    touchEndEvent.position = it->position;
                    SendEvent(touchEndEvent);
                    touches.Erase(it);
                    break;
                }
            }
        }
    }

    void Input::OnGainFocus()
    {
    }

    void Input::OnLoseFocus()
    {
        mouseButtons = 0;
        mouseButtonsPressed = 0;
        mouseMove = IntVector2::ZERO;
        keyDown.Clear();
        keyPressed.Clear();
        rawKeyDown.Clear();
        rawKeyPress.Clear();
    }

}
