//
// Alimer is based on the Turso3D codebase.
// Copyright (c) 2018-2019 Amer Koleci and contributors.
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

#include "../../Base/AutoPtr.h"
#include "../Buffer.h"

namespace Turso3D
{
    class JSONValue;

    /// GPU buffer for shader constant data.
    class TURSO3D_API ConstantBuffer : public Buffer
    {
    public:
        /// Construct.
        ConstantBuffer();
        /// Destruct.
        ~ConstantBuffer();

        /// Load from JSON data. Return true on success.
        bool LoadJSON(const JSONValue& source);
        /// Save as JSON data.
        void SaveJSON(JSONValue& dest);
        /// Define the constants being used and create the GPU-side buffer. Return true on success.
        bool Define(const Vector<Constant>& srcConstants);
        /// Define the constants being used and create the GPU-side buffer. Return true on success.
        bool Define(size_t numConstants, const Constant* srcConstants);
        /// Set a constant by index. Optionally specify how many elements to update, default all. Return true on success.
        bool SetConstant(size_t index, const void* data, size_t numElements = 0);
        /// Set a constant by name. Optionally specify how many elements to update, default all. Return true on success.
        bool SetConstant(const String& name, const void* data, size_t numElements = 0);
        /// Set a constant by name. Optionally specify how many elements to update, default all. Return true on success.
        bool SetConstant(const char* name, const void* data, size_t numElements = 0);
        /// Apply to the GPU-side buffer if has changes. Can only be used once on an immutable buffer. Return true on success.
        bool Apply();
        /// Set raw data directly to the GPU-side buffer. Optionally copy back to the shadow constants. Return true on success.
        bool SetData(const void* data, bool copyToShadow = false);
        /// Set a constant by index, template version.
        template <class T> bool SetConstant(size_t index, const T& data, size_t numElements = 0) { return SetConstant(index, (const void*)&data, numElements); }
        /// Set a constant by name, template version.
        template <class T> bool SetConstant(const String& name, const T& data, size_t numElements = 0) { return SetConstant(name, (const void*)&data, numElements); }
        /// Set a constant by name, template version.
        template <class T> bool SetConstant(const char* name, const T& data, size_t numElements = 0) { return SetConstant(name, (const void*)&data, numElements); }

        /// Return number of constants.
        size_t NumConstants() const { return constants.Size(); }
        /// Return the constant descriptions.
        const Vector<Constant>& Constants() const { return constants; }
        /// Return the index of a constant, or NPOS if not found.
        size_t FindConstantIndex(const String& name) const;
        /// Return the index of a constant, or NPOS if not found.
        size_t FindConstantIndex(const char* name) const;
        /// Return pointer to the constant value, or null if not found.
        const void* ConstantValue(size_t index, size_t elementIndex = 0) const;
        /// Return pointer to the constant value, or null if not found.
        const void* ConstantValue(const String& name, size_t elementIndex = 0) const;
        /// Return pointer to the constant value, or null if not found.
        const void* ConstantValue(const char* name, size_t elementIndex = 0) const;

        /// Return constant value, template version.
        template <class T> T ConstantValue(size_t index, size_t elementIndex = 0) const
        {
            const void* value = ConstantValue(index, elementIndex);
            return value ? *(reinterpret_cast<const T*>(value)) : T();
        }

        /// Return constant value, template version.
        template <class T> T ConstantValue(const String& name, size_t elementIndex = 0) const
        {
            const void* value = ConstantValue(name, elementIndex);
            return value ? *(reinterpret_cast<const T*>(value)) : T();
        }

        /// Return constant value, template version.
        template <class T> T ConstantValue(const char* name, size_t elementIndex = 0) const
        {
            const void* value = ConstantValue(name, elementIndex);
            return value ? *(reinterpret_cast<const T*>(value)) : T();
        }

        
        /// Return whether buffer has unapplied changes.
        bool IsDirty() const { return dirty; }

        /// Index for "constant not found."
        static const uint32_t NPOS = (uint32_t)-1;

    private:
        /// Constant definitions.
        Vector<Constant> constants;
        /// Dirty flag.
        bool dirty = false;
    };

}
