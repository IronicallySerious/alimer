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

#include "../Scene/Entity.h"
#include "../Core/Log.h"

namespace alimer
{
    uint32_t ComponentIDMapping::ids;

    // ComponentStorage
#if TODO_ECS
    ComponentStorage::ComponentStorage(std::size_t size)
    {
        Expand(size);
    }

    void ComponentStorage::Expand(std::size_t size)
    {
        _data.resize(size);
    }

    void ComponentStorage::Reserve(std::size_t size)
    {
        _data.reserve(size);
    }

    BaseComponent* ComponentStorage::Get(std::size_t index)
    {
        assert(index < size());
        return _data[index].Get();
    }

    void ComponentStorage::Destroy(std::size_t index)
    {
        assert(index < size());
        auto& element = _data[index];
        element.Reset();
    }

    void ComponentStorage::Set(uint32_t index, const IntrusivePtr<BaseComponent>& component)
    {
        _data[index] = component;
    }

    // Entity
    const Entity::Id Entity::INVALID;

    void Entity::SetName(const std::string& name)
    {
        ALIMER_ASSERT(IsValid());
        _manager->SetEntityName(_id, name);
    }

    const std::string& Entity::GetName() const
    {
        ALIMER_ASSERT(IsValid());
        return _manager->GetEntityName(_id);
    }

    // EntityManager
    EntityManager::EntityManager()
        : _indexCounter(0)
    {

    }

    EntityManager::~EntityManager()
    {
        Reset();
    }

    void EntityManager::Reset()
    {
        //for (entity entity : all_entities())
        //{
        //    entity.destroy();
        //}

        _componentPools.clear();
        _entityComponentMask.clear();

        _entityVersion.clear();
        _freeList.clear();
        _indexCounter = 0;
    }

    Entity EntityManager::Create()
    {
        std::uint32_t index, version;
        if (_freeList.empty())
        {
            index = _indexCounter++;
            AccomodateEntity(index);
            version = _entityVersion[index] = 1;
        }
        else
        {
            index = _freeList.back();
            _freeList.pop_back();
            version = _entityVersion[index];
        }

        Entity entity(this, Entity::Id(index, version));
        // TODO: Fire event
        //onEntityCreated(entity);
        return entity;
    }

    void EntityManager::Destroy(Entity::Id id)
    {
        AssertValid(id);

        std::uint32_t index = id.index();
        auto mask = _entityComponentMask[index];
        for (size_t i = 0; i < _componentPools.size(); ++i)
        {
            if (mask.test(i))
            {
                auto& pool = _componentPools[i];
                if (pool)
                {
                    BaseComponent* component = pool->Get(index);
                    Remove(id, *component);
                }
            }
        }

        //OnEntityDestroyed(Get(id));
        _entityComponentMask[index].reset();
        _entityVersion[index]++;
        _freeList.push_back(index);
        // Remove name
        _entityNames[id.id()].clear();
    }

    Entity EntityManager::Get(Entity::Id id)
    {
        AssertValid(id);
        return Entity(this, id);
    }

    IntrusivePtr<BaseComponent> EntityManager::Assign(Entity::Id id, const ComponentHandle& component)
    {
        AssertValid(id);
        const auto family = component->GetFamily();

        // Placement new into the component pool.
        auto& pool = AccomodateComponent(family);

        pool.Set(id.index(), component);
        // Set the bit for this component.
        _entityComponentMask[id.index()].set(family);

        // Create and return handle.
        //component->_entity = Get(id);
        //component->OnEntitySet();
        //OnComponentAdded(Get(id), handle);
        return component;
    }

    void EntityManager::Remove(Entity::Id id, const BaseComponent& component)
    {
        Remove(id, component.GetFamily());
    }

    void EntityManager::Remove(Entity::Id id, uint32_t family)
    {
        AssertValid(id);
        const std::uint32_t index = id.index();

        // Find the pool for this component family.
        auto& pool = _componentPools[family];
        //BaseComponent* handle = pool->Get(id.index());
        //OnComponentRemoved(Get(id), handle);
        // Remove component bit.
        _entityComponentMask[id.index()].reset(family);

        // Call destructor.
        pool->Destroy(index);
    }

    bool EntityManager::HasComponent(Entity::Id id, const BaseComponent& component) const
    {
        return HasComponent(id, component.GetFamily());
    }

    bool EntityManager::HasComponent(Entity::Id id, uint32_t family) const
    {
        AssertValid(id);

        // We don't bother checking the component mask, as we return a nullptr anyway.
        if (family >= _componentPools.size())
        {
            return false;
        }
        auto& pool = _componentPools[family];
        return !(!pool || !_entityComponentMask[id.index()][family]);
    }

    std::vector<BaseComponent*> EntityManager::GetAllComponents(Entity::Id id) const
    {
        std::vector<BaseComponent*> components;
        auto mask = component_mask(id);
        for (size_t i = 0; i < _componentPools.size(); ++i)
        {
            if (mask.test(i))
            {
                auto& pool = _componentPools[i];
                if (pool)
                {
                    components.push_back(pool->Get(id.index()));
                }
            }
        }
        return components;
    }

    void EntityManager::SetEntityName(Entity::Id id, const std::string& name)
    {
        _entityNames[id.id()] = name;
    }

    const std::string& EntityManager::GetEntityName(Entity::Id id)
    {
        return _entityNames[id.id()];
    }
#endif // TODO_ECS

}
