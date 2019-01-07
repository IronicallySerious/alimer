// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;

namespace Alimer.Studio
{
    internal static class Program
    {
        [STAThread]
        public static void Main()
        {
            var size = IntPtr.Size;
            var v = Platform.Name;
            // Run editor.
            new Editor().Run();
        }

        class Editor : Application
        {
            protected override void Initialize()
            {
                base.Initialize();
            }

            protected override void Shutdown()
            {
                base.Shutdown();
            }
        }
    }
}
