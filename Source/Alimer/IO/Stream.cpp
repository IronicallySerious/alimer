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

#include "../IO/Stream.h"
#include "../Debug/Log.h"

namespace Alimer
{
	Stream::Stream()
		: _position(0)
        , _size(0)
	{
	}

    Stream::Stream(uint64_t sizeInBytes)
        : _position(0)
        , _size(sizeInBytes)
    {
    }

    void Stream::SetName(const String& name)
    {
        _name = name;
    }

    signed char Stream::ReadByte()
    {
        signed char ret;
        Read(&ret, sizeof ret);
        return ret;
    }

    unsigned char Stream::ReadUByte()
    {
        unsigned char ret;
        Read(&ret, sizeof ret);
        return ret;
    }

    unsigned short Stream::ReadUShort()
    {
        unsigned short ret;
        Read(&ret, sizeof ret);
        return ret;
    }

    unsigned Stream::ReadUInt()
    {
        unsigned ret;
        Read(&ret, sizeof ret);
        return ret;
    }

    bool Stream::ReadBool()
    {
        return ReadUByte() != 0;
    }

    float Stream::ReadFloat()
    {
        float ret;
        Read(&ret, sizeof ret);
        return ret;
    }

    double Stream::ReadDouble()
    {
        double ret;
        Read(&ret, sizeof ret);
        return ret;
    }

    uint32_t Stream::ReadVLE()
    {
        uint32_t ret;
        uint8_t byte;

        byte = ReadUByte();
        ret = byte & 0x7f;
        if (byte < 0x80)
            return ret;

        byte = ReadUByte();
        ret |= ((uint32_t)(byte & 0x7f)) << 7;
        if (byte < 0x80)
            return ret;

        byte = ReadUByte();
        ret |= ((uint32_t)(byte & 0x7f)) << 14;
        if (byte < 0x80)
            return ret;

        byte = ReadUByte();
        ret |= ((uint32_t)byte) << 21;
        return ret;
    }

    String Stream::ReadString()
    {
        String ret;

        while (!IsEof())
        {
            char c = ReadByte();
            if (!c)
                break;
            else
                ret += c;
        }

        return ret;
    }

    String Stream::ReadLine()
    {
        String result;

        while (!IsEof())
        {
            char c = ReadByte();
            if (c == 10)
                break;
            if (c == 13)
            {
                // Peek next char to see if it's 10, and skip it too
                if (!IsEof())
                {
                    char next = ReadByte();
                    if (next != 10)
                    {
                        Seek(_position - 1, SeekOrigin::Begin);
                    }
                }
                break;
            }

            result += c;
        }

        return result;
    }

    String Stream::ReadFileID()
    {
        String ret;
        ret.Resize(4);
        Read(&ret[0], 4);
        return ret;
    }

    StringHash Stream::ReadStringHash()
    {
        return StringHash(ReadUInt());
    }


	String Stream::ReadAllText()
	{
		String content;
		uint64_t length = _size;
		if (length)
		{
			content.Resize(static_cast<uint32_t>(length));
			Read(&content[0], length);
		}

		return content;
	}

	Vector<uint8_t> Stream::ReadBytes(uint64_t count)
	{
		if (!count)
			count = _size;

		Vector<uint8_t> result(static_cast<uint32_t>(count));

        uint64_t read = Read(result.Data(), count);
		if (read != count)
		{
            ALIMER_LOGERRORF("IO", "Failed to read complete contents of stream (amount read vs. file size: %u < %u).",
                static_cast<uint32_t>(read),
                static_cast<uint32_t>(count)
            );
			return {};
		}

		return result;
	}

    void Stream::WriteByte(signed char value)
    {
        Write(&value, sizeof value);
    }
    
    void Stream::WriteUByte(unsigned char value)
    {
        Write(&value, sizeof value);
    }

    void Stream::WriteUShort(unsigned short value)
    {
        Write(&value, sizeof value);
    }

    void Stream::WriteUInt(unsigned value)
    {
        Write(&value, sizeof value);
    }

    void Stream::WriteVLE(unsigned value)
    {
        uint8_t data[4];

        if (value < 0x80)
        {
            return WriteUByte((uint8_t)value);
        }
        else if (value < 0x4000)
        {
            data[0] = (uint8_t)(value | 0x80u);
            data[1] = (uint8_t)(value >> 7u);
            Write(data, 2);
        }
        else if (value < 0x200000)
        {
            data[0] = (uint8_t)(value | 0x80u);
            data[1] = (uint8_t)(value >> 7u | 0x80u);
            data[2] = (uint8_t)(value >> 14u);
            Write(data, 3);
        }
        else
        {
            data[0] = (uint8_t)(value | 0x80u);
            data[1] = (uint8_t)(value >> 7u | 0x80u);
            data[2] = (uint8_t)(value >> 14u | 0x80u);
            data[3] = (uint8_t)(value >> 21u);
            Write(data, 4);
        }
    }

    void Stream::WriteString(const String& value)
    {
        const char* chars = value.CString();
        // Count length to the first zero, because ReadString() does the same
        unsigned length = String::CStringLength(chars);
        Write(chars, length + 1);
    }

    void Stream::WriteFileID(const String& value)
    {
        unsigned length = Min(value.Length(), 4U);

        Write(value.CString(), length);
        for (unsigned i = value.Length(); i < 4; ++i)
        {
            WriteByte(' ');
        }
    }

    void Stream::WriteStringHash(const StringHash& value)
    {
        return WriteUInt(value.Value());
    }

    void Stream::WriteLine(const String& value)
    {
        Write(value.CString(), value.Length());
        WriteUByte('\r');
        WriteUByte('\n');
    }
}
