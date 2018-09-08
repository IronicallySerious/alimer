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

#include "Input.SDL2.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>

namespace Alimer
{
    SDL2Input::SDL2Input()
    {

    }

    SDL2Input::~SDL2Input()
    {

    }

    bool SDL2Input::IsCursorVisible() const
    {
        return SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE;
    }

    void SDL2Input::SetCursorVisible(bool visible)
    {
        SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
    }

    MouseButton SDL2Input::ConvertMouseButton(uint8_t sdlButton)
    {
        switch (sdlButton)
        {
        case SDL_BUTTON_LEFT:
            return MouseButton::Left;
        case SDL_BUTTON_MIDDLE:
            return MouseButton::Middle;
        case SDL_BUTTON_RIGHT:
            return MouseButton::Right;
        case SDL_BUTTON_X1:
            return MouseButton::X1;
        case SDL_BUTTON_X2:
            return MouseButton::X2;

        default:
            return MouseButton::None;
        }
    }
}
