// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;

namespace Alimer
{
    internal class RefCountedCache
    {
        private readonly Dictionary<IntPtr, ReferenceHolder<RefCounted>> _knownObjects =
            new Dictionary<IntPtr, ReferenceHolder<RefCounted>>(IntPtrEqualityComparer.Instance);

        public void Shutdown()
        {

        }

        public void Add(RefCounted refCounted)
        {
            lock (_knownObjects)
            {
                if (_knownObjects.TryGetValue(refCounted.Handle, out ReferenceHolder<RefCounted> knownObject))
                {
                    var existingObj = knownObject?.Reference;
                    if (existingObj != null
                        && !IsInHierarchy(existingObj.GetType(), refCounted.GetType()))
                    {
                        throw new InvalidOperationException($"Handle '{refCounted.Handle}' is in use by '{existingObj.GetType().Name}' (IsDeleted={existingObj.IsDeleted}). {refCounted.GetType()}");
                    }
                }

                _knownObjects[refCounted.Handle] = new ReferenceHolder<RefCounted>(refCounted, weak: refCounted.Refs() < 1 && !StrongRefByDefault(refCounted));
            }
        }

        public bool Remove(IntPtr ptr)
        {
            lock (_knownObjects)
            {
                return _knownObjects.Remove(ptr);
            }
        }

        private static bool StrongRefByDefault(RefCounted refCounted)
        {
            //if (refCounted is Scene)
            //    return true;
            //if (refCounted is Context)
            //return true;
            return false;
        }

        private static bool IsInHierarchy(Type t1, Type t2)
        {
            if (t1 == t2)
                return true;
            if (t1.GetTypeInfo().IsSubclassOf(t2))
                return true;
            if (t2.GetTypeInfo().IsSubclassOf(t1))
                return true;
            return false;
        }



        #region IntPtrEqualityComparer
        class IntPtrEqualityComparer : IEqualityComparer<IntPtr>
        {
            public static readonly IEqualityComparer<IntPtr> Instance = new IntPtrEqualityComparer();

            public bool Equals(IntPtr x, IntPtr y) => x == y;

            public int GetHashCode(IntPtr obj) => obj.GetHashCode();
        }
        #endregion
    }
}
