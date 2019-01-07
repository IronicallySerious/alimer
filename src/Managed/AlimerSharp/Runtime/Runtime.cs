// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Runtime.InteropServices;
using System.Text;

namespace Alimer
{
    internal static class Runtime
    {
        private static RefCountedCallback _refCallback; // Keep references to native callbacks (protect from GC)
        private static bool s_isInitialized;
        private static bool s_shuttingDown;

        public static bool IsInitialized => s_isInitialized;
        public static bool ShuttingDown => s_shuttingDown;

        public static RefCountedCache RefCountedCache { get; private set; } = new RefCountedCache();

        /// <summary>
        /// Convert a pointer to a Utf8 null-terminated string to a .NET System.String
        /// </summary>
        public static unsafe string Utf8ToString(IntPtr handle)
        {
            if (handle == IntPtr.Zero)
                return string.Empty;

            var ptr = (byte*)handle;
            while (*ptr != 0)
                ptr++;

            var len = ptr - (byte*)handle;
            if (len == 0)
                return string.Empty;

            var bytes = new byte[len];
            Marshal.Copy(handle, bytes, 0, bytes.Length);

            return Encoding.UTF8.GetString(bytes);
        }

        internal static void Setup()
        {
            if (_refCallback != null)
                return;

            AlimerRegisterRefCountedCallback(_refCallback = OnNativeRefCallback);
        }

        public static void Initialize()
        {
            s_shuttingDown = false;
            s_isInitialized = true;
        }

        public static void Shutdown()
        {
            s_shuttingDown = true;
            RefCountedCache.Shutdown();
            GC.Collect();
            GC.WaitForPendingFinalizers();
            GC.Collect();
            s_isInitialized = false;
        }

        public static void RegisterObject(RefCounted refCounted)
        {
            RefCountedCache.Add(refCounted);
        }

        public static void UnregisterObject(IntPtr handle)
        {
            RefCountedCache.Remove(handle);
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
