//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2008-2014 Avery Lee
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

#ifndef f_AT_UINATIVEWINDOW_H
#define f_AT_UINATIVEWINDOW_H

#include <windows.h>
#include <vd2/system/atomic.h>
#include <vd2/system/unknown.h>
#include <vd2/system/vectors.h>
#include <at/atnativeui/nativewindowproxy.h>
#include <at/atui/constants.h>

template<typename T> inline ATOM g_ATUINativeWindowClass;

class ATUINativeWindow : public IVDUnknown, public ATUINativeWindowProxy {
public:
	ATUINativeWindow();
	virtual ~ATUINativeWindow();

	static ATOM Register();
	static ATOM RegisterCustom(const WNDCLASS& wc);

	template<typename T>
	static ATOM RegisterForClass(const WCHAR *className) {
		if (!g_ATUINativeWindowClass<T>)
			g_ATUINativeWindowClass<T> = RegisterForClass(className, StaticClassSpecificWndProc<T>);

		return g_ATUINativeWindowClass<T>;
	}

	template<typename T>
	static T *FromHandle(HWND hwnd) {
		return hwnd ? static_cast<T *>((ATUINativeWindow *)GetWindowLongPtr(hwnd, 0)) : nullptr;
	}

	static void Unregister();

	int AddRef();
	int Release();
	void *AsInterface(uint32 iid) override;

	bool CreateChild(HWND hwndParent, UINT id, int x, int y, int w, int h, DWORD styles, DWORD exstyles = 0, const wchar_t *text = nullptr);

	// Enables immediate response to touch taps, disabling the touch delay for
	// right-clicks. Useful on windows that have no right-click actions or need
	// for gestures.
	void SetTouchMode(ATUITouchMode touchMode);

	static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam) = 0;

protected:
	virtual ATUITouchMode GetTouchModeAtPoint(const vdpoint32& pt) const;

	void UpdateTouchMode(ATUITouchMode touchMode);

	VDAtomicInt mRefCount;

	ATUITouchMode mTouchMode;

	static ATOM sWndClass;
	static ATOM sWndClassMain;

private:
	template<typename T>
	static LRESULT CALLBACK StaticClassSpecificWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_NCCREATE)
			return StaticWndProcClassSpecificCreate(hwnd, ClassFactory<T>, wParam, lParam);
		else
			return StaticWndProc(hwnd, msg, wParam, lParam);
	}

	static LRESULT CALLBACK StaticWndProcClassSpecificCreate(HWND hwnd, ATUINativeWindow *(*factory)(), WPARAM wParam, LPARAM lParam);

	template<typename T>
	static ATUINativeWindow *ClassFactory() {
		return new(std::nothrow) T;
	}

	static ATOM RegisterForClass(const WCHAR *className, WNDPROC classSpecificWndProc);
};

#endif
