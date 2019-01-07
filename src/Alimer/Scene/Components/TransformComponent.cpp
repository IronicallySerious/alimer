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

#include "../Components/TransformComponent.h"

namespace alimer
{
    static bool CheckValidParent(const Entity& e, const Entity& parent)
    {
        if (e == parent)
            return false;

        if (parent.IsValid() && !parent.HasComponent<TransformComponent>() )
            return false;

        if (e.IsValid())
        {
            auto transform = e.GetComponent<TransformComponent>();
            if (transform)
            {
                for (const auto& child : transform->GetChildren())
                {
                    if (!CheckValidParent(child, parent))
                    {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    TransformComponent::~TransformComponent()
    {

    }

    void TransformComponent::UpdateWorldTransform(bool force)
    {
        if (force || IsDirty())
        {
            if (_parent.IsValid())
            {
                auto parentTransform = _parent.GetComponent<TransformComponent>();
                if (parentTransform)
                {
                    //_worldTransform = parentTransform->GetTransform() * _localTransform;
                }
                else
                {
                    _worldTransform = _localTransform;
                }
            }
            else
            {
                _worldTransform = _localTransform;
            }

            SetDirty(false);
        }
    }

    void TransformComponent::SetDirty(bool dirty)
    {
        _dirty = dirty;

        if (_dirty)
        {
            for (const Entity& child : _children)
            {
                if (child.IsValid())
                {
                    auto childTransform = child.GetComponent<TransformComponent>();
                    if (childTransform)
                    {
                        childTransform->SetDirty(dirty);
                    }
                }
            }
        }
    }

    void TransformComponent::SetParent(Entity parent)
    {
        // Check if parent is valid.
        if (!CheckValidParent(_entity, parent))
        {
            return;
        }

        Transform cachedWorldTransform;
        const bool keepWorldTransform = true;
        if (keepWorldTransform)
        {
            UpdateWorldTransform(true);
            cachedWorldTransform = GetTransform();
        }

        if (_parent.IsValid())
        {
            auto parentTransform = _parent.GetComponent<TransformComponent>();
            if (parentTransform)
            {
                parentTransform->RemoveChild(_entity);
            }
        }

        _parent = parent;

        if (_parent.IsValid())
        {
            auto parentTransform = _parent.GetComponent<TransformComponent>();
            if (parentTransform)
            {
                parentTransform->AddChild(_entity);
            }
        }

        if (keepWorldTransform)
        {
            UpdateWorldTransform(true);
            SetTransform(cachedWorldTransform);
        }
        else
        {
           SetLocalTransform(Transform::Identity);
        }

        SetDirty(IsDirty());
    }

    void TransformComponent::AddChild(const Entity& child)
    {
        _children.push_back(child);

        SetDirty(IsDirty());
    }

    void TransformComponent::RemoveChild(const Entity& child)
    {
        _children.erase(std::remove_if(_children.begin(), _children.end(),
            [&child](const auto& other) { return child == other; }),
            _children.end()
        );
    }

    const Transform& TransformComponent::GetTransform()
    {
        UpdateWorldTransform();
        return _worldTransform;
    }

    const Transform& TransformComponent::GetLocalTransform() const
    {
        return _localTransform;
    }

    void TransformComponent::SetTransform(const Transform& transform)
    {
        vec3 position = transform.GetPosition();
        vec3 scaling = transform.GetScale();
        quat orientation = transform.GetRotation();

        Transform m = GetTransform();
        m.SetScale(scaling);
        m.SetRotation(orientation);
        m.SetPosition(position);

        if (_parent.IsValid())
        {
            auto parentTransform = _parent.GetComponent<TransformComponent>();
            if (parentTransform)
            {
                //Transform inverseParentTransform = inverse(parentTransform->GetTransform());
                //m = inverseParentTransform * m;
            }
        }

        SetLocalTransform(m);
    }

    void TransformComponent::SetLocalTransform(const Transform& transform)
    {
        SetDirty(true);
        _localTransform = transform;
    }
}
