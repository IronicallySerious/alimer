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

#pragma once

#include "../Base/Ptr.h"
#include "../Graphics/Types.h"

namespace alimer
{
	class GraphicsDevice;

	/// Defines a GPUResource created from GPUDevice.
	class ALIMER_API GPUResource
	{
	public:
        enum class Type
        {
            Buffer,
            Texture,
            Sampler,
            Framebuffer,
            Shader, 
            Program,
            Pipeline
        };

		/// Destructor.
		virtual ~GPUResource();

        /// Unconditionally destroy the GPU resource.
        virtual void Destroy() {}

        /// Get the creation device.
        GraphicsDevice* GetGraphicsDevice() const;

        /// Get the resource type.
        Type GetResourceType() const;

    protected:
        /// Constructor.
        explicit GPUResource(GraphicsDevice* device, Type resourceType);

        /// Graphics subsystem.
        WeakPtr<GraphicsDevice> _device;

        Type _resourceType;
	};
}
