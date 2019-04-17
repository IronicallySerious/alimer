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

#include "../Object/Object.h"
#include "ResourceRef.h"
#include "Stream.h"

#include "../Debug/DebugNew.h"

namespace Turso3D
{
    bool ResourceRef::FromString(const String& str)
    {
        return FromString(str.CString());
    }

    bool ResourceRef::FromString(const char* str)
    {
        Vector<String> values = String::Split(str, ';');
        if (values.Size() == 2)
        {
            type = values[0];
            name = values[1];
            return true;
        }
        else
            return false;
    }

    void ResourceRef::FromBinary(Stream& source)
    {
        type = source.Read<StringHash>();
        name = source.Read<String>();
    }

    String ResourceRef::ToString() const
    {
        return Object::TypeNameFromType(type) + ";" + name;
    }

    void ResourceRef::ToBinary(Stream& dest) const
    {
        dest.Write(type);
        dest.Write(name);
    }

    bool ResourceRefList::FromString(const String& str)
    {
        return FromString(str.CString());
    }

    bool ResourceRefList::FromString(const char* str)
    {
        Vector<String> values = String::Split(str, ';');
        if (values.Size() >= 1)
        {
            type = values[0];
            names.Clear();
            for (size_t i = 1; i < values.Size(); ++i)
                names.Push(values[i]);
            return true;
        }
        else
            return false;
    }

    void ResourceRefList::FromBinary(Stream& source)
    {
        type = source.Read<StringHash>();
        size_t num = source.ReadVLE();
        names.Clear();
        for (size_t i = 0; i < num && !source.IsEof(); ++i)
            names.Push(source.Read<String>());
    }

    String ResourceRefList::ToString() const
    {
        String ret(Object::TypeNameFromType(type));
        for (auto it = names.Begin(); it != names.End(); ++it)
        {
            ret += ";";
            ret += *it;
        }
        return ret;
    }

    void ResourceRefList::ToBinary(Stream& dest) const
    {
        dest.Write(type);
        dest.WriteVLE(names.Size());
        for (auto it = names.Begin(); it != names.End(); ++it) {
            dest.Write(*it);
        }
    }
}
