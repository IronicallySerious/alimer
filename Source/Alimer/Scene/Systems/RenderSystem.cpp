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

#include "../Systems/RenderSystem.h"
#include "../Scene.h"
#include "../../Graphics/Graphics.h"

namespace Alimer
{
    // TODO: Add frustum
    template <typename T>
    static void GatherVisibleRenderables(VisibilitySet &list, const T &objects)
    {
        for (auto &o : objects)
        {
            TransformComponent *transform = std::get<0>(o);
            RenderableComponent *renderable = std::get<1>(o);

            if (transform)
            {
                // if (frustum.intersects_fast(transform->world_aabb))
                list.push_back({ renderable, transform });
            }
            else
            {
                list.push_back({ renderable, nullptr });
            }
        }
    }

    RenderSystem::RenderSystem(EntityManager &entities)
        : _renderable(entities.GetComponentGroup<RenderableComponent, TransformComponent>())
    {

    }

    void RenderSystem::Update(double deltaTime)
    {
        // Gather visibles.
        _visibleSet.clear();

        /*auto view = entities.view<TransformComponent, RenderableComponent>();

        // TODO: Add async culling.
        for (auto entity : view)
        {
            auto &transformComponent = view.get<TransformComponent>(entity);
            auto &renderableComponent = view.get<RenderableComponent>(entity);

            //if (transformComponent)
            {
                // if (frustum.intersects_fast(transform->world_aabb))
                _visibleSet.push_back({ renderableComponent.renderable.Get(), nullptr });
            }
            //else
            //{
            //    _visibleSet.push_back({ renderableComponent.renderable.Get(), nullptr });
            //}
        }*/
        
        if (_visibleSet.size())
        {
            for (auto &visible : _visibleSet)
            {
            }
        }
    }

    void RenderSystem::Render(CommandBuffer* context)
    {
        //context->BeginRenderPass(nullptr, Color(0.0f, 0.2f, 0.4f, 1.0f));

        // Bind per camera UBO
        //context->SetUniformBuffer(0, 0, _perCameraUboBuffer.Get());

        for (auto &visible : _visibleSet)
        {
            //visible.renderable->Render(commandBuffer);
            //visible.renderable->Render(context, vis.transform, queue);
        }

        //context->EndRenderPass();
    }
}
