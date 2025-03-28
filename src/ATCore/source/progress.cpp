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
//	You should have received a copy of the GNU General Public License along
//	with this program. If not, see <http://www.gnu.org/licenses/>.
//
//	As a special exception, this library can also be redistributed and/or
//	modified under an alternate license. See COPYING.RMT in the same source
//	archive for details.

#include <stdafx.h>
#include <vd2/system/error.h>
#include <at/atcore/progress.h>

IATProgressHandler *g_pATProgressHandler;

void ATBeginProgress(uint32 total, const wchar_t *statusFormat, const wchar_t *desc) {
	if (g_pATProgressHandler)
		g_pATProgressHandler->Begin(total, statusFormat, desc);
}

void ATBeginProgressF(uint32 total, const wchar_t *statusFormat, const wchar_t *descFormat, va_list descArgs) {
	if (g_pATProgressHandler)
		g_pATProgressHandler->BeginF(total, statusFormat, descFormat, descArgs);
}

void ATUpdateProgress(uint32 count) {
	if (g_pATProgressHandler)
		g_pATProgressHandler->Update(count);
}

bool ATCheckProgressStatusUpdate() {
	return g_pATProgressHandler && g_pATProgressHandler->CheckForCancellationOrStatus();
}

void ATUpdateProgressStatus(const wchar_t *message) {
	if (g_pATProgressHandler)
		g_pATProgressHandler->UpdateStatus(message);
}

void ATEndProgress() {
	if (g_pATProgressHandler)
		g_pATProgressHandler->End();
}

void ATSetProgressHandler(IATProgressHandler *h) {
	g_pATProgressHandler = h;
}

class ATTaskProgressContextNull final : public IATTaskProgressContext {
public:
	bool CheckForCancellationOrStatus() override { return false; }
	void SetProgress(double progress) {}
	void SetProgressF(double progress, const wchar_t *format, ...) {}
};

void ATRunTaskWithProgress(const wchar_t *desc, const vdfunction<void(IATTaskProgressContext&)>& fn) {
	if (!g_pATProgressHandler || !g_pATProgressHandler->RunTask(desc, fn)) {
		try {
			ATTaskProgressContextNull ctx;
			fn(ctx);
		} catch(const MyUserAbortError&) {
		}
	}
}
