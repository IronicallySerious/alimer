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

#include "../Entity.h"
#include "../../Math/Math.h"
#include "../../Math/Transform.h"
#include <vector>

namespace alimer
{
	/// Defines a Transform Component.
    class ALIMER_API TransformComponent final : public Component<TransformComponent>
	{
        //ALIMER_OBJECT(TransformComponent, Component);

    public:
        TransformComponent() = default;
        virtual ~TransformComponent();

        void UpdateWorldTransform(bool force = false);

        /// Set parent entity
        void SetParent(Entity parent);

        /// Get all chidrens.
        const std::vector<Entity>& GetChildren() const { return _children; }

        void SetDirty(bool dirty);
        bool IsDirty() const { return _dirty; }

        /// Set transform in world space.
        void SetTransform(const Transform& transform);

        /// Set transform in local space.
        void SetLocalTransform(const Transform& transform);

        /// Get transform in world space.
        const Transform& GetTransform();

        /// Get transform in local space.
        const Transform& GetLocalTransform() const;

    private:
        void AddChild(const Entity& child);
        void RemoveChild(const Entity& child);

        /// Parent entity.
        Entity _parent;
        /// Children entitites.
        std::vector<Entity> _children;
        /// Local transformation relative to the parent
        Transform _localTransform;
        /// Cached world transformation at pivot point.
        Transform _worldTransform;
        /// Transform dirty state.
        bool _dirty = true;
	};
}
