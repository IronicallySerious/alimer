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

#include "../Scene/Component.h"
#include <vector>
#include <typeindex>

namespace Alimer
{
    class Scene;

	/// Defines a base ComponentSystem class.
    class ALIMER_API ComponentSystem
	{
        friend class Scene;

    protected:
        /// Constructor.
        ComponentSystem(std::type_index type);

    public:
        /// Destructor.
        virtual ~ComponentSystem() = default;

        /// Updates the system
        virtual void Update(double deltaTime);

        /// Gets the unique type ID of the system.
        std::type_index GetType() const { return _type; }

        /// Gets true if the system is currently active.
        bool IsActive() const { return _active; }

    private:
        void SetScene(Scene* scene);

        Scene* _scene;
        std::type_index _type;
        bool _active;

    private:
        DISALLOW_COPY_MOVE_AND_ASSIGN(ComponentSystem);
	};
}
