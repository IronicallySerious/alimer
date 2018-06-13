// Copyright (c) Amer Koleci and contributors.
// Licensed under the Apache License, Version 2.0.

using System;
using System.Runtime.InteropServices;

namespace Alimer
{
    internal partial class AlimerApi
    {
        public const string Library = "libAlimerSharp";

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ActionVoidWithReturnInt();


        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void ActionVoid();

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr Application_new(ActionVoidWithReturnInt setup, ActionVoid start, ActionVoid exit);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public extern static int Application_Run(IntPtr handle);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public extern static void Application_Tick(IntPtr handle);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public extern static void Application_Exit(IntPtr handle);
    }
}
