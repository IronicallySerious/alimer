/* Copyright (c) 2017-2018 Hans-Kristian Arntzen
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <queue>
#include <mutex>

namespace alimer
{
    template<typename T>
    class ThreadSafeQueue
    {
    public:
        ThreadSafeQueue();
        ThreadSafeQueue(const ThreadSafeQueue& copy);

        /**
         * Push a value into the back of the queue.
         */
        void Push(T value);

        /**
         * Try to pop a value from the front of the queue.
         * @returns false if the queue is empty.
         */
        bool TryPop(T& value);

        /**
         * Check to see if there are any items in the queue.
         */
        bool Empty() const;

        /**
         * Retrieve the number of items in the queue.
         */
        size_t Size() const;

    private:
        std::queue<T> _queue;
        mutable std::mutex _mutex;
    };

    template<typename T>
    ThreadSafeQueue<T>::ThreadSafeQueue()
    {}

    template<typename T>
    ThreadSafeQueue<T>::ThreadSafeQueue(const ThreadSafeQueue<T>& copy)
    {
        std::lock_guard<std::mutex> lock(copy._mutex);
        _queue = other._queue;
    }

    template<typename T>
    void ThreadSafeQueue<T>::Push(T value)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(std::move(value));
    }

    template<typename T>
    bool ThreadSafeQueue<T>::TryPop(T& value)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.empty()) {
            return false;
        }

        value = _queue.front();
        _queue.pop();

        return true;
    }

    template<typename T>
    bool ThreadSafeQueue<T>::Empty() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.empty();
    }

    template<typename T>
    size_t ThreadSafeQueue<T>::Size() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.size();
    }
}
