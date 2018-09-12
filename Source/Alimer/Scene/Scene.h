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

#include "../Serialization/Serializable.h"
#include "../Scene/Entity.h"
#include "../Scene/ComponentSystem.h"
#include "../Math/MathUtil.h"

namespace Alimer
{
    struct TransformComponent;
    struct CameraComponent;
    class Renderable;

    struct RenderableInfo
    {
        Renderable *renderable;
        const TransformComponent *transform;
    };
    using VisibilitySet = std::vector<RenderableInfo>;


    /// Defines a scene, which is a container of SceneObject's.
    class ALIMER_API Scene final : public Serializable
    {
        ALIMER_OBJECT(Scene, Serializable);

    public:
        /// Constructor.
        explicit Scene(EntityManager& entities);

        /// Destructor.
        ~Scene();

        /// Creates a new entity in the Scene.
        Entity CreateEntity();

        /// Return the Entity containing the default camera.
        Entity GetDefaultCamera() const { return _defaultCamera; }

        /// Return the Entity containing the active camera.
        Entity GetActiveCamera() const { return _activeCamera; }

        /// Update scene 
        void Update(double deltaTime);

    private:
        void UpdateCachedTransforms();

    protected:
        EntityManager& _entities;

        Entity _defaultCamera;
        Entity _activeCamera;
        
        std::vector<Entity> _pendingEntities;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Scene);
    };
}
