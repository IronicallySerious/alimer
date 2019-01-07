// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using Alimer.Graphics;

namespace Alimer
{
    public sealed class ApplicationSettings
    {
        public GraphicsBackend PreferredGraphicsBackend { get; set; }

        public bool Validation { get; set; }

        public static readonly ApplicationSettings Default = new ApplicationSettings
        {
            PreferredGraphicsBackend = GraphicsBackend.Default,
#if DEBUG
            Validation = true,
#endif
        };
    }
}
