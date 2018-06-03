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

#include "../Foundation/Ptr.h"

namespace Alimer
{
	RefCounted::RefCounted()
		: _refCount(new RefCount())
	{
#if !ALIMER_DISABLE_THREADING
		// Hold a weak ref to self to avoid possible double delete of the refcount
		_refCount->weakRefs.fetch_add(1, std::memory_order_relaxed);
#else
		(_refCount->weakRefs)++;
#endif
	}

	RefCounted::~RefCounted()
	{
		assert(_refCount);
#if !ALIMER_DISABLE_THREADING
		assert(_refCount->refs.load(std::memory_order_relaxed) == 0);
		assert(_refCount->weakRefs.load(std::memory_order_relaxed) > 0);

		// Mark object as expired, release the self weak ref and delete the refcount if no other weak refs exist
		_refCount->refs.store(-1, std::memory_order_relaxed);
		if (_refCount->weakRefs.fetch_sub(1, std::memory_order_relaxed) == 1)
			delete _refCount;
#else
		assert(_refCount->refs == 0);
		assert(_refCount->weakRefs > 0);

		// Mark object as expired, release the self weak ref and delete the refcount if no other weak refs exist
		_refCount->refs = -1;
		(_refCount->weakRefs)--;
		if (!_refCount->weakRefs)
			delete _refCount;

#endif

		_refCount = nullptr;
	}

	void RefCounted::AddRef()
	{
#if !ALIMER_DISABLE_THREADING
		assert(_refCount->refs.load(std::memory_order_relaxed) >= 0);
		_refCount->refs.fetch_add(1, std::memory_order_relaxed);
#else
		assert(_refCount->refs >= 0);
		(_refCount->refs)++;
#endif
	}

	void RefCounted::Release()
	{
#if !ALIMER_DISABLE_THREADING
		assert(_refCount->refs.load(std::memory_order_relaxed) > 0);
		if (_refCount->refs.fetch_sub(1, std::memory_order_relaxed) == 1)
		{
			delete this;
		}

#else
		assert(_refCount->refs > 0);
		(_refCount->refs)--;
		if (!_refCount->refs)
		{
			delete this;
		}
#endif
	}

	int RefCounted::Refs() const
	{
#if !ALIMER_DISABLE_THREADING
		return _refCount->refs.load(std::memory_order_relaxed);
#else
		return _refCount->refs;
#endif
	}

	int RefCounted::WeakRefs() const
	{
		// Subtract one to not return the internally held reference
#if !ALIMER_DISABLE_THREADING
		return _refCount->weakRefs.load(std::memory_order_relaxed) - 1;
#else
		return _refCount->weakRefs - 1;
#endif
	}
}
