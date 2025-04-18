//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2009-2015 Avery Lee
//	Application core library - memory utilities
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
//	You should have received a copy of the GNU General Public License along
//	with this program. If not, see <http://www.gnu.org/licenses/>.
//
//	As a special exception, this library can also be redistributed and/or
//	modified under an alternate license. See COPYING.RMT in the same source
//	archive for details.

#ifndef f_AT_ATCORE_MEMORYUTILS_H
#define f_AT_ATCORE_MEMORYUTILS_H

#include <vd2/system/vdtypes.h>

// Fill memory with random data. Returns the state of the random number
// generator. The initial seed may be an arbitrary non-zero number but must
// not be zero. This is a quick-and-dirty RNG.
//
uint32 ATRandomizeMemory(void *dst, size_t len, uint32 seed);

#endif	// f_AT_ATCORE_MEMORYUTILS_H
