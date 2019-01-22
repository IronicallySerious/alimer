//
// Copyright (c) 2017-2019 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include <foundation/platform.h>
#include <foundation/types.h>

#if !ALIMER_COMPILER_MSVC
#   include <stdatomic.h>
#endif

/**
* Atomically load 32 bit value
* @param src The source value
* @param order The memory synchronization order for this operation. The order parameter cannot be memory_order_release or memory_order_acq_rel
* @return Current value 
*/
static ALIMER_FORCEINLINE int32_t alimerAtomicLoad32(const atomic32_t* src, memory_order order);

/**
* Atomically load 64 bit value
* @param src   Value
* @param order The memory synchronization order for this operation. The order parameter cannot be memory_order_release or memory_order_acq_rel
* @return      Current value 
*/
static ALIMER_FORCEINLINE int64_t alimerAtomicLoad64(const atomic64_t* src, memory_order order);

/**
* Atomically load pointer value
* @param src   Value
* @param order The memory synchronization order for this operation. The order parameter cannot be memory_order_release or memory_order_acq_rel
* @return      Current value 
*/
static ALIMER_FORCEINLINE void* alimerAtomicLoadPtr(const atomicptr_t* src, memory_order order);

/**
* Atomically store 32 bit value
* @param dst   Target
* @param order The memory synchronization order for this operation. The order argument cannot be memory_order_acquire, memory_order_consume, or memory_order_acq_rel.
* @param val   Value to store 
*/
static ALIMER_FORCEINLINE void alimerAtomicStore32(atomic32_t* dst, int32_t val, memory_order order);

/**
* Atomically store 64 bit value
* @param dst   Target
* @param order The memory synchronization order for this operation. The order argument cannot be memory_order_acquire, memory_order_consume, or memory_order_acq_rel.
* @param val   Value to store
*/
static ALIMER_FORCEINLINE void alimerAtomicStore64(atomic64_t* dst, int64_t val, memory_order order);

/**
* Atomically store pointer value
* @param dst   Target
* @param order The memory synchronization order for this operation. The order argument cannot be memory_order_acquire, memory_order_consume, or memory_order_acq_rel.
* @param val   Value to store 
*/
static ALIMER_FORCEINLINE void alimerAtomicStorePtr(atomicptr_t* dst, void* val, memory_order order);

/**
* Atomically add to the value of the 32 bit integer and returns its new value
* @param val   Value to change
* @param add   Value to add
* @param order The memory synchronization order for this operation
return      New value after addition 
*/
static ALIMER_FORCEINLINE int32_t atomic_add32(atomic32_t* val, int32_t add, memory_order order);

/**
* Atomically add to the value of the 64 bit integer and returns its new value
* @param val Value to change
* @param add Value to add
* @param order The memory synchronization order for this operation
* @return New value after addition 
*/
static ALIMER_FORCEINLINE int64_t atomic_add64(atomic64_t* val, int64_t add, memory_order order);

/**
* Atomically increases the value of the 32 bit integer and returns its new value
* @param val   Value to change
* @param order The memory synchronization order for this operation
* @return      New value after addition
*/
static ALIMER_FORCEINLINE int32_t atomic_incr32(atomic32_t* val, memory_order order);

/**
* Atomically increases the value of the 64 bit integer and returns its new value
* @param val Value to change
* @param order The memory synchronization order for this operation
* @return New value after addition
*/
static ALIMER_FORCEINLINE int64_t atomic_incr64(atomic64_t* val, memory_order order);

/**
* Atomically decreases the value of the 32 bit integer and returns its new value
* @param val Value to change
* @param order The memory synchronization order for this operation
* @return New value after addition 
*/
static ALIMER_FORCEINLINE int32_t atomic_decr32(atomic32_t* val, memory_order order);

/**
* Atomically decreases the value of the 64 bit integer and returns its new value
* @param val   Value to change
* @param order The memory synchronization order for this operation
* @return New value after addition 
*/
static ALIMER_FORCEINLINE int64_t atomic_decr64(atomic64_t* val, memory_order order);

/**
* Atomically add to the value of the 32 bit integer and returns its old value
* @param val   Value to change
* @param add   Value to add
* @param order The memory synchronization order for this operation
* @return      Old value before addition 
*/
static ALIMER_FORCEINLINE int32_t atomic_exchange_and_add32(atomic32_t* val, int32_t add, memory_order order);

/**
* Atomically add to the value of the 64 bit integer and returns its old value
* @param val   Value to change
* @param add   Value to add
* @param order The memory synchronization order for this operation
* @return      Old value before addition 
*/
static ALIMER_FORCEINLINE int64_t atomic_exchange_and_add64(atomic64_t* val, int64_t add, memory_order order);

/**
* Atomically compare and swap (CAS). The value in the destination location is compared to
* the reference value, and if equal the new value is stored in the destination location.
* A weak compare-and-exchange operation might fail spuriously. That is, even when the contents
* of memory referred to by expected and object are equal, it might return false.
* @param dst   Value to change
* @param val   Value to set
* @param ref   Reference value
* @param success The memory synchronization order for for the read-modify-write operation if the comparison succeeds
* @param failure The memory synchronization order for the load operation if the comparison fails. This parameter cannot be memory_order_release or memory_order_acq_rel. You cannot specify it with a memory synchronization order stronger than success
\* @return true if operation was successful and new value stored, false if comparison failed and value was unchanged 
*/
static ALIMER_FORCEINLINE bool atomic_cas32(atomic32_t* dst, int32_t val, int32_t ref, memory_order success, memory_order failure);

/**
* Atomically compare and swap (CAS). The value in the destination location is compared to
* the reference value, and if equal the new value is stored in the destination location.
* A weak compare-and-exchange operation might fail spuriously. That is, even when the contents
* of memory referred to by expected and object are equal, it might return false.
* @param dst   Value to change
* @param val   Value to set
* @param ref   Reference value
* @param success The memory synchronization order for for the read-modify-write operation if the comparison succeeds
* @param failure The memory synchronization order for the load operation if the comparison fails. This parameter cannot be memory_order_release or memory_order_acq_rel. You cannot specify it with a memory synchronization order stronger than success
* @return true if operation was successful and new value stored, false if comparison failed and value was unchanged 
*/
static ALIMER_FORCEINLINE bool atomic_cas64(atomic64_t* dst, int64_t val, int64_t ref, memory_order success, memory_order failure);

/**
* Atomically compare and swap (CAS). The value in the destination location is compared to
* the reference value, and if equal the new value is stored in the destination location.
* A weak compare-and-exchange operation might fail spuriously. That is, even when the contents
* of memory referred to by expected and object are equal, it might return false.
* @param dst   Value to change
* @param val   Value to set
* @param ref   Reference value
* @param success The memory synchronization order for for the read-modify-write operation if the comparison succeeds
* @param failure The memory synchronization order for the load operation if the comparison fails. This parameter cannot be memory_order_release or memory_order_acq_rel. You cannot specify it with a memory synchronization order stronger than success
* @return true if operation was successful and new value stored, false if comparison failed and value was unchanged
*/
static ALIMER_FORCEINLINE bool atomic_cas_ptr(atomicptr_t* dst, void* val, void* ref, memory_order success, memory_order failure);

/**
* Signal fence making prior writes made to other memory locations done by a thread on
* the same core doing a release fence visible to the calling thread. Implemented as a compile
* barrier on all supported platforms 
*/
static ALIMER_FORCEINLINE void atomic_signal_fence_acquire(void);

/**
* Signal fence to make prior writes to functions doing an acquire fence in threads on
* the same core. Implemented as a compile barrier on all supported platforms 
*/
static ALIMER_FORCEINLINE void atomic_signal_fence_release(void);

/**
* Signal fence combining acquire and release order as well as providing a single total
* order on all sequentially consistent fences for threads on the same core. Implemented as
* a compile barrier on all supported platforms 
*/
static ALIMER_FORCEINLINE void atomic_signal_fence_sequentially_consistent(void);

/**
* Thread fence making prior writes made to other memory locations done by a thread doing
* a release fence visible to the calling thread. 
*/
static ALIMER_FORCEINLINE void atomic_thread_fence_acquire(void);

/**
* Thread fence making prior writes visible to other threads to do an acquire fence. 
*/
static ALIMER_FORCEINLINE void atomic_thread_fence_release(void);

/**
* Thread fence combining an acquire and release fence as well as enforcing a single
* total order on all sequentially consistent fences. 
*/
static ALIMER_FORCEINLINE void atomic_thread_fence_sequentially_consistent(void);

// Implementations
#if !ALIMER_COMPILER_MSVC && !defined(__STDC_NO_ATOMICS__)

// C11
#   if ALIMER_COMPILER_CLANG
// Really, atomic load operations should be const-able
// C atomic_load_explicit(const volatile A* object, memory_order order);
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wcast-qual"

static ALIMER_FORCEINLINE int32_t alimerAtomicLoad32(const atomic32_t* src, memory_order order) {
    return atomic_load_explicit((atomic32_t*)src, order);
}

static ALIMER_FORCEINLINE int64_t alimerAtomicLoad64(const atomic64_t* src, memory_order order) {
    return atomic_load_explicit((atomic64_t*)src, order);
}

static ALIMER_FORCEINLINE void* alimerAtomicLoadPtr(const atomicptr_t* src, memory_order order) {
    return atomic_load_explicit((atomicptr_t*)src, order);
}

#       pragma clang diagnostic pop
#   else

static ALIMER_FORCEINLINE int32_t alimerAtomicLoad32(const atomic32_t* src, memory_order order) {
    return atomic_load_explicit(src, order);
}

static ALIMER_FORCEINLINE int64_t alimerAtomicLoad64(const atomic64_t* src, memory_order order) {
    return atomic_load_explicit(src, order);
}

static ALIMER_FORCEINLINE void* alimerAtomicLoadPtr(const atomicptr_t* src, memory_order order) {
    return atomic_load_explicit(src, order);
}

#   endif

static ALIMER_FORCEINLINE void alimerAtomicStore32(atomic32_t* dst, int32_t val, memory_order order) {
    atomic_store_explicit(dst, val, order);
}

static ALIMER_FORCEINLINE void alimerAtomicStore64(atomic64_t* dst, int64_t val, memory_order order) {
    atomic_store_explicit(dst, val, order);
}

static ALIMER_FORCEINLINE void alimerAtomicStorePtr(atomicptr_t* dst, void* val, memory_order order) {
    atomic_store_explicit(dst, val, order);
}


#elif ALIMER_COMPILER_MSVC

static ALIMER_FORCEINLINE int32_t alimerAtomicLoad32(const atomic32_t* val, memory_order order) {
    if (order != memory_order_relaxed) {
        _ReadWriteBarrier();
    }
    return val->nonatomic;
}

static ALIMER_FORCEINLINE int64_t alimerAtomicLoad64(const atomic64_t* val, memory_order order) {
    if (order != memory_order_relaxed)
    {
        _ReadWriteBarrier();
    }

#if ALIMER_ARCH_X86
    int64_t result;
    __asm {
        mov esi, val;
        mov ebx, eax;
        mov ecx, edx;
        lock cmpxchg8b[esi];
        mov dword ptr result, eax;
        mov dword ptr result[4], edx;
    }
    return result;
#else
    return val->nonatomic;
#endif
}

static ALIMER_FORCEINLINE void* alimerAtomicLoadPtr(const atomicptr_t* val, memory_order order) {
    if (order != memory_order_relaxed)
    {
        _ReadWriteBarrier();
    }

    return (void*)val->nonatomic;
}

static ALIMER_FORCEINLINE void alimerAtomicStore32(atomic32_t* dst, int32_t val, memory_order order) {
    dst->nonatomic = val;
    if (order >= memory_order_release) {
        _ReadWriteBarrier();
    }
}

static ALIMER_FORCEINLINE void alimerAtomicStore64(atomic64_t* dst, int64_t val, memory_order order) {
#if ALIMER_ARCH_X86
#   pragma warning(disable : 4731)
    __asm {
        push ebx;
        mov esi, dst;
        mov ebx, dword ptr val;
        mov ecx, dword ptr val[4];
    retry:
        cmpxchg8b[esi];
        jne retry;
        pop ebx;
    }
#else
    dst->nonatomic = val;
#endif

    if (order >= memory_order_release) {
        _ReadWriteBarrier();
    }
}

static ALIMER_FORCEINLINE void alimerAtomicStorePtr(atomicptr_t* dst, void* val, memory_order order) {
    dst->nonatomic = val;
    if (order >= memory_order_release) {
        _ReadWriteBarrier();
    }
}

#else // __STDC_NO_ATOMICS__
#  error Atomic operations not implemented
#endif
