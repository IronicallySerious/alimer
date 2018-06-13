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
