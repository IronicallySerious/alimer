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

#include "../Scene/Entity.h"
#include "../Core/Log.h"

namespace Alimer
{
    const Entity::Id Entity::INVALID;

    uint32_t Detail::IDMapping::_ids;
    uint32_t Detail::IDMapping::_systemIds;

    void Entity::SetName(const std::string& name)
    {
        _name = name;
    }

    EntityManager::EntityManager()
    {
    }

    EntityManager::~EntityManager()
    {
        Reset();
    }

    void EntityManager::Reset()
    {
        _entityComponentMask.clear();
        _entityVersion.clear();
        _freeList.clear();
        _indexCounter = 0;
    }

    void EntityManager::DeleteEntity(Entity *entity)
    {
        /*auto &components = entity->GetComponents();
        for (auto &component : components)
        {
            if (component.second)
                FreeComponent(component.first, component.second);
        }

        _pool.Free(entity);

        auto itr = std::find(std::begin(_entities), std::end(_entities), entity);
        auto offset = size_t(itr - std::begin(_entities));
        if (offset != _entities.size() - 1)
        {
            std::swap(_entities[offset], _entities.back());
        }
        _entities.pop_back();*/
    }


    void EntityManager::FreeComponent(uint32_t id, Component *component)
    {
        _components[id]->FreeComponent(component);
    }
}
