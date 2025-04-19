//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2009-2022 Avery Lee
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

#include <stdafx.h>
#include "uiconfgeneric.h"

bool ATUIConfDevVideoStillImage(VDGUIHandle hParent, ATPropertySet& props) {
	return ATUIShowDialogGenericConfig(hParent, props, L"Video still image",
		[](IATUIConfigView& view) {
			view.AddPath().SetTypeImage().AsStringView()->SetLabel(L"&Path").SetTag("path");
		}
	);
}
