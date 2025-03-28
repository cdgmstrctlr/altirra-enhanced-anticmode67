//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2014 Avery Lee, All Rights Reserved.
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

#include <stdafx.h>
#include <algorithm>
#include <vd2/system/function.h>

vdfuncbase::vdfuncbase(const vdfuncbase& src)
	: mpFn(src.mpFn)
	, mData(src.mData)
	, mpTraits(src.mpTraits)
{
	if (mpTraits)
		mpTraits->mpCopy(mData.p, src.mData.p);
}

vdfuncbase& vdfuncbase::operator=(const vdfuncbase& src) {
	if (&src != this) {
		clear();

		mData = src.mData;
		if (src.mpTraits)
			src.mpTraits->mpCopy(mData.p, src.mData.p);

		mpFn = src.mpFn;
		mpTraits = src.mpTraits;
	}

	return *this;
}

vdnothrow vdfuncbase& vdfuncbase::operator=(vdfuncbase&& src) vdnoexcept {
	clear();

	mpFn = src.mpFn;
	mData = src.mData;

	if (src.mpTraits && src.mpTraits->mpMove)
		src.mpTraits->mpMove(mData.p, &src.mData.p);

	// must copy instead of moving traits
	mpTraits = src.mpTraits;

	src.mpFn = nullptr;
	src.mpTraits = nullptr;

	return *this;
}

void vdfuncbase::swap(vdfuncbase& other) {
	std::swap(mpFn, other.mpFn);

	Data tmp = mData;
	if (mpTraits && mpTraits->mpMove)
		mpTraits->mpMove(tmp.p, mData.p);
		
	mData = other.mData;
	if (other.mpTraits && other.mpTraits->mpMove)
		other.mpTraits->mpMove(mData.p, other.mData.p);

	other.mData = tmp;
	if (mpTraits && mpTraits->mpMove)
		mpTraits->mpMove(other.mData.p, tmp.p);

	std::swap(mpTraits, other.mpTraits);
}

void vdfuncbase::clear() {
	if (mpTraits)
		mpTraits->mpDestroy(mData.p);

	mpFn = nullptr;
	mpTraits = nullptr;
}
