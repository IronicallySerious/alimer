// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.IO;

namespace Alimer.Studio
{
    public static class Program
    {
        public static void Main()
        {
            using (var application = new StudioApplication())
            {
                application.Run();
            }
        }
    }
}
