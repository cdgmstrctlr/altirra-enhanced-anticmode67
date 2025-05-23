//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2009-2023 Avery Lee
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
#include <vd2/system/constexpr.h>
#include <at/atcore/audiomixer.h>
#include <at/atcore/logging.h>
#include <at/atdevices/supersalt.h>

ATLogChannel g_ATLCSuperSALT(false, false, "SUPERSALT", "CPS SuperSALT Test Assembly emulation");

extern const ATDeviceDefinition g_ATDeviceDefSuperSALT {
	"supersalt",
	nullptr,
	L"SuperSALT Test Assembly",
	[](const ATPropertySet& pset, IATDevice** dev) {
		vdrefptr<ATDeviceSuperSALT> devp(new ATDeviceSuperSALT);
		
		*dev = devp.release();
	}
};

ATDeviceSuperSALT::ATDeviceSuperSALT()
	: mpAudioEdgeBuffer(new ATSyncAudioEdgeBuffer)
{
	mpAudioEdgeBuffer->mpDebugLabel = "SuperSALT";
}

ATDeviceSuperSALT::~ATDeviceSuperSALT() {
}

void ATDeviceSuperSALT::GetDeviceInfo(ATDeviceInfo& info) {
	info.mpDef = &g_ATDeviceDefSuperSALT;
}

void ATDeviceSuperSALT::Init() {
	IATDevicePortManager *devPorts = GetService<IATDevicePortManager>();

	for(int i=0; i<4; ++i) {
		devPorts->AllocControllerPort(i, ~mpPorts[i]);
		mpPorts[i]->SetDirOutputMask(15);
		mpPorts[i]->SetOnDirOutputChanged(15, [this] { UpdatePortInputs(); }, false);
	}

	mpSIOMgr = GetService<IATDeviceSIOManager>();
	mpSIOMgr->AddRawDevice(this);
	mpScheduler = GetService<IATDeviceSchedulingService>()->GetMachineScheduler();

	mpAudioMixer = GetService<IATAudioMixer>();
	mpAudioMixer->AddSyncAudioSource(this);

	ColdReset();
	UpdatePortInputs();
}

void ATDeviceSuperSALT::Shutdown() {
	if (mpAudioMixer) {
		mpAudioMixer->RemoveSyncAudioSource(this);
		mpAudioMixer = nullptr;
	}

	if (mpSIOMgr) {
		mpSIOMgr->RemoveRawDevice(this);
		mpSIOMgr = nullptr;
	}

	if (mpScheduler) {
		mpScheduler->UnsetEvent(mpEventSendByte);
		mpScheduler = nullptr;
	}

	for(auto& port : mpPorts)
		port = nullptr;
}

void ATDeviceSuperSALT::ColdReset() {
}

void ATDeviceSuperSALT::OnCommandStateChanged(bool asserted) {
	g_ATLCSuperSALT("%ccommand\n", asserted ? '+' : '-');

	UpdatePortInputs();

	mpSIOMgr->SetSIOInterrupt(this, asserted);
}

void ATDeviceSuperSALT::OnMotorStateChanged(bool asserted) {
	g_ATLCSuperSALT("%cmotor\n", asserted ? '+' : '-');

	UpdatePortInputs();

	mpSIOMgr->SetSIOProceed(this, !asserted);
}

void ATDeviceSuperSALT::OnBreakStateChanged(bool asserted) {
	g_ATLCSuperSALT("%cbreak\n", asserted ? '+' : '-');

	if (!asserted)
		StartADC();

	UpdatePortInputs();
}

void ATDeviceSuperSALT::OnBeginReceiveByte(uint8 c, bool command, uint32 cyclesPerBit) {
	StartADC();
}

void ATDeviceSuperSALT::OnReceiveByte(uint8 c, bool command, uint32 cyclesPerBit) {
	if (mbSIOLoopbackEnabled) {
		// check if SIO clock out is being routed back to SIO clock in
		const bool syncClockEnabled = 0 != (ComputeDirBus() & 1);
		if (syncClockEnabled)
			mpSIOMgr->SetExternalClock(this, 0, cyclesPerBit);

		// We don't have time to simulate input with loopback because POKEY is
		// supposed to be shifting the bits in as soon as they are shifted out;
		// CPS SuperSALT actually times this and complains if there is too much
		// delay.
		mpSIOMgr->SendRawByte(c, cyclesPerBit, syncClockEnabled, false, false);
	}
}

void ATDeviceSuperSALT::OnSendReady() {
}

void ATDeviceSuperSALT::OnSerialBiClockChanged(uint32 cyclesPerBit) {
	SetAudioInputPeriodBclk(cyclesPerBit);
}

// This is pretty complex.
//
// Joystick direction lines are split into even and odd ports, with ports
// 1+3 and 2+4 connected together to form low and high 4-bit data ports,
// respectively. These are multiplexed to a number of functions:
//
// - Pot B and trigger inputs are connected to joystick left (bit 2) on
//   ports 1+2, and joystick up (bit 0) on ports 3+4.
// - Pot A is connected to joystick right (bit 3) on ports 1+2, and joystick
//   down (bit 1) on ports 3+4.
// - The two paired 4-bit buses form a single 8-bit data port to receive
//   the ADC output.
// - Port 1+3 bit 0 selects the baud rate generator clock as SIO clock in
//   (0), or the computer's SIO clock out (1).
// - Port 1+3 bit 1 selects the serial generator output for SIO data in
//   (0), or the computer's SIO data out (1).
// - Port 1+3 bit 2 combines with computer SIO motor control to control
//   audio output. Both must be high to send the clock to SIO audio in
// - Port 1+3 bit 3 is used as an error display control line.
// - SIO data out + motor control both low connects the port 1+2 and port
//   3+4 buses together.
// - Port 2+4 bits 0-3 select baud rate:
//		0000	2400 baud
//		0001	4800 baud
//		0010	9600 baud
//		0011	19200 baud
//		0100	38400 baud
//		0101	76800 baud
//		0110	19200 baud
//		0111	stopped
//		1000	300 baud
//		1001	600 baud
//		1010	1200 baud
//		1011	2400 baud
//		1100	4800 baud
//		1101	9600 baud
//		1110	2400 baud
//		1111	stopped
// - SIO command acts as ADC output enable.
// - SIO data out acts as ADC start.
// - Port 1+3 bits 0-3 select the ADC input:
//		0000	+5V
//		0001	SIO +5V/ready
//		0010	Port 1 +5V
//		0011	Port 2 +5V
//		0100	Port 3 +5V
//		0101	Port 4 +5V
//		0110	SIO ground
//		0111	Port 1 ground
//		1000	Port 2 ground
//		1001	Port 3 ground
//		1010	Port 4 ground
//		1011	SIO motor
//		1100	SIO +12V, divided by 10K/(10K+14K)
//		1101	UUT +9VAC
//		1110	+2.5V calibration voltage
//		1111	Ground

void ATDeviceSuperSALT::OnScheduledEvent(uint32 id) {
	mpEventSendByte = nullptr;

	if (mbSIOLoopbackEnabled)
		return;

	const uint8 bus = ComputeDirBus();
	uint32 cyclesPerByte = ComputeSerialCyclesPerByte(bus);

	if (cyclesPerByte) {
		// synchronous mode will only work if the baud generator clock is routed
		// to SIO clock in
		const bool baudClockEnabled = !(bus & 1);

		mpSIOMgr->SendRawByte(0x55, cyclesPerByte / 16, baudClockEnabled);
		mpScheduler->SetEvent(cyclesPerByte, this, 1, mpEventSendByte);
	}
}

bool ATDeviceSuperSALT::RequiresStereoMixingNow() const {
	return false;
}

void ATDeviceSuperSALT::WriteAudio(const ATSyncAudioMixInfo& mixInfo) {
	FlushAudio(mixInfo.mStartTime + 28*mixInfo.mCount);

	mpAudioEdgeBuffer->mLeftVolume = 0.25f;
	mpAudioEdgeBuffer->mRightVolume = 0.25f;
	mpAudioMixer->GetEdgePlayer().AddEdgeBuffer(mpAudioEdgeBuffer);
}

uint8 ATDeviceSuperSALT::ComputeDirBus() const {
	uint8 p0 = mpPorts[0]->GetCurrentDirOutput();
	uint8 p1 = mpPorts[1]->GetCurrentDirOutput();
	uint8 p2 = mpPorts[2]->GetCurrentDirOutput();
	uint8 p3 = mpPorts[3]->GetCurrentDirOutput();

	uint8 bus = (p0 & p2) + ((p1 & p3) << 4);

	return bus;
}

uint32 ATDeviceSuperSALT::ComputeSerialCyclesPerByte(uint8 bus) const {
	static constexpr int kBaudGeneratorRates[16] {
		2400, 4800, 9600, 19200, 38400, 76800, 19200, 0,
		300, 600, 1200, 2400, 4800, 9600, 2400, 0,
	};

	if (bus & 0x01) {
		return mpSIOMgr->GetCyclesPerBitSend() * 10;
	}

	static constexpr auto kBaudGeneratorCpbs = VDCxArray<uint32, 16>::transform(
		kBaudGeneratorRates,
		[](int baud) -> uint32 {
			// Normally, communication would use 10 bits per byte (1 start +
			// 8 data + 1 stop), but the Test Assembly uses a 4-bit counter.
			return baud ? (uint32)(int)(1789772.5f * 16.0f / (float)baud + 0.5f) : 0;
		}
	);

	return kBaudGeneratorCpbs[bus >> 4];
}

uint32 ATDeviceSuperSALT::ComputeSerialCyclesPerBit(uint8 bus) const {
	static constexpr int kBaudGeneratorRates[16] {
		2400, 4800, 9600, 19200, 38400, 76800, 19200, 0,
		300, 600, 1200, 2400, 4800, 9600, 2400, 0,
	};

	if (bus & 0x01) {
		return mpSIOMgr->GetCyclesPerBitSend();
	}

	static constexpr auto kBaudGeneratorCpbs = VDCxArray<uint32, 16>::transform(
		kBaudGeneratorRates,
		[](int baud) -> uint32 {
			// Normally, communication would use 10 bits per byte (1 start +
			// 8 data + 1 stop), but the Test Assembly uses a 4-bit counter.
			return baud ? (uint32)(int)(1789772.5f / (float)baud + 0.5f) : 0;
		}
	);

	return kBaudGeneratorCpbs[bus >> 4];
}

void ATDeviceSuperSALT::UpdatePortInputs() {
	const bool cmd = mpSIOMgr->IsSIOCommandAsserted();
	uint8 bus = ComputeDirBus();
	const uint8 bus0 = bus;

	if (cmd)
		bus &= mADCValue;

	const bool motor = mpSIOMgr->IsSIOMotorAsserted();
	if (motor && mpSIOMgr->IsSIOForceBreakAsserted()) {
		bus &= (bus << 4) | (bus >> 4);
	}

	mpPorts[0]->SetDirInput(bus & 15);
	mpPorts[2]->SetDirInput(bus & 15);

	mpPorts[1]->SetDirInput(bus >> 4);
	mpPorts[3]->SetDirInput(bus >> 4);

	// - Pot B and trigger inputs are connected to joystick left (bit 2) on
	//   ports 1+2, and joystick up (bit 0) on ports 3+4.
	// - Pot A is connected to joystick right (bit 3) on ports 1+2, and joystick
	//   down (bit 1) on ports 3+4.

	const bool potA1 = (bus & 0x08) != 0;
	const bool potB1 = (bus & 0x04) != 0;
	const bool potA2 = (bus & 0x80) != 0;
	const bool potB2 = (bus & 0x40) != 0;
	const bool potA3 = (bus & 0x02) != 0;
	const bool potB3 = (bus & 0x01) != 0;
	const bool potA4 = (bus & 0x20) != 0;
	const bool potB4 = (bus & 0x10) != 0;

	mpPorts[0]->SetPotHiresPosition(false, potA1 ? 50<<16 : 228<<16, !potA1);
	mpPorts[1]->SetPotHiresPosition(false, potA2 ? 50<<16 : 228<<16, !potA2);
	mpPorts[2]->SetPotHiresPosition(false, potA3 ? 50<<16 : 228<<16, !potA3);
	mpPorts[3]->SetPotHiresPosition(false, potA4 ? 50<<16 : 228<<16, !potA4);

	mpPorts[0]->SetPotHiresPosition(true, potB1 ? 50<<16 : 228<<16, !potB1);
	mpPorts[1]->SetPotHiresPosition(true, potB2 ? 50<<16 : 228<<16, !potB2);
	mpPorts[2]->SetPotHiresPosition(true, potB3 ? 50<<16 : 228<<16, !potB3);
	mpPorts[3]->SetPotHiresPosition(true, potB4 ? 50<<16 : 228<<16, !potB4);

	// This intentionally diverges from the schematic to fix a P1A failure
	// caused by a discrepancy -- the trigger lines are connected to the SIO
	// MOTOR line by a pair of inverters in the actual rev. O board and not
	// the port A data lines per the schematic or technical manual.
	//
	// More info:
	// https://forums.atariage.com/topic/363930-wanted-atari-cps-super-salt-extended-hardware-test-assembly/?do=findComment&comment=5553307

	mpPorts[0]->SetTriggerDown(!motor);
	mpPorts[1]->SetTriggerDown(!motor);
	mpPorts[2]->SetTriggerDown(!motor);
	mpPorts[3]->SetTriggerDown(!motor);

	mbSIOLoopbackEnabled = (bus & 0x02) != 0;
	if (!mbSIOLoopbackEnabled) {
		if (!mpEventSendByte)
			mpScheduler->SetEvent(1, this, 1, mpEventSendByte);
	}

	uint32 cyclesPerByte = ComputeSerialCyclesPerByte(bus);
	mpSIOMgr->SetExternalClock(this, 0, cyclesPerByte >> 4);
	
	SetAudioInputPeriodBaud(ComputeSerialCyclesPerBit(bus));
	SetAudioInputEnabled((bus & 4) && motor);

	g_ATLCSuperSALT("bus update: %02X -> %02X\n", bus0, bus);
}

void ATDeviceSuperSALT::StartADC() {
	uint8 bus = ComputeDirBus();

	static constexpr float kBaseVoltages[16] {
		5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		5.0f,
		12.0f * (10.0f / 24.0f),
		2.5f,
		2.5f,
		0.0f
	};

	const auto voltageToCode = [](float v) -> uint8 {
		int code = (int)((v * 255.0f) / 6.0f + 0.5f);
		return code < 0 ? 0 : code > 255 ? 255 : (uint8)code;
	};

	static constexpr auto kBaseValues = VDCxArray<uint8, 16>::transform(
		kBaseVoltages, voltageToCode
	);

	const int idx = bus & 15;
	uint8 adcVal = kBaseValues[idx];

	// SIO motor needs to be special-cased
	if (idx == 11) {
		adcVal = voltageToCode(mpSIOMgr->IsSIOMotorAsserted() ? 5.0f : 0.0f);
	}
	
	g_ATLCSuperSALT("ADC started: %d -> %02X\n", idx, adcVal);

	if (mADCValue != adcVal) {
		mADCValue = adcVal;

		if (mpSIOMgr->IsSIOCommandAsserted())
			UpdatePortInputs();
	}
}

void ATDeviceSuperSALT::SetAudioInputEnabled(bool enabled) {
	if (mbAudioInputEnabled == enabled)
		return;

	const uint64 t = mpScheduler->GetTick64();

	if (mbAudioInputEnabled)
		FlushAudio(t);

	mLastAudioTickTime = t;
	mbAudioInputEnabled = enabled;

	if (enabled) {
		// we haven't been generating audio, so we have to advance the relevant
		// audio clocks to be within the window again
		if (mAudioPeriodBaud && mLastAudioTimeBaud < t) {
			uint64 delta = t - mLastAudioTimeBaud + mAudioPeriodBaud - 1;

			mLastAudioTimeBaud += delta - delta % mAudioPeriodBaud;

			VDASSERT(mLastAudioTimeBaud >= t);
			VDASSERT(mLastAudioTimeBaud < t + mAudioPeriodBaud);
		}

		if (mAudioPeriodBclk && mLastAudioTimeBclk < t) {
			uint64 delta = t - mLastAudioTimeBclk + mAudioPeriodBclk - 1;

			mLastAudioTimeBclk += delta - delta % mAudioPeriodBclk;

			VDASSERT(mLastAudioTimeBclk >= t);
			VDASSERT(mLastAudioTimeBclk < t + mAudioPeriodBclk);
		}

		if (!mbRegisteredForClockOutChanges) {
			mbRegisteredForClockOutChanges = true;

			mpSIOMgr->SetBiClockNotifyEnabled(this, true);
		}

		OnSerialBiClockChanged(mpSIOMgr->GetCyclesPerBitBiClock());
	} else {
		if (mbRegisteredForClockOutChanges) {
			mbRegisteredForClockOutChanges = false;

			mpSIOMgr->SetBiClockNotifyEnabled(this, false);
		}
	}
}

void ATDeviceSuperSALT::SetAudioInputPeriodBclk(uint32 cycles) {
	if (mAudioPeriodBclk != cycles) {
		const uint64 t = mpScheduler->GetTick64();

		FlushAudio(t);

		if (!mAudioPeriodBclk)
			mLastAudioTimeBclk = t;
		else
			VDASSERT(!mbAudioInputEnabled || mLastAudioTimeBclk >= t);

		mAudioPeriodBclk = cycles;
	}
}

void ATDeviceSuperSALT::SetAudioInputPeriodBaud(uint32 cycles) {
	if (mAudioPeriodBaud != cycles) {
		const uint64 t = mpScheduler->GetTick64();

		FlushAudio(t);

		if (!mAudioPeriodBaud)
			mLastAudioTimeBaud = t;
		else
			VDASSERT(!mbAudioInputEnabled || mLastAudioTimeBaud >= t);

		mAudioPeriodBaud = cycles;
	}
}

void ATDeviceSuperSALT::FlushAudio(uint64 t) {
	if (!mbAudioInputEnabled)
		return;

	// We have to handle both the bidirectional clock and baud generator being active
	// as this is the configuration set up by CPS SuperSALT -- it has been confirmed
	// via scope that the baud generator's 19200 baud output is superimposed on the
	// clock from POKEY during the 2-way clock test. This produces a somewhat more
	// muted tone than would otherwise result from the timer 4 based serial clock.
	//
	// If only one of the clocks is active, we only output the other one. This is
	// realistic for when the bidirectional clock is set to input. It's more questionable
	// when the baud generator is paused as it still has a CMOS output, but the precise
	// behavior is not currently known.

	if (!mAudioPeriodBclk) {
		if (!mAudioPeriodBaud)
			return;

		// only baud rate generator active
		VDASSERT(mLastAudioTimeBaud >= mLastAudioTickTime);

		for(;;) {
			const bool newPolarity = (mAudioOutputBaud & 1) != 0;
			if (mLastAudioPolarity != newPolarity) {
				mLastAudioPolarity = newPolarity;
				mpAudioEdgeBuffer->mEdges.emplace_back((uint32)mLastAudioTickTime, newPolarity ? -1.0f : 1.0f);
			}

			if (mLastAudioTimeBaud >= t)
				break;

			mLastAudioTickTime = mLastAudioTimeBaud;
			mLastAudioTimeBaud += ((mAudioPeriodBaud + mAudioOutputBaud) >> 1);
			mAudioOutputBaud ^= 1;
		}

		VDASSERT(mLastAudioTimeBaud >= mLastAudioTickTime);
	} else if (!mAudioPeriodBaud) {
		// only BCLK active

		VDASSERT(mLastAudioTimeBclk >= mLastAudioTickTime);

		for(;;) {
			const bool newPolarity = (mAudioOutputBclk & 1) != 0;
			if (mLastAudioPolarity != newPolarity) {
				mLastAudioPolarity = newPolarity;
				mpAudioEdgeBuffer->mEdges.emplace_back((uint32)mLastAudioTickTime, newPolarity ? -1.0f : 1.0f);
			}

			if (mLastAudioTimeBclk >= t)
				break;

			mLastAudioTickTime = mLastAudioTimeBclk;
			mLastAudioTimeBclk += ((mAudioPeriodBclk + mAudioOutputBclk) >> 1);
			mAudioOutputBclk ^= 1;
		}

		VDASSERT(mLastAudioTimeBclk >= mLastAudioTickTime);
	} else {
		// both BCLK and baud rate generator active

		VDASSERT(mLastAudioTimeBaud >= mLastAudioTickTime);
		VDASSERT(mLastAudioTimeBclk >= mLastAudioTickTime);

		for(;;) {
			const bool newPolarity = ((mAudioOutputBaud & mAudioOutputBclk) & 1) != 0;
			if (mLastAudioPolarity != newPolarity) {
				mLastAudioPolarity = newPolarity;
				mpAudioEdgeBuffer->mEdges.emplace_back((uint32)mLastAudioTickTime, newPolarity ? -1.0f : 1.0f);
			}

			const uint64 nextBclk = mLastAudioTimeBclk;
			const uint64 nextBaud = mLastAudioTimeBaud;
			const uint64 next = std::min<uint64>(nextBclk, nextBaud);

			if (next >= t)
				break;

			mLastAudioTickTime = next;

			if (nextBclk == next) {
				mLastAudioTimeBclk += ((mAudioPeriodBclk + mAudioOutputBclk) >> 1);
				mAudioOutputBclk ^= 1;
			}

			if (nextBaud == next) {
				mLastAudioTimeBaud += ((mAudioPeriodBaud + mAudioOutputBaud) >> 1);
				mAudioOutputBaud ^= 1;
			}
		}

		VDASSERT(mLastAudioTimeBaud >= mLastAudioTickTime);
		VDASSERT(mLastAudioTimeBclk >= mLastAudioTickTime);
	}

	mLastAudioTickTime = t;
}
