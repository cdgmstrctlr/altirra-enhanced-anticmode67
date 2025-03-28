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

#pragma once

#define _SCL_SECURE_NO_WARNINGS
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <tchar.h>
#include <vd2/system/error.h>
#include <vd2/system/fraction.h>
#include <vd2/system/function.h>
#include <vd2/system/linearalloc.h>
#include <vd2/system/refcount.h>
#include <vd2/system/time.h>
#include <vd2/system/unknown.h>
#include <vd2/system/vdalloc.h>
#include <vd2/system/vdtypes.h>
#include <vd2/system/vdstl.h>
#include <vd2/system/VDString.h>
#include <vd2/system/vectors.h>
#include <vd2/Kasumi/pixmap.h>
#include <at/atcore/device.h>
#include <at/atcore/enumparse.h>
#include <atomic>
#include <deque>
#include <iterator>
#include <utility>
#include <optional>

struct IUnknown;