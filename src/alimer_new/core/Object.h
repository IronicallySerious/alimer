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

#include "foundation/Ptr.h"
#include <string>

namespace alimer
{
    using TypeHash = size_t;

    /// Type info.
    class ALIMER_API TypeInfo final
    {
    public:
        /// Construct.
        TypeInfo(const char* typeName, const TypeInfo* baseTypeInfo);
        /// Destruct.
        ~TypeInfo() = default;

        /// Check current type is type of specified type.
        bool IsTypeOf(TypeHash type) const;
        /// Check current type is type of specified type.
        bool IsTypeOf(const TypeInfo* typeInfo) const;
        /// Check current type is type of specified class type.
        template<typename T> bool IsTypeOf() const { return IsTypeOf(T::GetTypeInfoStatic()); }

        /// Return type.
        TypeHash GetType() const { return _type; }
        /// Return type name.
        const std::string& GetTypeName() const { return _typeName; }
        /// Return base type info.
        const TypeInfo* GetBaseTypeInfo() const { return _baseTypeInfo; }        

    private:
        /// Type name.
        std::string _typeName;
        /// Type hash id.
        TypeHash _type;
        /// Base class type info.
        const TypeInfo* _baseTypeInfo;
    };

#define ALIMER_OBJECT(typeName, baseTypeName) \
    public: \
        using ClassName = typeName; \
        using Parent = baseTypeName; \
        virtual size_t GetType() const override { return GetTypeInfoStatic()->GetType(); } \
        virtual const std::string& GetTypeName() const override { return GetTypeInfoStatic()->GetTypeName(); } \
        virtual const alimer::TypeInfo* GetTypeInfo() const override { return GetTypeInfoStatic(); } \
        static size_t GetTypeStatic() { return GetTypeInfoStatic()->GetType(); } \
        static const std::string& GetTypeNameStatic() { return GetTypeInfoStatic()->GetTypeName(); } \
        static const alimer::TypeInfo* GetTypeInfoStatic() { static const alimer::TypeInfo typeInfoStatic(#typeName, Parent::GetTypeInfoStatic()); return &typeInfoStatic; } \

    class ObjectFactory;
    template <class T> class ObjectFactoryImpl;

    /// Base class for objects with type identification, subsystem access and event sending/receiving capability.
    class ALIMER_API Object : public RefCounted
    {
    public:
        /// Constructor.
        Object();

        /// Destructor.
        ~Object() override;

        /// Return type hash.
        virtual size_t GetType() const = 0;
        /// Return type name.
        virtual const std::string& GetTypeName() const = 0;
        /// Return type info.
        virtual const TypeInfo* GetTypeInfo() const = 0;

        /// Return type info static.
        static const TypeInfo* GetTypeInfoStatic() { return nullptr; }

        /// Check current instance is type of specified type.
        bool IsInstanceOf(TypeHash type) const;
        /// Check current instance is type of specified type.
        bool IsInstanceOf(const TypeInfo* typeInfo) const;
        /// Check current instance is type of specified class.
        template<typename T> bool IsInstanceOf() const { return IsInstanceOf(T::GetTypeInfoStatic()); }
        /// Cast the object to specified most derived class.
        template<typename T> T* Cast() { return IsInstanceOf<T>() ? static_cast<T*>(this) : nullptr; }
        /// Cast the object to specified most derived class.
        template<typename T> const T* Cast() const { return IsInstanceOf<T>() ? static_cast<const T*>(this) : nullptr; }

        /// Add a subsystem that can be accessed globally. Note that the subsystems container does not own the objects.
        static void AddSubsystem(Object* subsystem);
        /// Remove a subsystem by object pointer.
        static void RemoveSubsystem(Object* subsystem);
        /// Remove a subsystem by type.
        static void RemoveSubsystem(TypeHash type);
        /// Return a subsystem by type, or null if not registered.
        static Object* GetSubsystem(TypeHash type);

        /// Return a subsystem, template version.
        template <class T> static T* GetSubsystem() { return static_cast<T*>(GetSubsystem(T::GetTypeStatic())); }

        /// Register an object factory.
        static void RegisterFactory(ObjectFactory* factory);

        /// Register an object factory, template version.
        template <class T> static void RegisterFactory() { RegisterFactory(new ObjectFactoryImpl<T>()); }

        /// Remove object factory.
        static void RemoveFactory(TypeHash type);

        /// Create an object by type. Return pointer to it or null if no factory found.
        template <class T> static inline SharedPtr<T> CreateObject()
        {
            return StaticCast<T>(CreateObject(T::GetTypeStatic()));
        }

        /// Create and return an object through a factory. The caller is assumed to take ownership of the object. Return null if no factory registered. 
        static SharedPtr<Object> CreateObject(TypeHash type);
    };

    /// Base class for object factories.
    class ALIMER_API ObjectFactory
    {
    public:
        /// Destruct.
        virtual ~ObjectFactory() = default;

        /// Create and return an object.
        virtual SharedPtr<Object> CreateObject() = 0;

        /// Return type info of objects created by this factory.
        const TypeInfo* GetTypeInfo() const { return _typeInfo; }

        /// Return type hash of objects created by this factory.
        TypeHash GetType() const { return _typeInfo->GetType(); }

        /// Return type name of objects created by this factory.
        const std::string& GetTypeName() const { return _typeInfo->GetTypeName(); }

    protected:
        /// Type info.
        const TypeInfo* _typeInfo{};
    };

    /// Template implementation of the object factory.
    template <class T> class ObjectFactoryImpl : public ObjectFactory
    {
    public:
        /// Construct.
        ObjectFactoryImpl()
        {
            _typeInfo = T::GetTypeInfoStatic();
        }

        /// Create and return an object of the specific type.
        SharedPtr<Object> CreateObject() override { return SharedPtr<Object>(new T()); }
    };

}
