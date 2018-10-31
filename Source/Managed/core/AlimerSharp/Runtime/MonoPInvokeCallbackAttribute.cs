// Copyright (c) Amer Koleci and contributors.
// Licensed under the MIT License.

using System;

namespace Alimer
{
    [AttributeUsage(AttributeTargets.Method)]
    public sealed class MonoPInvokeCallbackAttribute : Attribute
    {
        public Type Type { get; private set; }

        public MonoPInvokeCallbackAttribute(Type type)
        {
            Type = type;
        }
    }
}
