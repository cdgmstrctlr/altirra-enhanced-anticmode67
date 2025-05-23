//	Altirra - Atari 800/800XL/5200 emulator
//	PAL artifacting - scalar
//	Copyright (C) 2009-2011 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <stdafx.h>

void ATArtifactPALLuma(uint32 *dst, const uint8 *src, uint32 n, const uint32 *kernels) {
	uint32 x0 = 0x40004000;
	uint32 x1 = 0x40004000;
	uint32 x2 = 0x40004000;
	const uint32 *f;

	do {
		f = kernels + 32*(*src++);
		*dst++ = x0 + f[0];
		x0 = x1 + f[1];
		x1 = x2 + f[2];
		x2 = f[3];

		if (!--n)
			break;

		f = kernels + 32*(*src++) + 4;
		*dst++ = x0 + f[0];
		x0 = x1 + f[1];
		x1 = x2 + f[2];
		x2 = f[3];

		if (!--n)
			break;

		f = kernels + 32*(*src++) + 8;
		*dst++ = x0 + f[0];
		x0 = x1 + f[1];
		x1 = x2 + f[2];
		x2 = f[3];

		if (!--n)
			break;

		f = kernels + 32*(*src++) + 12;
		*dst++ = x0 + f[0];
		x0 = x1 + f[1];
		x1 = x2 + f[2];
		x2 = f[3];

		if (!--n)
			break;

		f = kernels + 32*(*src++) + 16;
		*dst++ = x0 + f[0];
		x0 = x1 + f[1];
		x1 = x2 + f[2];
		x2 = f[3];

		if (!--n)
			break;

		f = kernels + 32*(*src++) + 20;
		*dst++ = x0 + f[0];
		x0 = x1 + f[1];
		x1 = x2 + f[2];
		x2 = f[3];

		if (!--n)
			break;

		f = kernels + 32*(*src++) + 24;
		*dst++ = x0 + f[0];
		x0 = x1 + f[1];
		x1 = x2 + f[2];
		x2 = f[3];

		if (!--n)
			break;

		f = kernels + 32*(*src++) + 28;
		*dst++ = x0 + f[0];
		x0 = x1 + f[1];
		x1 = x2 + f[2];
		x2 = f[3];

	} while(--n);

	*dst++ = x0;
	*dst++ = x1;
	*dst++ = x2;
}

void ATArtifactPALChroma(uint32 *dst, const uint8 *src, uint32 n, const uint32 *kernels) {
	ptrdiff_t koffset = 0;

	do {
		const uint32 *f = kernels + 96*(*src++) + koffset;
		dst[0] += f[0];
		dst[1] += f[1];
		dst[2] += f[2];
		dst[3] += f[3];
		dst[4] += f[4];
		dst[5] += f[5];
		dst[6] += f[6];
		dst[7] += f[7];
		dst[8] += f[8];
		dst[9] += f[9];
		dst[10] += f[10];
		dst[11] += f[11];
		++dst;

		koffset += 12;

		if (koffset == 12*8)
			koffset = 0;
	} while(--n);
}

void ATArtifactPALFinal(uint32 *dst, const uint32 *ybuf, const uint32 *ubuf, const uint32 *vbuf, uint32 *ulbuf, uint32 *vlbuf, uint32 n) {
	const sint32 coug_coub = -3182;		// -co_ug / co_ub * 16384
	const sint32 covg_covr = -8346;		// -co_vg / co_vr * 16384

	for(uint32 i=0; i<n; ++i) {
		const uint32 y = ybuf[i];
		uint32 u = ubuf[i + 4];
		uint32 v = vbuf[i + 4];

		const uint32 up = ulbuf[i + 4];
		const uint32 vp = vlbuf[i + 4];

		ulbuf[i + 4] = u;
		vlbuf[i + 4] = v;

		u += up;
		v += vp;

		sint32 y1 = y & 0xffff;
		sint32 u1 = u & 0xffff;
		sint32 v1 = v & 0xffff;
		sint32 y2 = y >> 16;
		sint32 u2 = u >> 16;
		sint32 v2 = v >> 16;

		sint32 r1 = (y1 + v1 - 0x8020) >> 6;
		sint32 g1 = ((y1 << 14) + u1*coug_coub + v1*covg_covr + 0x80000 - 0x10000000 - 0x4000*(coug_coub + covg_covr)) >> 20;
		sint32 b1 = (y1 + u1 - 0x8020) >> 6;

		sint32 r2 = (y2 + v2 - 0x8020) >> 6;
		sint32 g2 = ((y2 << 14) + u2*coug_coub + v2*covg_covr + 0x80000 - 0x10000000 - 0x4000*(coug_coub + covg_covr)) >> 20;
		sint32 b2 = (y2 + u2 - 0x8020) >> 6;

		if (r1 < 0) r1 = 0; else if (r1 > 255) r1 = 255;
		if (g1 < 0) g1 = 0; else if (g1 > 255) g1 = 255;
		if (b1 < 0) b1 = 0; else if (b1 > 255) b1 = 255;

		if (r2 < 0) r2 = 0; else if (r2 > 255) r2 = 255;
		if (g2 < 0) g2 = 0; else if (g2 > 255) g2 = 255;
		if (b2 < 0) b2 = 0; else if (b2 > 255) b2 = 255;

		dst[0] = (r1 << 16) + (g1 << 8) + b1;
		dst[1] = (r2 << 16) + (g2 << 8) + b2;
		dst += 2;
	}
}

void ATArtifactPALFinalMono(uint32 *dst, const uint32 *ybuf, uint32 n, const uint32 *monoTable) {
	for(uint32 i=0; i<n; ++i) {
		const uint32 y = ybuf[i];

		sint32 y1 = (y & 0xffff) - 0x4000;
		sint32 y2 = (y >> 16) - 0x4000;

		if (y1 < 0) y1 = 0; else if (y1 > 0x3FFF) y1 = 0x3FFF;
		if (y2 < 0) y2 = 0; else if (y2 > 0x3FFF) y2 = 0x3FFF;

		dst[0] = monoTable[(uint32)y1 >> 6];
		dst[1] = monoTable[(uint32)y2 >> 6];
		dst += 2;
	}
}

template<bool T_CompressExtendedRange>
void ATArtifactPAL32(void *dst, void *delayLine, uint32 n) {
	// For this path, we assume that the alpha channel holds precomputed luminance. This works because
	// the only source of raw RGB32 input is VBXE, and though it outputs 21-bit RGB, it can only do so
	// from 4 x 256 palettes. All we need to do is average the YRGB pixels between the delay line and
	// the current line, and then recorrect the luminance back.

	uint8 *VDRESTRICT dst8 = (uint8 *)dst;
	uint8 *VDRESTRICT delay8 = (uint8 *)delayLine;

	for(uint32 i=0; i<n; ++i) {
		int b1 = delay8[0];
		int g1 = delay8[1];
		int r1 = delay8[2];
		int y1 = delay8[3];

		int b2 = dst8[0];
		int g2 = dst8[1];
		int r2 = dst8[2];
		int y2 = dst8[3];
		delay8[0] = b2;
		delay8[1] = g2;
		delay8[2] = r2;
		delay8[3] = y2;
		delay8 += 4;

		int adj = y2 - y1;
		int rf = ((r1 + r2 + adj + 1) >> 1);
		int gf = ((g1 + g2 + adj + 1) >> 1);
		int bf = ((b1 + b2 + adj + 1) >> 1);

		if (T_CompressExtendedRange) {
			rf = rf + rf - 128;
			gf = gf + gf - 128;
			bf = bf + bf - 128;
		}

		if ((unsigned)rf >= 256)
			rf = (~rf >> 31);

		if ((unsigned)gf >= 256)
			gf = (~gf >> 31);

		if ((unsigned)bf >= 256)
			bf = (~bf >> 31);

		dst8[0] = (uint8)bf;
		dst8[1] = (uint8)gf;
		dst8[2] = (uint8)rf;
		dst8 += 4;
	}
}

void ATArtifactPAL32(void *dst, void *delayLine, uint32 n, bool compressExtendedRange) {
	if (compressExtendedRange)
		ATArtifactPAL32<true>(dst, delayLine, n);
	else
		ATArtifactPAL32<false>(dst, delayLine, n);
}
