//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2007 Avery Lee, All Rights Reserved.
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

#ifndef f_VD2_SYSTEM_BITMATH_H
#define f_VD2_SYSTEM_BITMATH_H

#ifdef _MSC_VER
	#pragma once
#endif

#ifndef f_VD2_SYSTEM_VDTYPES_H
	#include <vd2/system/vdtypes.h>
#endif

inline int VDCountBits8(uint8 v) {
	v -= (v >> 1) & 0x55;
	v = ((v & 0xcc) >> 2) + (v & 0x33);
	return (int)((v + (v >> 4)) & 15);
}

int VDCountBits(uint32 v);
int VDFindLowestSetBit(uint32 v);
int VDFindHighestSetBit(uint32 v);
uint32 VDCeilToPow2(uint32 v);

union VDFloatAsInt {
	float f;
	sint32 i;
};

union VDIntAsFloat {
	sint32 i;
	float f;
};

inline sint32 VDGetFloatAsInt(float f) {
	const VDFloatAsInt conv = { f };
	return conv.i;
}

inline float VDGetIntAsFloat(sint32 i) {
	const VDIntAsFloat conv = { i };
	return conv.f;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef VD_COMPILER_MSVC_VC8_OR_LATER
	#include <vd2/system/win32/intrin.h>

	inline int VDFindLowestSetBit(uint32 v) {
		unsigned long index;
		return _BitScanForward(&index, v) ? index : 32;
	}

	inline int VDFindHighestSetBit(uint32 v) {
		unsigned long index;
		return _BitScanReverse(&index, v) ? index : -1;
	}

	inline int VDFindLowestSetBitFast(uint32 v) {
		unsigned long index;
		_BitScanForward(&index, v);
		return index;
	}

	inline int VDFindHighestSetBitFast(uint32 v) {
		unsigned long index;
		_BitScanReverse(&index, v);
		return index;
	}
#else
	#define VDFindLowestSetBitFast	VDFindLowestSetBit
	#define VDFindHighestSetBitFast	VDFindHighestSetBit
#endif

#endif
