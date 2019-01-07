// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using static Alimer.AlimerApi;

namespace Alimer
{
    /// <summary>
    /// Class describing the current running platform.
    /// </summary>
    public static class Platform
    {
        public static string Name { get; }

        /// <summary>
        /// Gets the running platform type.
        /// </summary>
        public static PlatformType PlatformType { get; }

        /// <summary>
        /// Gets the running platform family.
        /// </summary>
        public static PlatformFamily PlatformFamily { get; }

        /// <summary>
		/// Gets the running framework description.
		/// </summary>
		public static string FrameworkDescription { get; }

        /// <summary>
		/// Get the operating system description.
		/// </summary>
		public static string OSDescription { get; }

        static Platform()
        {
            Name = GetPlatformName();
            PlatformType = alimerGetPlatformType();
            PlatformFamily = alimerGetPlatformFamily();
            //FrameworkDescription = RuntimeInformation.FrameworkDescription;
            //OSDescription = RuntimeInformation.OSDescription;
        }
    }
}
