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

#include "../Input/Input.h"
#include "../Core/Log.h"

namespace alimer
{
    Input::Input()
        : _mouseButtons(static_cast<uint32_t>(MouseButton::Count))
    {
        PlatformConstruct();
        AddSubsystem(this);
    }

    Input::~Input()
    {
        RemoveSubsystem(this);
    }

    void Input::Update()
    {
        _mouseButtons.Update();
        _previousMousePosition = _mousePosition;
    }

    bool Input::IsMouseButtonDown(MouseButton button)
    {
        return _mouseButtons.IsDown(static_cast<uint32_t>(button));
    }

    bool Input::IsMouseButtonUp(MouseButton button)
    {
        return _mouseButtons.IsUp(static_cast<uint32_t>(button));
    }

    bool Input::IsMouseButtonHeld(MouseButton button)
    {
        return _mouseButtons.IsHeld(static_cast<uint32_t>(button));
    }

    void Input::MouseButtonEvent(MouseButton button, int32_t x, int32_t y, bool pressed)
    {
        _mouseButtons.Post(static_cast<uint32_t>(button), pressed);
        _mousePosition.x = x;
        _mousePosition.y = y;
    }

    void Input::MouseMoveEvent(MouseButton button, int32_t x, int32_t y)
    {
        ALIMER_UNUSED(button);
        _mousePosition.x = x;
        _mousePosition.y = y;
    }

    bool IsModifierOnlyButton(uint32_t button)
    {
        return button == 0;
    }

    static bool KeyModifiersMatch(uint32_t slot, KeyModifiers modifiers, KeyModifiers stateModifiers)
    {
        // TODO: better enum handle
        const bool areModifiersActive = (ecast(modifiers) & ecast(stateModifiers)) == ecast(modifiers);
        const bool areOnlyModifiersActive = modifiers == stateModifiers;
        return IsModifierOnlyButton(slot) ? areModifiersActive : areOnlyModifiersActive;
    }

    // ActionState
    Input::ActionState::ActionState(size_t count)
        : _dirty(false)
    {
        _slots.resize(count);
    }

    void Input::ActionState::Update()
    {
        if (!_dirty)
            return;

        for (ActionSlot& slot : _slots)
        {
            if (slot.bits.any())
            {
                slot.bits.reset(static_cast<uint32_t>(ActionSlotBits::Down));
                slot.bits.reset(static_cast<uint32_t>(ActionSlotBits::Up));
                slot.modifiers = slot.bits.test(static_cast<uint32_t>(ActionSlotBits::Held)) ? slot.modifiers : KeyModifiers::None;
            }
        }

        _dirty = false;
    }

    void Input::ActionState::Post(uint32_t slot, bool down, KeyModifiers modifiers)
    {
        ActionSlot& actionSlot = _slots[slot];
        const bool wasHeld = actionSlot.bits.test(static_cast<uint32_t>(ActionSlotBits::Held));
        const bool isReleasing = wasHeld && !down;
        const bool hasModifiersRemaining = modifiers != KeyModifiers::None;
        const bool isModifierUpdate = IsModifierOnlyButton(slot) && isReleasing && hasModifiersRemaining;
        if (isModifierUpdate)
        {
            actionSlot.modifiers = modifiers;
        }
        else
        {
            _dirty = true;
            actionSlot.bits.set(static_cast<uint32_t>(ActionSlotBits::Down), !wasHeld && down);
            actionSlot.bits.set(static_cast<uint32_t>(ActionSlotBits::Held), down);
            actionSlot.bits.set(static_cast<uint32_t>(ActionSlotBits::Up), wasHeld && !down);
            actionSlot.modifiers = down ? modifiers : actionSlot.modifiers;
        }
    }

    bool Input::ActionState::IsUp(uint32_t slot, KeyModifiers modifiers) const
    {
        return Test(slot, modifiers, ActionSlotBits::Up);
    }

    bool Input::ActionState::IsDown(uint32_t slot, KeyModifiers modifiers) const
    {
        return Test(slot, modifiers, ActionSlotBits::Down);
    }

    bool Input::ActionState::IsHeld(uint32_t slot, KeyModifiers modifiers) const
    {
        return Test(slot, modifiers, ActionSlotBits::Held);
    }

    bool Input::ActionState::Test(uint32_t slot, KeyModifiers modifiers, ActionSlotBits bit) const
    {
        const ActionSlot& actionSlotstate = _slots[slot];
        return actionSlotstate.bits.test(static_cast<uint32_t>(bit)) && KeyModifiersMatch(slot, modifiers, actionSlotstate.modifiers);
    }
}
