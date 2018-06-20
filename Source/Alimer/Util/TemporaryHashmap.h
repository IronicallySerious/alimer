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
#include "../Util/HashMap.h"
#include "../Util/ObjectPool.h"
#include "../Util/IntrusiveList.h"
#include <vector>

namespace Alimer
{
    template <typename T>
    class TemporaryHashmapEnabled
    {
    public:
        void SetHash(Hash hash) { _hash = hash; }
        Hash GetHash() const { return _hash; }

        void SetIndex(unsigned index) { _index = index; }
        unsigned GetIndex() const { return _index; }

    private:
        Hash _hash = 0;
        unsigned _index = 0;
    };

    template <typename T, unsigned RingSize = 4, bool ReuseObjects = false>
    class TemporaryHashmap
    {
    public:
        ~TemporaryHashmap()
        {
            Clear();
        }

        void Clear()
        {
            for (auto &ring : _rings)
            {
                for (auto &node : ring)
                    _objectPool.Free(static_cast<T *>(&node));
                ring.Clear();
            }
            hashmap.clear();

            for (auto &vacant : _vacants)
                _objectPool.Free(static_cast<T *>(&*vacant));
            _vacants.clear();
            _objectPool.Clear();
        }

        void BeginFrame()
        {
            index = (index + 1) & (RingSize - 1);
            for (auto &node : _rings[index])
            {
                hashmap.erase(node.GetHash());
                FreeObject(&node, ReuseTag<ReuseObjects>());
            }
            _rings[index].Clear();
        }

        T *Request(Hash hash)
        {
            auto itr = hashmap.find(hash);
            if (itr != end(hashmap))
            {
                auto node = itr->second;
                if (node->GetIndex() != index)
                {
                    _rings[index].MoveToFront(_rings[node->GetIndex()], node);
                    node->SetIndex(index);
                }

                return &*node;
            }
            else
                return nullptr;
        }

        template <typename... P>
        void MakeVacant(P &&... p)
        {
            _vacants.push_back(_objectPool.Allocate(std::forward<P>(p)...));
        }

        T *RequestVacant(Hash hash)
        {
            if (_vacants.empty())
                return nullptr;

            auto top = _vacants.back();
            _vacants.pop_back();
            top->SetIndex(index);
            top->SetHash(hash);
            hashmap[hash] = top;
            _rings[index].InsertFront(top);
            return &*top;
        }

        template <typename... P>
        T *Emplace(Hash hash, P &&... p)
        {
            auto *node = _objectPool.Allocate(std::forward<P>(p)...);
            node->SetIndex(index);
            node->SetHash(hash);
            hashmap[hash] = node;
            _rings[index].InsertFront(node);
            return node;
        }

    private:
        IntrusiveList<T> _rings[RingSize];
        ObjectPool<T> _objectPool;
        unsigned index = 0;
        HashMap<typename IntrusiveList<T>::Iterator> hashmap;
        std::vector<typename IntrusiveList<T>::Iterator> _vacants;

        template <bool reuse>
        struct ReuseTag
        {
        };

        void FreeObject(T *object, const ReuseTag<false> &)
        {
            _objectPool.Free(object);
        }

        void FreeObject(T *object, const ReuseTag<true> &)
        {
            _vacants.push_back(object);
        }
    };
}
