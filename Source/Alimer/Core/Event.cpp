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

#include "../Core/Event.h"
#include "../Core/Object.h"

namespace Alimer
{
    EventHandler::EventHandler(Object* receiver)
        : _receiver(receiver)
    {
    }

    Event::Event()
        : _currentSender(nullptr)
    {

    }

    void Event::Send(Object* sender)
    {
        //if (!Thread::IsMainThread())
        //{
        //    ALIMER_LOGERROR("Attempted to send an event from outside the main thread");
        //    return;
        //}

        // Retain a weak pointer to the sender on the stack for safety, in case it is destroyed
        // as a result of event handling, in which case the current event may also be destroyed
        WeakPtr<Object> safeCurrentSender(sender);
        _currentSender = sender;

        for (auto it = _handlers.begin(); it != _handlers.end();)
        {
            const std::unique_ptr<EventHandler>& handler = *it;
            bool remove = true;

            if (handler)
            {
                const Object* receiver = handler->GetReceiver();
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
                it = _handlers.erase(it);
            else
                ++it;
        }

        _currentSender = nullptr;
    }

    void Event::Subscribe(EventHandler* handler)
    {
        if (!handler)
            return;

        // Check if the same receiver already exists; in that case replace the handler data
        for (auto it = _handlers.begin(); it != _handlers.end(); ++it)
        {
            const std::unique_ptr<EventHandler>& existing = *it;
            if (existing
                && existing->GetReceiver() == handler->GetReceiver())
            {
                it->reset(handler);
                return;
            }
        }

        _handlers.push_back(std::unique_ptr<EventHandler>(handler));
    }

    void Event::Unsubscribe(Object* receiver)
    {
        for (auto it = _handlers.begin(); it != _handlers.end(); ++it)
        {
            const std::unique_ptr<EventHandler>& handler = *it;
            if (handler
                && handler->GetReceiver() == receiver)
            {
                // If event sending is going on, only clear the pointer but do not remove the element from the handler vector
                // to not confuse the event sending iteration; the element will eventually be cleared by the next SendEvent().
                if (_currentSender)
                    *it = nullptr;
                else
                    _handlers.erase(it);
                return;
            }
        }
    }

    bool Event::HasReceivers() const
    {
        for (const std::unique_ptr<EventHandler>& handler : _handlers)
        {
            if (handler
                && handler->GetReceiver())
            {
                return true;
            }
        }

        return false;
    }

    bool Event::HasReceiver(const Object* receiver) const
    {
        for (const std::unique_ptr<EventHandler>& handler : _handlers)
        {
            if (handler
                && handler->GetReceiver() == receiver)
            {
                return true;
            }
        }

        return false;
    }
}
