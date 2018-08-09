// Copyright (c) Amer Koleci and contributors.
// Licensed under the MIT License.

using System;
using System.Runtime.InteropServices;

namespace Alimer
{
    internal static partial class AlimerApi
    {
        public const string Library = "libAlimerSharp";

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void ActionVoid();

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr Application_new(ActionVoid start, ActionVoid exit);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public extern static int Application_Run(IntPtr handle);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public extern static void Application_RunFrame(IntPtr handle);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public extern static void Application_Exit(IntPtr handle);
    }
}
