//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2009-2014 Avery Lee
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
#include <at/atcore/deviceimpl.h>

ATDevice::ATDevice() {
}

ATDevice::~ATDevice() {
	VDASSERT(!mpDeviceParent);
	VDASSERT(!mpDeviceManager);
}

void *ATDevice::AsInterface(uint32 iid) {
	if (iid == IATDevice::kTypeID)
		return static_cast<IATDevice *>(this);

	return nullptr;
}

void ATDevice::SetManager(IATDeviceManager *devMgr) {
	mpDeviceManager = devMgr;
}

IATDeviceParent *ATDevice::GetParent() {
	return mpDeviceParent;
}

uint32 ATDevice::GetParentBusIndex() {
	return mDeviceParentBusIndex;
}

void ATDevice::SetParent(IATDeviceParent *parent, uint32 busIndex) {
	mpDeviceParent = parent;
	mDeviceParentBusIndex = busIndex;
}

void ATDevice::GetSettingsBlurb(VDStringW& buf) {
}

void ATDevice::GetSettings(ATPropertySet& settings) {
}

bool ATDevice::SetSettings(const ATPropertySet& settings) {
	return true;
}

void ATDevice::Init() {
}

void ATDevice::Shutdown() {
}

uint32 ATDevice::GetComputerPowerOnDelay() const {
	return 0;
}

void ATDevice::WarmReset() {
}

void ATDevice::ColdReset() {
}

void ATDevice::ComputerColdReset() {
}

void ATDevice::PeripheralColdReset() {
}

void ATDevice::SetTraceContext(ATTraceContext *context) {
}

bool ATDevice::GetErrorStatus(uint32 idx, VDStringW& error) {
	return false;
}

bool ATDevice::IsSaveStateAgnostic() const {
	return mbSaveStateAgnostic;
}

void *ATDevice::GetService(uint32 iid) const {
	return mpDeviceManager ? mpDeviceManager->GetService(iid) : nullptr;
}

void ATDevice::NotifyStatusChanged() {
	if (mpDeviceManager)
		mpDeviceManager->NotifyDeviceStatusChanged(*this);
}
