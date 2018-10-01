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
#include <bitset>

namespace Alimer
{
    struct ComponentIDMapping
    {
    public:
        template <typename T>
        static uint32_t GetId()
        {
            static uint32_t id = ids++;
            return id;
        }

    private:
        static uint32_t ids;
    };

    class BaseComponent;
    class ComponentStorage
    {
    public:
        ComponentStorage(std::size_t size = 100);

        inline std::size_t size() const
        {
            return _data.size();
        }
        inline std::size_t capacity() const
        {
            return _data.capacity();
        }
        /// Ensure at least n elements will fit in the pool.
        void Expand(std::size_t size);
        void Reserve(std::size_t size);
        IntrusivePtr<BaseComponent> Get(std::size_t index) const;

        template <typename T>
        IntrusivePtr<T> Get(std::size_t index) const
        {
            static_assert(std::is_base_of<Component, T>::value, "Invalid component type.");

            return IntrusivePtr<T>(Get(index));
        }

        void Destroy(std::size_t index);

        template <typename T, typename... Args>
        IntrusivePtr<T> Set(uint32_t index, Args&&... args)
        {
            auto element = MakeHandle<T>(std::forward<Args>(args)...);
            _data[index] = element;
            return element;
        }

        void Set(uint32_t index, const IntrusivePtr<BaseComponent>& component);

    private:
        std::vector<IntrusivePtr<BaseComponent>> _data;
    };

    /// Base component class.
    class ALIMER_API BaseComponent : public IntrusivePtrEnabled<BaseComponent>
    {
        friend class EntityManager;
    public:
        BaseComponent() = default;
        virtual ~BaseComponent() = default;

    protected:
        virtual uint32_t GetFamily() const = 0;
    };

    template <typename T>
    class Component : public BaseComponent
    {
    private:
        uint32_t GetFamily() const override
        {
            return GetStaticFamilyId();
        }

    public:
        Component() = default;
        Component(Component& rhs) = delete;
        Component& operator=(Component& rhs) = delete;

        static uint32_t GetStaticFamilyId()
        {
            return ComponentIDMapping::GetId<T>();
        }
    };

    using ComponentHandle = IntrusivePtr<BaseComponent>;

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

        /// Is this Entity handle valid?
        bool IsValid() const;

        /// Check if Entity handle is invalid.
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

        Id GetId() const { return _id; }

        void SetName(const std::string& name);
        const std::string& GetName() const;

        /// Assign component to entity.
        template <typename C, typename... Args>
        IntrusivePtr<C> Assign(Args&&... args);

    private:
        friend class EntityManager;

        EntityManager* _manager = nullptr;
        Entity::Id _id = INVALID;
    };

    /// Manages the relationship between an Entity and its components
    static const std::size_t MAX_COMPONENTS = 64;
    class ALIMER_API EntityManager final
    {
    public:
        using ComponentMask = std::bitset<MAX_COMPONENTS>;

        explicit EntityManager();
        ~EntityManager();
        void Reset();

        /// Create a new entity.
        Entity Create();

        /// Number of managed entities.
        size_t GetSize() const { return _entityComponentMask.size() - _freeList.size(); }

        /// Gets the current entity capacity.
        size_t GetCapacity() const { return _entityComponentMask.size(); }

        /// Return true if the given entity ID is still valid.
        bool IsValid(Entity::Id id) const
        {
            return id.index() < _entityVersion.size() && _entityVersion[id.index()] == id.version();
        }

        template <typename T, typename... Args>
        IntrusivePtr<T> Assign(Entity::Id id, Args&&... args)
        {
            static_assert(std::is_base_of<BaseComponent, T>(), "T is not a component, cannot add T to entity");

            IntrusivePtr<T> handle = MakeHandle<T>(std::forward<Args>(args)...);
            Assign(id, handle);
            return handle;
        }

        IntrusivePtr<BaseComponent> Assign(Entity::Id id, const ComponentHandle& component);

        void SetEntityName(Entity::Id id, const std::string& name);
        const std::string& GetEntityName(Entity::Id id);

    private:
        friend class Entity;

        inline void AssertValid(Entity::Id id) const
        {
            assert(id.index() < _entityComponentMask.size() && "entity::Id ID outside entity vector range");
            assert(_entityVersion[id.index()] == id.version() &&
                "Attempt to access entity via a stale entity::Id");
        }

        inline void AccomodateEntity(std::uint32_t index)
        {
            if (_entityComponentMask.size() <= index)
            {
                _entityComponentMask.resize(index + 1);
                _entityVersion.resize(index + 1);
                for (auto& pool : _componentPools)
                {
                    if (pool)
                    {
                        pool->Expand(index + 1);
                    }
                }
            }
        }

        template <typename C>
        ComponentStorage& AccomodateComponent()
        {
            auto family = rtti::type_index_sequential_t::id<Component, C>();
            return AccomodateComponent(family);
        }

        ComponentStorage& AccomodateComponent(uint32_t family)
        {
            if (_componentPools.size() <= family)
            {
                _componentPools.resize(family + 1);
            }

            auto& pool = _componentPools[family];
            if (!pool)
            {
                pool = std::make_unique<ComponentStorage>();
                pool->Expand(_indexCounter);
            }

            return *pool;
        }

        std::uint32_t _indexCounter = 0;
        // Each element in component_pools_ corresponds to a Pool for a component.
        // The index into the vector is the component::family().
        std::vector<std::unique_ptr<ComponentStorage>> _componentPools;
        // Bitmask of components associated with each entity. Index into the vector is
        // the entity::Id.
        std::vector<ComponentMask> _entityComponentMask;
        // Vector of entity version numbers. Incremented each time an entity is destroyed
        std::vector<uint32_t> _entityVersion;
        // List of available entity slots.
        std::vector<uint32_t> _freeList;
        /// Map of entity names.
        std::unordered_map<std::uint64_t, std::string> _entityNames;

        DISALLOW_COPY_MOVE_AND_ASSIGN(EntityManager);
    };

    inline bool Entity::IsValid() const
    {
        return (_manager != nullptr) && _manager->IsValid(_id);
    }

    template <typename C, typename... Args>
    IntrusivePtr<C> Entity::Assign(Args&&... args)
    {
        ALIMER_ASSERT(IsValid());
        return _manager->Assign<C>(_id, std::forward<Args>(args)...);
    }
}

namespace std
{
    template <> struct hash<Alimer::Entity>
    {
        std::size_t operator () (const Alimer::Entity &entity) const {
            return static_cast<std::size_t>(entity.GetId().index() ^ entity.GetId().version());
        }
    };

    template <> struct hash<const Alimer::Entity>
    {
        std::size_t operator () (const Alimer::Entity &entity) const {
            return static_cast<std::size_t>(entity.GetId().index() ^ entity.GetId().version());
        }
    };
}
