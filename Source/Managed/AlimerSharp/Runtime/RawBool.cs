// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Runtime.InteropServices;
using System.Text;

namespace Alimer
{
    /// <summary>
    /// A boolean value stored on 4 bytes (instead of 1 in .NET).
    /// </summary>
    [StructLayout(LayoutKind.Sequential, Size = 4)]
    public struct RawBool : IEquatable<RawBool>
    {
        private readonly int _value;

        /// <summary>
        /// Represents the boolean "true" value. Has a raw value of 1.
        /// </summary>
        public static readonly RawBool True = new RawBool(1);

        /// <summary>
        /// Represents the boolean "true" value. Has a raw value of 0.
        /// </summary>
        public static readonly RawBool False = new RawBool(0);

        /// <summary>
        /// Initializes a new instance of the <see cref="RawBool" /> class.
        /// </summary>
        /// <param name="boolValue">if set to <c>true</c> [bool value].</param>
        public RawBool(bool boolValue)
        {
            _value = boolValue ? 1 : 0;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="RawBool" /> class.
        /// </summary>
        /// <param name="value"></param>
        public RawBool(int value)
        {
            _value = value;
        }

        /// <summary>
        /// Indicates whether this instance and a specified object are equal.
        /// </summary>
        /// <param name="other">The other.</param>
        /// <returns>true if <paramref name="other" /> and this instance are the same type and represent the same value; otherwise, false.</returns>
        public bool Equals(RawBool other) => _value == other._value;

        public override bool Equals(object obj) => obj is RawBool b && Equals(b);

        public override int GetHashCode() => _value.GetHashCode();

        /// <summary>
        /// Implements the ==.
        /// </summary>
        /// <param name="left">The left.</param>
        /// <param name="right">The right.</param>
        /// <returns>The result of the operator.</returns>
        public static bool operator ==(RawBool left, RawBool right) => left._value == right._value;

        /// <summary>
        /// Implements the !=.
        /// </summary>
        /// <param name="left">The left.</param>
        /// <param name="right">The right.</param>
        /// <returns>The result of the operator.</returns>
        public static bool operator !=(RawBool left, RawBool right) => left._value != right._value;

        public static implicit operator bool(RawBool booleanValue) => booleanValue._value != 0;
        public static implicit operator int(RawBool booleanValue) => booleanValue._value;
        public static implicit operator RawBool(bool boolValue) =>new RawBool(boolValue);
        public static implicit operator RawBool(int value) => new RawBool(value);

        public override string ToString()
        {
            return $"{_value != 0}";
        }
    }
}
