//
// Copyright (c) 2008-2019 the Urho3D project.
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

#include "foundation/RefCounted.h"
#include "foundation/Assert.h"

namespace alimer
{
    RefCounted::RefCounted()
        : _refCount(new RefCount())
    {
        // Hold a weak ref to self to avoid possible double delete of the refcount
        (_refCount->weakRefs_)++;
    }

    RefCounted::~RefCounted()
    {
        ALIMER_ASSERT(_refCount);
        ALIMER_ASSERT(_refCount->refs_ == 0);
        ALIMER_ASSERT(_refCount->weakRefs_ > 0);

        // Mark object as expired, release the self weak ref and delete the refcount if no other weak refs exist
        _refCount->refs_ = -1;
        (_refCount->weakRefs_)--;
        if (!_refCount->weakRefs_) {
            delete _refCount;
        }

        _refCount = nullptr;
    }

    void RefCounted::AddRef()
    {
        ALIMER_ASSERT(_refCount->refs_ >= 0);
        (_refCount->refs_)++;
    }

    void RefCounted::Release()
    {
        ALIMER_ASSERT(_refCount->refs_ > 0);
        (_refCount->refs_)--;
        if (!_refCount->refs_)
            delete this;
    }

    int RefCounted::Refs() const
    {
        return _refCount->refs_;
    }

    int RefCounted::WeakRefs() const
    {
        // Subtract one to not return the internally held reference
        return _refCount->weakRefs_ - 1;
    }

}
