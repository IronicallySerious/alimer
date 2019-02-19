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

#pragma once

#include <foundation/platform.h>
#include <functional>

namespace alimer
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

    class ALIMER_API Entity
    {
    public:
        // this can be used to create an array of to-be-filled entities (see create())
        Entity() noexcept = default;

        // Entities can be copied
        Entity(const Entity& e) noexcept = default;
        Entity(Entity&& e) noexcept = default;
        Entity& operator=(const Entity& e) noexcept = default;
        Entity& operator=(Entity&& e) noexcept = default;

        // Entities can be compared
        bool operator==(Entity e) const { return e._id == _id; }
        bool operator!=(Entity e) const { return e._id != _id; }

        // Entities can be sorted
        bool operator<(Entity e) const { return e._id < _id; }

        bool isNull() const noexcept {
            return _id == 0;
        }

        uint32_t getId() const noexcept {
            return _id;
        }

        explicit operator bool() const noexcept { return !isNull(); }

    private:
        friend class EntityManager;
        friend class EntityManagerImpl;
        friend struct std::hash<Entity>;
        using Type = uint32_t;

        explicit Entity(Type id) noexcept : _id(id) { }

        uint32_t _id = 0;
    };
}

namespace std {
    template<>
    struct hash<alimer::Entity> {
        typedef alimer::Entity argument_type;
        typedef size_t result_type;

        result_type operator()(argument_type const& e) const {
            return std::hash<alimer::Entity::Type>{}(e.getId());
        }
    };

} // namespace std
