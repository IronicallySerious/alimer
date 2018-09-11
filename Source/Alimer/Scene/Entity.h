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

#include <tuple>
#include <vector>
#include <bitset>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include "../Core/Object.h"
#include "../Util/ObjectPool.h"
#include "../Scene/Component.h"
#include <assert.h>

namespace Alimer
{
    namespace Detail
    {
        // Maximum number of types on a single entity (and max bits in a bitset).
        static constexpr size_t MaxComponents = 64;

        struct IDMapping
        {
        public:
            template <typename T>
            static uint32_t GetId()
            {
                static uint32_t id = _ids++;
                return id;
            }

            template <typename TSystem>
            static uint32_t GetSystemId()
            {
                static uint32_t id = _systemIds++;
                return id;
            }

        private:
            static uint32_t _ids;
            static uint32_t _systemIds;
        };
    };

    class EntityManager;

    /// Defines a Entity class.
    class ALIMER_API Entity final 
    {
    public:
        struct Id
        {
            Id() : _id(0) {}
            constexpr explicit Id(uint64_t id) : _id(id) {}
            constexpr Id(uint32_t index, uint32_t version) : _id(uint64_t(index) | uint64_t(version) << 32UL) {}

            uint64_t GetId() const { return _id; }

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
        Entity(const Entity&) = default;
        Entity &operator = (const Entity&) = default;

        /// Check if Entity handle is invalid.
        operator bool() const { return IsValid(); }

        /// Is this Entity handle valid?
        bool IsValid() const;

        bool operator == (const Entity &other) const
        {
            return other._manager == _manager && other._id == _id;
        }

        bool operator != (const Entity &other) const
        {
            return other._manager != _manager && other._id != _id;
        }

        bool operator < (const Entity &other) const
        {
            return other._id < _id;
        }

        bool HasComponent(uint32_t id) const
        {
            auto it = _components.find(id);
            return it != std::end(_components) && it->second;
        }

        template <typename T>
        bool HasComponent() const
        {
            return HasComponent(Detail::IDMapping::GetId<T>());
        }

        template <typename T>
        T *GetComponent()
        {
            auto it = _components.find(Detail::IDMapping::GetId<T>());
            if (it == std::end(_components))
                return nullptr;

            return static_cast<T*>(it->second);
        }

        template <typename T>
        const T *GetComponent() const
        {
            auto it = _components.find(Detail::IDMapping::GetId<T>());
            if (it == std::end(_components))
                return nullptr;

            return static_cast<T*>(it->second);
        }

        template <typename T, typename... Args>
        T *AddComponent(Args&&...);

        template <typename T>
        void RemoveComponent();

        Id id() const { return _id; }

        /// Gets the name of this object.
        std::string GetName() const { return _name; }

        /// Sets the name of this object.
        void SetName(const std::string& name);

    private:
        EntityManager *_manager = nullptr;
        std::string _name;
        Entity::Id _id = INVALID;
        std::unordered_map<uint32_t, Component*> _components;
    };

    class ComponentAllocatorBase
    {
    public:
        virtual ~ComponentAllocatorBase() = default;
        virtual void FreeComponent(Component *component) = 0;
    };

    template <typename T>
    struct ComponentAllocator final : public ComponentAllocatorBase
    {
        ObjectPool<T> pool;

        void FreeComponent(Component *component) override
        {
            pool.Free(static_cast<T*>(component));
        }
    };

    /// Manages Entity and Component logic.
    class ALIMER_API EntityManager final
    {
    public:
        explicit EntityManager();
        virtual ~EntityManager();

        /// Destroy all entities and reset the EntityManager.
        void Reset();

        /// Create a new Entity.
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
            //event_manager_.emit<EntityCreatedEvent>(entity);
            return entity;
        }

        void DeleteEntity(Entity *entity);

        /// Return true if the given entity ID is still valid.
        bool IsValid(Entity::Id id) const
        {
            return
                id.index() < _entityVersion.size()
                && _entityVersion[id.index()] == id.version();
        }

        template <typename T, typename... Args>
        T *AllocateComponent(Entity &entity, Args&&... args)
        {
            uint32_t id = Detail::IDMapping::GetId<T>();
            auto itr = _components.find(id);
            if (itr == _components.end())
            {
                auto tmp = _components.insert(std::make_pair(id, std::unique_ptr<ComponentAllocatorBase>(new ComponentAllocator<T>)));
                itr = tmp.first;
            }

            auto *allocator = static_cast<ComponentAllocator<T> *>(itr->second.get());
            auto &component = entity.GetComponents()[id];
            if (component)
            {
                allocator->FreeComponent(component);
                component = allocator->pool.Allocate(std::forward<Args>(args)...);
                component->_entity = &entity;
                for (auto &groupId : _componentToGroups[id])
                {
                    _groups[groupId]->AddEntity(entity);
                }
            }
            else
            {
                component = allocator->pool.Allocate(std::forward<Args>(args)...);
                component->_entity = &entity;
                for (auto &groupId : _componentToGroups[id])
                {
                    _groups[groupId]->AddEntity(entity);
                }
            }

            return static_cast<T *>(component);
        }

        void FreeComponent(uint32_t id, Component *component);

    private:
        inline void AccomodateEntity(uint32_t index)
        {
            if (_entityComponentMask.size() <= index)
            {
                _entityComponentMask.resize(index + 1);
                _entityVersion.resize(index + 1);
                //for (BasePool *pool : component_pools_)
                //    if (pool) pool->expand(index + 1);
            }
        }

    private:
        using ComponentMask = std::bitset<Detail::MaxComponents>;

        uint32_t _indexCounter = 0;

        // Vector of entity version numbers. Incremented each time an entity is destroyed
        std::vector<uint32_t> _entityVersion;

        // List of available entity slots.
        std::vector<uint32_t> _freeList;

        // Bitmask of components associated with each entity. Index into the vector is the Entity::Id.
        std::vector<ComponentMask> _entityComponentMask;

        ObjectPool<Entity> _pool;
        std::vector<Entity*> _entities;
        std::unordered_map<uint32_t, std::unique_ptr<ComponentAllocatorBase>> _components;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(EntityManager);
    };

    template <typename T, typename... Args>
    T *Entity::AddComponent(Args&&... args)
    {
        return _manager->AllocateComponent<T>(*this, std::forward<Args>(args)...);
    }

    template <typename T>
    void Entity::RemoveComponent()
    {
        auto id = Detail::IDMapping::GetId<T>();
        auto itr = components.find(id);
        if (itr != std::end(components))
        {
            assert(itr->second);
            pool->free_component(id, itr->second);
            components.erase(itr);
        }
    }

    // Inlines
    inline bool Entity::IsValid() const
    {
        return _manager && _manager->IsValid(_id);
    }
}
