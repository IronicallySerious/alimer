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

// Inspired from different ECS systems:
// EnTT: https://github.com/skypjack/entt/blob/master/LICENSE
// EntityX: https://github.com/alecthomas/entityx
// Granite: https://github.com/Themaister/Granite

#include  "../Serialization/Serializable.h"
#include  "../Base/ObjectPool.h"
#include  "../Base/IntrusivePtr.h"
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

namespace Alimer
{
    using Family = uint32_t;
    class Component;

    namespace Detail
    {
        struct IDMapping
        {
        public:
            template <typename T>
            static Family GetId()
            {
                static Family id = ids++;
                return id;
            }

            template <typename... TComponent>
            static Family GetGroupId()
            {
                static uint32_t id = groupIds++;
                return id;
            }

            template <typename TSystem>
            static Family GetSystemId()
            {
                static uint32_t id = systemIds++;
                return id;
            }

        private:
            static Family ids;
            static Family groupIds;
            static Family systemIds;
        };

        template <size_t Index>
        struct HasComponent
        {
            template <typename T>
            static bool Contains(const T &t, const Component *component)
            {
                return static_cast<Component*>(
                    std::get<Index>(t)) == component
                    || HasComponent<Index - 1>::Contains(t, component);
            }
        };

        template <>
        struct HasComponent<0>
        {
            template <typename T>
            static bool Contains(const T &t, const Component *component)
            {
                return static_cast<Component*>(std::get<0>(t)) == component;
            }
        };
    }

    

    /// Base component class.
    class ALIMER_API Component : public Serializable
    {
        ALIMER_OBJECT(Component, Serializable);

    public:
        /// Destruct.
        virtual ~Component() = default;
    };

    /// Entity id
    struct EntityId
    {
        EntityId() : _id(0) {}
        explicit EntityId(uint64_t id) : _id(id) {}
        EntityId(uint32_t index, uint32_t version) : _id(uint64_t(index) | uint64_t(version) << 32UL) {}

        uint64_t id() const { return _id; }

        bool operator == (const EntityId &other) const { return _id == other._id; }
        bool operator != (const EntityId &other) const { return _id != other._id; }
        bool operator < (const EntityId &other) const { return _id < other._id; }

        uint32_t index() const { return _id & 0xffffffffUL; }
        uint32_t version() const { return _id >> 32; }

    private:
        friend class EntityManager;

        uint64_t _id;
    };

    class Entity;
    class EntityManager;

    struct EntityDeleter
    {
        void operator()(Entity* entity);
    };

    class ALIMER_API Entity final : public IntrusivePtrEnabled<Entity, EntityDeleter>
    {
    public:
        /// Id of an invalid Entity.
        static const EntityId Invalid;

        Entity() = default;
        Entity(const Entity &other) = default;
        Entity &operator = (const Entity &other) = default;

        Entity(EntityManager* manager, EntityId id)
            : _manager(manager)
            , _id(id)
        {
        }

        bool HasComponent(Family family) const
        {
            auto it = _componentsMap.find(family);
            return it != std::end(_componentsMap) && it->second;
        }

        template <typename T>
        bool HasComponent() const
        {
            return HasComponent(Detail::IDMapping::GetId<T>());
        }

        template <typename T>
        T* GetComponent()
        {
            auto it = _componentsMap.find(Detail::IDMapping::GetId<T>());
            if (it == std::end(_componentsMap))
                return nullptr;

            return static_cast<T*>(it->second);
        }

        template <typename T>
        const T* GetComponent() const
        {
            auto it = _componentsMap.find(Detail::IDMapping::GetId<T>());
            if (it == std::end(_componentsMap))
                return nullptr;

            return static_cast<T*>(it->second);
        }


        template <typename T, typename... Args>
        T* AddComponent(Args&&... args);

        EntityManager* GetManager() const {
            return _manager;
        }

        EntityId GetId() const { return _id; }

        std::unordered_map<uint32_t, Component*> &GetComponentsMap() {
            return _componentsMap;
        }

    private:
        void Invalidate()
        {
            _manager = nullptr;
            _id = Invalid;
        }

    private:
        friend class EntityManager;

        EntityManager* _manager;
        EntityId _id = Invalid;
        std::unordered_map<Family, Component*> _componentsMap;
    };
    using EntityHandle = IntrusivePtr<Entity>;

    class ComponentAllocatorBase
    {
    public:
        virtual ~ComponentAllocatorBase() = default;
        virtual void Free(Component* component) = 0;
    };

    template <typename TComponent>
    struct ComponentAllocator : public ComponentAllocatorBase
    {
        ObjectPool<TComponent> pool;

        void Free(Component* component) override final
        {
            pool.Free(static_cast<TComponent*>(component));
        }
    };

    class EntityGroupBase
    {
    public:
        virtual ~EntityGroupBase() = default;
        virtual void AddEntity(Entity &entity) = 0;
        virtual void RemoveComponent(Component* component) = 0;
    };

    template <typename... TComponent>
    class EntityGroup : public EntityGroupBase
    {
    public:
        void AddEntity(Entity &entity) override final
        {
            if (has_all_components<TComponent...>(entity))
            {
                _entities.push_back(&entity);
                _groups.push_back(std::make_tuple(entity.GetComponent<TComponent>()...));
            }
        }

        void RemoveComponent(Component* component) override final
        {
            auto it = std::find_if(std::begin(_groups), std::end(_groups), [&](const std::tuple<TComponent*...> &t)
            {
                return HasComponent(t, component);
            });

            if (it == _groups.end())
                return;

            auto offset = size_t(it - begin(_groups));
            if (offset != _groups.size() - 1)
            {
                std::swap(_groups[offset], _groups.back());
                std::swap(_entities[offset], _entities.back());
            }
            _groups.pop_back();
            _entities.pop_back();
        }

        std::vector<std::tuple<TComponent*...>> &GetGroups()
        {
            return _groups;
        }

    private:
        std::vector<std::tuple<TComponent*...>> _groups;
        std::vector<Entity*> _entities;

        template <typename... Us>
        struct HasAllComponents;

        template <typename U, typename... Us>
        struct HasAllComponents<U, Us...>
        {
            static bool Contains(const Entity &entity)
            {
                return entity.HasComponent(Detail::IDMapping::GetId<U>())
                    && HasAllComponents<Us...>::Contains(entity);
            }
        };

        template <typename U>
        struct HasAllComponents<U>
        {
            static bool Contains(const Entity &entity)
            {
                return entity.HasComponent(Detail::IDMapping::GetId<U>());
            }
        };

        template <typename... Us>
        bool has_all_components(const Entity &entity)
        {
            return HasAllComponents<Us...>::Contains(entity);
        }

        template <typename... Us>
        static bool HasComponent(const std::tuple<Us *...> &t, const Component* component)
        {
            return Detail::HasComponent<sizeof...(Us) - 1>::Contains(t, component);
        }
    };

    /// Manages the relationship between an Entity and its components
    class ALIMER_API EntityManager final
    {
    public:
        explicit EntityManager();
        ~EntityManager();
        void Reset();

        EntityHandle CreateEntity()
        {
            uint32_t index, version;
            if (_freeList.empty())
            {
                index = _indexCounter++;
                _entityVersion.resize(index + 1);
                version = _entityVersion[index] = 1;
            }
            else {
                index = _freeList.back();
                _freeList.pop_back();
                version = _entityVersion[index];
            }

            auto itr = EntityHandle(_pool.Allocate(this, EntityId(index, version)));
            _entities.push_back(itr.Get());
            return itr;
        }

        void DeleteEntity(Entity *entity)
        {
            // Free id first.
            entity->Invalidate();
            uint32_t index = entity->GetId().index();
            _entityVersion[index]++;
            _freeList.push_back(index);

            auto &components = entity->GetComponentsMap();
            for (auto &component : components)
            {
                if (component.second)
                {
                    FreeComponent(component.first, component.second);
                }
            }
            _pool.Free(entity);

            auto itr = std::find(std::begin(_entities), std::end(_entities), entity);
            auto offset = size_t(itr - std::begin(_entities));
            if (offset != _entities.size() - 1)
            {
                std::swap(_entities[offset], _entities.back());
            }
            _entities.pop_back();
        }

        template <typename... TComponent>
        std::vector<std::tuple<TComponent*...>> &GetComponentGroup()
        {
            Family family = Detail::IDMapping::GetGroupId<TComponent...>();
            auto it = _groups.find(family);
            if (it == _groups.end())
            {
                RegisterGroup<TComponent...>(family);
                auto tmp = _groups.insert(std::make_pair(family, std::unique_ptr<EntityGroupBase>(new EntityGroup<TComponent...>())));
                it = tmp.first;

                auto *group = static_cast<EntityGroup<TComponent...> *>(it->second.get());
                for (auto &entity : _entities)
                {
                    group->AddEntity(*entity);
                }
            }

            auto *group = static_cast<EntityGroup<TComponent...>*>(it->second.get());
            return group->GetGroups();
        }

        template <typename T, typename... Args>
        T* AllocateComponent(Entity &entity, Args&&... args)
        {
            Family id = Detail::IDMapping::GetId<T>();
            auto itr = _components.find(id);
            if (itr == std::end(_components))
            {
                auto tmp = _components.insert(std::make_pair(id, std::unique_ptr<ComponentAllocatorBase>(new ComponentAllocator<T>)));
                itr = tmp.first;
            }

            auto *allocator = static_cast<ComponentAllocator<T> *>(itr->second.get());
            auto &component = entity.GetComponentsMap()[id];
            if (component)
            {
                // Found: Free old one.
                FreeComponent(id, component);
            }

            // Allocate new
            component = allocator->pool.Allocate(std::forward<Args>(args)...);
            for (auto &group : _componentToGroups[id])
            {
                _groups[group]->AddEntity(entity);
            }

            return static_cast<T*>(component);
        }

        void FreeComponent(uint32_t id, Component* component)
        {
            _components[id]->Free(component);
            for (auto &group : _componentToGroups[id])
            {
                _groups[group]->RemoveComponent(component);
            }
        }

    private:
        ObjectPool<Entity> _pool;
        std::vector<Entity*> _entities;
        std::unordered_map<Family, std::unique_ptr<ComponentAllocatorBase>> _components;
        std::unordered_map<Family, std::unique_ptr<EntityGroupBase>> _groups;
        std::unordered_map<uint32_t, std::unordered_set<uint32_t>> _componentToGroups;

        template <typename... Us>
        struct GroupRegisters;

        template <typename U, typename... Us>
        struct GroupRegisters<U, Us...>
        {
            static void RegisterGroup(
                std::unordered_map<uint32_t, std::unordered_set<uint32_t>> &groups,
                uint32_t groupId)
            {
                groups[Detail::IDMapping::GetId<U>()].insert(groupId);
                GroupRegisters<Us...>::RegisterGroup(groups, groupId);
            }
        };

        template <typename U>
        struct GroupRegisters<U>
        {
            static void RegisterGroup(
                std::unordered_map<uint32_t, std::unordered_set<uint32_t>> &groups,
                uint32_t groupId)
            {
                groups[Detail::IDMapping::GetId<U>()].insert(groupId);
            }
        };

        template <typename U, typename... Us>
        void RegisterGroup(uint32_t groupId)
        {
            GroupRegisters<U, Us...>::RegisterGroup(_componentToGroups, groupId);
        }

        uint32_t _indexCounter = 0;

        // List of available entity slots.
        std::vector<uint32_t> _freeList;
        // Vector of entity version numbers. Incremented each time an entity is destroyed
        std::vector<uint32_t> _entityVersion;

        DISALLOW_COPY_MOVE_AND_ASSIGN(EntityManager);
    };

    template <typename T, typename... Args>
    T* Entity::AddComponent(Args&&... args)
    {
        return _manager->AllocateComponent<T>(*this, std::forward<Args>(args)...);
    }
}
