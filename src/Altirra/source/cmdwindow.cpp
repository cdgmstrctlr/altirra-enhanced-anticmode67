//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2009-2015 Avery Lee
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
#include <at/atnativeui/uiframe.h>
#include "uiaccessors.h"

extern ATContainerWindow *g_pMainWindow;

void OnCommandWindowClose() {
	if (!g_pMainWindow)
		return;

	if (g_pMainWindow->GetModalFrame() || g_pMainWindow->GetFullScreenFrame())
		return;

	ATFrameWindow *w = g_pMainWindow->GetActiveFrame();
	if (!w)
		return;

	g_pMainWindow->CloseFrame(w);
}

void OnCommandWindowUndock() {
	if (!g_pMainWindow)
		return;

	if (g_pMainWindow->GetModalFrame() || g_pMainWindow->GetFullScreenFrame())
		return;

	ATFrameWindow *w = g_pMainWindow->GetActiveFrame();
	if (!w)
		return;

	g_pMainWindow->UndockFrame(w);
}

void OnCommandWindowNextPane() {
	if (g_pMainWindow)
		g_pMainWindow->CycleActiveFrame(+1);
}

void OnCommandWindowPrevPane() {
	if (g_pMainWindow)
		g_pMainWindow->CycleActiveFrame(-1);
}
