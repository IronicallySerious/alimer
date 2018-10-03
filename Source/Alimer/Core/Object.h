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
#include "../Base/StringHash.h"
#include "../Core/Event.h"

namespace Alimer
{
    class ObjectFactory;
    template <class T> class ObjectFactoryImpl;

    /// Type info.
    class ALIMER_API TypeInfo final
    {
    public:
        /// Constructor.
        TypeInfo(const char* typeName, const TypeInfo* baseTypeInfo);
        /// Destructor.
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
        const std::string& GetTypeName() const { return _typeName; }
        /// Return base type info.
        const TypeInfo* GetBaseTypeInfo() const { return _baseTypeInfo; }

    private:
        /// Type.
        StringHash _type;
        /// Type name.
        std::string _typeName;
        /// Base class type info.
        const TypeInfo* _baseTypeInfo;

        DISALLOW_COPY_MOVE_AND_ASSIGN(TypeInfo);
    };

    /// Base class for objects with type identification and possibility to create through a factory.
    class ALIMER_API Object : public RefCounted
    {
    public:
        /// Destructor.
        virtual ~Object() = default;

        /// Return hash of the type name.
        virtual StringHash GetType() const = 0;
        /// Return type name.
        virtual const std::string& GetTypeName() const = 0;
        /// Return type info.
        virtual const TypeInfo* GetTypeInfo() const = 0;

        /// Return type info static.
        static const TypeInfo* GetTypeInfoStatic() { return nullptr; }

        /// Check current instance is type of specified type.
        bool IsInstanceOf(StringHash type) const;
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
        static void RemoveSubsystem(StringHash type);
        /// Return a subsystem by type, or null if not registered.
        static Object* GetSubsystem(StringHash type);

        /// Return a subsystem, template version.
        template <class T> static T* GetSubsystem() { return static_cast<T*>(GetSubsystem(T::GetTypeStatic())); }

        /// Subscribe to an event.
        void SubscribeToEvent(Event& event, EventHandler* handler);
        /// Unsubscribe from an event.
        void UnsubscribeFromEvent(Event& event);
        /// Send an event.
        void SendEvent(Event& event);

        /// Subscribe to an event, template version.
        template <class T, class U> void SubscribeToEvent(U& event, void (T::*handlerFunction)(U&))
        {
            SubscribeToEvent(event, new EventHandlerImpl<T, U>(this, handlerFunction));
        }

        /// Return whether is subscribed to an event.
        bool IsSubscribedToEvent(const Event& event) const;
    };
}

#define ALIMER_OBJECT(typeName, baseTypeName) \
	public: \
		using ClassName = typeName; \
		using Parent = baseTypeName; \
		virtual Alimer::StringHash GetType() const override { return GetTypeInfoStatic()->GetType(); } \
		virtual const std::string& GetTypeName() const override { return GetTypeInfoStatic()->GetTypeName(); } \
		virtual const Alimer::TypeInfo* GetTypeInfo() const override { return GetTypeInfoStatic(); } \
		static Alimer::StringHash GetTypeStatic() { return GetTypeInfoStatic()->GetType(); } \
		static const std::string& GetTypeNameStatic() { return GetTypeInfoStatic()->GetTypeName(); } \
		static const Alimer::TypeInfo* GetTypeInfoStatic() { static const Alimer::TypeInfo typeInfoStatic(#typeName, Parent::GetTypeInfoStatic()); return &typeInfoStatic; } \
