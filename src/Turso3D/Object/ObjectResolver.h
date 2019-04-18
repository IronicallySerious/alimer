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
#pragma once

#include "../Base/HashMap.h"

namespace Turso3D
{
    class Attribute;
    class Serializable;
    struct ObjectRef;

    /// Stored object ref attribute.
    struct TURSO3D_API StoredObjectRef
    {
        /// Construct undefined.
        StoredObjectRef() :
            object(nullptr),
            attr(nullptr),
            oldId(0)
        {
        }

        /// Construct with values.
        StoredObjectRef(Serializable* object_, Attribute* attr_, unsigned oldId_) :
            object(object_),
            attr(attr_),
            oldId(oldId_)
        {
        }

        /// %Object that contains the attribute.
        Serializable* object;
        /// Description of the object ref attribute.
        Attribute* attr;
        /// Old id from the serialized data.
        unsigned oldId;
    };

    /// Helper class for resolving object ref attributes when loading a scene.
    class TURSO3D_API ObjectResolver
    {
    public:
        /// Store an object along with its old id from the serialized data.
        void StoreObject(unsigned oldId, Serializable* object);
        /// Store an object ref attribute that needs to be resolved later.
        void StoreObjectRef(Serializable* object, Attribute* attr, const ObjectRef& value);
        /// Resolve the object ref attributes.
        void Resolve();

    private:
        /// Mapping of old id's to objects.
        HashMap<unsigned, Serializable*> objects;
        /// Stored object ref attributes.
        Vector<StoredObjectRef> objectRefs;
    };

}
