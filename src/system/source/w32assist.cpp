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

#include <stdafx.h>
#include <vd2/system/w32assist.h>
#include <vd2/system/filesys.h>
#include <vd2/system/seh.h>
#include <vd2/system/text.h>
#include <vd2/system/vdstdc.h>
#include <vd2/system/vdstl.h>

bool VDTestOSVersionW32(uint8 major, uint8 minor) {
	OSVERSIONINFOEX vervals = {};
	vervals.dwMajorVersion = major;
	vervals.dwMinorVersion = minor;

	ULONGLONG cond = 0;
	VER_SET_CONDITION(cond, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
	return 0 != VerifyVersionInfo(&vervals, VER_MAJORVERSION | VER_MINORVERSION, cond);
}

bool VDTestOSBuildW32(uint32 build) {
	OSVERSIONINFOEX vervals {};
	vervals.dwBuildNumber = build;
	ULONGLONG cond = 0;
	VER_SET_CONDITION(cond, VER_BUILDNUMBER, VER_GREATER_EQUAL);
	return 0 != VerifyVersionInfo(&vervals, VER_BUILDNUMBER, cond);
}

bool VDIsAtLeast7W32() {
	static const bool result = VDTestOSVersionW32(6,1);

	return result;
}

bool VDIsAtLeast8W32() {
	static const bool result = VDTestOSVersionW32(6,2);

	return result;
}

bool VDIsAtLeast81W32() {
	static const bool result = VDTestOSVersionW32(6,3);

	return result;
}

bool VDIsAtLeast10W32() {
	static const bool result = VDTestOSVersionW32(10,0);

	return result;
}

bool VDIsAtLeast10_1803W32() {
	// We need to use two separate tests here as VerifyVersionInfo() only
	// special cases major > minor > service, otherwise it does an AND.
	static const bool result = VDTestOSVersionW32(10,0) && VDTestOSBuildW32(17134);

	return result;
}

bool VDIsForegroundTaskW32() {
	HWND hwndFore = GetForegroundWindow();

	if (!hwndFore)
		return false;

	DWORD dwProcessId = 0;
	GetWindowThreadProcessId(hwndFore, &dwProcessId);

	return dwProcessId == GetCurrentProcessId();
}

LPVOID VDConvertThreadToFiberW32(LPVOID parm) {
	return ConvertThreadToFiber(parm);
}

void VDSwitchToFiberW32(LPVOID fiber) {
	return SwitchToFiber(fiber);
}

int VDGetSizeOfBitmapHeaderW32(const BITMAPINFOHEADER *pHdr) {
	int palents = 0;

	if ((pHdr->biCompression == BI_RGB || pHdr->biCompression == BI_RLE4 || pHdr->biCompression == BI_RLE8) && pHdr->biBitCount <= 8) {
		palents = pHdr->biClrUsed;
		if (!palents)
			palents = 1 << pHdr->biBitCount;
	}
	int size = pHdr->biSize + palents * sizeof(RGBQUAD);

	if (pHdr->biSize < sizeof(BITMAPV4HEADER) && pHdr->biCompression == BI_BITFIELDS)
		size += sizeof(DWORD) * 3;

	return size;
}

void VDSetWindowTextW32(HWND hwnd, const wchar_t *s) {
	SetWindowTextW(hwnd, s);
}

void VDSetWindowTextFW32(HWND hwnd, const wchar_t *format, ...) {
	va_list val;

	va_start(val, format);
	{
		wchar_t buf[512];
		int r = vdvswprintf(buf, 512, format, val);

		if ((unsigned)r < 512) {
			VDSetWindowTextW32(hwnd, buf);
			va_end(val);
			return;
		}
	}

	VDStringW s;
	s.append_vsprintf(format, val);
	VDSetWindowTextW32(hwnd, s.c_str());

	va_end(val);
}

VDStringA VDGetWindowTextAW32(HWND hwnd) {
	char buf[512];

	int len = GetWindowTextLengthA(hwnd);

	if (len > 511) {
		vdblock<char> tmp(len + 1);
		len = GetWindowTextA(hwnd, tmp.data(), tmp.size());

		const char *s = tmp.data();
		VDStringA text(s, s+len);
		return text;
	} else if (len > 0) {
		len = GetWindowTextA(hwnd, buf, 512);

		return VDStringA(buf, buf + len);
	}

	return VDStringA();
}

VDStringW VDGetWindowTextW32(HWND hwnd) {
	union {
		wchar_t w[256];
		char a[512];
	} buf;

	int len = GetWindowTextLengthW(hwnd);

	if (len > 255) {
		vdblock<wchar_t> tmp(len + 1);
		len = GetWindowTextW(hwnd, tmp.data(), tmp.size());

		VDStringW text(tmp.data(), len);
		return text;
	} else if (len > 0) {
		len = GetWindowTextW(hwnd, buf.w, 256);

		VDStringW text(buf.w, len);
		return text;
	}

	return VDStringW();
}

void VDAppendMenuW32(HMENU hmenu, UINT flags, UINT id, const wchar_t *text){
	VDVERIFY(AppendMenuW(hmenu, flags, id, text));
}

bool VDAppendPopupMenuW32(HMENU hmenu, UINT flags, HMENU hmenuPopup, const wchar_t *text){
	flags |= MF_POPUP;

	return 0 != AppendMenuW(hmenu, flags, (UINT_PTR)hmenuPopup, text);
}

void VDAppendMenuSeparatorW32(HMENU hmenu) {
	int pos = GetMenuItemCount(hmenu);
	if (pos < 0)
		return;

	MENUITEMINFOW mmiW {};
	mmiW.cbSize		= sizeof(MENUITEMINFOW);
	mmiW.fMask		= MIIM_TYPE;
	mmiW.fType		= MFT_SEPARATOR;

	VDVERIFY(InsertMenuItemW(hmenu, pos, TRUE, &mmiW));
}

void VDInsertMenuW32(HMENU hmenu, UINT pos, UINT flags, UINT id, const wchar_t *text){
	MENUITEMINFOW mmiW {};
	mmiW.cbSize		= sizeof(MENUITEMINFOW);
	mmiW.fMask		= MIIM_FTYPE | MIIM_STRING | MIIM_ID | MIIM_STATE;
	mmiW.fType		= MFT_STRING;
	mmiW.wID		= id;
	mmiW.dwTypeData	= (LPWSTR)text;

	mmiW.fState		= 0;

	if (flags & MF_CHECKED)		mmiW.fState |= MFS_CHECKED;
	if (flags & MF_DISABLED)	mmiW.fState |= MFS_DISABLED;
	if (flags & MF_GRAYED)		mmiW.fState |= MFS_GRAYED;
	if (flags & MF_HILITE)		mmiW.fState |= MFS_HILITE;

	VDVERIFY(InsertMenuItemW(hmenu, pos, TRUE, &mmiW));
}

void VDInsertMenuSeparatorW32(HMENU hmenu, UINT pos) {
	MENUITEMINFOW mmiW {};
	mmiW.cbSize		= sizeof(MENUITEMINFOW);
	mmiW.fMask		= MIIM_TYPE;
	mmiW.fType		= MFT_SEPARATOR;

	InsertMenuItemW(hmenu, pos, TRUE, &mmiW);
}

void VDCheckMenuItemByPositionW32(HMENU hmenu, uint32 pos, bool checked) {
	CheckMenuItem(hmenu, pos, checked ? MF_BYPOSITION|MF_CHECKED : MF_BYPOSITION|MF_UNCHECKED);
}

void VDCheckMenuItemByCommandW32(HMENU hmenu, UINT cmd, bool checked) {
	CheckMenuItem(hmenu, cmd, checked ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED);
}

void VDCheckRadioMenuItemByPositionW32(HMENU hmenu, uint32 pos, bool checked) {
	MENUITEMINFOW mii;

	mii.cbSize = sizeof(MENUITEMINFOW);
	mii.fMask = MIIM_FTYPE | MIIM_STATE;
	if (GetMenuItemInfoW(hmenu, pos, TRUE, &mii)) {
		mii.fType |= MFT_RADIOCHECK;
		mii.fState &= ~MFS_CHECKED;
		if (checked)
			mii.fState |= MFS_CHECKED;
		SetMenuItemInfoW(hmenu, pos, TRUE, &mii);
	}
}

void VDCheckRadioMenuItemByCommandW32(HMENU hmenu, UINT cmd, bool checked) {
	MENUITEMINFOW mii;

	mii.cbSize = sizeof(MENUITEMINFOW);
	mii.fMask = MIIM_FTYPE | MIIM_STATE;
	if (GetMenuItemInfoW(hmenu, cmd, FALSE, &mii)) {
		mii.fType |= MFT_RADIOCHECK;
		mii.fState &= ~MFS_CHECKED;
		if (checked)
			mii.fState |= MFS_CHECKED;
		SetMenuItemInfoW(hmenu, cmd, FALSE, &mii);
	}
}

void VDEnableMenuItemByCommandW32(HMENU hmenu, UINT cmd, bool checked) {
	EnableMenuItem(hmenu, cmd, checked ? MF_BYCOMMAND|MF_ENABLED : MF_BYCOMMAND|MF_GRAYED);
}

VDStringW VDGetMenuItemTextByCommandW32(HMENU hmenu, UINT cmd) {
	VDStringW s;

	MENUITEMINFOW mmiW;
	vdfastfixedvector<wchar_t, 256> bufW;

	mmiW.cbSize		= sizeof(MENUITEMINFOW);
	mmiW.fMask		= MIIM_TYPE;
	mmiW.fType		= MFT_STRING;
	mmiW.dwTypeData	= NULL;
	mmiW.cch		= 0;		// required to avoid crash on NT4

	if (GetMenuItemInfoW(hmenu, cmd, FALSE, &mmiW)) {
		bufW.resize(mmiW.cch + 1, 0);
		++mmiW.cch;
		mmiW.dwTypeData = bufW.data();

		if (GetMenuItemInfoW(hmenu, cmd, FALSE, &mmiW))
			s = bufW.data();
	}

	return s;
}

void VDSetMenuItemTextByCommandW32(HMENU hmenu, UINT cmd, const wchar_t *text) {
	MENUITEMINFOW mmiW;

	mmiW.cbSize		= sizeof(MENUITEMINFOW);
	mmiW.fMask		= MIIM_TYPE;
	mmiW.fType		= MFT_STRING;
	mmiW.dwTypeData	= (LPWSTR)text;

	SetMenuItemInfoW(hmenu, cmd, FALSE, &mmiW);
}

EXECUTION_STATE VDSetThreadExecutionStateW32(EXECUTION_STATE esFlags) {
	// SetThreadExecutionState(): requires Windows 98+/2000+.
	return SetThreadExecutionState(esFlags);
}

bool VDSetFilePointerW32(HANDLE h, sint64 pos, DWORD dwMoveMethod) {
	LONG posHi = (LONG)(pos >> 32);
	DWORD result = SetFilePointer(h, (LONG)pos, &posHi, dwMoveMethod);

	if (result != INVALID_SET_FILE_POINTER)
		return true;

	DWORD dwError = GetLastError();

	return (dwError == NO_ERROR);
}

bool VDGetFileSizeW32(HANDLE h, sint64& size) {
	DWORD dwSizeHigh;
	DWORD dwSizeLow = GetFileSize(h, &dwSizeHigh);

	if (dwSizeLow == (DWORD)-1 && GetLastError() != NO_ERROR)
		return false;

	size = dwSizeLow + ((sint64)dwSizeHigh << 32);
	return true;
}

bool VDDrawTextW32(HDC hdc, const wchar_t *s, int nCount, LPRECT lpRect, UINT uFormat) {
	RECT r;

	// If multiline and vcentered (not normally supported...)
	if (!((uFormat ^ DT_VCENTER) & (DT_VCENTER|DT_SINGLELINE))) {
		uFormat &= ~DT_VCENTER;

		r = *lpRect;
		if (!DrawTextW(hdc, s, nCount, &r, uFormat | DT_CALCRECT))
			return false;

		int dx = ((lpRect->right - lpRect->left) - (r.right - r.left)) >> 1;
		int dy = ((lpRect->bottom - lpRect->top) - (r.bottom - r.top)) >> 1;

		r.left += dx;
		r.right += dx;
		r.top += dy;
		r.bottom += dy;
		lpRect = &r;
	}

	return !!DrawTextW(hdc, s, nCount, lpRect, uFormat);
}

bool VDPatchModuleImportTableW32(HMODULE hmod, const char *srcModule, const char *name, void *pCompareValue, void *pNewValue, void *volatile *ppOldValue) {
	char *pBase = (char *)hmod;

	vd_seh_guard_try {
		// The PEheader offset is at hmod+0x3c.  Add the size of the optional header
		// to step to the section headers.

		const uint32 peoffset = ((const long *)pBase)[15];
		const uint32 signature = *(uint32 *)(pBase + peoffset);

		if (signature != IMAGE_NT_SIGNATURE)
			return false;

		const IMAGE_FILE_HEADER *pHeader = (const IMAGE_FILE_HEADER *)(pBase + peoffset + 4);

		// Verify the PE optional structure.

		if (pHeader->SizeOfOptionalHeader < 104)
			return false;

		// Find import header.

		const IMAGE_IMPORT_DESCRIPTOR *pImportDir;
		int nImports;

		switch(*(short *)((char *)pHeader + IMAGE_SIZEOF_FILE_HEADER)) {

#ifdef _M_AMD64
		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
			{
				const IMAGE_OPTIONAL_HEADER64 *pOpt = (IMAGE_OPTIONAL_HEADER64 *)((const char *)pHeader + sizeof(IMAGE_FILE_HEADER));

				if (pOpt->NumberOfRvaAndSizes < 2)
					return false;

				pImportDir = (const IMAGE_IMPORT_DESCRIPTOR *)(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
				nImports = pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
			}
			break;
#else
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
			{
				const IMAGE_OPTIONAL_HEADER32 *pOpt = (IMAGE_OPTIONAL_HEADER32 *)((const char *)pHeader + sizeof(IMAGE_FILE_HEADER));

				if (pOpt->NumberOfRvaAndSizes < 2)
					return false;

				pImportDir = (const IMAGE_IMPORT_DESCRIPTOR *)(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
				nImports = pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
			}
			break;
#endif

		default:		// reject PE32+
			return false;
		}

		// Hmmm... no imports?

		if ((const char *)pImportDir == pBase)
			return false;

		// Scan down the import entries.  We are looking for MSVFW32.

		int i;

		for(i=0; i<nImports; ++i) {
			if (!_stricmp(pBase + pImportDir[i].Name, srcModule))
				break;
		}

		if (i >= nImports)
			return false;

		// Found it.  Start scanning MSVFW32 imports until we find DrawDibDraw.

		const long *pImports = (const long *)(pBase + pImportDir[i].OriginalFirstThunk);
		void * volatile *pVector = (void * volatile *)(pBase + pImportDir[i].FirstThunk);

		while(*pImports) {
			if (*pImports >= 0) {
				const char *pName = pBase + *pImports + 2;

				if (!strcmp(pName, name)) {

					// Found it!  Reset the protection.

					DWORD dwOldProtect;

					if (VirtualProtect((void *)pVector, sizeof(void *), PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
						if (ppOldValue) {
							for(;;) {
								void *old = *pVector;
								if (pCompareValue && pCompareValue != old)
									return false;

								*ppOldValue = old;
								if (old == VDAtomicCompareExchangePointer(pVector, pNewValue, old))
									break;
							}
						} else {
							*pVector = pNewValue;
						}

						VirtualProtect((void *)pVector, sizeof(void *), dwOldProtect, &dwOldProtect);

						return true;
					}

					break;
				}
			}

			++pImports;
			++pVector;
		}
	} vd_seh_guard_except {
	}

	return false;
}

bool VDPatchModuleExportTableW32(HMODULE hmod, const char *name, void *pCompareValue, void *pNewValue, void *volatile *ppOldValue) {
	char *pBase = (char *)hmod;

	vd_seh_guard_try {
		// The PEheader offset is at hmod+0x3c.  Add the size of the optional header
		// to step to the section headers.

		const uint32 peoffset = ((const long *)pBase)[15];
		const uint32 signature = *(uint32 *)(pBase + peoffset);

		if (signature != IMAGE_NT_SIGNATURE)
			return false;

		const IMAGE_FILE_HEADER *pHeader = (const IMAGE_FILE_HEADER *)(pBase + peoffset + 4);

		// Verify the PE optional structure.

		if (pHeader->SizeOfOptionalHeader < 104)
			return false;

		// Find export directory.

		const IMAGE_EXPORT_DIRECTORY *pExportDir;

		switch(*(short *)((char *)pHeader + IMAGE_SIZEOF_FILE_HEADER)) {

#ifdef _M_AMD64
		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
			{
				const IMAGE_OPTIONAL_HEADER64 *pOpt = (IMAGE_OPTIONAL_HEADER64 *)((const char *)pHeader + sizeof(IMAGE_FILE_HEADER));

				if (pOpt->NumberOfRvaAndSizes < 1)
					return false;

				DWORD exportDirRVA = pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

				if (!exportDirRVA)
					return false;

				pExportDir = (const IMAGE_EXPORT_DIRECTORY *)(pBase + exportDirRVA);
			}
			break;
#else
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
			{
				const IMAGE_OPTIONAL_HEADER32 *pOpt = (IMAGE_OPTIONAL_HEADER32 *)((const char *)pHeader + sizeof(IMAGE_FILE_HEADER));

				if (pOpt->NumberOfRvaAndSizes < 1)
					return false;

				DWORD exportDirRVA = pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

				if (!exportDirRVA)
					return false;

				pExportDir = (const IMAGE_EXPORT_DIRECTORY *)(pBase + exportDirRVA);
			}
			break;
#endif

		default:		// reject PE32+
			return false;
		}

		// Scan for the export name.
		DWORD nameCount = pExportDir->AddressOfNames;
		const DWORD *nameRVAs = (const DWORD *)(pBase + pExportDir->AddressOfNames);
		const WORD *nameOrdinals = (const WORD *)(pBase + pExportDir->AddressOfNameOrdinals);
		DWORD *functionTable = (DWORD *)(pBase + pExportDir->AddressOfFunctions);

		for(DWORD i=0; i<nameCount; ++i) {
			DWORD nameRVA = nameRVAs[i];
			const char *pName = (const char *)(pBase + nameRVA);

			// compare names
			if (!strcmp(pName, name)) {

				// name matches -- look up the function entry
				WORD ordinal = nameOrdinals[i];
				DWORD *pRVA = &functionTable[ordinal];
				
				// Reset the protection.

				DWORD newRVA = (DWORD)(uintptr_t)pNewValue - (DWORD)(uintptr_t)pBase;

				DWORD dwOldProtect;
				if (VirtualProtect((void *)pRVA, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
					if (ppOldValue) {
						for(;;) {
							DWORD oldRVA = *pRVA;
							void *old = pBase + oldRVA;
							if (pCompareValue && pCompareValue != old)
								return false;

							*ppOldValue = pBase + oldRVA;
							if (oldRVA == VDAtomicInt::staticCompareExchange((volatile int *)pRVA, newRVA, oldRVA))
								break;
						}
					} else {
						*pRVA = newRVA;
					}

					VirtualProtect((void *)pRVA, sizeof(DWORD), dwOldProtect, &dwOldProtect);

					return true;
				}

				break;
			}
		}
	} vd_seh_guard_except {
	}

	return false;
}

HMODULE VDLoadSystemLibraryWithAllowedOverrideW32(const char *name) {
	HMODULE hmod = LoadLibraryW(VDMakePath(VDGetProgramPath().c_str(), VDTextAToW(name).c_str()).c_str());

	if (hmod)
		return hmod;

	return VDLoadSystemLibraryW32(name);
}

HMODULE VDLoadSystemLibraryW32(const char *name) {
	vdfastvector<wchar_t> pathW(MAX_PATH, 0);

	size_t len = GetSystemDirectoryW(pathW.data(), MAX_PATH);

	if (!len)
		return NULL;

	if (len > MAX_PATH) {
		pathW.resize(len + 1, 0);

		len = GetSystemDirectoryW(pathW.data(), len);
		if (!len || len >= pathW.size())
			return NULL;
	}

	pathW.resize(len);

	if (pathW.back() != '\\')
		pathW.push_back('\\');

	while(const char c = *name++)
		pathW.push_back(c);

	pathW.push_back(0);

	return LoadLibraryW(pathW.data());
}
