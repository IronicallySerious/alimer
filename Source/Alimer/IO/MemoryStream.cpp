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

#include "../IO/MemoryStream.h"
#include "../Debug/Log.h"

namespace Alimer
{
    MemoryStream::MemoryStream(void* data, size_t sizeInBytes)
        : Stream(data ? sizeInBytes : 0)
        , _buffer(static_cast<uint8_t*>(data))
        , _readOnly(false)
	{
        SetName("Memory");
	}

    MemoryStream::MemoryStream(const void* data, size_t sizeInBytes)
        : Stream(data ? sizeInBytes : 0)
        , _buffer((unsigned char*)data)
        , _readOnly(true)
    {
        SetName("Memory");
    }

    MemoryStream::MemoryStream(std::vector<uint8_t>& data)
        : Stream(data.size())
        , _buffer(data.data())
        , _readOnly(false)
    {
        SetName("Memory");
    }

    MemoryStream::MemoryStream(const std::vector<uint8_t>& data)
        : Stream(data.size())
        , _buffer(const_cast<uint8_t*>(data.data()))
        , _readOnly(true)
    {
        SetName("Memory");
    }

    bool MemoryStream::CanRead() const
    {
        return _buffer != nullptr;
    }

    bool MemoryStream::CanWrite() const
    {
        return _buffer != nullptr && !_readOnly;
    }

    bool MemoryStream::CanSeek() const
    {
        return _buffer != nullptr;
    }

    size_t MemoryStream::Read(void* dest, size_t size)
    {
        if (size + _position > _size)
        {
            size = _size - _position;
        }

        if (!size)
            return 0;

        uint8_t* srcPtr = &_buffer[_position];
        uint8_t* destPtr = (uint8_t*)dest;
        _position += size;

        size_t copySize = size;
        while (copySize >= sizeof(unsigned))
        {
            *((unsigned*)destPtr) = *((unsigned*)srcPtr);
            srcPtr += sizeof(unsigned);
            destPtr += sizeof(unsigned);
            copySize -= sizeof(unsigned);
        }
        if (copySize & sizeof(unsigned short))
        {
            *((unsigned short*)destPtr) = *((unsigned short*)srcPtr);
            srcPtr += sizeof(unsigned short);
            destPtr += sizeof(unsigned short);
        }
        if (copySize & 1)
            *destPtr = *srcPtr;

        return size;
    }

    size_t MemoryStream::Write(const void* data, size_t size)
    {
        if (size + _position > _size)
        {
            size = _size - _position;
        }

        if (!size || _readOnly)
            return 0;

        uint8_t* srcPtr = (uint8_t*)data;
        uint8_t* destPtr = &_buffer[_position];
        _position += size;

        size_t copySize = size;
        while (copySize >= sizeof(unsigned))
        {
            *((unsigned*)destPtr) = *((unsigned*)srcPtr);
            srcPtr += sizeof(unsigned);
            destPtr += sizeof(unsigned);
            copySize -= sizeof(unsigned);
        }
        if (copySize & sizeof(unsigned short))
        {
            *((unsigned short*)destPtr) = *((unsigned short*)srcPtr);
            srcPtr += sizeof(unsigned short);
            destPtr += sizeof(unsigned short);
        }
        if (copySize & 1)
            *destPtr = *srcPtr;

        return size;
    }
}
