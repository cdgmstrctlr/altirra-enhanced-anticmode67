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

#ifndef f_AT_ATDEVICES_VIDEOGENERATOR_H
#define f_AT_ATDEVICES_VIDEOGENERATOR_H

#include <at/atcore/deviceimpl.h>
#include <at/atcore/devicevideosource.h>
#include <at/atcore/scheduler.h>

class ATDeviceVideoGenerator final : public ATDevice, public IATDeviceVideoSource {
public:
	ATDeviceVideoGenerator();

	void *AsInterface(uint32 id) override;

	void GetDeviceInfo(ATDeviceInfo& info) override;
	void Init() override;

public:	// IATDeviceVideoSource
	uint32 ReadVideoSample(float x, int y) const override;

private:
	uint8 mImage[480][640] {};
};

#endif
