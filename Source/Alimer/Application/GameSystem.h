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

#include "../Scene/Entity.h"

namespace Alimer
{
    /// Defines a base Game System class.
    class ALIMER_API GameSystem : public IntrusivePtrEnabled<GameSystem>
    {
    public:
        /// Constructor.
        GameSystem() = default;

        /// Destructor.
        virtual ~GameSystem() = default;

        /// Updates the system
        virtual void Update(double deltaTime) = 0;
    };

    class ALIMER_API SystemManager final
    {
    public:
        SystemManager() = default;
        ~SystemManager() = default;

        /// Add new System.
        template <typename S>
        void Add(const IntrusivePtr<S> system)
        {
            _systems.insert(std::make_pair(Detail::IDMapping::GetSystemId<S>(), system));
        }

        /// Creates and add new System.
        template <typename S, typename ... Args>
        IntrusivePtr<S> Add(Args && ... args)
        {
            IntrusivePtr<S> instance(new S(std::forward<Args>(args) ...));
            Add(instance);
            return instance;
        }

        template <typename S>
        IntrusivePtr<S> GetSystem()
        {
            auto it = _systems.find(Detail::IDMapping::GetSystemId<S>());
            assert(it != _systems.end());
            return it == _systems.end()
                ? IntrusivePtr<S>()
                : IntrusivePtr<S>(std::static_pointer_cast<S>(it->second));
        }

        /// Update all systems.
        void Update(double deltaTime);

    private:
        std::unordered_map<uint32_t, IntrusivePtr<GameSystem>> _systems;

        DISALLOW_COPY_MOVE_AND_ASSIGN(SystemManager);
    };
}
