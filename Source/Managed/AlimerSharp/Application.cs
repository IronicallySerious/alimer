// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Runtime.InteropServices;
using static Alimer.AlimerApi;
using NativePointerDictionary = System.Collections.Concurrent.ConcurrentDictionary<System.IntPtr, Alimer.Application>;

namespace Alimer
{
    [PreserveAttribute(AllMembers = true)]
    public partial class Application
    {
        private static Application s_current;
        private static readonly NativePointerDictionary _managedApps = new NativePointerDictionary();

        // references needed to prevent GC from collecting callbacks passed to native code
        private static readonly ActionIntPtr _initializeCallback;
        private static readonly ActionIntPtr _exitingCallback;

        private readonly IntPtr _handle;

        public static event Action Initialized;

        public static Application Current
        {
            get
            {
                if (s_current == null)
                    throw new InvalidOperationException("The application is not configured yet");
                return s_current;
            }
            private set { s_current = value; }
        }

        [Preserve]
        static Application()
        {
            _initializeCallback = new ActionIntPtr(InitializeInternal);
            _exitingCallback = new ActionIntPtr(ExitingInternal);

            alimer_app_set_delegates(
                Marshal.GetFunctionPointerForDelegate(_initializeCallback),
                Marshal.GetFunctionPointerForDelegate(_exitingCallback)
                );
        }

        [Preserve]
        public Application()
            : this(ApplicationSettings.Default)
        {
        }

        [Preserve]
        public Application(ApplicationSettings options)
        {
            _handle = alimer_app_new();
            _managedApps.TryAdd(_handle, this);
        }

        /// <summary>
		/// Initialize the engine and run the main loop, then return the application exit code. Catch out-of-memory exceptions while running.
		/// </summary>
		public int Run()
        {
            return alimer_app_run(_handle);
        }

        public void RunFrame()
        {
            alimer_app_run_frame(_handle);
        }

        public void Exit()
        {
            alimer_app_run_exit(_handle);
        }

        private void SubscribeToAppEvents()
        {
        }

        protected virtual void Initialize()
        {
        }
        protected virtual void Shutdown()
        {
        }

        [MonoPInvokeCallback(typeof(ActionIntPtr))]
        private static void InitializeInternal(IntPtr handle)
        {
            Runtime.Initialize();
            Current = AsManagedApp(handle);
            Current.SubscribeToAppEvents();

            Current.Initialize();
            Initialized?.Invoke();
        }

        [MonoPInvokeCallback(typeof(ActionIntPtr))]
        private static void ExitingInternal(IntPtr handle)
        {
            var application = AsManagedApp(handle);
            application.Shutdown();
            Runtime.Shutdown();
            Current = null;
        }

        private static Application AsManagedApp(IntPtr ptr)
        {
            if (AsManagedApp(ptr, out var target))
            {
                return target;
            }
            throw new ObjectDisposedException($"{nameof(Application)}: " + ptr);
        }

        private static bool AsManagedApp(IntPtr ptr, out Application target)
        {
            if (_managedApps.TryGetValue(ptr, out target))
            {
                return true;
            }

            target = null;
            return false;
        }

        #region Native methods
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public extern static void alimer_app_set_delegates(IntPtr pInitialize, IntPtr pShutdown);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr alimer_app_new();

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public extern static int alimer_app_run(IntPtr handle);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public extern static void alimer_app_run_frame(IntPtr handle);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public extern static void alimer_app_run_exit(IntPtr handle);
        #endregion
    }
}
