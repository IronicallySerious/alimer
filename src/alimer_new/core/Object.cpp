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

#include "core/Object.h"
#include "foundation/Hash.h"

namespace alimer
{
    TypeInfo::TypeInfo(const char* typeName, const TypeInfo* baseTypeInfo)
        : _typeName(typeName)
        , _type(Hash(_typeName))
        , _baseTypeInfo(baseTypeInfo)
    {
    }

    Object::Object()
    {
    }

    Object::~Object()
    {
    }

    bool TypeInfo::IsTypeOf(size_t type) const
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
}
