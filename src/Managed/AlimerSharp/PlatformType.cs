// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Alimer
{
    /// <summary>
    /// Identifiers the running platform type.
    /// </summary>
    public enum PlatformType
    {
        /// <summary>
        /// Unknown platform.
        /// </summary>
        Unknown = 0,

        /// <summary>
        /// Windows platform.
        /// </summary>
        Windows = 1,

        /// <summary>
		/// Universal Windows platform.
		/// </summary>
		UWP = 2,

        /// <summary>
        /// Xbox One platform.
        /// </summary>
        XboxOne = 3,

        /// <summary>
        /// Linux platform.
        /// </summary>
        Linux = 4,

        /// <summary>
        /// macOS (OSX) platform.
        /// </summary>
        macOS = 5,

        /// <summary>
        /// Android platform.
        /// </summary>
        Android = 6,

        /// <summary>
        /// Apple iOS platform.
        /// </summary>
        iOS = 7,

        /// <summary>
        /// Apple TV platform.
        /// </summary>
        AppleTV = 8,

        /// <summary>
        /// Web (Emscripten/WASM) platform.
        /// </summary>
        Web = 9,
    }
}
