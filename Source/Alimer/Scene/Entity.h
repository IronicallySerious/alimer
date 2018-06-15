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

// Adopted from EntityX: https://github.com/alecthomas/entityx
// Licensed under MIT: https://github.com/alecthomas/entityx/blob/master/COPYING

#pragma once

#include "../Core/Event.h"
#include "../Scene/Component.h"
#include <bitset>
#include <vector>

namespace Alimer
{
    class EntityManager;

    namespace Detail
    {
        /**
        * Provides a resizable, semi-contiguous pool of memory for constructing
        * objects in. Pointers into the pool will be invalided only when the pool is
        * destroyed.
        *
        * The semi-contiguous nature aims to provide cache-friendly iteration.
        *
        * Lookups are O(1).
        * Appends are amortized O(1).
        */
        class BasePool {
        public:
            explicit BasePool(std::size_t element_size, std::size_t chunk_size = 8192)
                : element_size_(element_size), chunk_size_(chunk_size), capacity_(0) {}
            virtual ~BasePool();

            std::size_t size() const { return size_; }
            std::size_t capacity() const { return capacity_; }
            std::size_t chunks() const { return blocks_.size(); }

            /// Ensure at least n elements will fit in the pool.
            inline void expand(std::size_t n) {
                if (n >= size_) {
                    if (n >= capacity_) reserve(n);
                    size_ = n;
                }
            }

            inline void reserve(std::size_t n) {
                while (capacity_ < n) {
                    char *chunk = new char[element_size_ * chunk_size_];
                    blocks_.push_back(chunk);
                    capacity_ += chunk_size_;
                }
            }

            inline void *get(std::size_t n) {
                assert(n < size_);
                return blocks_[n / chunk_size_] + (n % chunk_size_) * element_size_;
            }

            inline const void *get(std::size_t n) const {
                assert(n < size_);
                return blocks_[n / chunk_size_] + (n % chunk_size_) * element_size_;
            }

            virtual void destroy(std::size_t n) = 0;

        protected:
            std::vector<char *> blocks_;
            std::size_t element_size_;
            std::size_t chunk_size_;
            std::size_t size_ = 0;
            std::size_t capacity_;
        };


        /**
        * Implementation of BasePool that provides type-"safe" deconstruction of
        * elements in the pool.
        */
        template <typename T, std::size_t ChunkSize = 8192>
        class Pool : public BasePool {
        public:
            Pool() : BasePool(sizeof(T), ChunkSize) {}
            virtual ~Pool() {
                // Component destructors *must* be called by owner.
            }

            virtual void destroy(std::size_t n) override {
                assert(n < size_);
                T *ptr = static_cast<T*>(get(n));
                ptr->~T();
            }
        };
    }

    static constexpr uint32_t MaxComponents = 64u;

    /// Defines a Entity class.
    class ALIMER_API Entity final 
    {
    public:
        struct Id {
            Id() : _id(0) {}
            explicit Id(uint64_t id) : _id(id) {}
            Id(uint32_t index, uint32_t version) : _id(uint64_t(index) | uint64_t(version) << 32UL) {}

            uint64_t id() const { return _id; }

            bool operator == (const Id &other) const { return _id == other._id; }
            bool operator != (const Id &other) const { return _id != other._id; }
            bool operator < (const Id &other) const { return _id < other._id; }

            uint32_t index() const { return _id & 0xffffffffUL; }
            uint32_t version() const { return _id >> 32; }

        private:
            friend class EntityManager;

            uint64_t _id;
        };

        /// Id of an invalid Entity.
        static const Id INVALID;

        Entity() = default;
        Entity(EntityManager *manager, Entity::Id id) : _manager(manager), _id(id) {}
        Entity(const Entity &other) = default;
        Entity &operator = (const Entity &other) = default;

        /**
        * Check if Entity handle is invalid.
        */
        operator bool() const {
            return IsValid();
        }

        bool operator == (const Entity &other) const {
            return other._manager == _manager && other._id == _id;
        }

        bool operator != (const Entity &other) const {
            return !(other == *this);
        }

        bool operator < (const Entity &other) const {
            return other._id < _id;
        }

        /**
        * Is this Entity handle valid?
        *
        * In older versions of EntityX, there were no guarantees around entity
        * validity if a previously allocated entity slot was reassigned. That is no
        * longer the case: if a slot is reassigned, old Entity::Id's will be
        * invalid.
        */
        bool IsValid() const;

        /// Gets the name of this object.
        std::string GetName() const { return _name; }

        /// Sets the name of this object.
        void SetName(const std::string& name);

    private:
        EntityManager* _manager = nullptr;
        Entity::Id _id = INVALID;
        std::string _name;
    };

    /**
    * Emitted when an entity is added to the system.
    */
    struct EntityCreatedEvent : public Event<EntityCreatedEvent>
    {
        explicit EntityCreatedEvent(Entity entity) : entity(entity) {}
        virtual ~EntityCreatedEvent() = default;

        Entity entity;
    };

    class ALIMER_API EntityManager final
    {
    public:
        using ComponentMask = std::bitset<MaxComponents>;

        explicit EntityManager(EventManager &eventManager);
        virtual ~EntityManager();

        /// Destroy all entities and reset the EntityManager.
        void Reset();

        /**
        * Create a new Entity.
        *
        * Emits EntityCreatedEvent.
        */
        Entity CreateEntity()
        {
            uint32_t index, version;
            if (_freeList.empty())
            {
                index = _indexCounter++;
                AccomodateEntity(index);
                version = _entityVersion[index] = 1;
            }
            else {
                index = _freeList.back();
                _freeList.pop_back();
                version = _entityVersion[index];
            }

            Entity entity(this, Entity::Id(index, version));
            _eventManager.Emit<EntityCreatedEvent>(entity);
            return entity;
        }

        /// Number of managed entities.
        size_t Size() const { return _entityComponentMask.size() - _freeList.size(); }

        /// Current entity capacity.
        size_t Capacity() const { return _entityComponentMask.size(); }

        /**
        * Return true if the given entity ID is still valid.
        */
        bool IsValid(Entity::Id id) const {
            return
                id.index() < _entityVersion.size()
                && _entityVersion[id.index()] == id.version();
        }


    private:
        inline void AccomodateEntity(uint32_t index)
        {
            if (_entityComponentMask.size() <= index)
            {
                _entityComponentMask.resize(index + 1);
                _entityVersion.resize(index + 1);
                for (Detail::BasePool *pool : _componentPools)
                {
                    if (pool) pool->expand(index + 1);
                }
            }
        }

    private:
        EventManager& _eventManager;

        uint32_t _indexCounter;

        // Each element in component_pools_ corresponds to a Pool for a Component.
        // The index into the vector is the Component::family().
        std::vector<Detail::BasePool*> _componentPools;
        // Each element in component_helpers_ corresponds to a ComponentHelper for a Component type.
        // The index into the vector is the Component::family().
        //std::vector<BaseComponentHelper*> component_helpers_;
        // Bitmask of components associated with each entity. Index into the vector is the Entity::Id.
        std::vector<ComponentMask> _entityComponentMask;
        // Vector of entity version numbers. Incremented each time an entity is destroyed
        std::vector<uint32_t> _entityVersion;
        // List of available entity slots.
        std::vector<uint32_t> _freeList;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(EntityManager);
    };

    inline bool Entity::IsValid() const {
        return _manager && _manager->IsValid(_id);
    }
}
