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

#include "../Serialization/Serializer.h"

namespace Alimer
{
    class JsonSerializerImpl;

	/// Json Serializer class.
	class ALIMER_API JsonSerializer final : public Serializer
	{
	public:
		/// Constructor.
        JsonSerializer(Stream& outStream);

		/// Destructor.
		~JsonSerializer() override;

        using Serializer::Serialize;

        void Serialize(const char* key, bool value) override;
        void Serialize(const char* key, int16_t value) override;
        void Serialize(const char* key, uint16_t value) override;
        void Serialize(const char* key, int32_t value) override;
        void Serialize(const char* key, uint32_t value) override;
        void Serialize(const char* key, int64_t value) override;
        void Serialize(const char* key, uint64_t value) override;
        void Serialize(const char* key, float value) override;
        void Serialize(const char* key, double value) override;

        void Serialize(const char* key, char value) override;
        void Serialize(const char* key, const char* value) override;
        void Serialize(const char* key, const std::string& value) override;

        void Serialize(const char* key, const float* values, uint32_t count) override;

        void BeginObject(const char* key, bool isArray) override;
        void EndObject() override;

	private:
        JsonSerializerImpl* _impl;
	};
}
