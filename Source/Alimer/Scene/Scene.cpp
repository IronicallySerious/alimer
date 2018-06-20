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
#include "../Scene/TransformComponent.h"
#include "../Scene/CameraComponent.h"
#include "../Scene/Renderable.h"

#include "../Core/Log.h"
using namespace std;
namespace Alimer
{
	Scene::Scene()
        : _entityManager(events)
        , _spatials(_entityManager.GetComponentGroup<TransformComponent>())
        , _cameras(_entityManager.GetComponentGroup<CameraComponent, TransformComponent>())
        , _renderables(_entityManager.GetComponentGroup<TransformComponent, RenderableComponent>())
	{
        _defaultCamera = CreateEntity();
        _defaultCamera->AddComponent<TransformComponent>();
        _defaultCamera->AddComponent<CameraComponent>();
        //_defaultCamera->GetComponent<CameraComponent>().setViewport(getDefaultViewport());
        //_defaultCamera->AddComponent<AudioListener>();

        _activeCamera = _defaultCamera;
	}

	Scene::~Scene()
	{
        _perCameraUboBuffer.Reset();
        _entityManager.ResetGroups();
	}

    EntityHandle Scene::CreateEntity()
    {
        EntityHandle entity = _entityManager.CreateEntity();
        _pendingEntities.push_back(_entityManager.CreateEntity());
        return entity;
    }

    EntityManager &Scene::GetEntityManager()
    {
        return _entityManager;
    }

    void Scene::UpdateCachedTransforms()
    {
        const glm::mat4 &parentTransform = glm::mat4(1.0f);

        for (auto &s : _spatials)
        {
            TransformComponent *transform;
            tie(transform) = s;
            ComputeTransform(
                transform->transform.translation, transform->transform.rotation, transform->transform.scale,
                transform->worldTransform,
                parentTransform);
            //std::tie(aabb, cached_transform, timestamp) = s;
            //if (transform->lastTimestamp != *transform->currentTimestamp)
            //{
            //    transform->lastTimestamp = *transform->currentTimestamp;
            //}
        }

        // Update camera transforms.
        for (auto &c : _cameras)
        {
            CameraComponent *camera;
            TransformComponent *transform;
            tie(camera, transform) = c;
            camera->view = glm::inverse(transform->worldTransform);
            camera->projection = glm::perspective(camera->fovy, camera->aspect, camera->znear, camera->zfar);
        }
    }

    // TODO: Add frustum
    template <typename T>
    static void GatherVisibleRenderables(VisibilitySet &list, const T &objects)
    {
        for (auto &o : objects)
        {
            TransformComponent *transform = get<0>(o);
            RenderableComponent *renderable = get<1>(o);

            if (transform)
            {
               // if (frustum.intersects_fast(transform->world_aabb))
                    list.push_back({ renderable->renderable.Get(), transform });
            }
            else
            {
                list.push_back({ renderable->renderable.Get(), nullptr });
            }
        }
    }

    void Scene::Render(CommandBuffer* commandBuffer)
    {
        _camera.viewMatrix = glm::mat4(1.0f);
        _camera.projectionMatrix = glm::mat4(1.0f);

        if (_perCameraUboBuffer.IsNull())
        {
            GpuBufferDescription uboBufferDesc = {};
            uboBufferDesc.usage = BufferUsage::Uniform;
            uboBufferDesc.elementCount = 3;
            uboBufferDesc.elementSize = sizeof(PerCameraCBuffer);
            _perCameraUboBuffer = gGraphics().CreateBuffer(uboBufferDesc, &_camera);
        }

        // Gather visibles.
        _visibleSet.clear();
        GatherVisibleRenderables(_visibleSet, _renderables);

        // Bind per camera UBO
        commandBuffer->SetUniformBuffer(0, 0, _perCameraUboBuffer.Get());

        for (auto &visible : _visibleSet)
        {
            visible.renderable->Render(commandBuffer);
            //visible.renderable->Render(context, vis.transform, queue);
        }
    }
}
