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

#if TODO_ENTITY
#include "../Scene/Scene.h"
#include "../Scene/Components/TransformComponent.h"
#include "../Scene/Components/CameraComponent.h"
#include "../Core/Log.h"

namespace alimer
{
    Scene::Scene(EntityManager& entities)
        : _entities(entities)
    {
        _defaultCamera = CreateEntity("Default Camera");
        _defaultCamera.Assign<TransformComponent>();
        _defaultCamera.Assign<CameraComponent>();
        ALIMER_ASSERT(_defaultCamera.HasComponent<TransformComponent>());
        ALIMER_ASSERT(_defaultCamera.HasComponent<CameraComponent>());
        //_defaultCamera->AddComponent<AudioListener>();
        _activeCamera = _defaultCamera;
    }

    Scene::~Scene()
    {
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        Entity entity = _entities.Create();
        entity.SetName(name);
        return entity;
    }
}

#endif // TODO_ENTITY
