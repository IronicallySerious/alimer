// Copyright (c) Amer Koleci and contributors.
// Licensed under the MIT License.

using System;
using System.Runtime.InteropServices;

namespace Alimer
{
    internal static class Runtime
    {
        private static bool _initialized;
        private static RefCountedCallback _refCallback; // Keep references to native callbacks (protect from GC)

        public static bool IsInitialized => _initialized;

        internal static void Setup()
        {
            if (_refCallback != null)
                return;

            AlimerRegisterRefCountedCallback(_refCallback = OnNativeRefCallback);
        }

        public static void Initialize()
        {
            _initialized = true;
        }

        public static void Shutdown()
        {
            _initialized = false;
        }

        [MonoPInvokeCallback(typeof(RefCountedCallback))]
        static void OnNativeRefCallback(RefCountedCallbackType type, IntPtr target)
        {
        }

        private enum RefCountedCallbackType
        {
            AddRef,
            Delete,
        }

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void RefCountedCallback(RefCountedCallbackType type, IntPtr handle);

        [DllImport(AlimerApi.Library, CallingConvention = CallingConvention.Cdecl)]
        private static extern void AlimerRegisterRefCountedCallback(RefCountedCallback callback);

    }
}
