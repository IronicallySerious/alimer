/* Copyright (c) 2017 Hans-Kristian Arntzen
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

// Adopted from Granite: https://github.com/Themaister/Granite
// Licensed under MIT: https://github.com/Themaister/Granite/blob/master/LICENSE

#pragma once
#include <memory>
#include <stdint.h>
#include <unordered_map>

namespace Alimer
{
    template <typename T>
    struct IntrusiveListEnabled
    {
        IntrusiveListEnabled<T> *prev = nullptr;
        IntrusiveListEnabled<T> *next = nullptr;
    };

    template <typename T>
    class IntrusiveList
    {
    public:
        void Clear()
        {
            _head = nullptr;
        }

        class Iterator
        {
        public:
            friend class IntrusiveList<T>;
            Iterator(IntrusiveListEnabled<T> *node)
                : node(node)
            {
            }

            Iterator() = default;

            explicit operator bool() const
            {
                return node != nullptr;
            }

            bool operator==(const Iterator &other) const
            {
                return node == other.node;
            }

            bool operator!=(const Iterator &other) const
            {
                return node != other.node;
            }

            T &operator*()
            {
                return *static_cast<T *>(node);
            }

            const T &operator*() const
            {
                return *static_cast<T *>(node);
            }

            T *operator->()
            {
                return static_cast<T *>(node);
            }

            const T *operator->() const
            {
                return static_cast<T *>(node);
            }

            Iterator &operator++()
            {
                node = node->next;
                return *this;
            }

        private:
            IntrusiveListEnabled<T> *node = nullptr;
            IntrusiveListEnabled<T> *get()
            {
                return node;
            }
        };

        Iterator begin()
        {
            return Iterator(_head);
        }

        Iterator end()
        {
            return Iterator();
        }

        void erase(Iterator itr)
        {
            auto *node = itr.get();
            auto *next = node->next;
            auto *prev = node->prev;

            if (prev)
                prev->next = next;
            else
                _head = next;

            if (next)
                next->prev = prev;
        }

        void InsertFront(Iterator itr)
        {
            auto *node = itr.get();
            if (_head)
                _head->prev = node;

            node->next = _head;
            node->prev = nullptr;
            _head = node;
        }

        void MoveToFront(IntrusiveList<T> &other, Iterator itr)
        {
            other.erase(itr);
            InsertFront(itr);
        }

    private:
        IntrusiveListEnabled<T> *_head = nullptr;
    };
}
