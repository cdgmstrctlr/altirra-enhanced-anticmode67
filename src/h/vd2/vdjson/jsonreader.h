//	VirtualDub - Video processing and capture application
//	JSON I/O library
//	Copyright (C) 1998-2010 Avery Lee
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

#ifndef f_VD2_VDJSON_JSONREADER_H
#define f_VD2_VDJSON_JSONREADER_H

#include <vd2/system/vdstl.h>
#include <vd2/system/function.h>

class VDJSONNameTable;
class VDJSONDocument;
struct VDJSONValue;
class VDStringW;

class VDJSONReader {
public:
	VDJSONReader();
	~VDJSONReader();

	void SetMemberNameFilter(vdfunction<bool(const wchar_t *)> fn);

	bool Parse(const void *buf, size_t len, VDJSONDocument& doc);
	void GetErrorLocation(int& line, int& offset) const;

protected:
	bool ParseDocument();
	bool ParseObject(VDJSONValue& obj);
	bool ParseArray(VDJSONValue& arr);
	bool ParseValue(VDJSONValue& val);
	bool ParseValue(wchar_t c, VDJSONValue& val);
	bool ParseString();
	bool Expect(wchar_t c);

	void ClearNameBuffer();
	bool AddNameChar(wchar_t c);
	bool EndName();

	static bool IsWhitespaceChar(wchar_t c);

	wchar_t GetNonWhitespaceChar();
	void UngetChar();
	wchar_t GetChar();
	wchar_t GetCharSlow();

	uint32 GetTokenForName();

	VDJSONNameTable *mpNameTable;
	VDJSONDocument *mpDocument;

	wchar_t *mNameBuffer;
	int mNameBufferIndex;
	int mNameBufferLength;

	enum { kInputBufferSize = 256 };
	const wchar_t *mpInputBase;
	const wchar_t *mpInputNext;
	const wchar_t *mpInputEnd;
	uint32 mInputLine;
	uint32 mInputChar;
	uint32 mInputLineNext;
	uint32 mInputCharNext;

	wchar_t mInputBuffer[kInputBufferSize];

	bool mbPendingCR;
	bool mbUTF16Mode;
	bool mbUTF32Mode;
	bool mbBigEndian;
	const uint8 *mpSrc;
	const uint8 *mpSrcEnd;
	bool mbSrcError;
	bool mbSuppressAdvanceLine;

	vdfunction<bool(const wchar_t *)> mpMemberFilter;
	vdfastvector<const VDJSONValue *> mArrayStack;
};

#endif
