//
// Copyright (c) 2017-2019 Amer Koleci and contributors.
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

#include "core/platform.h"

#include <stdint.h>     // uint32_t, int64_t, etc.

namespace vortice
{
    enum class Key : unsigned
    {
        Unknown,
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        Return,
        LeftCtrl,
        LeftAlt,
        LeftShift,
        Space,
        Escape,
        Left, Right, Up, Down,
        Count
    };

    enum class KeyState
    {
        Pressed,
        Released,
        Repeat,
        Count
    };

    class KeyboardEvent
    {
    public:
        KeyboardEvent(Key key_, KeyState state_)
            : key(key_), state(state_)
        {
        }

        Key get_key() const
        {
            return key;
        }

        KeyState get_key_state() const
        {
            return state;
        }

    private:
        Key key;
        KeyState state;
    };

    class VORTICE_API InputListener
    {
    public:
        virtual bool key_event(const KeyboardEvent& evt) {
            return false;
        }
    };

    class VORTICE_API Input
    {
        friend class Application;

    public:
        Input();

        bool key_event(Key key, KeyState state);

        bool key_pressed(Key key) const
        {
            return (key_state & (1ull << static_cast<unsigned>(key))) != 0;
        }

        void set_listener(InputListener* listener);

    private:
        uint64_t key_state = 0;
        InputListener* _listener = nullptr;
    };

} // namespace vortice
