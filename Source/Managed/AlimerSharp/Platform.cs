// Copyright (c) Amer Koleci and contributors.
// Licensed under the Apache License, Version 2.0.

using System;
using System.Runtime.InteropServices;

namespace Alimer
{
    /// <summary>
	/// Class describing the current running platform.
	/// </summary>
	public static class Platform
    {
        /// <summary>
		/// Gets the running platform type.
		/// </summary>
		public static PlatformType PlatformType { get; }

        /// <summary>
        /// Gets the running platform family.
        /// </summary>
        public static PlatformFamily PlatformFamily { get; }

        static Platform()
        {
#if WINDOWS_UWP
			IsUnix = false;
			IsWindows = true;
#elif NET_STANDARD
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                PlatformType = PlatformType.Windows;
                PlatformFamily = PlatformFamily.Desktop;
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                PlatformType = PlatformType.Linux;
                PlatformFamily = PlatformFamily.Desktop;
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                PlatformType = PlatformType.MacOS;
                PlatformFamily = PlatformFamily.Desktop;
            }
#else
            switch (Environment.OSVersion.Platform)
            {
                case PlatformID.Unix:
                    PlatformType = PlatformType.Linux;
                    PlatformFamily = PlatformFamily.Desktop;
                    break;

                case PlatformID.MacOSX:
                    PlatformType = PlatformType.MacOS;
                    PlatformFamily = PlatformFamily.Desktop;
                    break;

                default:
                    PlatformType = PlatformType.Windows;
                    PlatformFamily = PlatformFamily.Desktop;
                    break;
            }
#endif
        }
    }
}
