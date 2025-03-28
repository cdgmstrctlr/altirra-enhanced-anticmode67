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

#ifndef f_VD2_SYSTEM_FRACTION_H
#define f_VD2_SYSTEM_FRACTION_H

#include <vd2/system/vdtypes.h>

class VDFraction {
	friend VDFraction operator*(uint32 b, const VDFraction f);
private:
	uint32	hi, lo;

	static VDFraction reduce(uint64 hi, uint64 lo);

public:
	VDFraction() = default;
	explicit constexpr VDFraction(uint32 i) : hi(i), lo(1) { }
	explicit constexpr VDFraction(uint32 i, uint32 j) : hi(i), lo(j) {}

	explicit VDFraction(double d);

	bool	operator<(VDFraction b) const;
	bool	operator<=(VDFraction b) const;
	bool	operator>(VDFraction b) const;
	bool	operator>=(VDFraction b) const;
	bool	operator==(VDFraction b) const;
	bool	operator!=(VDFraction b) const;

	VDFraction operator*(VDFraction b) const;
	VDFraction operator/(VDFraction b) const;

	VDFraction operator*(uint32 b) const;
	VDFraction operator/(uint32 b) const;

	VDFraction& operator*=(VDFraction b);
	VDFraction& operator/=(VDFraction b);
	VDFraction& operator*=(uint32 b);
	VDFraction& operator/=(uint32 b);

	void	Assign(uint32 n, uint32 d) {
		hi = n;
		lo = d;
	}

	sint64 scale64t(sint64) const;
	sint64 scale64r(sint64) const;
	sint64 scale64u(sint64) const;
	sint64 scale64it(sint64) const;
	sint64 scale64ir(sint64) const;
	sint64 scale64iu(sint64) const;

	constexpr double asDouble() const {
		return (double)hi / (double)lo;
	}

	constexpr double AsInverseDouble() const {
		return (double)lo / (double)hi;
	}

	uint32 roundup32ul() const;

	uint32 getHi() const { return hi; }
	uint32 getLo() const { return lo; }

	VDFraction reduce() const { return reduce(hi, lo); }

	bool Parse(const char *s);

	static inline VDFraction reduce64(sint64 hi, sint64 lo) { return reduce(hi, lo); }
};

inline VDFraction operator*(uint32 b, const VDFraction f) { return f*b; }

typedef VDFraction Fraction;

#endif
