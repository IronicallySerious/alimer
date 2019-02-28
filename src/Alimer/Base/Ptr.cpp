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

#include "../Base/Ptr.h"

#if ALIMER_CSHARP
#ifdef _MSC_VER
#   define CSHARP_EXPORT __declspec(dllexport)
#else
#   define CSHARP_EXPORT
#endif

enum RefCountedCallbackType
{
    RefCounted_AddRef,
    RefCounted_Delete,
};

typedef void(*RefCountedCallback)(RefCountedCallbackType, alimer::RefCounted*);
RefCountedCallback _refCountedNativeCallback;

extern "C" CSHARP_EXPORT void AlimerRegisterRefCountedCallback(RefCountedCallback callback)
{
    _refCountedNativeCallback = callback;
}

void InvokeRefCountedCallback(RefCountedCallbackType type, alimer::RefCounted* instance)
{
    if (_refCountedNativeCallback)
        _refCountedNativeCallback(type, instance);
}

#endif

namespace alimer
{
    RefCounted::RefCounted()
        : _refCount(new RefCount())
    {
        (_refCount->weakRefs)++;
    }

    RefCounted::~RefCounted()
    {
        assert(_refCount);
        assert(_refCount->refs == 0);
        assert(_refCount->weakRefs > 0);

#if ALIMER_CSHARP
        InvokeRefCountedCallback(RefCounted_Delete, this);
#endif

        // Mark object as expired, release the self weak ref and delete the refcount if no other weak refs exist
        _refCount->refs = -1;
        (_refCount->weakRefs)--;
        if (!_refCount->weakRefs) {
            delete _refCount;
        }

        _refCount = nullptr;
    }

    void RefCounted::AddRef()
    {
        assert(_refCount->refs >= 0);
        (_refCount->refs)++;

#if ALIMER_CSHARP
        InvokeRefCountedCallback(RefCounted_AddRef, this);
#endif
    }

    void RefCounted::Release()
    {
        assert(_refCount->refs > 0);
        (_refCount->refs)--;
        if (!_refCount->refs)
        {
            delete this;
        }
    }

    int RefCounted::Refs() const
    {
        return _refCount->refs;
    }

    int RefCounted::WeakRefs() const
    {
        // Subtract one to not return the internally held reference
        return _refCount->weakRefs - 1;
    }
}
