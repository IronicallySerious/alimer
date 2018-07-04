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

#include "../Scene/Scene.h"
#include "../Scene/CameraComponent.h"

namespace Alimer
{
    class RenderPass;
    class Graphics;

    class Renderable;
    class RenderableComponent;

    struct RenderableInfo
    {
        Renderable *renderable;
        const TransformComponent *transform;
    };
    using VisibilitySet = std::vector<RenderableInfo>;

	/// Defines a scene renderer.
    class ALIMER_API SceneRenderer
	{
    public:
        SceneRenderer(Graphics* graphics);
        virtual ~SceneRenderer() = default;

        void Render(Scene* scene, RenderPass* frameRenderPass);

    private:
        /// Graphics subsystem.
        WeakPtr<Graphics> _graphics;

        struct PerCameraCBuffer
        {
            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;
        };

        PerCameraCBuffer _camera;
        SharedPtr<GpuBuffer> _perCameraUboBuffer;

        VisibilitySet _visibleSet;
	};
}
