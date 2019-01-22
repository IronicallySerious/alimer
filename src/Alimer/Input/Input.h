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

#include "../Core/Platform.h"
#include "../Core/Object.h"
#include "../Math/Math.h"
#include <string>
#include <vector>
#include <bitset>

namespace alimer
{
    /// Defines input key modifiers.
    enum class KeyModifiers : uint8_t
    {
        None = 0,
    };

    /// Defines input mouse buttons.
    enum class MouseButton : uint32_t
    {
        None = 0,
        Left,
        Right,
        Middle,
        X1,
        X2,
        Count
    };

    /// Defines input cursor type
    enum class CursorType : uint32_t
    {
        Arrow = 0,
        Cross,
        Hand,
        Count
    };

    /// Input system class.
    class ALIMER_API Input final : public Object
    {
        ALIMER_OBJECT(Input, Object);

    public:
        /// Constructor.
        Input();

        /// Destructor.
        ~Input() override;

        /// Get if given mouse button is down.
        bool IsMouseButtonDown(MouseButton button);

        /// Get if given mouse button is up.
        bool IsMouseButtonUp(MouseButton button);

        /// Get if given mouse button is held.
        bool IsMouseButtonHeld(MouseButton button);

        
        // Events
        void MouseButtonEvent(MouseButton button, int32_t x, int32_t y, bool pressed);
        void MouseMoveEvent(MouseButton button, int32_t x, int32_t y);

        /// Update input state and poll devices.
        void Update();

    private:
        void PlatformConstruct();

        enum class ActionSlotBits
        {
            Up,
            Down,
            Held,
            Count
        };

        class ActionState
        {
        public:
            ActionState(size_t count);

            void Update();
            void Post(uint32_t slot, bool down, KeyModifiers modifiers = KeyModifiers::None);

            bool IsUp(uint32_t slot, KeyModifiers modifiers = KeyModifiers::None) const;
            bool IsDown(uint32_t slot, KeyModifiers modifiers = KeyModifiers::None) const;
            bool IsHeld(uint32_t slot, KeyModifiers modifiers = KeyModifiers::None) const;

        private:
            bool Test(uint32_t slot, KeyModifiers modifiers, ActionSlotBits bit) const;

            struct ActionSlot
            {
                KeyModifiers modifiers = KeyModifiers::None;
                std::bitset<static_cast<uint32_t>(ActionSlotBits::Count)> bits;
            };

            bool _dirty = false;
            std::vector<ActionSlot> _slots;
        };

        ActionState _mouseButtons;
        ivec2 _mousePosition;
        ivec2 _previousMousePosition;

        bool _cursorVisible = true;

    private:
        Input(const Input&) = delete;
        Input& operator=(const Input&) = delete;
    };
}
