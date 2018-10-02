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
        BaseComponent* Get(std::size_t index);

        template <typename T>
        T* Get(std::size_t index)
        {
            static_assert(std::is_base_of<BaseComponent, T>::value, "Invalid component type.");

            return static_cast<T*>(Get(index));
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

    /// 
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
        Entity(const Entity& other) = default;
        Entity& operator = (const Entity& other) = default;

        /// Destroy and invalidate this entity.
        void Destroy();

        /// Is this Entity handle valid?
        bool IsValid() const;

        ///  Invalidate entity handle, disassociating it from an manager and invalidating its ID.
        void Invalidate();

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
        template <typename T, typename... Args>
        IntrusivePtr<T> Assign(Args&&... args);

        /// Assign component to entity.
        IntrusivePtr<BaseComponent> Assign(const IntrusivePtr<BaseComponent>& component);

        /// Remove component from entity.
        template <typename T>
        void Remove();

        /// Remove component from entity.
        void Remove(const BaseComponent& component);

        /// Checks if entity has given component
        template <typename T>
        bool HasComponent() const;

        /// Checks if entity has given component
        bool HasComponent(const BaseComponent& component) const;

        template <typename T>
        T* GetComponent() const;

    private:
        EntityManager* _manager = nullptr;
        Entity::Id _id = INVALID;
    };

    /// Base component class.
    class ALIMER_API BaseComponent : public IntrusivePtrEnabled<BaseComponent>
    {
        friend class EntityManager;

    public:
        BaseComponent() = default;
        virtual ~BaseComponent() = default;

        Entity GetEntity()
        {
            return _entity;
        }

    protected:
        virtual uint32_t GetFamily() const = 0;

        /// Owning entity
        Entity _entity;
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

        /// Destroy an existing Entity and all its Components.
        void Destroy(Entity::Id id);

        /// Get entity by id.
        Entity Get(Entity::Id id);

        Entity::Id CreateId(std::uint32_t index) const
        {
            return Entity::Id(index, _entityVersion[index]);
        }

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

        /// Remove a component from an Entity.
        template <typename T>
        void Remove(Entity::Id id)
        {
            Remove(id, ComponentIDMapping::GetId<T>());
        }

        void Remove(Entity::Id id, const BaseComponent& component);
        void Remove(Entity::Id id, uint32_t family);

        /// Check if an entity has a component.
        template <typename T>
        bool HasComponent(Entity::Id id) const
        {
            return HasComponent(id, ComponentIDMapping::GetId<T>());
        }

        bool HasComponent(Entity::Id id, const  BaseComponent& component) const;
        bool HasComponent(Entity::Id id, uint32_t family) const;

        template <typename T>
        T* GetComponent(Entity::Id id)
        {
            AssertValid(id);
            auto family = ComponentIDMapping::GetId<T>();
            if (family >= _componentPools.size())
            {
                return nullptr;
            }
            auto& pool = _componentPools[family];
            if (!pool || !_entityComponentMask[id.index()][family])
            {
                return nullptr;
            }
            return pool->template Get<T>(id.index());
        }

        std::vector<BaseComponent*> GetAllComponents(Entity::Id id) const;

        /// Set entity name
        void SetEntityName(Entity::Id id, const std::string& name);

        /// Get entity name
        const std::string& GetEntityName(Entity::Id id);

        /// An iterator over a view of the entities in an EntityManager.
        /// If All is true it will iterate over all valid entities and will ignore the entity mask.
        template <class Delegate, bool All = false>
        class ViewIterator : public std::iterator<std::input_iterator_tag, Entity::Id> {
        public:
            Delegate &operator ++() {
                ++i_;
                next();
                return *static_cast<Delegate*>(this);
            }
            bool operator == (const Delegate& rhs) const { return i_ == rhs.i_; }
            bool operator != (const Delegate& rhs) const { return i_ != rhs.i_; }
            Entity operator * () { return Entity(manager_, manager_->CreateId(i_)); }
            const Entity operator * () const { return Entity(manager_, manager_->CreateId(i_)); }

        protected:
            ViewIterator(EntityManager *manager, uint32_t index)
                : manager_(manager), i_(index), capacity_(manager_->GetCapacity()), free_cursor_(~0UL) {
                if (All) {
                    std::sort(manager_->_freeList.begin(), manager_->_freeList.end());
                    free_cursor_ = 0;
                }
            }
            ViewIterator(EntityManager *manager, const ComponentMask mask, uint32_t index)
                : manager_(manager), mask_(mask), i_(index), capacity_(manager_->GetCapacity()), free_cursor_(~0UL) {
                if (All) {
                    std::sort(manager_->_freeList.begin(), manager_->_freeList.end());
                    free_cursor_ = 0;
                }
            }

            void next() {
                while (i_ < capacity_ && !predicate()) {
                    ++i_;
                }

                if (i_ < capacity_) {
                    Entity entity = manager_->Get(manager_->CreateId(i_));
                    static_cast<Delegate*>(this)->next_entity(entity);
                }
            }

            inline bool predicate() {
                return (All && valid_entity()) || (manager_->_entityComponentMask[i_] & mask_) == mask_;
            }

            inline bool valid_entity() {
                const std::vector<uint32_t> &free_list = manager_->_freeList;
                if (free_cursor_ < free_list.size() && free_list[free_cursor_] == i_) {
                    ++free_cursor_;
                    return false;
                }
                return true;
            }

            EntityManager *manager_;
            ComponentMask mask_;
            uint32_t i_;
            size_t capacity_;
            size_t free_cursor_;
        };

        template <bool All>
        class BaseView {
        public:
            class Iterator : public ViewIterator<Iterator, All> {
            public:
                Iterator(EntityManager *manager,
                    const ComponentMask mask,
                    uint32_t index) : ViewIterator<Iterator, All>(manager, mask, index) {
                    ViewIterator<Iterator, All>::next();
                }

                void next_entity(Entity &entity) {}
            };

            Iterator begin() { return Iterator(manager_, mask_, 0); }
            Iterator end() { return Iterator(manager_, mask_, uint32_t(manager_->GetCapacity())); }
            const Iterator begin() const { return Iterator(manager_, mask_, 0); }
            const Iterator end() const { return Iterator(manager_, mask_, manager_->GetCapacity()); }

        private:
            friend class EntityManager;

            explicit BaseView(EntityManager *manager) : manager_(manager) { mask_.set(); }
            BaseView(EntityManager *manager, ComponentMask mask) :
                manager_(manager), mask_(mask) {}

            EntityManager *manager_;
            ComponentMask mask_;
        };

        template <bool All, typename ... Components>
        class TypedView : public BaseView<All> {
        public:
            template <typename T> struct identity { typedef T type; };

            void each(typename identity<std::function<void(Entity entity, Components&...)>>::type f)
            {
                for (auto it : *this)
                {
                    f(it, *(it.template GetComponent<Components>())...);
                }
            }

        private:
            friend class EntityManager;

            explicit TypedView(EntityManager *manager) : BaseView<All>(manager) {}
            TypedView(EntityManager *manager, ComponentMask mask) : BaseView<All>(manager, mask) {}
        };

        template <typename ... Components> using View = TypedView<false, Components...>;
        using DebugView = BaseView<true>;

        template <typename ... Components>
        class UnpackingView {
        public:
            struct Unpacker {
                explicit Unpacker(IntrusivePtr<Components> & ... handles)
                    : handles(std::tuple<IntrusivePtr<Components> & ...>(handles...))
                {
                }

                void unpack(Alimer::Entity &entity) const
                {
                    unpack_<0, Components...>(entity);
                }


            private:
                template <int N, typename C>
                void unpack_(Alimer::Entity &entity) const {
                    std::get<N>(handles) = entity.component<C>();
                }

                template <int N, typename C0, typename C1, typename ... Cn>
                void unpack_(Alimer::Entity &entity) const {
                    std::get<N>(handles) = entity.component<C0>();
                    unpack_<N + 1, C1, Cn...>(entity);
                }

                std::tuple<IntrusivePtr<Components> & ...> handles;
            };


            class Iterator : public ViewIterator<Iterator> {
            public:
                Iterator(EntityManager *manager,
                    const ComponentMask mask,
                    uint32_t index,
                    const Unpacker &unpacker) : ViewIterator<Iterator>(manager, mask, index), unpacker_(unpacker) {
                    ViewIterator<Iterator>::next();
                }

                void next_entity(Entity &entity) {
                    unpacker_.unpack(entity);
                }

            private:
                const Unpacker &unpacker_;
            };


            Iterator begin() { return Iterator(manager_, mask_, 0, unpacker_); }
            Iterator end() { return Iterator(manager_, mask_, static_cast<uint32_t>(manager_->GetCapacity()), unpacker_); }
            const Iterator begin() const { return Iterator(manager_, mask_, 0, unpacker_); }
            const Iterator end() const { return Iterator(manager_, mask_, static_cast<uint32_t>(manager_->GetCapacity()), unpacker_); }


        private:
            friend class EntityManager;

            UnpackingView(EntityManager *manager, ComponentMask mask, IntrusivePtr<Components> & ... handles)
                : manager_(manager), mask_(mask), unpacker_(handles...)
            {
            }

            EntityManager *manager_;
            ComponentMask mask_;
            Unpacker unpacker_;
        };

        template <typename ... Components>
        View<Components...> EntitiesWithComponents() {
            auto mask = component_mask<Components ...>();
            return View<Components...>(this, mask);
        }

        template <typename T> struct identity { typedef T type; };

        template <typename ... Components>
        void Each(typename identity<std::function<void(Entity entity, Components&...)>>::type f) {
            return EntitiesWithComponents<Components...>().each(f);
        }

        template <typename ... Components>
        UnpackingView<Components...> EntitiesWithComponents(IntrusivePtr<Components> & ... components) {
            auto mask = component_mask<Components...>();
            return UnpackingView<Components...>(this, mask, components...);
        }

    private:
        friend class Entity;

        inline void AssertValid(Entity::Id id) const
        {
            assert(id.index() < _entityComponentMask.size() && "entity::Id ID outside entity vector range");
            assert(_entityVersion[id.index()] == id.version() &&
                "Attempt to access entity via a stale entity::Id");
        }

        ComponentMask component_mask(Entity::Id id)
        {
            AssertValid(id);
            return _entityComponentMask.at(id.index());
        }

        ComponentMask component_mask(Entity::Id id) const
        {
            AssertValid(id);
            return _entityComponentMask.at(id.index());
        }

        template <typename T>
        ComponentMask component_mask()
        {
            ComponentMask mask;
            mask.set(ComponentIDMapping::GetId<T>());
            return mask;
        }

        template <typename C1, typename C2, typename... Components>
        ComponentMask component_mask()
        {
            return component_mask<C1>() | component_mask<C2, Components...>();
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

    inline void Entity::Destroy()
    {
        assert(IsValid());
        _manager->Destroy(_id);
        Invalidate();
    }

    inline void Entity::Invalidate()
    {
        _manager = nullptr;
        _id = INVALID;
    }

    template <typename T, typename... Args>
    IntrusivePtr<T> Entity::Assign(Args&&... args)
    {
        ALIMER_ASSERT(IsValid());
        return _manager->Assign<T>(_id, std::forward<Args>(args)...);
    }

    inline IntrusivePtr<BaseComponent> Entity::Assign(const IntrusivePtr<BaseComponent>& component)
    {
        assert(IsValid());
        return _manager->Assign(_id, component);
    }

    template <typename T>
    void Entity::Remove()
    {
        assert(IsValid() && HasComponent<T>());
        _manager->Remove<T>(_id);
    }

    inline void Entity::Remove(const BaseComponent& component)
    {
        assert(IsValid() && HasComponent(component));
        _manager->Remove(_id, component);
    }

    template <typename T>
    bool Entity::HasComponent() const
    {
        if (!IsValid())
        {
            return false;
        }
        return _manager->HasComponent<T>(_id);
    }

    inline bool Entity::HasComponent(const BaseComponent& component) const
    {
        if (!IsValid())
        {
            return false;
        }

        return _manager->HasComponent(_id, component);
    }

    template <typename T>
    T* Entity::GetComponent() const
    {
        assert(IsValid());
        return _manager->GetComponent<T>(_id);
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
