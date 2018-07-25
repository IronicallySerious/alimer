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
    class Entity;
    class EntityManager;

    struct ComponentIDMapping
    {
    public:
        template <typename T>
        static uint32_t GetId()
        {
            static uint32_t id = _ids++;
            return id;
        }

        template <typename... Ts>
        static uint32_t GetGroupId()
        {
            static uint32_t id = _groupIds++;
            return id;
        }

    private:
        static uint32_t _ids;
        static uint32_t _groupIds;
    };

    /// Defines a base ComponentSystem.
    class EntityGroupBase
    {
    public:
        /// Destructor.
        virtual ~EntityGroupBase() = default;

        virtual void AddEntity(Entity &entity) = 0;
        virtual void RemoveComponent(Component *component) = 0;
    };

    template <typename... T>
    class EntityGroup : public EntityGroupBase
    {
    public:
        void AddEntity(Entity &entity) override final
        {
            if (HasAllComponents<T...>(entity))
            {
                _entities.push_back(&entity);
                _groups.push_back(std::make_tuple(entity.GetComponent<T>()...));
            }
        }

        void RemoveComponent(Component *component) override final
        {
            auto itr = std::find_if(std::begin(_groups), std::end(_groups), [&](const std::tuple<T*...> &t) {
                return HasComponent(t, component);
            });

            if (itr == std::end(_groups))
                return;

            auto offset = size_t(itr - begin(_groups));
            if (offset != _groups.size() - 1)
            {
                std::swap(_groups[offset], _groups.back());
                std::swap(_entities[offset], _entities.back());
            }
            _groups.pop_back();
            _entities.pop_back();
        }

        std::vector<std::tuple<T *...>> &GetGroups()
        {
            return _groups;
        }

    private:
        std::vector<std::tuple<T *...>> _groups;
        std::vector<Entity*> _entities;

        template <size_t Index>
        struct HasComponentTraits
        {
            template <typename T>
            static bool HasComponent(const T &t, const Component *component)
            {
                return static_cast<Component*>(std::get<Index>(t)) == component
                    || HasComponentTraits<Index - 1>::HasComponent(t, component);
            }
        };

        template <>
        struct HasComponentTraits<0>
        {
            template <typename T>
            static bool HasComponent(const T &t, const Component *component)
            {
                return static_cast<Component*>(std::get<0>(t)) == component;
            }
        };

        template <typename... Us>
        struct HasAllComponentsTraits;

        template <typename U, typename... Us>
        struct HasAllComponentsTraits<U, Us...>
        {
            static bool HasComponent(const Entity &entity)
            {
                return entity.HasComponent(ComponentIDMapping::GetId<U>())
                    && HasAllComponentsTraits<Us...>::HasComponent(entity);
            }
        };

        template <typename U>
        struct HasAllComponentsTraits<U>
        {
            static bool HasComponent(const Entity &entity)
            {
                return entity.HasComponent(ComponentIDMapping::GetId<U>());
            }
        };

        template <typename... Us>
        bool HasAllComponents(const Entity &entity)
        {
            return HasAllComponentsTraits<Us...>::HasComponent(entity);
        }

        template <typename... Us>
        static bool HasComponent(const std::tuple<Us *...> &t, const Component *component)
        {
            return HasComponentTraits<sizeof...(Us) - 1>::HasComponent(t, component);
        }
    };

    /// Defines a Entity class.
    class ALIMER_API Entity final : public Object
    {
        ALIMER_OBJECT(Entity, Object);

    public:
        Entity() = default;
        Entity(EntityManager *manager) : _manager(manager) {}
        Entity(const Entity &other) = default;
        Entity &operator = (const Entity &other) = default;

        bool HasComponent(uint32_t id) const
        {
            auto it = _components.find(id);
            return it != std::end(_components) && it->second;
        }

        template <typename T>
        bool HasComponent() const
        {
            auto it = _components.find(ComponentIDMapping::GetId<T>());
            return it != std::end(_components);
        }

        template <typename T>
        T *GetComponent()
        {
            auto it = _components.find(ComponentIDMapping::GetId<T>());
            if (it == std::end(_components))
                return nullptr;

            return static_cast<T *>(it->second);
        }

        template <typename T>
        const T *GetComponent() const
        {
            auto it = _components.find(ComponentIDMapping::GetId<T>());
            if (it == std::end(_components))
                return nullptr;

            return static_cast<T *>(it->second);
        }

        template <typename T, typename... Args>
        T *AddComponent(Args&&...);

        template <typename T>
        void RemoveComponent();

        std::unordered_map<uint32_t, Component*> &GetComponents()
        {
            return _components;
        }

        EntityManager *GetManager() const
        {
            return _manager;
        }

        /// Gets the name of this object.
        std::string GetName() const { return _name; }

        /// Sets the name of this object.
        void SetName(const std::string& name);

    private:
        EntityManager * _manager = nullptr;
        std::string _name;
        std::unordered_map<uint32_t, Component*> _components;
    };

    using EntityHandle = SharedPtr<Entity>;

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

    class ALIMER_API EntityManager final
    {
    public:
        explicit EntityManager();
        virtual ~EntityManager();

        /**
        * Create a new Entity.
        *
        * Emits EntityCreatedEvent.
        */
        EntityHandle CreateEntity();

        void DeleteEntity(Entity *entity);

        template <typename... T>
        std::vector<std::tuple<T*...>> &GetComponentGroup()
        {
            uint32_t groupId = ComponentIDMapping::GetGroupId<T...>();
            auto itr = _groups.find(groupId);
            if (itr == std::end(_groups))
            {
                RegisterGroup<T...>(groupId);
                auto tmp = _groups.insert(std::make_pair(groupId, std::unique_ptr<EntityGroupBase>(new EntityGroup<T...>())));
                itr = tmp.first;

                auto *group = static_cast<EntityGroup<T...> *>(itr->second.get());
                for (auto &entity : _entities)
                {
                    group->AddEntity(*entity);
                }
            }

            auto *group = static_cast<EntityGroup<T...> *>(itr->second.get());
            return group->GetGroups();
        }

        template <typename T, typename... Args>
        T *AllocateComponent(Entity &entity, Args&&... args)
        {
            uint32_t id = ComponentIDMapping::GetId<T>();
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

        void ResetGroups();

    private:
        ObjectPool<Entity> _pool;
        std::vector<Entity *> _entities;
        std::unordered_map<uint32_t, std::unique_ptr<EntityGroupBase>> _groups;
        std::unordered_map<uint32_t, std::unique_ptr<ComponentAllocatorBase>> _components;
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
                groups[ComponentIDMapping::GetId<U>()].insert(groupId);
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
                groups[ComponentIDMapping::GetId<U>()].insert(groupId);
            }
        };

        template <typename U, typename... Us>
        void RegisterGroup(uint32_t groupId)
        {
            GroupRegisters<U, Us...>::RegisterGroup(_componentToGroups, groupId);
        }

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
        auto id = ComponentIDMapping::GetId<T>();
        auto itr = components.find(id);
        if (itr != std::end(components))
        {
            assert(itr->second);
            pool->free_component(id, itr->second);
            components.erase(itr);
        }
    }
}
