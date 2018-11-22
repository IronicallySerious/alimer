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

#include "../Base/StringHash.h"
#include <vector>

namespace Alimer
{
    class Stream;

    /// Reference to an object with id for serialization.
    struct ALIMER_API ObjectRef
    {
        /// Object id.
        uint32_t id = 0;

        /// Construct with no reference.
        ObjectRef() noexcept = default;

        /// Copy-construct.
        ObjectRef(const ObjectRef& ref) noexcept : id(ref.id)
        {
        }

        /// Construct with object id.
        ObjectRef(uint32_t id_) noexcept : id(id_) {}

        /// Test for equality with another reference.
        bool operator == (const ObjectRef& rhs) const { return id == rhs.id; }
        /// Test for inequality with another reference.
        bool operator != (const ObjectRef& rhs) const { return !(*this == rhs); }
    };

    /// Typed resource reference for serialization.
    struct ALIMER_API ResourceRef
    {
        /// Resource type.
        StringHash type;
        /// Resource name.
        String name;

        /// Construct.
        ResourceRef() = default;

        /// Construct from another ResourceRef.
        ResourceRef(const ResourceRef& rhs)
            : type(rhs.type)
            , name(rhs.name)
        {
        }

        /// Construct with type and resource name.
        ResourceRef(StringHash type_, const String& name_ = String::EMPTY)
            : type(type_)
            , name(name_)
        {
        }

        /// Construct with type and resource name.
        ResourceRef(const String& type_, const String& name_)
            : type(type_)
            , name(name_)
        {
        }

        /// Construct from a string.
        ResourceRef(const String& str)
        {
            FromString(str);
        }

        /// Construct from a C string.
        ResourceRef(const char* str)
        {
            FromString(str);
        }

        /// Set from a string that contains the type and name separated by a semicolon. Return true on success.
        bool FromString(const String& str);
        /// Set from a C string that contains the type and name separated by a semicolon. Return true on success.
        bool FromString(const char* str);
        /// Deserialize from a binary stream.
        void FromBinary(Stream& source);

        /// Return as a string.
        String ToString() const;
        /// Serialize to a binary stream.
        void ToBinary(Stream& dest) const;

        /// Test for equality with another reference.
        bool operator ==(const ResourceRef& rhs) const { return type == rhs.type && name == rhs.name; }

        /// Test for inequality with another reference.
        bool operator !=(const ResourceRef& rhs) const { return type != rhs.type || name != rhs.name; }
	};

    /// %List of typed resource references for serialization.
    struct ALIMER_API ResourceRefList
    {
        /// Resource type.
        StringHash type;
        /// List of resource names.
        std::vector<String> names;

        /// Construct.
        ResourceRefList() = default;

        // Copy-construct.
        ResourceRefList(const ResourceRefList& rhs) 
            : type(rhs.type)
            , names(rhs.names)
        {
        }

        /// Construct from a string.
        ResourceRefList(const String& str)
        {
            FromString(str);
        }

        /// Construct from a C string.
        ResourceRefList(const char* str)
        {
            FromString(str);
        }

        /// Construct with type and name list.
        ResourceRefList(StringHash type_, const std::vector<String>& names_ = std::vector<String>()) 
            : type(type_)
            , names(names_)
        {
        }

        /// Set from a string that contains the type and names separated by semicolons. Return true on success.
        bool FromString(const String& str);
        /// Set from a C string that contains the type and names separated by semicolons. Return true on success.
        bool FromString(const char* str);
        /// Deserialize from a binary stream.
        void FromBinary(Stream& source);

        /// Return as a string.
        String ToString() const;
        /// Deserialize from a binary stream.
        void ToBinary(Stream& dest) const;

        /// Test for equality with another reference list.
        bool operator == (const ResourceRefList& rhs) const { return type == rhs.type && names == rhs.names; }
        /// Test for inequality with another reference list.
        bool operator != (const ResourceRefList& rhs) const { return !(*this == rhs); }
    };

}
