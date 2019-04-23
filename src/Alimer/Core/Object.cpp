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

#include "../Core/Object.h"
#include "../Core/Log.h"
#include <memory>
#include <map>

using namespace std;

namespace alimer
{
    namespace details
    {
        struct SubSystemContext
        {
            void AddSubsystem(Object* subsystem)
            {
                _subsystems[subsystem->GetType()] = subsystem;
            }

            void RemoveSubsystem(Object* subsystem)
            {
                _subsystems.erase(subsystem->GetType());
            }

            void RemoveSubsystem(StringHash type)
            {
                _subsystems.erase(type);
            }

            Object* GetSubsystem(StringHash type)
            {
                auto it = _subsystems.find(type);
                return it != _subsystems.end() ? it->second : nullptr;
            }

            void RegisterFactory(ObjectFactory* factory)
            {
                auto it = _factories.find(factory->GetType());
                if (it == _factories.end())
                {
                    _factories[factory->GetType()].reset(factory);
                }
                else
                {
                    ALIMER_LOGERROR("Type already registered: %s", factory->GetTypeName().c_str());
                }
                
            }

            void RemoveFactory(StringHash type)
            {
                _factories.erase(type);
            }

            SharedPtr<Object> CreateObject(StringHash type)
            {
                auto it = _factories.find(type);
                if (it != _factories.end())
                {
                    return it->second->CreateObject();
                }

                return nullptr;
            }

            const std::string& GetTypeNameFromType(StringHash type)
            {
                auto it = _factories.find(type);
                if (it != _factories.end()) {
                    return it->second->GetTypeName();
                }

                static std::string empty_string = "";
                return empty_string;
            }

        private:
            /// Registered subsystems.
            map<StringHash, Object*> _subsystems;

            /// Registered object factories.
            map<StringHash, unique_ptr<ObjectFactory>> _factories;
        };

        SubSystemContext& Context()
        {
            static SubSystemContext s_context;
            return s_context;
        }
    }

    TypeInfo::TypeInfo(const char* typeName, const TypeInfo* baseTypeInfo)
        : _type(typeName)
        , _typeName(typeName)
        , _baseTypeInfo(baseTypeInfo)
    {
    }

    bool TypeInfo::IsTypeOf(StringHash type) const
    {
        const TypeInfo* current = this;
        while (current)
        {
            if (current->GetType() == type)
                return true;

            current = current->GetBaseTypeInfo();
        }

        return false;
    }

    bool TypeInfo::IsTypeOf(const TypeInfo* typeInfo) const
    {
        const TypeInfo* current = this;
        while (current)
        {
            if (current == typeInfo)
                return true;

            current = current->GetBaseTypeInfo();
        }

        return false;
    }

    bool Object::IsInstanceOf(StringHash type) const
    {
        return GetTypeInfo()->IsTypeOf(type);
    }



    bool Object::IsInstanceOf(const TypeInfo* typeInfo) const
    {
        return GetTypeInfo()->IsTypeOf(typeInfo);
    }

    void Object::AddSubsystem(Object* subsystem)
    {
        details::Context().AddSubsystem(subsystem);
    }

    void Object::RemoveSubsystem(Object* subsystem)
    {
        details::Context().RemoveSubsystem(subsystem);
    }

    void Object::RemoveSubsystem(StringHash type)
    {
        details::Context().RemoveSubsystem(type);
    }

    Object* Object::GetSubsystem(StringHash type)
    {
        return details::Context().GetSubsystem(type);
    }

    void Object::RegisterFactory(ObjectFactory* factory)
    {
        ALIMER_ASSERT(factory);
        details::Context().RegisterFactory(factory);
    }

    void Object::RemoveFactory(StringHash type)
    {
        details::Context().RemoveFactory(type);
    }

    SharedPtr<Object> Object::CreateObject(StringHash type)
    {
        return details::Context().CreateObject(type);
    }

    const std::string& Object::GetTypeNameFromType(StringHash type)
    {
        return details::Context().GetTypeNameFromType(type);
    }
}
