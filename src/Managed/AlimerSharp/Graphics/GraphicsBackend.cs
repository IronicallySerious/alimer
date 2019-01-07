// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Alimer.Graphics
{
    /// <summary>
    /// Enum describing the Graphics backend.
    /// </summary>
    public enum GraphicsBackend
    {
        /// <summary>
        /// Best device supported for running platform.
        /// </summary>
        Default = 0,

        /// <summary>
        /// Empty/Headless device type.
        /// </summary>
        Empty = 1,

        /// <summary>
        /// Vulkan backend.
        /// </summary>
        Vulkan = 2,

        /// <summary>
        /// Direct3D 11 backend.
        /// </summary>
        D3D11 = 3,

        /// <summary>
        /// Direct3D 12 backend.
        /// </summary>
        D3D12 = 4,

        /// <summary>
        /// Metal backend.
        /// </summary>
        Metal = 5,

        /// <summary>
        /// OpenGL backend.
        /// </summary>
        OpenGL = 6,
    }
}
