// Copyright (c) Amer Koleci and contributors.
// Licensed under the Apache License, Version 2.0.

using System;
using System.Runtime.InteropServices;
using static Alimer.AlimerApi;

namespace Alimer
{
    [Preserve(AllMembers = true)]
    public class Application : IDisposable
    {
        private static Application _current;
        // references needed to prevent GC from collecting callbacks passed to native code
        private static ActionVoidWithReturnInt setupCallback;
        private static ActionVoid startCallback;
        private static ActionVoid exitCallback;

        private readonly IntPtr _handle;

        public static Application Current
        {
            get
            {
                if (_current == null)
                    throw new InvalidOperationException("The application is not configured yet");

                return _current;
            }
        }

        [Preserve]
        static Application()
        {
            Runtime.Setup();
        }

        [Preserve]
        public Application()
        {
            setupCallback = ProxySetup;
            startCallback = ProxyStart;
            exitCallback = ProxyExit;

            _handle = Application_new(setupCallback, startCallback, exitCallback);
            _current = this;
        }

        public void Dispose()
        {
            _current = null;
        }

        public int Run()
        {
            return Application_Run(_handle);
        }

        protected virtual bool Setup()
        {
            return true;
        }

        protected virtual void Initialize()
        {
        }

        [MonoPInvokeCallback(typeof(ActionVoidWithReturnInt))]
        private static int ProxySetup()
        {
            if (_current.Setup())
                return 1;

            return 0;
        }

        [MonoPInvokeCallback(typeof(ActionVoid))]
        private static void ProxyStart()
        {
            Runtime.Initialize();
            //_current.SubscribeToAppEvents();
            _current.Initialize();
        }

        [MonoPInvokeCallback(typeof(ActionVoid))]
        private static void ProxyExit()
        {
            Runtime.Shutdown();
            _current = null;
        }
    }
}
