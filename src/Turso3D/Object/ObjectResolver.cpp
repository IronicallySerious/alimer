//
// Alimer is based on the Turso3D codebase.
// Copyright (c) 2018-2019 Amer Koleci and contributors.
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

#include "../Debug/Log.h"
#include "../IO/ObjectRef.h"
#include "ObjectResolver.h"
#include "Serializable.h"

#include "../Debug/DebugNew.h"

namespace Turso3D
{
    void ObjectResolver::StoreObject(unsigned oldId, Serializable* object)
    {
        if (object)
            objects[oldId] = object;
    }

    void ObjectResolver::StoreObjectRef(Serializable* object, Attribute* attr, const ObjectRef& value)
    {
        if (object && attr && attr->Type() == ATTR_OBJECTREF)
            objectRefs.Push(StoredObjectRef(object, attr, value.id));
    }

    void ObjectResolver::Resolve()
    {
        for (auto it = objectRefs.Begin(); it != objectRefs.End(); ++it)
        {
            auto refIt = objects.Find(it->oldId);
            // See if we can find the referred to object
            if (refIt != objects.End())
            {
                AttributeImpl<ObjectRef>* typedAttr = static_cast<AttributeImpl<ObjectRef>*>(it->attr);
                typedAttr->SetValue(it->object, ObjectRef(refIt->second->Id()));
            }
            else {
                TURSO3D_LOGWARN("Could not resolve object reference " + String(it->oldId));
            }
        }
    }
}
