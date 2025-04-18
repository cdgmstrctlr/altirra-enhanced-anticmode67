//	Altirra - Atari 800/800XL/5200 emulator
//	Execution tracing - video recording
//	Copyright (C) 2009-2017 Avery Lee
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

#ifndef f_AT_TRACEVIDEO_H
#define f_AT_TRACEVIDEO_H

#include <vd2/system/refcount.h>

struct VDPixmap;
class IATTraceChannel;
class IATGTIAVideoTap;
class ATTraceMemoryTracker;

class IATTraceChannelVideo : public IVDRefUnknown {
public:
	static constexpr uint32 kTypeID = 'tciv';

	virtual IATTraceChannel *AsTraceChannel() = 0;

	virtual sint32 GetNearestFrameIndex(double startTime, double endTime, double& frameTime) = 0;
	virtual uint64 GetTraceSize() const = 0;
	virtual uint32 GetFrameBufferCount() const = 0;
	virtual sint32 GetFrameBufferIndexForFrame(sint32 frameIdx) = 0;
	virtual double GetTimeForFrame(uint32 frameIdx) = 0;
	virtual const VDPixmap& GetFrameBufferByIndex(uint32 fbIdx) = 0;

	virtual void AddRawFrameBuffer(const VDPixmap& px) = 0;
	virtual void AddFrame(double timestamp, uint32 frameBufferIndex) = 0;
};

class IATVideoTracer : public IVDRefCount {
public:
	virtual IATGTIAVideoTap *AsVideoTap() = 0;

	virtual void Init(IATTraceChannelVideo *dst, uint64 timeOffset, double timeScale, uint32 divisor) = 0;
	virtual void Shutdown() = 0;
};

vdrefptr<IATTraceChannelVideo> ATCreateTraceChannelVideo(const wchar_t *name, ATTraceMemoryTracker *memTracker);
vdrefptr<IATVideoTracer> ATCreateVideoTracer();

#endif	// f_AT_TRACEVIDEO_H
