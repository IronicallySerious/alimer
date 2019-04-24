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

#include "input.hpp"

namespace vortice
{
    Input::Input()
    {
    }

    void Input::set_listener(InputListener* listener)
    {
        _listener = listener;
    }

    bool Input::key_event(Key key, KeyState state)
    {
        if (state == KeyState::Released)
            key_state &= ~(1ull << static_cast<unsigned>(key));
        else if (state == KeyState::Pressed || state == KeyState::Repeat)
            key_state |= 1ull << static_cast<unsigned>(key);

        if (_listener)
        {
            // Propagate events.
            KeyboardEvent event(key, state);
            return _listener->key_event(event);
        }

        return false;
    }

} // namespace vortice
