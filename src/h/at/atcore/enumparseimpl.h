//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2017 Avery Lee
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

#ifndef f_AT_ATCORE_ENUMPARSEIMPL_H
#define f_AT_ATCORE_ENUMPARSEIMPL_H

#include <initializer_list>
#include <at/atcore/enumparse.h>

template<typename T>
struct ATEnumEntry {
	T mValue;
	const char *mpName;
};

struct ATEnumLookupEntry {
	const char *mpName;
	uint32 mValue;
	uint32 mHash;
};

struct ATEnumLookupTable {
	const ATEnumLookupEntry *mpTable;
	size_t mTableEntries;
	uint32 mDefaultValue;
};

template<typename T, size_t N>
struct ATEnumLookupTableImpl {
	ATEnumLookupEntry mHashEntries[N];
};

constexpr uint32 ATEnumHashNameCT(const char *s) {
	uint32 hash = 2166136261U;

	while(const char c = *s++) {
		hash = (uint32)((uint64)hash * 16777619U);		// force 64-bit arithmetic to dodge warning
		hash ^= (unsigned char)c & 0xDFU;
	}

	return hash;
}


template<typename T, size_t N>
constexpr ATEnumLookupTableImpl<T, N> ATCreateEnumLookupTable(const ATEnumEntry<T> (&table)[N]) {
	ATEnumLookupTableImpl<T, N> outputTable {};

	for(size_t i=0; i<N; ++i) {
		outputTable.mHashEntries[i] = { table[i].mpName, (uint32)table[i].mValue, ATEnumHashNameCT(table[i].mpName) };
	}

	return outputTable;
}

//=========================================================================
// Enum table definition
//
// In a header file:
//
//	AT_DECLARE_ENUM_TABLE(MyEnumType)
//
// In a .cpp file:
//
//	AT_DEFINE_ENUM_TABLE_BEGIN(MyEnumType)
//		{ MyEnumType::value, "value_string" },
//		...
//	AT_DEFINE_ENUM_TABLE_END(MyEnumType)
//

#define AT_DEFINE_ENUM_TABLE_BEGIN(enumName)	\
	template<> const ATEnumLookupTable& ATGetEnumLookupTable<enumName>() {	\
		static constexpr ATEnumEntry<enumName> kTable[] = {

#define AT_DEFINE_ENUM_TABLE_END(enumName, defaultValue)	\
		};	\
	constexpr static const auto s_enumTable = ATCreateEnumLookupTable(kTable);	\
	constexpr static const ATEnumLookupTable s_enumLookupTable = ATEnumLookupTable { &s_enumTable.mHashEntries[0], vdcountof(s_enumTable.mHashEntries), (uint32)(defaultValue) };	\
		\
	return s_enumLookupTable;	\
}

#endif
