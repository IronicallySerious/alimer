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

#include "../IO/ResourceRef.h"
#include "../IO/Stream.h"
#include "../Core/Object.h"

namespace alimer
{
    bool ResourceRef::FromString(const String& str)
    {
        return FromString(str.CString());
    }

    bool ResourceRef::FromString(const char* str)
    {
        Vector<String> values = String::Split(str, ';');
        if (values.size() == 2)
        {
            type = values[0];
            name = values[1];
            return true;
        }
        
        return false;
    }

    void ResourceRef::FromBinary(Stream& source)
    {
        type = source.ReadStringHash();
        name = source.ReadString();
    }


    String ResourceRef::ToString() const
    {
        return Object::GetTypeNameFromType(type) + ";" + name;
    }

    void ResourceRef::ToBinary(Stream& dest) const
    {
        dest.WriteStringHash(type);
        dest.WriteString(name);
    }

    bool ResourceRefList::FromString(const String& str)
    {
        return FromString(str.CString());
    }

    bool ResourceRefList::FromString(const char* str)
    {
        Vector<String> values = String::Split(str, ';');
        if (values.size() >= 1)
        {
            type = values[0];
            names.clear();
            for (size_t i = 1; i < values.size(); ++i)
            {
                names.push_back(values[i]);
            }
            return true;
        }
        
        return false;
    }

    void ResourceRefList::FromBinary(Stream& source)
    {
        type = source.ReadStringHash();
        uint32_t count = source.ReadVLE();
        names.clear();
        for (size_t i = 0; i < count && !source.IsEof(); ++i)
        {
            names.push_back(source.ReadString());
        }
    }

    String ResourceRefList::ToString() const
    {
        String ret(Object::GetTypeNameFromType(type));
        for (auto name : names)
        {
            ret += ";";
            ret += name;
        }
        return ret;
    }

    void ResourceRefList::ToBinary(Stream& dest) const
    {
        dest.WriteStringHash(type);
        dest.WriteVLE(names.size());
        for (auto name : names)
        {
            dest.WriteString(name);
        }
    }
}
