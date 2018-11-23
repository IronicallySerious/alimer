//
// Alimer is based on the Turso3D codebase.
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

#include "../Base/Swap.h"
#include "../Base/Iterator.h"

namespace Alimer
{
    /// Vector base class.
    /** Note that to prevent extra memory use due to vtable pointer, %VectorBase intentionally does not declare a virtual destructor
        and therefore %VectorBase pointers should never be used.
    */
	class ALIMER_API VectorBase
	{
	public:
        /// Construct.
        VectorBase() noexcept = default;

        /// Swap with another vector.
        void Swap(VectorBase& rhs)
        {
            Alimer::Swap(_size, rhs._size);
            Alimer::Swap(_capacity, rhs._capacity);
            Alimer::Swap(_buffer, rhs._buffer);
        }

    protected:
        static uint8_t* AllocateBuffer(size_t size);
        static void FreeBuffer(const uint8_t* buffer);

        /// Size of vector.
        uint32_t _size = 0;
        /// Buffer capacity.
        uint32_t _capacity = 0;
        /// Buffer.
        uint8_t* _buffer = nullptr;
	};
}
