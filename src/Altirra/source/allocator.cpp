//	Altirra - Atari 800/800XL emulator
//	Copyright (C) 2008 Avery Lee
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
#include <crtdbg.h>
#include <intrin.h>

#ifdef _DEBUG

#if VD_PTR_SIZE > 4
	extern "C" char __ImageBase;
	#define ENCODED_RETURN_ADDRESS ((int)((uintptr_t)_ReturnAddress() - (uintptr_t)&__ImageBase))
#else
	#define ENCODED_RETURN_ADDRESS ((int)_ReturnAddress())
#endif

void *VDCDECL operator new(size_t bytes) {
	static const char fname[]="return address";

	void *p = _malloc_dbg(bytes, _NORMAL_BLOCK, fname, ENCODED_RETURN_ADDRESS);
	if (!p)
		throw std::bad_alloc();
	return p;
}

void *VDCDECL operator new(size_t bytes, const std::nothrow_t&) noexcept {
	static const char fname[]="return address";

	return _malloc_dbg(bytes, _NORMAL_BLOCK, fname, ENCODED_RETURN_ADDRESS);
}

void *VDCDECL operator new[](size_t bytes) {
	static const char fname[]="return address";

	void *p = _malloc_dbg(bytes, _NORMAL_BLOCK, fname, ENCODED_RETURN_ADDRESS);
	if (!p)
		throw std::bad_alloc();
	return p;
}

void *VDCDECL operator new[](size_t bytes, const std::nothrow_t&) noexcept {
	static const char fname[]="return address";

	return _malloc_dbg(bytes, _NORMAL_BLOCK, fname, ENCODED_RETURN_ADDRESS);
}

//
// C++17 aligned allocation
//

void *VDCDECL operator new(size_t bytes, std::align_val_t al) {
	static const char fname[]="return address";

	void *p = _aligned_malloc_dbg(bytes, (size_t)al, fname, ENCODED_RETURN_ADDRESS);
	if (!p)
		throw std::bad_alloc();
	return p;
}

void *VDCDECL operator new(size_t bytes, std::align_val_t al, const std::nothrow_t&) noexcept {
	static const char fname[]="return address";

	return _aligned_malloc_dbg(bytes, (size_t)al, fname, ENCODED_RETURN_ADDRESS);
}

void *VDCDECL operator new[](size_t bytes, std::align_val_t al) {
	static const char fname[]="return address";

	void *p = _aligned_malloc_dbg(bytes, (size_t)al, fname, ENCODED_RETURN_ADDRESS);
	if (!p)
		throw std::bad_alloc();
	return p;
}

void *VDCDECL operator new[](size_t bytes, std::align_val_t al, const std::nothrow_t&) noexcept {
	static const char fname[]="return address";

	return _aligned_malloc_dbg(bytes, (size_t)al, fname, ENCODED_RETURN_ADDRESS);
}

#endif
