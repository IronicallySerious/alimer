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

#include "../Serialization/Serializer.h"
#include "../Core/Log.h"
#include <vector>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

namespace Alimer
{
    class SerializerImpl
    {
    public:
        virtual ~SerializerImpl() {}
        virtual bool Serialize(const char* key, bool& value) = 0;
        virtual bool Serialize(const char* key, int16_t& value) = 0;
        virtual bool Serialize(const char* key, uint16_t& value) = 0;
        virtual bool Serialize(const char* key, int32_t& value) = 0;
        virtual bool Serialize(const char* key, uint32_t& value) = 0;
        virtual bool Serialize(const char* key, float& value) = 0;
        virtual bool Serialize(const char* key, double& value) = 0;
        virtual bool Serialize(const char* key, Vector2& value) = 0;
        virtual bool Serialize(const char* key, Vector3& value) = 0;
        virtual bool Serialize(const char* key, Vector4& value) = 0;
        virtual bool Serialize(const char* key, Quaternion& value) = 0;
        virtual bool Serialize(const char* key, Color& value) = 0;
    };

    class SerializerImplReadJson : public SerializerImpl
    {
    public:
        SerializerImplReadJson(Stream& stream)
            : _inStream(stream)
        {
            std::vector<char> json;
            json.resize(stream.GetSize());
            stream.Read(json.data(), json.size());

            if (_document.Parse<0>(json.data(), json.size()).HasParseError())
            {
                ALIMER_LOGERROR("Could not parse JSON data from string with error: %s", _document.GetParseError());
                return;
            }

            _objectStack.push_back(&_document);
        }

        ~SerializerImplReadJson()
        {
            ALIMER_ASSERT(_objectStack.size() == 1);
        }

        rapidjson::Value& GetObject() { return *_objectStack.back(); }

        rapidjson::Value& GetObjectWithKey(const char* key)
        {
            if (key)
            {
                return GetObject()[key];
            }
            else
            {
                uint32_t index = _vectorStack.back()++;
                return GetObject()[index];
            }
        }

        bool Serialize(const char* key, bool& value) override
        {
            auto& object = GetObjectWithKey(key);
            if (object.IsBool())
            {
                value = object.GetBool();
                return true;
            }
            return false;
        }

        bool Serialize(const char* key, int16_t& value) override
        {
            auto& object = GetObjectWithKey(key);
            if (object.IsInt())
            {
                value = static_cast<int16_t>(object.GetInt());
                return true;
            }

            return false;
        }

        bool Serialize(const char* key, uint16_t& value) override
        {
            auto& object = GetObjectWithKey(key);
            if (object.IsUint())
            {
                value = static_cast<uint16_t>(object.GetUint());
                return true;
            }

            return false;
        }

        bool Serialize(const char* key, int32_t& value)  override
        {
            auto& object = GetObjectWithKey(key);
            if (object.IsInt())
            {
                value = object.GetInt();
                return true;
            }

            return false;
        }

        bool Serialize(const char* key, uint32_t& value) override
        {
            auto& object = GetObjectWithKey(key);
            if (object.IsUint())
            {
                value = object.GetUint();
                return true;
            }

            return false;
        }

        bool Serialize(const char* key, float& value) override
        {
            auto& object = GetObjectWithKey(key);
            if (object.IsFloat())
            {
                value = object.GetFloat();
                return true;
            }

            return false;
        }

        bool Serialize(const char* key, double& value)  override
        {
            auto& object = GetObjectWithKey(key);
            if (object.IsDouble())
            {
                value = object.GetDouble();
                return true;
            }

            return false;
        }

        bool Serialize(const char* key, Vector2& value) override
        {
            auto& object = GetObjectWithKey(key);
            if (object.IsArray())
            {
                auto jArray = object.GetArray();
                value.Set(jArray[0].GetFloat(), jArray[1].GetFloat());
                return true;
            }

            return false;
        }

        bool Serialize(const char* key, Vector3& value) override
        {
            auto& object = GetObjectWithKey(key);
            if (object.IsArray())
            {
                auto jArray = object.GetArray();
                value.Set(jArray[0].GetFloat(), jArray[1].GetFloat(), jArray[2].GetFloat());
                return true;
            }

            return false;
        }

        bool Serialize(const char* key, Vector4& value) override
        {
            auto& object = GetObjectWithKey(key);
            if (object.IsArray())
            {
                auto jArray = object.GetArray();
                value.Set(jArray[0].GetFloat(), jArray[1].GetFloat(), jArray[2].GetFloat(), jArray[3].GetFloat());
                return true;
            }

            return false;
        }

        bool Serialize(const char* key, Quaternion& value) override
        {
            auto& object = GetObjectWithKey(key);
            if (object.IsArray())
            {
                auto jArray = object.GetArray();
                value.Set(jArray[0].GetFloat(), jArray[1].GetFloat(), jArray[2].GetFloat(), jArray[3].GetFloat());
                return true;
            }

            return false;
        }

        bool Serialize(const char* key, Color& value) override
        {
            auto& object = GetObjectWithKey(key);
            if (object.IsArray())
            {
                auto jArray = object.GetArray();
                value = Color(jArray[0].GetFloat(), jArray[1].GetFloat(), jArray[2].GetFloat(), jArray[3].GetFloat());
                return true;
            }

            return false;
        }

    private:
        Stream & _inStream;
        rapidjson::Document _document;
        std::vector<rapidjson::Value*> _objectStack;
        std::vector<uint32_t> _vectorStack;
    };

    Serializer::Serializer()
    {

    }

    void Serializer::Serialize(const char* key, const std::string& value)
    {
        Serialize(key, value.c_str());
    }

    void Serializer::Serialize(const char* key, Vector2& value)
    {
        Serialize(key, &value.x, 2);
    }

    void Serializer::Serialize(const char* key, Vector3& value)
    {
        Serialize(key, &value.x, 3);
    }

    void Serializer::Serialize(const char* key, Vector4& value)
    {
        Serialize(key, &value.x, 4);
    }

    void Serializer::Serialize(const char* key, Quaternion& value)
    {
        Serialize(key, &value.x, 4);
    }

    void Serializer::Serialize(const char* key, const Color& value)
    {
        Serialize(key, &value.r, 4);
    }
}
