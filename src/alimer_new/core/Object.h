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

#include "foundation/Assert.h"
#include <string>

namespace alimer
{
    // TODO: Replace with own String
    using StringHash = std::string;
    using String = std::string;

    /// Type info.
    class ALIMER_API TypeInfo final
    {
    public:
        /// Construct.
        TypeInfo(const char* typeName, const TypeInfo* baseTypeInfo);
        /// Destruct.
        ~TypeInfo() = default;

        /// Check current type is type of specified type.
        bool IsTypeOf(StringHash type) const;
        /// Check current type is type of specified type.
        bool IsTypeOf(const TypeInfo* typeInfo) const;
        /// Check current type is type of specified class type.
        template<typename T> bool IsTypeOf() const { return IsTypeOf(T::GetTypeInfoStatic()); }

        /// Return type.
        StringHash GetType() const { return _type; }
        /// Return type name.
        const String& GetTypeName() const { return _typeName; }
        /// Return base type info.
        const TypeInfo* GetBaseTypeInfo() const { return _baseTypeInfo; }        

    private:
        /// Type.
        StringHash _type;
        /// Type name.
        String _typeName;
        /// Base class type info.
        const TypeInfo* _baseTypeInfo;
    };

#define ALIMER_OBJECT(typeName, baseTypeName) \
    public: \
        using ClassName = typeName; \
        using Parent = baseTypeName; \
        virtual alimer::StringHash GetType() const override { return GetTypeInfoStatic()->GetType(); } \
        virtual const alimer::String& GetTypeName() const override { return GetTypeInfoStatic()->GetTypeName(); } \
        virtual const alimer::TypeInfo* GetTypeInfo() const override { return GetTypeInfoStatic(); } \
        static alimer::StringHash GetTypeStatic() { return GetTypeInfoStatic()->GetType(); } \
        static const alimer::String& GetTypeNameStatic() { return GetTypeInfoStatic()->GetTypeName(); } \
        static const alimer::TypeInfo* GetTypeInfoStatic() { static const alimer::TypeInfo typeInfoStatic(#typeName, Parent::GetTypeInfoStatic()); return &typeInfoStatic; } \

    /// Base class for objects with type identification, subsystem access and event sending/receiving capability.
    class ALIMER_API Object 
    {
    public:
        /// Constructor.
        Object();

        /// Destructor.
        virtual ~Object();

        /// Return type hash.
        virtual StringHash GetType() const = 0;
        /// Return type name.
        virtual const String& GetTypeName() const = 0;
        /// Return type info.
        virtual const TypeInfo* GetTypeInfo() const = 0;

        /// Return type info static.
        static const TypeInfo* GetTypeInfoStatic() { return nullptr; }
    };
}
