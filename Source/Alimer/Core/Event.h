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

#include "../Core/Ptr.h"
#include <vector>

namespace Alimer
{
    class Object;
    class Event;

    /// Internal helper class for invoking event handler functions.
    class ALIMER_API EventHandler
    {
    public:
        /// Construct with receiver object pointer.
        EventHandler(Object* receiver);
        /// Destruct.
        virtual ~EventHandler() = default;

        /// Invoke the handler function. Implemented by subclasses.
        virtual void Invoke(Event& event) = 0;

        /// Return the receiver object.
        const Object* GetReceiver() const { return _receiver; }

    protected:
        /// Receiver object.
        Object* _receiver;
    };

    /// Template implementation of the event handler invoke helper, stores a function pointer of specific class.
    template <class T, class U>
    class EventHandlerImpl : public EventHandler
    {
    public:
        typedef void (T::*HandlerFunctionPtr)(U&);

        /// Construct with receiver and function pointers.
        EventHandlerImpl(Object* receiver, HandlerFunctionPtr function)
            : EventHandler(receiver)
            , _function(function)
        {
            assert(function);
        }

        /// Invoke the handler function.
        void Invoke(Event& event) override
        {
            T* typedReceiver = static_cast<T*>(_receiver.Get());
            U& typedEvent = static_cast<U&>(event);
            (typedReceiver->*_function)(typedEvent);
        }

    private:
        /// Pointer to the event handler function.
        HandlerFunctionPtr _function;
    };

    /// Notification and data passing mechanism, to which objects can subscribe by specifying a handler function. Subclass to include event-specific data.
    class ALIMER_API Event
    {
    public:
        /// Construct.
        Event();
        /// Destruct.
        virtual ~Event() = default;

        /// Send the event.
        void Send(Object* sender);
        /// Subscribe to the event. The event takes ownership of the handler data. If there is already handler data for the same receiver, it is overwritten.
        void Subscribe(EventHandler* handler);
        /// Unsubscribe from the event.
        void Unsubscribe(Object* receiver);

        /// Return whether has at least one valid receiver.
        bool HasReceivers() const;
        /// Return whether has a specific receiver.
        bool HasReceiver(const Object* receiver) const;
        /// Return current sender.
        const Object* GetSender() const { return _currentSender; }

    private:
        /// Event handlers.
        std::vector<std::unique_ptr<EventHandler>> _handlers;
        /// Current sender.
        Object* _currentSender;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Event);
    };
}
