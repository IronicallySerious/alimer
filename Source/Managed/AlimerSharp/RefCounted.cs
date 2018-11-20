// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Runtime.InteropServices;
using static Alimer.AlimerApi;

namespace Alimer
{
    public abstract partial class RefCounted : IDisposable
    {
        internal IntPtr _handle;

        public IntPtr Handle => _handle;

        /// <summary>
		/// True if underlying native object is deleted.
		/// </summary>
		public bool IsDeleted { get; private set; }

        protected virtual bool AllowNativeDelete => true;

        [Preserve]
        internal RefCounted(AlimerObjectFlag empty) { }

        [Preserve]
        protected RefCounted(IntPtr handle)
        {
            if (handle == IntPtr.Zero)
                throw new ArgumentException($"Attempted to instantiate a {GetType()} with a null handle");

            _handle = handle;
            Runtime.RegisterObject(this);
        }

        ~RefCounted()
        {
            if (!Runtime.ShuttingDown
                && !IsDeleted)
            {
                var deleteHandle = Handle;
                ref_counted_try_delete(deleteHandle);
                //Application.InvokeOnMain(() => TryDeleteRefCounted(deleteHandle));
            }

            Dispose(false);
        }

        public void Dispose()
        {
            DeleteNativeObject();
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (IsDeleted)
                return;

            if (disposing)
            {
                IsDeleted = true;
                OnDeleted();
            }

            Runtime.UnregisterObject(_handle);
        }

        /// <summary>
		/// Return reference count.
		/// </summary>
		public uint Refs()
        {
            return ref_counted_refs(_handle);
        }

        /// <summary>
		/// Return weak reference count.
		/// </summary>
		public uint WeakRefs()
        {
            return ref_counted_weak_refs(_handle);
        }

        /// <summary>
        /// Called by RefCounted::~RefCounted - we don't need to check Refs here - just mark it as deleted and remove from cache
        /// </summary>
        internal void HandleNativeDelete()
        {
            //LogSharp.Trace($"{GetType().Name}: HandleNativeDelete");
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
		/// Try to delete underlying native object if nobody uses it (Refs==0)
		/// </summary>
		private void DeleteNativeObject()
        {
            if (!IsDeleted && AllowNativeDelete)
            {
                //LogSharp.Trace($"{GetType().Name}: DeleteNativeObject");
                ref_counted_try_delete(_handle);
            }
        }

        protected virtual void OnDeleted() { }

        public override bool Equals(object obj)
        {
            if (obj == null)
                return false;

            if (obj.GetType() != GetType())
                return false;

            var refCounted = obj as RefCounted;
            return refCounted != null && refCounted._handle == _handle;
        }

        public static bool operator ==(RefCounted left, RefCounted right)
        {
            object a = left;
            object b = right;

            if (a == null)
            {
                return b == null;
            }
            else
            {
                if (b == null)
                {
                    return false;
                }

                return left._handle == right._handle;
            }
        }

        public static bool operator !=(RefCounted left, RefCounted right)
        {
            object a = left;
            object b = right;
            if (a == null)
            {
                return b != null;
            }
            else
            {
                if (b == null)
                {
                    return true;
                }

                return left._handle != right._handle;
            }
        }

        public override int GetHashCode()
        {
            if (IntPtr.Size == 8)
            {
                return unchecked((int)(long)_handle);
            }

            return (int)_handle;
        }

        #region Native methods
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public extern static void ref_counted_try_delete(IntPtr handle);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public extern static uint ref_counted_refs(IntPtr handle);

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        public extern static uint ref_counted_weak_refs(IntPtr handle);
        #endregion
    }

    internal enum AlimerObjectFlag
    {
        Empty
    }
}
