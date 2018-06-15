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

#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <list>
#include <unordered_map>
#include <memory>
#include <utility>
#include "simplesignal/simplesignal.h"
#include "../AlimerConfig.h"

namespace Alimer
{
    /// Used internally by the EventManager.
    class BaseEvent {
    public:
        virtual ~BaseEvent() = default;

    protected:
        static size_t _familyCounter;
    };

    template <typename Derived>
    class Event : public BaseEvent {
    public:
        /// Used internally for registration.
        static size_t Family() {
            static size_t family = _familyCounter++;
            return family;
        }
    };

    /**
    * Handles event subscription and delivery.
    *
    * Subscriptions are automatically removed when receivers are destroyed..
    */
    class ALIMER_API EventManager final
    {
    public:
        EventManager();
        virtual ~EventManager();

        template <typename E>
        void Emit(const E &event)
        {
            auto sig = SignalFor(Event<E>::Family());
            sig->emit(&event);
        }

        /// Emit an already constructed event.
        template <typename E>
        void Emit(std::unique_ptr<E> event)
        {
            auto sig = SignalFor(Event<E>::Family());
            sig->emit(event.get());
        }

        template <typename E, typename ... Args>
        void Emit(Args && ... args)
        {
            // Using 'E event(std::forward...)' causes VS to fail with an internal error. Hack around it.
            E event = E(std::forward<Args>(args) ...);
            auto sig = SignalFor(std::size_t(Event<E>::Family()));
            sig->emit(&event);
        }

    private:
        using EventSignal = Simple::Signal<void(const void*)>;
        using EventSignalPtr = std::shared_ptr<EventSignal>;

        EventSignalPtr& SignalFor(std::size_t id)
        {
            if (id >= _handlers.size())
                _handlers.resize(id + 1);
            if (!_handlers[id])
                _handlers[id] = std::make_shared<EventSignal>();
            return _handlers[id];
        }

        // Functor used as an event signal callback that casts to E.
        template <typename E>
        struct EventCallbackWrapper {
            explicit EventCallbackWrapper(std::function<void(const E &)> callback) : callback(callback) {}
            void operator()(const void *event) { callback(*(static_cast<const E*>(event))); }
            std::function<void(const E &)> callback;
        };

        std::vector<EventSignalPtr> _handlers;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(EventManager);
    };
}
