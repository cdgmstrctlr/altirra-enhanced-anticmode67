//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2004 Avery Lee, All Rights Reserved.
//
//	Beginning with 1.6.0, the VirtualDub system library is licensed
//	differently than the remainder of VirtualDub.  This particular file is
//	thus licensed as follows (the "zlib" license):
//
//	This software is provided 'as-is', without any express or implied
//	warranty.  In no event will the authors be held liable for any
//	damages arising from the use of this software.
//
//	Permission is granted to anyone to use this software for any purpose,
//	including commercial applications, and to alter it and redistribute it
//	freely, subject to the following restrictions:
//
//	1.	The origin of this software must not be misrepresented; you must
//		not claim that you wrote the original software. If you use this
//		software in a product, an acknowledgment in the product
//		documentation would be appreciated but is not required.
//	2.	Altered source versions must be plainly marked as such, and must
//		not be misrepresented as being the original software.
//	3.	This notice may not be removed or altered from any source
//		distribution.

#ifndef f_VD2_SYSTEM_ATOMIC_H
#define f_VD2_SYSTEM_ATOMIC_H

#include <vd2/system/vdtypes.h>

#ifdef VD_COMPILER_MSVC
	#include <vd2/system/win32/intrin.h>
#endif

inline void *VDAtomicCompareExchangePointer(void *volatile *pp, void *p, void *compare) {
#if defined(VD_COMPILER_MSVC)
	#ifdef _M_AMD64
		return _InterlockedCompareExchangePointer(pp, p, compare);
	#else
		return (void *)(sintptr)_InterlockedCompareExchange((volatile long *)(volatile sintptr *)pp, (long)(sintptr)p, (long)(sintptr)compare);
	#endif
#elif defined(VD_COMPILER_CLANG)
	return __sync_val_compare_and_swap(pp, compare, p);
#else
	#error not implemented
#endif
}

///////////////////////////////////////////////////////////////////////////
/// \class VDAtomicInt
/// \brief Wrapped integer supporting thread-safe atomic operations.
///
/// VDAtomicInt allows integer values shared between threads to be
/// modified with several common operations in a lock-less manner and
/// without the need for explicit barriers. This is particularly useful
/// for thread-safe reference counting.
///
class VDAtomicInt {
protected:
	volatile int n;

public:
	VDAtomicInt() {}
	VDAtomicInt(int v) : n(v) {}

	bool operator!() const { return !n; }
	bool operator!=(int v) const  { return n!=v; }
	bool operator==(int v) const { return n==v; }
	bool operator<=(int v) const { return n<=v; }
	bool operator>=(int v) const { return n>=v; }
	bool operator<(int v) const { return n<v; }
	bool operator>(int v) const { return n>v; }

	///////////////////////////////

	/// Atomically exchanges a value with an integer in memory.
	static inline int staticExchange(volatile int *dst, int v) {
		#if defined(VD_COMPILER_MSVC)
			return (int)_InterlockedExchange((volatile long *)dst, v);
		#elif defined(VD_COMPILER_CLANG)
			return __sync_lock_test_and_set((int *)dst, v);
		#else
			#error not implemented
		#endif
	}

	/// Atomically adds one to an integer in memory.
	static inline void staticIncrement(volatile int *dst) {
		#if defined(VD_COMPILER_MSVC)
			_InterlockedExchangeAdd((volatile long *)dst, 1);
		#elif defined(VD_COMPILER_CLANG)
			__sync_fetch_and_add(dst, 1);
		#else
			#error not implemented
		#endif
	}

	/// Atomically subtracts one from an integer in memory.
	static inline void staticDecrement(volatile int *dst) {
		#if defined(VD_COMPILER_MSVC)
			_InterlockedExchangeAdd((volatile long *)dst, -1);
		#elif defined(VD_COMPILER_CLANG)
			__sync_fetch_and_sub(dst, 1);
		#else
			#error not implemented
		#endif
	}

	/// Atomically subtracts one from an integer in memory and returns
	/// true if the result is zero.
	static inline bool staticDecrementTestZero(volatile int *dst) {
		#if defined(VD_COMPILER_MSVC)
			return 1 == _InterlockedExchangeAdd((volatile long *)dst, -1);
		#elif defined(VD_COMPILER_CLANG)
			return 1 == __sync_fetch_and_sub((int *)dst, 1);
		#else
			#error not implemented
		#endif
	}

	/// Atomically adds a value to an integer in memory and returns the
	/// result.
	static inline int staticAdd(volatile int *dst, int v) {
		#if defined(VD_COMPILER_MSVC)
			return (int)_InterlockedExchangeAdd((volatile long *)dst, v) + v;
		#elif defined(VD_COMPILER_CLANG)
			return __sync_fetch_and_add((int *)dst, v) + v;
		#else
			#error not implemented
		#endif
	}

	/// Atomically adds a value to an integer in memory and returns the
	/// old result (post-add).
	static inline int staticExchangeAdd(volatile int *dst, int v) {
		#if defined(VD_COMPILER_MSVC)
			return _InterlockedExchangeAdd((volatile long *)dst, v);
		#elif defined(VD_COMPILER_CLANG)
			return __sync_fetch_and_add((int *)dst, v);
		#else
			#error not implemented
		#endif
	}

	/// Atomically compares an integer in memory to a compare value and
	/// swaps the memory location with a second value if the compare
	/// succeeds. The return value is the memory value prior to the swap.
	static inline int staticCompareExchange(volatile int *dst, int v, int compare) {
		#if defined(VD_COMPILER_MSVC)
			return _InterlockedCompareExchange((volatile long *)dst, v, compare);
		#elif defined(VD_COMPILER_CLANG)
			return __sync_val_compare_and_swap((int *)dst, compare, v);
		#else
			#error not implemented
		#endif
	}

	///////////////////////////////

	int operator=(int v) { n = v; return v; }

	int operator++()		{ return staticAdd(&n, 1); }
	int operator--()		{ return staticAdd(&n, -1); }
	int operator++(int)		{ return staticExchangeAdd(&n, 1); }
	int operator--(int)		{ return staticExchangeAdd(&n, -1); }
	int operator+=(int v)	{ return staticAdd(&n, v); }
	int operator-=(int v)	{ return staticAdd(&n, -v); }

#if _MSC_VER >= 1310

	void operator&=(int v)	{ _InterlockedAnd((volatile long *)&n, v); }	///< Atomic bitwise AND.
	void operator|=(int v)	{ _InterlockedOr((volatile long *)&n, v); }		///< Atomic bitwise OR.
	void operator^=(int v)	{ _InterlockedXor((volatile long *)&n, v); }	///< Atomic bitwise XOR.

#elif defined(VD_COMPILER_CLANG)

	void operator&=(int v) {
		__sync_fetch_and_and(&n, v);
	}

	void operator|=(int v) {
		__sync_fetch_and_or(&n, v);
	}

	void operator^=(int v) {
		__sync_fetch_and_xor(&n, v);
	}	

#else
	/// Atomic bitwise AND.
	void operator&=(int v) {
		__asm mov eax,v
		__asm mov ecx,this
		__asm lock and dword ptr [ecx],eax
	}

	/// Atomic bitwise OR.
	void operator|=(int v) {
		__asm mov eax,v
		__asm mov ecx,this
		__asm lock or dword ptr [ecx],eax
	}

	/// Atomic bitwise XOR.
	void operator^=(int v) {
		__asm mov eax,v
		__asm mov ecx,this
		__asm lock xor dword ptr [ecx],eax
	}
#endif

	operator int() const {
		return n;
	}

	/// Atomic exchange.
	int xchg(int v) {
		return staticExchange(&n, v);
	}

	/// Compare/exchange (486+).
	int compareExchange(int newValue, int oldValue) {
		return staticCompareExchange(&n, newValue, oldValue);
	}

	// 486 only, but much nicer.  They return the actual result.

	int inc()			{ return operator++(); }				///< Atomic increment.
	int dec()			{ return operator--(); }				///< Atomic decrement.
	int add(int v)		{ return operator+=(v); }				///< Atomic add.

	// These return the result before the operation, which is more inline with
	// what XADD allows us to do.

	int postinc()		{ return operator++(0); }				///< Atomic post-increment.
	int postdec()		{ return operator--(0); }				///< Atomic post-decrement.
	int postadd(int v)	{ return staticExchangeAdd(&n, v); }	///< Atomic post-add.

};

///////////////////////////////////////////////////////////////////////////

class VDAtomicFloat {
protected:
	volatile float n;

public:
	VDAtomicFloat() {}
	VDAtomicFloat(float v) : n(v) {}

	bool operator!=(float v) const  { return n!=v; }
	bool operator==(float v) const { return n==v; }
	bool operator<=(float v) const { return n<=v; }
	bool operator>=(float v) const { return n>=v; }
	bool operator<(float v) const { return n<v; }
	bool operator>(float v) const { return n>v; }

	float operator=(float v) { n = v; return v; }

	operator float() const {
		return n;
	}

	/// Atomic exchange.
	float xchg(float v) {
		union { int i; float f; } converter = {VDAtomicInt::staticExchange((volatile int *)&n, *(const int *)&v)};

		return converter.f;
	}
};

///////////////////////////////////////////////////////////////////////////

class VDAtomicBool {
protected:
	volatile char n;

public:
	VDAtomicBool() {}
	VDAtomicBool(bool v) : n(v) {}

	bool operator!=(bool v) const { return (n != 0) != v; }
	bool operator==(bool v) const { return (n != 0) == v; }

	bool operator=(bool v) { n = v; return v; }

	operator bool() const {
		return n != 0;
	}

	/// Atomic exchange.
	bool xchg(bool v) {
		const uint32 mask = ((uint32)0xFF << (8 * (int)((size_t)&n & 3)));
		const int andval = (int)~mask; 
		const int orval = v ? (int)(mask & 0x01010101) : 0;
		volatile int *p = (volatile int *)((uintptr)&n & ~(uintptr)3);

		for(;;) {
			const uint32 prevval = *p;
			const uint32 newval = (prevval & andval) + orval;

			if (prevval == (uint32)VDAtomicInt::staticCompareExchange(p, newval, prevval))
				return (prevval & mask) != 0;
		}
	}
};

///////////////////////////////////////////////////////////////////////////
/// \class VDAtomicPtr
/// \brief Wrapped pointer supporting thread-safe atomic operations.
///
/// VDAtomicPtr allows a shared pointer to be safely manipulated by
/// multiple threads without locks. Note that atomicity is only guaranteed
/// for the pointer itself, so any operations on the object must be dealt
/// with in other manners, such as an inner lock or other atomic
/// operations. An atomic pointer can serve as a single entry queue.
///
template<typename T>
class VDAtomicPtr {
protected:
	T *volatile ptr;

public:
	VDAtomicPtr() {}
	VDAtomicPtr(T *p) : ptr(p) { }

	operator T*() const { return ptr; }
	T* operator->() const { return ptr; }

	T* operator=(T* p) {
		ptr = p;
		return p;
	}

	/// Atomic pointer exchange.
	T *xchg(T* p) {
		#if defined(VD_COMPILER_MSVC)
			#if VD_PTR_SIZE > 4
				return ptr == p ? p : (T *)_InterlockedExchangePointer((void *volatile *)&ptr, p);
			#else
				return ptr == p ? p : (T *)_InterlockedExchange((volatile long *)&ptr, (long)p);
			#endif
		#elif defined(VD_COMPILER_CLANG)
			return __sync_lock_test_and_set(&ptr, p);
		#endif
	}

	T *compareExchange(T *newValue, T *oldValue) {
		#if defined(VD_COMPILER_MSVC)
			#if VD_PTR_SIZE > 4
				return (T *)_InterlockedCompareExchangePointer((void *volatile *)&ptr, (void *)newValue, (void *)oldValue);
			#else
				return (T *)_InterlockedCompareExchange((volatile long *)&ptr, (long)(size_t)newValue, (long)(size_t)oldValue);
			#endif
		#elif defined(VD_COMPILER_CLANG)
			return __sync_val_compare_and_swap(&ptr, oldValue, newValue);
		#endif
	}
};

#endif
