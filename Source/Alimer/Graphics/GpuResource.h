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

#include "../Core/Ptr.h"
#include "../Graphics/Types.h"

namespace Alimer
{
	class Graphics;

    enum class GpuResourceType : uint32_t
    {
        Unknown,
        Buffer,
        Texture,
        RenderPass,
        Shader,
    };

	/// Defines a base GPU Resource.
	class ALIMER_API GpuResource //: public RefCounted
	{
	protected:
        /// Constructor.
        GpuResource(Graphics* graphics, GpuResourceType resourceType);

	public:
		/// Destructor.
		virtual ~GpuResource();

        /// Unconditionally destroy the GPU resource.
        virtual void Destroy() {}

        /// Return the graphics subsystem associated with this GPU object.
        Graphics* GetGraphics() const { return _graphics.Get(); }

        inline GpuResourceType GetResourceType() const { return _resourceType; }

    protected:
        /// Graphics subsystem.
        WeakPtr<Graphics> _graphics;
        GpuResourceType _resourceType;
		
	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(GpuResource);
	};
}
