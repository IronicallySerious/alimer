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

namespace Alimer
{
    enum class DrawPipeline : unsigned
    {
        Opaque,
        AlphaTest,
        AlphaBlend,
    };

    class ALIMER_API Renderable : public IntrusivePtrEnabled<Renderable>
    {
    public:
        virtual DrawPipeline getDrawPipeline() const
        {
            return DrawPipeline::Opaque;
        }
    };

    class Mesh;
    class ALIMER_API MeshRenderable : public Renderable
    {
    public:
        MeshRenderable(Mesh* mesh);

    private:
        Mesh* _mesh;
    };

    using RenderableHandle = IntrusivePtr<Renderable>;

    struct RenderableComponent : public Component<RenderableComponent>
    {
        RenderableHandle renderable;
    };

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
        Entity CreateEntity(const std::string& name);

        /// Return the Entity containing the default camera.
        Entity GetDefaultCamera() const { return _defaultCamera; }

        /// Return the Entity containing the active camera.
        Entity GetActiveCamera() const { return _activeCamera; }

    private:
        EntityManager& _entities;
        //ComponentManager<NameComponent> _names;
        Entity _defaultCamera;
        Entity _activeCamera;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(Scene);
    };
}
