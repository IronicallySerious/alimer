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

#include "../Scene/Component.h"
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <cassert>

namespace Alimer
{
    class Scene;
    class EntityManager;

    /// Defines a base System class.
    class ALIMER_API System
    {
    public:
        /// Constructor.
        System() = default;

        /// Destructor.
        virtual ~System() = default;

        virtual void Configure(EntityManager &entities/*, EventManager &events*/)
        {
            ALIMER_UNUSED(entities);
        }

        /// Updates the system
        virtual void Update(EntityManager &entities, double deltaTime) = 0;

    protected:
        template <typename T>
        void RequireComponent()
        {
            _componentTypes.push_back(Detail::IDMapping::GetId<T>());
        }

        std::vector<uint32_t> _componentTypes;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(System);
    };

    class ALIMER_API SystemManager final
    {
    public:
        SystemManager(EntityManager &entityManager)
            : _entityManager(entityManager)
        {

        }

        ~SystemManager() = default;
        SystemManager(const SystemManager&) = delete;
        SystemManager(const SystemManager&&) = delete;
        SystemManager& operator = (const SystemManager&) = delete;
        SystemManager& operator = (const SystemManager&&) = delete;

        /// Add new System.
        template <typename S>
        void Add(std::shared_ptr<S> system)
        {
            _systems.insert(std::make_pair(Detail::IDMapping::GetSystemId<S>(), system));
        }

        /// Creates and add new System.
        template <typename S, typename ... Args>
        std::shared_ptr<S> Add(Args && ... args)
        {
            std::shared_ptr<S> instance(new S(std::forward<Args>(args) ...));
            Add(instance);
            return instance;
        }

        ///  Configure the system. Call after adding all Systems.
        void Configure();

        /// Update all systems.
        void Update(double deltaTime);

        /// Submits an entity to all available systems
        void AddToSystems(Entity entity);

    private:
        bool _initialized = false;
        EntityManager &_entityManager;
        std::unordered_map<uint32_t, std::shared_ptr<System>> _systems;
    };
}
