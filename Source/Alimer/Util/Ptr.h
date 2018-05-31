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

#include "../PlatformDef.h"
#include <cassert>
#include <cstddef>
#include <utility>

namespace Alimer
{
	/// Reference count structure.
	struct RefCount
	{
		/// Construct.
		RefCount() = default;

		/// Destruct.
		~RefCount()
		{
			// Set reference counts below zero to fire asserts if this object is still accessed
			refs = static_cast<uint32_t>(-1);
			weakRefs = static_cast<uint32_t>(-1);
		}

		/// Reference count. If below zero, the object has been destroyed.
		uint32_t refs{ 0 };
		/// Weak reference count.
		uint32_t weakRefs{ 0 };
	};

	/// Base class for intrusively reference counted objects that can be pointed to with SharedPtr and WeakPtr. These are not copy-constructible and not assignable.
	class RefCounted
	{
	public:
		/// Construct. The reference count is not allocated yet; it will be allocated on demand.
		RefCounted();

		/// Destruct. If no weak references, destroy also the reference count, else mark it expired.
		virtual ~RefCounted();

		/// Add a strong reference. Allocate the reference count structure first if necessary.
		uint32_t AddRef();
		/// Release a strong reference. Destroy the object when the last strong reference is gone.
		uint32_t Release();

		/// Return the number of strong references.
		uint32_t Refs() const;
		/// Return the number of weak references.
		uint32_t WeakRefs() const;
		/// Return pointer to the reference count structure. Allocate if not allocated yet.
		RefCount* RefCountPtr() { return _refCount; }

	private:
		/// Reference count structure, allocated on demand.
		RefCount* _refCount;

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(RefCounted);
	};
}
