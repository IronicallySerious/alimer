// Copyright (c) Amer Koleci and contributors.
// Licensed under the Apache License, Version 2.0.

using System;
using System.Runtime.InteropServices;

namespace Alimer.Studio
{
    internal static class Program
    {
        public static void Main(string[] args)
        {
            using (var application = new Editor())
            {
                application.Run();
            }
        }
    }
}
