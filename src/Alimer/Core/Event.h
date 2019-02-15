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

// Based on https://github.com/GameFoundry/bsf event implementation
//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//

#pragma once

#include "../Base/Ptr.h"
#include "../Core/Debug.h"
#include <mutex>
#include <memory>
#include <functional>

namespace alimer
{
    /** Data common to all event connections. */
    class BaseConnectionData
    {
    public:
        BaseConnectionData() = default;

        virtual ~BaseConnectionData()
        {
            ALIMER_ASSERT(!handleLinks && !isActive);
        }

        virtual void deactivate()
        {
            isActive = false;
        }

        BaseConnectionData* prev = nullptr;
        BaseConnectionData* next = nullptr;
        bool isActive = true;
        uint32_t handleLinks = 0;
    };

    /** Internal data for an Event, storing all connections. */
    class EventInternalData final : public RefCounted
    {
    public:
        EventInternalData() = default;

        ~EventInternalData()
        {
            BaseConnectionData* conn = _connections;
            while (conn != nullptr)
            {
                BaseConnectionData* next = conn->next;
                delete conn;

                conn = next;
            }

            conn = _freeConnections;
            while (conn != nullptr)
            {
                BaseConnectionData* next = conn->next;
                delete conn;
                conn = next;
            }

            conn = _newConnections;
            while (conn != nullptr)
            {
                BaseConnectionData* next = conn->next;
                delete conn;

                conn = next;
            }
        }

        /// Add a new connection to the active connection array.
        void Add(BaseConnectionData* conn)
        {
            conn->prev = _lastConnection;

            if (_lastConnection != nullptr)
                _lastConnection->next = conn;

            _lastConnection = conn;

            // First connection
            if (_connections == nullptr)
                _connections = conn;
        }

        /**
         * Disconnects the connection with the specified data, ensuring the event doesn't call its callback again.
         *
         * @note	Only call this once.
         */
        void Remove(BaseConnectionData* conn)
        {
            std::unique_lock<std::recursive_mutex> lock(_mutex);

            conn->deactivate();
            conn->handleLinks--;

            if (conn->handleLinks == 0)
            {
                Free(conn);
            }
        }

        /** Disconnects all connections in the event. */
        void Clear()
        {
            std::unique_lock<std::recursive_mutex> lock(_mutex);

            BaseConnectionData* conn = _connections;
            while (conn != nullptr)
            {
                BaseConnectionData* next = conn->next;
                conn->deactivate();

                if (conn->handleLinks == 0)
                {
                    Free(conn);
                }

                conn = next;
            }

            _connections = nullptr;
            _lastConnection = nullptr;
        }

        void FreeHandle(BaseConnectionData* conn)
        {
            std::unique_lock<std::recursive_mutex> lock(_mutex);

            conn->handleLinks--;

            if (conn->handleLinks == 0 && !conn->isActive)
            {
                Free(conn);
            }
        }

        /// Releases connection data and makes it available for re-use when next connection is formed. 
        void Free(BaseConnectionData* conn)
        {
            if (conn->prev != nullptr)
            {
                conn->prev->next = conn->next;
            }
            else
            {
                _connections = conn->next;
            }

            if (conn->next != nullptr)
            {
                conn->next->prev = conn->prev;
            }
            else
            {
                _lastConnection = conn->prev;
            }

            conn->prev = nullptr;
            conn->next = nullptr;

            if (_freeConnections != nullptr)
            {
                conn->next = _freeConnections;
                _freeConnections->prev = conn;
            }

            _freeConnections = conn;
            _freeConnections->~BaseConnectionData();
        }

        BaseConnectionData* _connections = nullptr;
        BaseConnectionData* _lastConnection = nullptr;
        BaseConnectionData* _freeConnections = nullptr;
        BaseConnectionData* _newConnections = nullptr;

        std::recursive_mutex _mutex;
        bool _isCurrentlyTriggering = false;
    };

    /// Event handle. Allows you to track to which events you subscribed to and disconnect from them when needed.
    class HEvent
    {
    public:
        HEvent() = default;

        explicit HEvent(SharedPtr<EventInternalData> eventData, BaseConnectionData* connection)
            : _eventData(eventData)
            , mConnection(connection)
        {
            connection->handleLinks++;
        }

        ~HEvent()
        {
            if (mConnection != nullptr)
            {
                _eventData->FreeHandle(mConnection);
            }
        }

        /// Disconnect from previously subscribed event.
        void Disconnect()
        {
            if (mConnection != nullptr)
            {
                _eventData->Remove(mConnection);
                mConnection = nullptr;
                _eventData = nullptr;
            }
        }

        /** @cond IGNORE */
        struct Bool_struct { int _Member; };

        /** @endcond */

        /**
        * Allows direct conversion of a handle to bool.
        *
        * @note
        * Additional struct is needed because we can't directly convert to bool since then we can assign pointer to bool
        * and that's wrong.
        */
        operator int Bool_struct::*() const
        {
            return (mConnection != nullptr ? &Bool_struct::_Member : 0);
        }

        HEvent& operator=(const HEvent& rhs)
        {
            mConnection = rhs.mConnection;
            _eventData = rhs._eventData;

            if (mConnection != nullptr)
                mConnection->handleLinks++;

            return *this;
        }

    private:
        BaseConnectionData* mConnection = nullptr;
        SharedPtr<EventInternalData> _eventData;
    };

    template <class RetType, class... Args>
    class TEvent
    {
        struct ConnectionData : BaseConnectionData
        {
        public:
            void deactivate() override
            {
                func = nullptr;

                BaseConnectionData::deactivate();
            }

            std::function<RetType(Args...)> func;
        };

    public:
        TEvent()
            : _internalData(MakeShared<EventInternalData>()) 
        {
        }

        ~TEvent()
        {
            Clear();
        }

        /** Register a new callback that will get notified once the event is triggered. */
        HEvent Connect(std::function<RetType(Args...)> func)
        {
            std::unique_lock<std::recursive_mutex> lock(_internalData->_mutex);

            ConnectionData* connectionData = nullptr;
            if (_internalData->_freeConnections != nullptr)
            {
                connectionData = static_cast<ConnectionData*>(_internalData->_freeConnections);
                _internalData->_freeConnections = connectionData->next;

                new (connectionData )ConnectionData();
                if (connectionData->next != nullptr)
                    connectionData->next->prev = nullptr;

                connectionData->isActive = true;
            }

            if (connectionData == nullptr)
            {
                connectionData = new ConnectionData();
            }

            // If currently iterating over the connection list, delay modifying it until done
            if (_internalData->_isCurrentlyTriggering)
            {
                connectionData->prev = _internalData->_newConnections;

                if (_internalData->_newConnections != nullptr)
                    _internalData->_newConnections->next = connectionData;

                _internalData->_newConnections = connectionData;
            }
            else
            {
                _internalData->Add(connectionData);
            }

            connectionData->func = func;
            return HEvent(_internalData, connectionData);
        }

        /// Trigger the event, notifying all register callback methods.
        void operator() (Args... args)
        {
            // Increase ref count to ensure this event data isn't destroyed if one of the callbacks
            // deletes the event itself.
            SharedPtr<EventInternalData> internalData = _internalData;

            std::unique_lock<std::recursive_mutex> lock(internalData->_mutex);
            internalData->_isCurrentlyTriggering = true;

            ConnectionData* conn = static_cast<ConnectionData*>(internalData->_connections);
            while (conn != nullptr)
            {
                // Save next here in case the callback itself disconnects this connection
                ConnectionData* next = static_cast<ConnectionData*>(conn->next);

                if (conn->func != nullptr)
                    conn->func(std::forward<Args>(args)...);

                conn = next;
            }

            internalData->_isCurrentlyTriggering = false;

            // If any new connections were added during the above calls, add them to the connection list
            if (internalData->_newConnections != nullptr)
            {
                BaseConnectionData* lastNewConnection = internalData->_newConnections;
                while (lastNewConnection != nullptr)
                    lastNewConnection = lastNewConnection->next;

                BaseConnectionData* currentConnection = lastNewConnection;
                while (currentConnection != nullptr)
                {
                    BaseConnectionData* prevConnection = currentConnection->prev;
                    currentConnection->next = nullptr;
                    currentConnection->prev = nullptr;

                    _internalData->Add(currentConnection);
                    currentConnection = prevConnection;
                }

                internalData->_newConnections = nullptr;
            }
        }

        /// Clear all callbacks from the event.
        void Clear()
        {
            _internalData->Clear();
        }

        /**
         * Check if event has any callbacks registered.
         *
         * @note	It is safe to trigger an event even if no callbacks are registered.
         */
        bool IsEmpty() const
        {
            std::unique_lock<std::recursive_mutex> lock(_internalData->_mutex);

            return _internalData->_connections == nullptr;
        }

    private:
        SharedPtr<EventInternalData> _internalData;
    };

    /** @copydoc TEvent */
    template <typename Signature>
    class Event;

    /** @copydoc TEvent */
    template <class RetType, class... Args>
    class Event<RetType(Args...) > : public TEvent <RetType, Args...>
    { };
}
