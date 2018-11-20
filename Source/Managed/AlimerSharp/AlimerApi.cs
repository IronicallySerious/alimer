// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Runtime.InteropServices;
using voidptr_t = System.IntPtr;
using alimer_app_t = System.IntPtr;
using alimer_ref_counted_t = System.IntPtr;

namespace Alimer
{
    internal static class AlimerApi
    {
#if __TVOS__ && __UNIFIED__
		public const string Library = "@rpath/libAlimer.framework/libAlimer";
#elif __WATCHOS__ && __UNIFIED__
		public const string Library = "@rpath/libAlimer.framework/libAlimer";
#elif __IOS__ && __UNIFIED__
		public const string Library = "@rpath/libAlimer.framework/libAlimer";
#elif __ANDROID__
		public const string Library = "libAlimer.so";
#elif __MACOS__
		public const string Library = "libAlimer.dylib";
#elif __DESKTOP__
		public const string Library = "libAlimer.dll";
#elif WINDOWS_UWP
		public const string Library = "libAlimer.dll";
#elif NET_STANDARD
		public const string Library = "libAlimer";
#else
        public const string Library = "libAlimer";
#endif

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void ActionIntPtr(IntPtr handle);

        public static string GetPlatformName()
        {
            return Runtime.Utf8ToString(alimerGetPlatformName());
        }

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr alimerGetPlatformName();

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public static extern PlatformType alimerGetPlatformType();

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public static extern PlatformFamily alimerGetPlatformFamily();

        
    }
}
