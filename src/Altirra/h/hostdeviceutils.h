//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2009-2016 Avery Lee
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

#ifndef f_AT_HOSTFILEUTILS_H
#define f_AT_HOSTFILEUTILS_H

bool ATHostDeviceIsDevice(const wchar_t *s);

inline bool ATHostDeviceIsPathWild(const wchar_t *s) {
	return wcschr(s, L'*') || wcschr(s, L'?');
}

inline bool ATHostDeviceIsValidPathChar(uint8 c) {
	return c == '_' || (uint8)(c - '0') < 10 || (uint8)(c - 'A') < 26;
}

inline bool ATHostDeviceIsValidPathCharWide(wchar_t c) {
	return c == L'_' || (uint32)(c - L'0') < 10 || (uint32)(c - 'A') < 26;
}

inline bool ATHostDeviceIsValidPathCharLFN(uint8 c) {
	return c >= 0x20 && c < 0x7C;
}

inline bool ATHostDeviceIsValidPathCharWideLFN(wchar_t c) {
	return c >= 0x20 && c < 0x7C;
}

void ATHostDeviceEncodeName(VDStringA& encodedName, const wchar_t *hostName, bool useLongNameEncoding, bool useLongNames);

#endif
