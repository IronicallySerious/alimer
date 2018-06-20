// Copyright (c) Amer Koleci and contributors.
// Licensed under the Apache License, Version 2.0.

using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Alimer.Studio
{
    /// <summary>
    /// Alimer Assets database initialized from given (.alm) project file.
    /// </summary>
    public static class AssetDatabase
    {
        public static readonly string AssetsFolderName = "Assets";

        private static readonly FileSystemWatcher _watcher;

        static AssetDatabase()
        {
            _watcher = new FileSystemWatcher
            {
                IncludeSubdirectories = true,
                EnableRaisingEvents = false
            };
            _watcher.Created += Watcher_FileCreated;
            _watcher.Deleted += Watcher_FileDeleted;
            _watcher.Changed += Watcher_FileChanged;
            _watcher.Renamed += Watcher_Renamed;
        }

        private static void Watcher_FileCreated(object sender, FileSystemEventArgs e)
        {
        }

        private static void Watcher_FileDeleted(object sender, FileSystemEventArgs e)
        {
        }

        private static void Watcher_FileChanged(object sender, FileSystemEventArgs e)
        {
        }

        private static void Watcher_Renamed(object sender, RenamedEventArgs e)
        {
        }
    }
}
