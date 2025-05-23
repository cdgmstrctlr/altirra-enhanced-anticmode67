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

#ifndef f_AT_XEP80_H
#define f_AT_XEP80_H

#include <vd2/system/vectors.h>
#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmaputils.h>
#include <at/atcore/scheduler.h>

class ATPIAEmulator;
class IATDevicePortManager;
class IATObjectState;
template<typename T> class vdrefptr;

struct ATXEP80TextDisplayInfo {
	int mColumns;
	int mRows;
};

class ATXEP80Emulator final : public IATSchedulerCallback {
	ATXEP80Emulator(const ATXEP80Emulator&);
	ATXEP80Emulator& operator=(const ATXEP80Emulator&);
public:
	ATXEP80Emulator();
	~ATXEP80Emulator();

	void Init(ATScheduler *sched, IATDevicePortManager *pia);
	void Shutdown();

	void ColdReset();

	void SoftReset();

	void InitFonts();
	
	void SetPortIndex(uint8 portIndex);

	void SetOnPrinterOutput(vdfunction<void(uint8)> fn);

	bool IsVideoSignalValid() const { return mbValidSignal; }
	float GetVideoHorzRate() const { return mHorzRate; }
	float GetVideoVertRate() const { return mVertRate; }
	const VDPixmap& GetFrameBuffer() const { return mFrame; }

	void Tick(uint32 ticks300Hz);
	void UpdateFrame();

	uint32 GetFrameLayoutChangeCount();
	uint32 GetFrameChangeCount() const;
	const vdrect32 GetDisplayArea() const;
	double GetPixelAspectRatio() const;

	uint32 GetDataReceivedCount();

	const ATXEP80TextDisplayInfo GetTextDisplayInfo() const;
	const vdpoint32 PixelToCaretPos(const vdpoint32& pixelPos) const;
	const vdrect32 CharToPixelRect(const vdrect32& r) const;
	int ReadRawText(uint8 *dst, int x, int y, int n) const;

	void LoadState(const IATObjectState *state);
	vdrefptr<IATObjectState> SaveState() const;

public:
	void OnScheduledEvent(uint32 id) override;

private:
	struct CommandInfo;

	void OnPIAOutputChanged(uint32 outputState);
	
	static const CommandInfo *LookupCommand(uint8 ch);

	void OnReceiveByte(uint32 ch);
	void SendCursor(uint8 offset, uint32 delay = 1);
	void BeginWrite(uint8 len, uint32 delay = 1);

	void OnChar(uint8);
	void OnCmdSetCursorHPos(uint8);
	void OnCmdSetCursorHPosHi(uint8);
	void OnCmdSetLeftMarginLo(uint8);
	void OnCmdSetLeftMarginHi(uint8);
	void OnCmdSetCursorVPos(uint8);
	void OnCmdSetGraphics(uint8);
	void OnCmdModifyGraphics50Hz(uint8);
	void OnCmdSetRightMarginLo(uint8);
	void OnCmdSetRightMarginHi(uint8);
	void OnCmdReadCharAndAdvance(uint8);
	void OnCmdRequestCursorHPos(uint8);
	void OnCmdMasterReset(uint8);
	void OnCmdPrinterPortStatus(uint8);
	void OnCmdFillPrevChar(uint8);
	void OnCmdFillSpace(uint8);
	void OnCmdFillEOL(uint8);
	void OnCmdReadChar(uint8);
	void OnCmdReadTimerCounter(uint8);
	void OnCmdClearListFlag(uint8);
	void OnCmdSetListFlag(uint8);
	void OnCmdSetNormalMode(uint8);
	void OnCmdSetBurstMode(uint8);
	void OnCmdSetCharSet(uint8);
	void OnCmdSetText50Hz(uint8);
	void OnCmdCursorOff(uint8);
	void OnCmdCursorOn(uint8);
	void OnCmdCursorOnBlink(uint8);
	void OnCmdMoveToLogicalStart(uint8);
	void OnCmdSetScrollX(uint8);
	void OnCmdSetPrinterOutput(uint8);
	void OnCmdSetReverseVideo(uint8);
	void OnCmdSetExtraByte(uint8);
	void OnCmdWriteCursor(uint8);
	void OnCmdSetCursorAddr(uint8);
	void OnCmdWriteByte(uint8);
	void OnCmdWriteInternalByte(uint8);
	void OnCmdSetHomeAddr(uint8);
	void OnCmdWriteVCR(uint8);
	void OnCmdSetTCP(uint8);
	void OnCmdWriteTCP(uint8);
	void WriteTimingChain(uint8 reg, uint8 val);
	void OnCmdSetBeginAddr(uint8);
	void OnCmdSetEndAddr(uint8);
	void OnCmdSetStatusAddr(uint8);
	void OnCmdSetAttrLatch(uint8 ch);
	void OnCmdSetBaudRate(uint8);
	void OnCmdSetUMX(uint8);

	void Clear();
	void ClearLine(int y);
	void InsertChar();
	void DeleteChar();
	void InsertLine();
	void DeleteLine();
	void Advance(bool extendLine);
	void Scroll();
	void UpdateCursorAddr();
	void InvalidateCursor();
	void InvalidateFrame();

	void RebuildBlockGraphics();
	void RebuildActiveFont();
	void RecomputeBaudRate();
	void RecomputeVideoTiming();
	void UpdatePIABits();
	void UpdatePIAInput();

	uint8 *GetRowPtr(int y) { return &mVRAM[(mRowPtrs[y] & 0x1F) << 8]; }

	enum : uint32 {
		kEventId_ReadBit = 1,
		kEventId_WriteBit = 2
	};

	enum CommandState {
		kState_WaitCommand,
		kState_ReturningData
	};

	CommandState mCommandState;
	int mReadBitState;
	int mWriteBitState;
	uint32 mCurrentData;
	uint32 mCurrentWriteData;
	uint16 mWriteBuffer[3];
	uint8 mWriteIndex;
	uint8 mWriteLength;
	uint8 mScrollX;
	uint8 mX;
	uint8 mY;
	uint16 mBeginAddr;			// BEGD register
	uint16 mEndAddr;			// ENDD register
	uint16 mHomeAddr;			// HOME register
	uint16 mCursorAddr;			// CURS register
	uint16 mStatusAddr;			// SROW register
	uint8 mLastX;
	uint8 mLastY;
	uint8 mLastChar;
	uint8 mLeftMargin;
	uint8 mRightMargin;
	uint8 mAttrA;
	uint8 mAttrB;
	uint8 mExtraByte;
	bool mbEscape;
	bool mbDisplayControl;
	bool mbBurstMode;
	bool mbGraphicsMode;
	bool mbInternalCharset;
	bool mbPrinterMode;
	bool mbCursorEnabled;
	bool mbCursorBlinkEnabled;
	bool mbCursorBlinkState;
	bool mbCursorReverseVideo;
	bool mbCharBlinkState;
	bool mbReverseVideo;
	bool mbReverseVideoBlinkField;
	bool mbPAL;
	uint8 mTickAccum;
	uint16 mBlinkAccum;
	uint8 mBlinkRate;
	uint8 mBlinkDutyCycle;
	uint8 mUnderlineStart;
	uint8 mUnderlineEnd;

	uint8 mUARTPrescale = 0;
	uint8 mUARTBaud = 0;
	uint8 mUARTMultiplex = 0;
	uint32 mXmitTimingAccum = 0;
	uint32 mCyclesPerBitXmitX256 = 0;
	uint32 mCyclesPerBitRecv = 0;

	uint8 mCharWidth = 0;
	uint8 mCharHeight = 0;
	uint8 mGfxColumns = 0;
	uint8 mGfxRowMid = 0;
	uint8 mGfxRowBot = 0;
	uint8 mTCP = 0;

	uint8 mHorzCount;
	uint8 mHorzBlankStart;
	uint8 mHorzSyncStart;
	uint8 mHorzSyncEnd;
	uint8 mVertCount;
	uint8 mVertBlankStart;
	uint8 mVertSyncBegin;
	uint8 mVertSyncEnd;
	uint8 mVertStatusRow;		// Timing chain register 8. Note that this is the last row before the status row.
	uint8 mVertExtraScans;
	float mHorzRate;
	float mVertRate;
	bool mbValidSignal;

	bool mbInvalidBlockGraphics;
	bool mbInvalidActiveFont;

	uint16 mPrefetchLFSR = 0x41;
	
	IATDevicePortManager *mpPIA = nullptr;
	int mPIAInput;
	int mPIAOutput;
	uint32 mPIAInputBit;		// XEP80 -> Computer
	uint32 mPIAOutputBit;		// Computer -> XEP80
	uint8 mPortIndex;

	ATScheduler *mpScheduler = nullptr;
	ATEvent *mpReadBitEvent = nullptr;
	ATEvent *mpWriteBitEvent = nullptr;

	uint32 mFrameLayoutChangeCount;
	uint32 mFrameChangeCount;
	uint32 mDataReceivedCount;

	vdfunction<void(uint8)> mpOnPrinterOutput;

	VDPixmapBuffer mFrame;

	uint8 mRowPtrs[25];

	uint8 mVRAM[8192];

	// Fonts by character, for three different modes:
	//
	//  A14=0, A13=0: Normal character set
	//  A14=0, A13=1: International character set
	//  A14=1:        Internal character set
	//
	// We pregen all of these at once because it is possible to
	// mix them by writing directly to row addresses.
	//
	// Within each character set, there are three subarrays, one for normal
	// mode, and another two for left and right halves of double width.
	//
	// The subarrays are in turn indexed by VRAM byte and then line (0-15).
	// Each 128 character half is generated according to the current attribute
	// latches.
	//
	uint16 mActiveFonts[3][3*256*16];

	// Source fonts: normal, int'l, internal, and block. The block graphics font
	// is dynamically updated based on graphics settings in the timing chain.
	uint16 mFonts[4][256*16];
	uint32 mPalette[256];

	static const CommandInfo kCommands[];
};

#endif
