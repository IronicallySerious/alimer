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

#include "../Scene/Scene.h"
#include "../Scene/Components/TransformComponent.h"
#include "../Scene/Components/CameraComponent.h"
#include "../Scene/Systems/CameraSystem.h"
#include "../Scene/Systems/RenderSystem.h"
#include "../Core/Log.h"
using namespace std;

namespace Alimer
{
	Scene::Scene()
        : _spatials(_entityManager.GetComponentGroup<TransformComponent>())
	{
        _defaultCamera = CreateEntity();
        _defaultCamera->AddComponent<TransformComponent>();
        _defaultCamera->AddComponent<CameraComponent>();
        //_defaultCamera->AddComponent<AudioListener>();

        _activeCamera = _defaultCamera;

        // Setup systems.
        AddSystem<CameraSystem>(_entityManager);
        AddSystem<RenderSystem>(_entityManager);
	}

	Scene::~Scene()
	{
        _entityManager.ResetGroups();
	}

    EntityHandle Scene::CreateEntity()
    {
        EntityHandle entity = _entityManager.CreateEntity();
        _entities.push_back(entity);
        return entity;
    }

    EntityManager &Scene::GetEntityManager()
    {
        return _entityManager;
    }

    void Scene::Update(double deltaTime)
    {
        for (auto& system : _activeSystems)
        {
            system->Update(deltaTime);
        }
    }

    void Scene::Render(CommandBuffer* context)
    {
        GetSystem<RenderSystem>().Render(context);
    }

    void Scene::UpdateCachedTransforms()
    {
        /*const glm::mat4 &parentTransform = glm::mat4(1.0f);

        for (auto &s : _spatials)
        {
            TransformComponent *transform;
            tie(transform) = s;
            //transform->Update();
            ComputeTransform(
                transform->transform.translation, transform->transform.rotation, transform->transform.scale,
                transform->worldTransform,
                parentTransform);
            //std::tie(aabb, cached_transform, timestamp) = s;
            //if (transform->lastTimestamp != *transform->currentTimestamp)
            //{
            //    transform->lastTimestamp = *transform->currentTimestamp;
            //}
        }*/
    }
}
