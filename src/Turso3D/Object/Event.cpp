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

#include "../Debug/Log.h"
#include "../Thread/Thread.h"
#include "Event.h"

#include "../Debug/DebugNew.h"

namespace Turso3D
{

    EventHandler::EventHandler(RefCounted* receiver_) :
        receiver(receiver_)
    {
    }

    EventHandler::~EventHandler()
    {
    }

    Event::Event()
    {
    }

    Event::~Event()
    {
    }

    void Event::Send(RefCounted* sender)
    {
        if (!Thread::IsMainThread())
        {
            TURSO3D_LOGERROR("Attempted to send an event from outside the main thread");
            return;
        }

        // Retain a weak pointer to the sender on the stack for safety, in case it is destroyed
        // as a result of event handling, in which case the current event may also be destroyed
        WeakPtr<RefCounted> safeCurrentSender = sender;
        currentSender = sender;

        for (auto it = handlers.Begin(); it != handlers.End();)
        {
            EventHandler* handler = *it;
            bool remove = true;

            if (handler)
            {
                RefCounted* receiver = handler->Receiver();
                if (receiver)
                {
                    remove = false;
                    handler->Invoke(*this);
                    // If the sender has been destroyed, abort processing immediately
                    if (safeCurrentSender.IsExpired())
                        return;
                }
            }

            if (remove)
                it = handlers.Erase(it);
            else
                ++it;
        }

        currentSender.Reset();
    }

    void Event::Subscribe(EventHandler* handler)
    {
        if (!handler)
            return;

        // Check if the same receiver already exists; in that case replace the handler data
        for (auto it = handlers.Begin(); it != handlers.End(); ++it)
        {
            EventHandler* existing = *it;
            if (existing && existing->Receiver() == handler->Receiver())
            {
                *it = handler;
                return;
            }
        }

        handlers.Push(handler);
    }

    void Event::Unsubscribe(RefCounted* receiver)
    {
        for (auto it = handlers.Begin(); it != handlers.End(); ++it)
        {
            EventHandler* handler = *it;
            if (handler && handler->Receiver() == receiver)
            {
                // If event sending is going on, only clear the pointer but do not remove the element from the handler vector
                // to not confuse the event sending iteration; the element will eventually be cleared by the next SendEvent().
                if (currentSender)
                    *it = nullptr;
                else
                    handlers.Erase(it);
                return;
            }
        }
    }

    bool Event::HasReceivers() const
    {
        for (auto it = handlers.Begin(); it != handlers.End(); ++it)
        {
            EventHandler* handler = *it;
            if (handler && handler->Receiver())
                return true;
        }

        return false;
    }

    bool Event::HasReceiver(const RefCounted* receiver) const
    {
        for (auto it = handlers.Begin(); it != handlers.End(); ++it)
        {
            EventHandler* handler = *it;
            if (handler && handler->Receiver() == receiver)
                return true;
        }

        return false;
    }

}
