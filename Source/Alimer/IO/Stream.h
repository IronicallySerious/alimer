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
#include <memory>
#include <string>
#include <vector>

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

		/**
		* Read data from stream.
		*
		* @param dest Destination buffer.
		* @param size Number of bytes to read.
		* @return Number of bytes actually read.
		*/
		virtual size_t Read(void* dest, size_t size) = 0;

		/**
		* Write bytes to the stream.
		*
		* @param data The source data to write.
		* @param size Number of bytes to write.
		*/
		virtual void Write(const void* data, size_t size) = 0;

		/// Read entire file as text.
		std::string ReadAllText();

		/// Read content as vector bytes.
		std::vector<uint8_t> ReadBytes(size_t count = 0);

		/**
		* Get current position in bytes.
		*/
		size_t GetPosition() const { return _position; }

		/**
		* Get the size in bytes of the stream.
		*/
		size_t GetSize() const { return _size; }

		/**
		* Get whether the end of stream has been reached.
		*/
		bool IsEof() const { return _position >= _size; }

	protected:
		StreamMode _mode;
		size_t _position = 0;
		size_t _size = 0;

	private:
		DISALLOW_COPY_MOVE_AND_ASSIGN(Stream);
	};
}
