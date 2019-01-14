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

#include "../Serialization/JsonSerializer.h"
#include "../Core/Log.h"
#if TODO
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

namespace alimer
{
    class JsonSerializerImpl final
    {
    public:
        JsonSerializerImpl(Stream& stream)
            : _outStream(stream)
        {
            _document.SetObject();
            _objectStack.push_back(&_document);
        }

        ~JsonSerializerImpl()
        {
            ALIMER_ASSERT(_objectStack.size() == 1);

            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            //writer.SetIndent(!indendation.Empty() ? indendation.Front() : '\0', indendation.Length());
            _document.Accept(writer);
            _outStream.Write(buffer.GetString(), buffer.GetSize());
        }

        rapidjson::Value& GetObject()
        {
            return *_objectStack.back();
        }

        rapidjson::Value& InsertValue(const char* key, rapidjson::Value& value)
        {
            assert(_objectStack.size() > 0);

            rapidjson::Value& currentValue = *_objectStack.back();

            if (_objectStack.back()->IsObject())
            {
                rapidjson::Value keyValue;
                keyValue.SetString(rapidjson::StringRef(key), _document.GetAllocator());
                currentValue.AddMember(keyValue, value, _document.GetAllocator());
                return currentValue[key];
            }
            else
            {
                rapidjson::SizeType newIndex = currentValue.Size();
                currentValue.PushBack(value, _document.GetAllocator());
                return currentValue[newIndex];
            }
        }

        template<typename T, typename = typename std::enable_if<std::is_fundamental<T>::value>::type>
        bool SerializePrimitive(const char* key, T value)
        {
            rapidjson::Value& object = GetObject();
            if (key && strlen(key))
            {
                rapidjson::Value keyValue;
                keyValue.SetString(rapidjson::StringRef(key), _document.GetAllocator());

                object.AddMember(keyValue, value, _document.GetAllocator());
            }
            else
            {
                object.PushBack(value, _document.GetAllocator());
            }

            return true;
        }

        void SerializeString(const char* key, const char* value)
        {
            rapidjson::Value& object = GetObject();
            rapidjson::Value stringValue;
            stringValue.SetString(rapidjson::StringRef(value), _document.GetAllocator());

            if (key && strlen(key))
            {
                rapidjson::Value keyValue;
                keyValue.SetString(rapidjson::StringRef(key), _document.GetAllocator());

                object.AddMember(
                    keyValue,
                    stringValue,
                    _document.GetAllocator());
            }
            else
            {
                object.PushBack(stringValue, _document.GetAllocator());
            }
        }

        void Serialize(
            const char* key,
            const float* values,
            uint32_t count)
        {
            auto& object = GetObject();
            rapidjson::Value jValue(rapidjson::kArrayType);

            for (uint32_t i = 0; i < count; ++i)
            {
                jValue.PushBack(values[i], _document.GetAllocator());
            }

            if (key && strlen(key))
            {
                rapidjson::Value keyValue;
                keyValue.SetString(rapidjson::StringRef(key), _document.GetAllocator());

                object.AddMember(keyValue, jValue, _document.GetAllocator());
            }
            else
            {
                object.PushBack(jValue, _document.GetAllocator());
            }
        }

        void BeginObject(const char* key, bool isArray)
        {
            if (isArray)
            {
                rapidjson::Value newObject(rapidjson::kArrayType);
                _objectStack.push_back(&InsertValue(key, newObject));
            }
            else
            {
                rapidjson::Value newObject(rapidjson::kObjectType);
                _objectStack.push_back(&InsertValue(key, newObject));
            }
        }

        void EndObject()
        {
            _objectStack.pop_back();
        }

    private:
        Stream & _outStream;
        rapidjson::Document _document;
        std::vector<rapidjson::Value*> _objectStack;
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

    void JsonSerializer::Serialize(const char* key, const std::string& value)
    {
        _impl->SerializeString(key, value.c_str());
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

#endif // TODO
