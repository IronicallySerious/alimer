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
#include "../Math/MathUtil.h"
#include "../Graphics/Graphics.h"
#include <vector>

namespace Alimer
{
    class TransformComponent;
    class CameraComponent;
    class RenderableComponent;
    class SceneManager;

    /// Defines a scene, which is a container of SceneObject's.
    class Scene final 
    {
        friend class SceneManager;
    public:
        /// Constructor.
        Scene();

        /// Destructor.
        ~Scene();

        /// Creates a new entity in the Scene.
        EntityHandle CreateEntity();

        /// Return the Entity containing the default camera.
        EntityHandle GetDefaultCamera() const { return _defaultCamera; }

        /// Return the Entity containing the active camera.
        EntityHandle GetActiveCamera() const { return _activeCamera; }

        EntityManager &GetEntityManager();

        void UpdateCachedTransforms();

    protected:
        SceneManager* _sceneManager = nullptr;

        EntityManager _entityManager;

        EntityHandle _defaultCamera;
        EntityHandle _activeCamera;

        std::vector<std::tuple<TransformComponent*>> &_spatials;
        std::vector<std::tuple<CameraComponent*, TransformComponent*>> &_cameras;
        
        std::vector<EntityHandle> _entities;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Scene);
    };
}
