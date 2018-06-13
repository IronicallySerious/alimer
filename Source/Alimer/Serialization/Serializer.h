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

#include "../IO/Stream.h"
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"
#include "../Math/Vector4.h"
#include "../Math/Quaternion.h"
#include "../Math/Color.h"

namespace Alimer
{
	/// Serializer class.
	class Serializer
	{
	protected:
		/// Constructor.
		Serializer();

    public:
		/// Destructor.
		virtual ~Serializer() = default;

		/*bool Serialize(const std::string& key, bool& value);
		bool Serialize(const std::string& key, int16_t& value);
		bool Serialize(const std::string& key, uint16_t& value);
		bool Serialize(const std::string& key, int32_t& value);
		bool Serialize(const std::string& key, uint32_t& value);
		bool Serialize(const std::string& key, float& value);
		bool Serialize(const std::string& key, double& value);
		bool Serialize(const std::string& key, Vector2& value);
		bool Serialize(const std::string& key, Vector3& value);
		bool Serialize(const std::string& key, Vector4& value);
		bool Serialize(const std::string& key, Quaternion& value);*/
        void Serialize(const char* key, const Color& value);
        virtual void Serialize(const char* key, const float* values, uint32_t count) = 0;

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(Serializer);
	};
}
