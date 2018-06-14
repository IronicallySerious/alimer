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

#include "../Serialization/JsonSerializer.h"
#include "../Core/Log.h"
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;

namespace Alimer
{
    class JsonSerializerImpl final
    {
    public:
        JsonSerializerImpl(Stream& stream)
            : _outStream(stream)
        {
            _root.object();
            _objectStack.push_back(&_root);
        }

        ~JsonSerializerImpl()
        {
            ALIMER_ASSERT(_objectStack.size() == 1);

            std::ostringstream ss;
            ss << std::setw(4) << _root << std::endl;
            std::string str = ss.str();
            _outStream.Write(str.c_str(), str.size());
        }

        json& GetObject()
        {
            return *_objectStack.back();
        }

        json& InsertValue(const char* key, json& value)
        {
            assert(_objectStack.size() > 0);

            json& currentValue = *_objectStack.back();

            if (_objectStack.back()->is_object())
            {
                currentValue[key] =  value;
                return currentValue[key];
            }
            else
            {
                size_t newIndex = currentValue.size();
                currentValue.push_back(value);
                return currentValue[newIndex];
            }
        }

        template<typename T, typename = typename std::enable_if<std::is_fundamental<T>::value>::type>
        bool SerializePrimitive(const char* key, T value)
        {
            json& object = GetObject();
            if (key && strlen(key))
            {
                object[key] = value;
            }
            else
            {
                object.push_back(value);
            }

            return true;
        }

        void SerializeString(const char* key, const char* value)
        {
            json& object = GetObject();
            if (key && strlen(key))
            {
                object[key] = value;
            }
            else
            {
                object.push_back(value);
            }
        }

        void Serialize(
            const char* key,
            const float* values,
            uint32_t count)
        {
            json& object = GetObject();
            json jValue;

            for (uint32_t i = 0; i < count; ++i)
            {
                jValue.push_back(values[i]);
            }

            if (key && strlen(key))
            {
                object[key] = jValue;
            }
            else
            {
                object.push_back(jValue);
            }
        }

        void BeginObject(const char* key, bool isArray)
        {
            json& object = GetObject();
            json newObject;
            if (isArray)
            {
                newObject.array();
                _objectStack.push_back(&InsertValue(key, newObject));
            }
            else
            {
                newObject.object();
                _objectStack.push_back(&InsertValue(key, newObject));
            }
        }

        void EndObject()
        {
            _objectStack.pop_back();
        }

    private:
        Stream& _outStream;
        json _root;
        vector<json*> _objectStack;
        std::unordered_map<const char*, double> c_umap;
    };

    JsonSerializer::JsonSerializer(Stream& outStream)
        : _impl(new JsonSerializerImpl(outStream))
    {
    }

    JsonSerializer::~JsonSerializer()
    {
        SafeDelete(_impl);
    }

    void JsonSerializer::Serialize(const char* key, bool value)
    {
        _impl->SerializePrimitive(key, value);
    }

    void JsonSerializer::Serialize(const char* key, int16_t value)
    {
        _impl->SerializePrimitive(key, value);
    }

    void JsonSerializer::Serialize(const char* key, uint16_t value)
    {
        _impl->SerializePrimitive(key, value);
    }

    void JsonSerializer::Serialize(const char* key, int32_t value)
    {
        _impl->SerializePrimitive(key, value);
    }

    void JsonSerializer::Serialize(const char* key, uint32_t value)
    {
        _impl->SerializePrimitive(key, value);
    }

    void JsonSerializer::Serialize(const char* key, int64_t value)
    {
        _impl->SerializePrimitive(key, value);
    }

    void JsonSerializer::Serialize(const char* key, uint64_t value)
    {
        _impl->SerializePrimitive(key, value);
    }

    void JsonSerializer::Serialize(const char* key, float value)
    {
        _impl->SerializePrimitive(key, value);
    }

    void JsonSerializer::Serialize(const char* key, double value)
    {
        _impl->SerializePrimitive(key, value);
    }

    void JsonSerializer::Serialize(const char* key, char value)
    {
        std::string str;
        str += value;
        _impl->SerializeString(key, str.c_str());
    }

    void JsonSerializer::Serialize(const char* key, const char* value)
    {
        _impl->SerializeString(key, value);
    }

    void JsonSerializer::Serialize(const char* key, const float* values, uint32_t count)
    {
        _impl->Serialize(key, values, count);
    }

    void JsonSerializer::BeginObject(const char* key, bool isArray)
    {
        _impl->BeginObject(key, isArray);
    }

    void JsonSerializer::EndObject()
    {
        _impl->EndObject();
    }
}
