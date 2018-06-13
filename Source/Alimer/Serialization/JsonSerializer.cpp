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
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

namespace Alimer
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

        rapidjson::Value& GetObject(int index = -1)
        {
            if (index < 0)
                return *_objectStack.back();

            return _objectStack.back()[index];
        }

        void Serialize(
            const char* key,
            const float* values,
            uint32_t count)
        {
            rapidjson::Value& object = GetObject();
            rapidjson::Value jValue(rapidjson::kArrayType);

            for (uint32_t i = 0; i < count; ++i)
            {
                jValue.PushBack(values[i], _document.GetAllocator());
            }

            if (key && strlen(key))
            {
                rapidjson::Value jKey(key, _document.GetAllocator());
                object.AddMember(jKey, jValue, _document.GetAllocator());
            }
            else
            {
                object.PushBack(jValue, _document.GetAllocator());
            }
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

    void JsonSerializer::Serialize(const char* key, const float* values, uint32_t count)
    {
        _impl->Serialize(key, values, count);
    }
}
