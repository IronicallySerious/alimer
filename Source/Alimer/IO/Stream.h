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

#include "../PlatformDef.h"
#include <string>

namespace Alimer
{
	enum class StreamMode
	{
		ReadOnly,
		WriteOnly,
		ReadWrite
	};

	/// Abstract stream for reading and writing.
	class Stream
	{
	public:
		/// Constructor.
		Stream();

		/// Destructor.
		virtual ~Stream() = default;

		/// Return whether stream supports reading.
		bool CanRead() const {
			return _mode == StreamMode::ReadOnly || _mode == StreamMode::ReadWrite;
		}

		/// Return whether stream supports writing.
		virtual bool CanWrite() const {
			return _mode == StreamMode::WriteOnly || _mode == StreamMode::ReadWrite;
		}

		/// Return whether stream supports seeking.
		virtual bool CanSeek() const = 0;

	protected:
		StreamMode _mode;

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(Stream);
	};
}
