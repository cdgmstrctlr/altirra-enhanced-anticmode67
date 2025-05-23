;	VirtualDub - Video processing and capture application
;	System library component
;	Copyright (C) 1998-2004 Avery Lee, All Rights Reserved.
;
;	Beginning with 1.6.0, the VirtualDub system library is licensed
;	differently than the remainder of VirtualDub.  This particular file is
;	thus licensed as follows (the "zlib" license):
;
;	This software is provided 'as-is', without any express or implied
;	warranty.  In no event will the authors be held liable for any
;	damages arising from the use of this software.
;
;	Permission is granted to anyone to use this software for any purpose,
;	including commercial applications, and to alter it and redistribute it
;	freely, subject to the following restrictions:
;
;	1.	The origin of this software must not be misrepresented; you must
;		not claim that you wrote the original software. If you use this
;		software in a product, an acknowledgment in the product
;		documentation would be appreciated but is not required.
;	2.	Altered source versions must be plainly marked as such, and must
;		not be misrepresented as being the original software.
;	3.	This notice may not be removed or altered from any source
;		distribution.

		.686
		.model	flat
		.mmx
		.xmm
		.code

_VDFastMemcpyPartialMMX2 proc
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		ebx, [esp+4+16]
		mov		edx, [esp+8+16]
		mov		eax, [esp+12+16]
		neg		eax
		add		eax, 63
		jbe		skipblastloop
blastloop:
		movq	mm0, [edx]
		movq	mm1, [edx+8]
		movq	mm2, [edx+16]
		movq	mm3, [edx+24]
		movq	mm4, [edx+32]
		movq	mm5, [edx+40]
		movq	mm6, [edx+48]
		movq	mm7, [edx+56]
		movntq	[ebx], mm0
		movntq	[ebx+8], mm1
		movntq	[ebx+16], mm2
		movntq	[ebx+24], mm3
		movntq	[ebx+32], mm4
		movntq	[ebx+40], mm5
		movntq	[ebx+48], mm6
		movntq	[ebx+56], mm7
		add		ebx, 64
		add		edx, 64
		add		eax, 64
		jnc		blastloop
skipblastloop:
		sub		eax, 63-7
		jns		noextras
quadloop:
		movq	mm0, [edx]
		movntq	[ebx], mm0
		add		edx, 8
		add		ebx, 8
		add		eax, 8
		jnc		quadloop
noextras:
		sub		eax, 7
		jz		nooddballs
		mov		ecx, eax
		neg		ecx
		mov		esi, edx
		mov		edi, ebx
		rep		movsb
nooddballs:
		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
_VDFastMemcpyPartialMMX2 endp

		end

