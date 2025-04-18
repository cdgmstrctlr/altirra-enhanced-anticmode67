//	Altirra - Atari 800/800XL/5200 emulator
//	Coprocessor library - Z80 state machine
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

///////////////////////////////////////////////////////////////////////////
// References:
//	Z80 Flag Affection
//	http://www.z80.info/z80sflag.htm
//
//	Z80 Documented
//

#define AT_CPU_READ_BYTE(addr) ATCP_READ_BYTE((addr))
#define AT_CPU_READ_BYTE_ADDR16(addr) ATCP_READ_BYTE((addr))
#define AT_CPU_READ_BYTE_HL(addrhi, addrlo) ATCP_READ_BYTE(((uint32)(uint8)(addrhi) << 8) + (uint8)(addrlo))
#define AT_CPU_WRITE_BYTE(addr, value) ATCP_WRITE_BYTE((addr), (value))
#define AT_CPU_WRITE_BYTE_HL(addrhi, addrlo, value) ATCP_WRITE_BYTE(((uint32)(uint8)(addrhi) << 8) + (uint8)(addrlo), (value))

#define INSN_FETCH() AT_CPU_READ_BYTE(mPC); ++mPC
#define INSN_FETCH_TO(dest) readData = AT_CPU_READ_BYTE(mPC); ++mPC; (dest) = readData
#define INSN_FETCH_NOINC() AT_CPU_READ_BYTE(mPC)

#define AT_SET_PARITY(val) if ((0x9669U >> ((val ^ (val >> 4)) & 15)) & 1) mF |= 0x04

while(mCyclesLeft) {
	uint8 readData;

	if (mTStatesLeft) {
		--mTStatesLeft;
		--mCyclesLeft;
		continue;
	}

	switch(*mpNextState++) {

		case kZ80StateNop:
			break;

		case kZ80StateReadOpcode:
			if (mpBreakpointMap[mPC]) {
				bool shouldExit = CheckBreakpoint();

				if (shouldExit) {
					goto force_exit;
				}
			}
		case kZ80StateReadOpcodeNoBreak:
			if (mbIntActionNeeded) {
				mbIntActionNeeded = false;

				if (mbEiPending) {
					mbEiPending = false;
					mbIntActionNeeded = mbIrqPending;
				} else if (mbIrqPending && mbIFF1 && !mbNmiPending) {
					mpNextState = mDecoderTables.mDecodeHeap + mDecoderTables.mIrqSequence;
					mbMarkHistoryIrq = true;
					break;
				}

				if (mbNmiPending) {
					mbNmiPending = false;

					mpNextState = mDecoderTables.mDecodeHeap + mDecoderTables.mNmiSequence;
					mbMarkHistoryNmi = true;
					mbIFF1 = false;
					mbIntActionNeeded = false;
					break;
				}
			}

			mInsnPC = mPC;

			{
				uint8 opcode;
				INSN_FETCH_TO(opcode);
				//VDDEBUG("Executing opcode AF=%02X%02X BC=%02X%02X DE=%02X%02x HL=%02X%02X %04X: %02X\n", mA, mF, mB, mC, mD, mE, mH, mL, mInsnPC, opcode);
				mpNextState = mDecoderTables.mDecodeHeap + mDecoderTables.mInsns[opcode];
				++mR;
			}
			break;

		case kZ80StateReadOpcodeCB:
			{
				uint8 opcode;
				INSN_FETCH_TO(opcode);
				//VDDEBUG("Executing opcode %04X: CB %02X\n", mInsnPC, opcode);
				mpNextState = mDecoderTables.mDecodeHeap + mDecoderTables.mInsnsCB[opcode];
			}
			break;

		case kZ80StateReadOpcodeED:
			{
				uint8 opcode;
				INSN_FETCH_TO(opcode);
				//VDDEBUG("Executing opcode %04X: ED %02X\n", mInsnPC, opcode);
				mpNextState = mDecoderTables.mDecodeHeap + mDecoderTables.mInsnsED[opcode];
			}
			break;

		case kZ80StateReadOpcodeDD:
			{
				uint8 opcode;
				INSN_FETCH_TO(opcode);
				//VDDEBUG("Executing opcode %04X: DD %02X\n", mInsnPC, opcode);
				mbUseIY = false;
				mpNextState = mDecoderTables.mDecodeHeap + mDecoderTables.mInsnsDDFD[opcode];
			}
			break;

		case kZ80StateReadOpcodeFD:
			{
				uint8 opcode;
				INSN_FETCH_TO(opcode);
				//VDDEBUG("Executing opcode %04X: FD %02X\n", mInsnPC, opcode);
				mbUseIY = true;
				mpNextState = mDecoderTables.mDecodeHeap + mDecoderTables.mInsnsDDFD[opcode];
			}
			break;

		case kZ80StateReadOpcodeDDFDCB:
			{
				uint8 opcode;
				INSN_FETCH_TO(opcode);
				mpNextState = mDecoderTables.mDecodeHeap + mDecoderTables.mInsnsDDFDCB[opcode];
			}
			break;

		case kZ80StateAddToHistory:
			{
				ATCPUHistoryEntry * VDRESTRICT he = &mpHistory[mHistoryIndex++ & 131071];

				he->mCycle = mCyclesBase - mCyclesLeft;
				he->mUnhaltedCycle = mCyclesBase - mCyclesLeft;
				he->mEA = 0xFFFFFFFFUL;
				he->mPC = mPC - 1;
				he->mZ80_A = mA;
				he->mZ80_F = mF;
				he->mZ80_B = mB;
				he->mZ80_C = mC;
				he->mbIRQ = mbMarkHistoryIrq;
				mbMarkHistoryIrq = false;
				he->mbNMI = mbMarkHistoryNmi;
				mbMarkHistoryNmi = false;
				he->mSubCycle = 0;
				he->mbEmulation = true;
				he->mZ80_D = mD;
				he->mExt.mZ80_E = mE;
				he->mExt.mZ80_H = mH;
				he->mExt.mZ80_L = mL;
				he->mB = 0;
				he->mK = 0;
				he->mZ80_SP = mSP;

				for(int i=0; i<4; ++i) {
					uint16 pc = mPC - 1 + i;
					he->mOpcode[i] = ATCP_DEBUG_READ_BYTE(pc);
				}
			}
			break;

		case kZ80StateBreakOnUnsupportedOpcode:
			mTStatesLeft = 1;
			--mpNextState;
			break;

		case kZ80StateRegenerateDecodeTables:
			mpNextState = RegenerateDecodeTables();
			break;

		case kZ80StateWait_1T: mTStatesLeft = 1; break;
		case kZ80StateWait_2T: mTStatesLeft = 2; break;
		case kZ80StateWait_3T: mTStatesLeft = 3; break;
		case kZ80StateWait_4T: mTStatesLeft = 4; break;
		case kZ80StateWait_5T: mTStatesLeft = 5; break;
		case kZ80StateWait_7T: mTStatesLeft = 7; break;
		case kZ80StateWait_8T: mTStatesLeft = 8; break;
		case kZ80StateWait_11T: mTStatesLeft = 11; break;

		case kZ80StateReadImm:
			mDataL = mDataH;
			mDataH = ATCP_READ_BYTE(mPC);
			++mPC;
			break;

		case kZ80StateReadImmAddr:
			mAddr >>= 8;
			mAddr += (uint32)ATCP_READ_BYTE(mPC) << 8;
			++mPC;
			break;

		case kZ80StateRead:
			mDataH = ATCP_READ_BYTE(mAddr);
			break;

		case kZ80StateReadL:
			mDataL = ATCP_READ_BYTE(mAddr);
			break;

		case kZ80StateReadH:
			mDataH = ATCP_READ_BYTE(mAddr+1);
			break;

		case kZ80StateReadIXdToAddr:
			if (mbUseIY)
				mAddr = mIYL + ((uint32)mIYH << 8);
			else
				mAddr = mIXL + ((uint32)mIXH << 8);

			mAddr += (sint16)(sint8)ATCP_READ_BYTE(mPC);
			++mPC;
			break;

		case kZ80StateReadLD:		// read (HL), set addr to DE, set bits 3/5 from bits 3/1 of A+(HL)
			{
				mDataH = ATCP_READ_BYTE(mL + ((uint32)mH << 8));
				uint8 temp = mDataH + mA;

				mF &= 0xD7;
				mF |= (temp & 0x08);
				mF |= (temp & 0x02) << 4;

				mAddr = mE + ((uint32)mD << 8);
			}
			break;

		case kZ80StateWrite:
			ATCP_WRITE_BYTE(mAddr, mDataH);
			break;

		case kZ80StateWriteL:
			ATCP_WRITE_BYTE(mAddr, mDataL);
			break;

		case kZ80StateWriteH:
			ATCP_WRITE_BYTE(mAddr+1, mDataH);
			break;

		case kZ80StateReadPort:
			mDataH = mpFnReadPort((uint8)(mAddr >> 8));
			break;

		case kZ80StateReadPortC:
			mDataH = mpFnReadPort(mC);
			break;

		case kZ80StateWritePort:
			mpFnWritePort((uint8)(mAddr >> 8), mDataH);
			break;

		case kZ80StateWritePortC:
			mpFnWritePort(mC, mDataH);
			break;

		case kZ80StatePush:
			--mSP;
			ATCP_WRITE_BYTE(mSP, mDataH);
			mDataH = mDataL;
			break;

		case kZ80StatePop:
			mDataL = mDataH;
			mDataH = ATCP_READ_BYTE(mSP);
			++mSP;
			break;

		case kZ80State0ToData: mDataH = 0; break;
		case kZ80StateAToData: mDataH = mA; break;
		case kZ80StateBToData: mDataH = mB; break;
		case kZ80StateCToData: mDataH = mC; break;
		case kZ80StateDToData: mDataH = mD; break;
		case kZ80StateEToData: mDataH = mE; break;
		case kZ80StateHToData: mDataH = mH; break;
		case kZ80StateLToData: mDataH = mL; break;
		case kZ80StateIXHToData: mDataH = mbUseIY ? mIYH : mIXH; break;
		case kZ80StateIXLToData: mDataH = mbUseIY ? mIYL : mIXL; break;

		case kZ80StateDataToA: mA = mDataH;	break;
		case kZ80StateDataToB: mB = mDataH;	break;
		case kZ80StateDataToC: mC = mDataH;	break;
		case kZ80StateDataToD: mD = mDataH;	break;
		case kZ80StateDataToE: mE = mDataH;	break;
		case kZ80StateDataToH: mH = mDataH;	break;
		case kZ80StateDataToL: mL = mDataH;	break;
		case kZ80StateDataToIXH: (mbUseIY ? mIYH : mIXH) = mDataH;	break;
		case kZ80StateDataToIXL: (mbUseIY ? mIYL : mIXL) = mDataH;	break;

		case kZ80StateAFToData:	mDataL = mF; mDataH = mA; break;
		case kZ80StateBCToData:	mDataL = mC; mDataH = mB; break;
		case kZ80StateDEToData:	mDataL = mE; mDataH = mD; break;
		case kZ80StateHLToData:	mDataL = mL; mDataH = mH; break;
		case kZ80StateIXToData:
			if (mbUseIY) {
				mDataL = mIYL;
				mDataH = mIYH;
			} else {
				mDataL = mIXL;
				mDataH = mIXH;
			}
			break;
		case kZ80StateSPToData:	mDataL = (uint8)mSP; mDataH = (uint8)(mSP >> 8); break;
		case kZ80StatePCToData:	mDataL = (uint8)mPC; mDataH = (uint8)(mPC >> 8); break;
		case kZ80StatePCp2ToData:	mDataL = (uint8)(mPC+2); mDataH = (uint8)((mPC + 2) >> 8); break;

		case kZ80StateBCToAddr:	mAddr = mC + ((uint32)mB << 8); break;
		case kZ80StateDEToAddr:	mAddr = mE + ((uint32)mD << 8); break;
		case kZ80StateHLToAddr:	mAddr = mL + ((uint32)mH << 8); break;
		case kZ80StateSPToAddr:	mAddr = mSP; break;

		case kZ80StateDataToAF: mA = mDataH; mF = mDataL; break;
		case kZ80StateDataToBC: mB = mDataH; mC = mDataL; break;
		case kZ80StateDataToDE: mD = mDataH; mE = mDataL; break;
		case kZ80StateDataToHL: mH = mDataH; mL = mDataL; break;
		case kZ80StateDataToIX:
			if (mbUseIY) {
				mIYH = mDataH;
				mIYL = mDataL;
			} else {
				mIXH = mDataH;
				mIXL = mDataL;
			}
			break;
		case kZ80StateDataToSP: mSP = (uint16)(mDataL + ((uint32)mDataH << 8)); break;
		case kZ80StateDataToPC: mPC = (uint16)(mDataL + ((uint32)mDataH << 8)); break;

		case kZ80StateAddrToPC: mPC = mAddr; break;

		case kZ80StateIToA_1T:
			mA = mI;
			mTStatesLeft = 1;
			mF &= 0xFB;
			if (mbIFF2)
				mF |= 0x04;
			break;

		case kZ80StateRToA_1T:
			mA = mR;
			mTStatesLeft = 1;
			mF &= 0xFB;
			if (mbIFF2)
				mF |= 0x04;
			break;

		case kZ80StateAToI_1T:
			mI = mA;
			mTStatesLeft = 1;
			break;

		case kZ80StateAToR_1T:
			mR = mA;
			mTStatesLeft = 1;
			break;

		case kZ80StateExaf:
			{
				uint8 t;
				t = mA; mA = mAltA; mAltA = t;
				t = mF; mF = mAltF; mAltF = t;
			}
			break;

		case kZ80StateExx:
			{
				uint8 t;
				t = mB; mB = mAltB; mAltB = t;
				t = mC; mC = mAltC; mAltC = t;
				t = mD; mD = mAltD; mAltD = t;
				t = mE; mE = mAltE; mAltE = t;
				t = mH; mH = mAltH; mAltH = t;
				t = mL; mL = mAltL; mAltL = t;
			}
			break;

		case kZ80StateExDEHL:
			{
				uint8 t;
				t = mD; mD = mH; mH = t;
				t = mE; mE = mL; mL = t;
			}
			break;

		case kZ80StateExHLData:
			{
				uint8 t;
				t = mDataH; mDataH = mH; mH = t;
				t = mDataL; mDataL = mL; mL = t;
			}
			break;

		case kZ80StateExIXData:
			if (mbUseIY) {
				uint8 t;
				t = mDataH; mDataH = mIYH; mIYH = t;
				t = mDataL; mDataL = mIYL; mIYL = t;
			} else {
				uint8 t;
				t = mDataH; mDataH = mIXH; mIXH = t;
				t = mDataL; mDataL = mIXL; mIXL = t;
			}
			break;

		case kZ80StateAddToA:	// SZHVNC changed
			{
				const uint8 carry7 = (uint8)((mA & 0x7f) + (mDataH & 0x7f));
				const uint32 r = (uint32)mA + mDataH;

				// copy S, bits 3 and 5 from result
				mF = r & 0xA8;

				if (!(uint8)r)
					mF |= 0x40;
				mF |= (mA ^ mDataH ^ r) & 0x10;
				mF |= ((r >> 6) ^ (carry7 >> 5)) & 0x4;
				mF |= (r >> 8) & 0x01;

				mA = (uint8)r;
			}
			break;

		case kZ80StateAdcToA:	// SZHVNC changed
			{
				const uint8 carry7 = (uint8)((mA & 0x7f) + (mDataH & 0x7f) + (mF & 1));
				const uint32 r = (uint32)mA + mDataH + (mF & 1);

				// copy S, bits 3 and 5 from result
				mF = r & 0xA8;

				if (!(uint8)r)
					mF |= 0x40;
				mF |= (mA ^ mDataH ^ r) & 0x10;
				mF |= ((r >> 6) ^ (carry7 >> 5)) & 0x4;
				mF |= (r >> 8) & 0x01;

				mA = (uint8)r;
			}
			break;

		case kZ80StateSubToA:	// SZHVNC changed
			{
				const uint8 carry7 = (uint8)((mA & 0x7f) + (~mDataH & 0x7f) + 1);
				const uint32 r = (uint32)mA + (uint8)~mDataH + 1;

				// copy S, bits 3 and 5 from result
				mF = r & 0xA8;

				if (!(uint8)r)
					mF |= 0x40;
				mF |= (mA ^ mDataH ^ r) & 0x10;
				mF |= ((r >> 6) ^ (carry7 >> 5)) & 0x4;
				mF |= ~(r >> 8) & 0x01;
				mF |= 0x02;

				mA = (uint8)r;
			}
			break;

		case kZ80StateSbcToA:	// SZHVNC changed
			{
				const uint8 carry7 = (uint8)((mA & 0x7F) + (~mDataH & 0x7F) + (~mF & 1));
				const uint32 r = (uint32)mA + (uint8)~mDataH + (~mF & 1);

				// copy S, bits 3 and 5 from result
				mF = r & 0xA8;

				if (!(uint8)r)
					mF |= 0x40;
				mF |= (mA ^ mDataH ^ r) & 0x10;
				mF |= ((r >> 6) ^ (carry7 >> 5)) & 0x4;
				mF |= ~(r >> 8) & 0x01;
				mF |= 0x02;

				mA = (uint8)r;
			}
			break;

		case kZ80StateCpToA:	// SZHVNC changed
			{
				const uint8 carry7 = (uint8)((mA & 0x7F) + (~mDataH & 0x7F) + 1);
				const uint32 r = (uint32)mA + (uint8)~mDataH + 1;

				// flags bits 3 and 5 copied from operand (NOT result)
				mF = mDataH & 0x28;

				// copy S from result
				mF |= (r & 0x80);

				// set Z if zero
				if (!(uint8)r)
					mF |= 0x40;

				// set H if borrow from bit 4
				mF |= (mA ^ mDataH ^ r) & 0x10;

				// set V if carries out of bits 6/7 different
				mF |= ((r >> 6) ^ (carry7 >> 5)) & 0x4;

				// set C if borrow (inverse of sum bit 8)
				mF |= ~(r >> 8) & 0x01;

				// set N
				mF |= 0x02;
			}
			break;

		case kZ80StateCpToA2:	// SZHVN changed
			{
				const uint8 carry7 = (uint8)((mA & 0x7F) + (~mDataH & 0x7F) + 1);
				const uint32 r = (uint32)mA + (uint8)~mDataH + 1;

				// preserve only C
				mF &= 0x01;

				// copy S from result
				mF |= (r & 0x80);

				// set Z if zero
				if (!(uint8)r)
					mF |= 0x40;

				// set H if borrow from bit 4
				mF |= (mA ^ mDataH ^ r) & 0x10;

				// set V if carries out of bits 6/7 different
				mF |= ((r >> 6) ^ (carry7 >> 5)) & 0x4;

				// set N
				mF |= 0x02;

				// compute altered undocumented flags (see Z80 Documented p.16)
				const uint8 n = r - ((mF & 0x10) >> 4);

				mF |= (n & 0x08);
				mF |= (n & 0x02) << 4;
			}
			break;

		case kZ80StateDec:		// SZHVN changed, C unaffected
			--mDataH;
			mF &= 0x01;

			// flag bits 3 and 5 copied from result, as well as S
			mF |= (mDataH & 0xA8);
			if (!mDataH)
				mF |= 0x40;
			if ((mDataH & 0x0F) == 0x0F)
				mF |= 0x10;
			if (mDataH == 0x7F)
				mF |= 0x04;
			mF |= 0x02;
			break;

		case kZ80StateInc:		// SZHVN changed, C unaffected
			++mDataH;
			mF &= 0x01;

			// flag bits 3 and 5 copied from result, as well as S
			mF |= (mDataH & 0xA8);

			if (!mDataH)
				mF |= 0x40;
			if ((mDataH & 0x0F) == 0)
				mF |= 0x10;
			if (mDataH == 0x80)
				mF |= 0x04;
			break;

		case kZ80StateAndToA:	// SZP changed, H set, NC cleared, bits 3/5 copied from result
			mA &= mDataH;
			mF = mA & 0xA8;
			mF |= 0x10;
			if (!mA)
				mF |= 0x40;

			AT_SET_PARITY(mA);
			break;

		case kZ80StateOrToA:	// SZP changed, HNC cleared, bits 3/5 copied from result
			mA |= mDataH;
			mF = mA & 0xA8;
			if (!mA)
				mF |= 0x40;

			AT_SET_PARITY(mA);
			break;

		case kZ80StateXorToA:	// SZP changed, HNC cleared, bits 3/5 copied from result
			mA ^= mDataH;
			mF = mA & 0xA8;
			if (!mA)
				mF |= 0x40;

			AT_SET_PARITY(mA);
			break;

		case kZ80StateDec16:
			if (!mDataL--)
				--mDataH;
			break;

		case kZ80StateInc16:
			if (!++mDataL)
				++mDataH;
			break;

		case kZ80StateRlca:		// C changed, NH reset, bits 5/3 from result
			mA = (mA << 1) + (mA >> 7);
			mF &= 0xC4;
			mF |= (mA & 0x29);
			break;

		case kZ80StateRla:		// C changed, NH reset, bits 5/3 from result
			{
				const uint8 r = (mA << 1) + (mF & 1);

				mF &= 0xC4;
				mF |= (mA >> 7);
				mF |= (r & 0x28);

				mA = r;
			}
			break;

		case kZ80StateRrca:		// C changed, NH reset, bits 5/3 from result
			mF &= 0xC4;
			mF |= (mA & 1);
			mA = (mA >> 1) + (mA << 7);
			mF |= (mA & 0x28);
			break;

		case kZ80StateRra:		// C changed, NH reset, bits 5/3 from result
			{
				const uint8 r = (mA >> 1) + (mF << 7);

				mF &= 0xC4;
				mF |= (mA & 1);
				mF |= (r & 0x28);

				mA = r;
			}
			break;

		case kZ80StateRld:		// SZP changed, H cleared, bits 5/3 from result
			{
				uint8 a = mA;

				mA = (a & 0xF0) + (mDataH >> 4);
				mDataH = (a & 0x0F) + (mDataH << 4);

				mF &= 0x01;
				mF |= (mA & 0xA8);
				if (!mA)
					mF |= 0x40;
				AT_SET_PARITY(mA);
			}
			break;

		case kZ80StateRrd:	// SZP set, HN cleared, bits 3/5 from result
			{
				uint8 a = mA;

				mA = (a & 0xF0) + (mDataH & 0x0F);
				mDataH = ((a & 0x0F) << 4) + (mDataH >> 4);

				mF &= 0x01;
				mF |= (mA & 0xA8);
				if (!mA)
					mF |= 0x40;
				AT_SET_PARITY(mA);
			}
			break;

		case kZ80StateRlc:		// SZPC changed, NH reset, bits 3/5 from result
			mDataH = (mDataH << 1) + (mDataH >> 7);
			mF = mDataH & 0xA9;
			if (!mDataH)
				mF |= 0x40;
			AT_SET_PARITY(mDataH);
			break;

		case kZ80StateRl:		// SZPC changed, NH reset, bits 3/5 from result
			{
				const uint8 r = (mDataH << 1) + (mF & 1);

				mF = r & 0xA8;
				mF |= (mDataH >> 7);
				if (!r)
					mF |= 0x40;

				AT_SET_PARITY(r);
				mDataH = r;
			}
			break;

		case kZ80StateRrc:		// SZPC changed, NH reset, bits 3/5 from result
			{
				const uint8 r = (mDataH >> 1) + (mDataH << 7);

				mF = r & 0xA8;
				mF |= (mDataH & 1);
				if (!r)
					mF |= 0x40;

				AT_SET_PARITY(r);
				mDataH = r;
			}
			break;

		case kZ80StateRr:		// SZPC changed, NH reset, bits 3/5 from result
			{
				const uint8 r = (mDataH >> 1) + (mF << 7);

				mF = r & 0xA8;
				mF |= (mDataH & 1);

				if (!r)
					mF |= 0x40;

				AT_SET_PARITY(r);
				mDataH = r;
			}
			break;

		case kZ80StateSll:		// SZPC changed, HN reset, bits 3/5 from result
			mF = (mDataH >> 7);
			mDataH = (mDataH << 1) + 1;
			if (!mDataH)
				mF |= 0x40;
			mF |= mDataH & 0xA8;
			AT_SET_PARITY(mDataH);
			break;

		case kZ80StateSla:		// SZPC changed, HN reset, bits 3/5 from result
			mF = (mDataH >> 7);
			mDataH <<= 1;
			if (!mDataH)
				mF |= 0x40;
			mF |= mDataH & 0xA8;
			AT_SET_PARITY(mDataH);
			break;

		case kZ80StateSrl:		// SZPC changed, HN reset, bits 3/5 from result
			mF = (mDataH & 1);
			mDataH >>= 1;
			if (!mDataH)
				mF |= 0x40;
			mF |= mDataH & 0x28;
			AT_SET_PARITY(mDataH);
			break;

		case kZ80StateSra:		// SZPC changed, HN reset, bits 3/5 from result
			mF = (mDataH & 0x81);
			mDataH = (mDataH & 0x80) + (mDataH >> 1);
			if (!mDataH)
				mF |= 0x40;
			mF |= mDataH & 0x28;
			AT_SET_PARITY(mDataH);
			break;

		case kZ80StateCplToA:	// HN set, bits 3/5 from result
			mA = ~mA;
			mF &= 0xC5;
			mF |= (mA & 0x28);
			mF |= 0x12;
			break;

		case kZ80StateNegA:		// SZHVC changed, N set, bits 3/5 from result
			mA = (uint8)(0-mA);
			mF = mA & 0xA8;
			if (!mA)				// set ZN, clear HVC
				mF |= 0x42;
			else if (mA == 0x80)	// set VNC, clear ZH
				mF |= 0x07;
			else if (mA & 0x0F)		// set HNC, clear ZV
				mF |= 0x13;
			else					// set NC, clear ZHV
				mF |= 0x03;
			break;

		case kZ80StateDaa:		// SZHPC changed, bits 3/5 from result
			{
				uint8 prevF = mF;

				// Rules for C and H -> C' and H', based on the public official:
				// and unofficial tables:
				//
				// - H indicates binary carry or borrow from bit 3 to bit 4.
				// - C indicates binary carry or borrow from bit 7 to bit 8.
				//
				// - H' is set whenever the low digit correction produces a
				//   carry or borrow from the high nibble. It does NOT get
				//   forced set to preserve the BCD carry/borrow state between
				//   nibbles. H' is sensitive to the N state since that changes
				//   the low correction applied.
				//
				// - C' is forced set if C was set even if no carry/borrow
				//   occurs during correction. This ensures that the BCD
				//   carry/borrow state is correct.
				//
				// - C'=1 with C=0 if there is a carry out according to
				//   addition correction rules, ignoring N. This does not affect
				//   normal operation since a BCD subtraction would never
				//   produce an invalid BCD byte >99h without C=1, but it
				//   does affect operation for invalid BCD+carry inputs.
				//
				// - The correction factors for both nibbles are computed the
				//   same way regardless of N. However, correction is subtracted
				//   if N=1, and this affects the computation of H.

				// preserve N, C (if there was a carry/borrow, there will be still be one after)
				mF &= 0x03;

				uint8 locorr = 0;
				uint8 hicorr = 0;

				// low correction occurs if there was a half carry/borrow or result nibble was
				// invalid
				if ((prevF & 0x10) || (mA & 0x0F) >= 0x0A)
					locorr = 0x06;

				// high correction occurs if there was a carry or the high nibble is invalid after
				// low correction by *addition rules* (which implies a carry lookahead)
				if ((prevF & 0x01) || mA >= 0x9A) {
					hicorr = 0x60;

					// if high correction occurs, an output carry is guaranteed
					mF |= 0x01;
				}

				if (mF & 0x02) {
					// subtract the correction, and set HF if there was a low nibble borrow
					uint8 corr1 = mA - locorr;

					mF |= ((mA ^ corr1) & 0x10);
					mA = corr1 - hicorr;
				} else {
					// add the correction, and set HF if there was a low nibble carry
					uint8 corr1 = mA + locorr;

					mF |= ((mA ^ corr1) & 0x10);
					mA = corr1 + hicorr;
				}

				// set S from bit 7, and copy bit 3/5 into flags
				mF |= mA & 0xA8;

				// set Z
				if (!mA)
					mF |= 0x40;

				// set P
				AT_SET_PARITY(mA);
			}
			break;

		// The TOMS Turbo Drive firmware relies on the following sequence working to
		// test bit 7 of the sector number:
		//
		//	BIT 7,(HL)
		//	JP M,addr
		//
		// Z changed, H set, N cleared, bits 3/5 copied from operand as if AND
		//
		case kZ80StateBit0: mF &= 0x01; mF |= 0x10; mF |= (mDataH & 0x00); if (!(mDataH & 0x01)) mF |= 0x44; break;
		case kZ80StateBit1: mF &= 0x01; mF |= 0x10; mF |= (mDataH & 0x00); if (!(mDataH & 0x02)) mF |= 0x44; break;
		case kZ80StateBit2: mF &= 0x01; mF |= 0x10; mF |= (mDataH & 0x00); if (!(mDataH & 0x04)) mF |= 0x44; break;
		case kZ80StateBit3: mF &= 0x01; mF |= 0x10; mF |= (mDataH & 0x08); if (!(mDataH & 0x08)) mF |= 0x44; break;
		case kZ80StateBit4: mF &= 0x01; mF |= 0x10; mF |= (mDataH & 0x00); if (!(mDataH & 0x10)) mF |= 0x44; break;
		case kZ80StateBit5: mF &= 0x01; mF |= 0x10; mF |= (mDataH & 0x20); if (!(mDataH & 0x20)) mF |= 0x44; break;
		case kZ80StateBit6: mF &= 0x01; mF |= 0x10; mF |= (mDataH & 0x00); if (!(mDataH & 0x40)) mF |= 0x44; break;
		case kZ80StateBit7: mF &= 0x01; mF |= 0x10; mF |= (mDataH & 0x80); if (!(mDataH & 0x80)) mF |= 0x44; break;

		case kZ80StateBitAdjustIX:	// BIT n,(IX+d) pulls bits 3/5 of F from address calc instead of AND operation
			mF &= 0xD7;
			mF |= (mAddr >> 8) & 0x28;
			break;

		case kZ80StateSet0: mDataH |= 0x01; break;
		case kZ80StateSet1: mDataH |= 0x02; break;
		case kZ80StateSet2: mDataH |= 0x04; break;
		case kZ80StateSet3: mDataH |= 0x08; break;
		case kZ80StateSet4: mDataH |= 0x10; break;
		case kZ80StateSet5: mDataH |= 0x20; break;
		case kZ80StateSet6: mDataH |= 0x40; break;
		case kZ80StateSet7: mDataH |= 0x80; break;
		case kZ80StateRes0: mDataH &= 0xFE; break;
		case kZ80StateRes1: mDataH &= 0xFD; break;
		case kZ80StateRes2: mDataH &= 0xFB; break;
		case kZ80StateRes3: mDataH &= 0xF7; break;
		case kZ80StateRes4: mDataH &= 0xEF; break;
		case kZ80StateRes5: mDataH &= 0xDF; break;
		case kZ80StateRes6: mDataH &= 0xBF; break;
		case kZ80StateRes7: mDataH &= 0x7F; break;

		case kZ80StateCCF:		// H=C, C=!C, N reset, bits 3/5 from A
			mF &= 0xC5;
			if (mF & 0x01)
				mF |= 0x10;
			mF |= (mA & 0x28);
			mF ^= 0x01;
			break;

		case kZ80StateSCF:		// C set, HN reset, bits 3/5 from A
			mF &= 0xC5;
			mF |= (mA & 0x28);
			mF |= 0x01;
			break;

		case kZ80StateAddToHL:	// HNC changed, bits 3/5 from high byte
			{
				const uint32 hl = mL + ((uint32)mH << 8);
				const uint32 arg = mDataL + ((uint32)mDataH << 8);
				const uint32 r = hl + arg;

				mF &= 0xC4;

				if ((hl ^ arg ^ r) & 0x1000)
					mF |= 0x10;

				mF |= (r >> 16) & 1;
				mF |= (r >> 8) & 0x28;

				mL = (uint8)r;
				mH = (uint8)(r >> 8);
			}
			break;

		case kZ80StateAdcToHL:	// SZHVNC changed, bits 3/5 from high byte
			{
				const uint32 hl = mL + ((uint32)mH << 8);
				const uint32 arg = mDataL + ((uint32)mDataH << 8);
				const uint32 c = mF & 1;
				const uint32 carry15 = (hl & 0x7FFF) + (arg & 0x7FFF) + c;
				const uint32 r = hl + (uint16)arg + c;

				mF = (r >> 8) & 0xA8;

				if (!(uint16)r)
					mF |= 0x40;

				mF |= ((hl ^ arg ^ r) & 0x1000) >> 8;
				mF |= ((r >> 14) ^ (carry15 >> 13)) & 0x04;
				mF |= (r >> 16) & 0x01;

				mL = (uint8)r;
				mH = (uint8)(r >> 8);
			}
			break;

		case kZ80StateSbcToHL:	// SZHVNC changed, bits 3/5 from high byte
			{
				const uint32 hl = mL + ((uint32)mH << 8);
				const uint32 arg = mDataL + ((uint32)mDataH << 8);
				const uint32 nc = ~mF & 1;
				const uint32 carry15 = (hl & 0x7FFF) + (~arg & 0x7FFF) + nc;
				const uint32 r = hl + (uint16)~arg + nc;

				mF = (r >> 8) & 0xA8;

				if (!(uint16)r)
					mF |= 0x40;

				mF |= ((hl ^ arg ^ r) & 0x1000) >> 8;
				mF |= ((r >> 14) ^ (carry15 >> 13)) & 0x04;
				mF |= ~(r >> 16) & 0x01;
				mF |= 0x02;

				mL = (uint8)r;
				mH = (uint8)(r >> 8);
			}
			break;

		case kZ80StateAddToIX:	// HNC changed, bits 3/5 from high byte
			{
				uint8& ixl = mbUseIY ? mIYL : mIXL;
				uint8& ixh = mbUseIY ? mIYH : mIXH;
				const uint32 hl = ixl + ((uint32)ixh << 8);
				const uint32 arg = mDataL + ((uint32)mDataH << 8);
				const uint32 r = hl + arg;

				mF &= 0xC4;
				mF |= (r >> 8) & 0x28;

				if ((hl ^ arg ^ r) & 0x1000)
					mF |= 0x10;

				mF |= (r >> 16) & 1;

				ixl = (uint8)r;
				ixh = (uint8)(r >> 8);
			}
			break;

		case kZ80StateDI:
			mbIFF1 = false;
			mbIFF2 = false;
			break;

		case kZ80StateEI:
			if (!mbIFF1) {
				mbIntActionNeeded = true;
				mbEiPending = true;
				mbIFF1 = true;
			}

			mbIFF2 = true;
			break;

		case kZ80StateRetn:
			if (mbIFF1 != mbIFF2) {
				mbIFF1 = mbIFF2;

				mbIntActionNeeded = true;
			}
			break;

		case kZ80StateReti:
			mpFnIntAck();
			break;

		case kZ80StateHaltEnter:
			mPC = mInsnPC;
			mpFnHaltChange(true);
			break;

		case kZ80StateHalt:
			mTStatesLeft = 4;

			if ((mbIrqPending && mbIFF1) || mbNmiPending) {
				++mPC;
				mpFnHaltChange(false);
			} else {
				--mpNextState;
			}
			break;

		case kZ80StateIM0_4T:
			mIntMode = 0;
			mTStatesLeft = 4;
			break;

		case kZ80StateIM1_4T:
			mIntMode = 1;
			mTStatesLeft = 4;
			break;

		case kZ80StateIM2_4T:
			mIntMode = 2;
			mTStatesLeft = 4;
			break;

		case kZ80StateRst00: mPC = 0x00; break;
		case kZ80StateRst08: mPC = 0x08; break;
		case kZ80StateRst10: mPC = 0x10; break;
		case kZ80StateRst18: mPC = 0x18; break;
		case kZ80StateRst20: mPC = 0x20; break;
		case kZ80StateRst28: mPC = 0x28; break;
		case kZ80StateRst30: mPC = 0x30; break;
		case kZ80StateRst38: mPC = 0x38; break;
		case kZ80StateRst66: mPC = 0x66; break;

		case kZ80StateRstIntVec:
			if (mIntMode == 2) {
				mAddr = ((uint16)mI << 8) + mpFnIntVec();
			} else {
				mPC = 0x38;
				mpNextState += 3;
			}
			break;

		case kZ80StateJP:
			mPC = (uint16)(mDataL + ((uint32)mDataH << 8));
			break;

		case kZ80StateJR:
			mPC = (uint16)(mPC + (sint8)mDataH);
			break;

		case kZ80StateStep1I:
			if (!++mL)
				++mH;

			if (!mC--)
				--mB;

			mF &= 0xFB;
			if (mB || mC)
				mF |= 0x04;
			break;

		case kZ80StateStep1I_IO:
			if (!++mL)
				++mH;

			mF |= 0x80;		// set N
			mF &= 0xBF;		// reset Z
			if (!--mB)
				mF |= 0x40;
			break;

		case kZ80StateStep1D:
			if (!mL--)
				--mH;

			if (!mC--)
				--mB;

			mF &= 0xFB;
			if (mB || mC)
				mF |= 0x04;
			break;

		case kZ80StateStep1D_IO:
			if (!mL--)
				--mH;

			mF |= 0x80;		// set N
			mF &= 0xBF;		// reset Z
			if (!--mB)
				mF |= 0x40;
			break;

		case kZ80StateStep2I:
			if (!++mL)
				++mH;

			if (!++mE)
				++mD;

			if (!mC--)
				--mB;

			mF &= 0xE9;		// reset HPN
			if (mB || mC)
				mF |= 0x04;
			break;

		case kZ80StateStep2D:
			if (!mL--)
				--mH;

			if (!mE--)
				--mD;

			if (!mC--)
				--mB;

			mF &= 0xE9;		// reset HPN
			if (mB || mC)
				mF |= 0x04;
			break;

		case kZ80StateRep:
			if (mF & 4) {
				mPC -= 2;
				mTStatesLeft = 5;
			}
			break;

		case kZ80StateRep_IO:
			if (!(mF & 0x40)) {
				mPC -= 2;
				mTStatesLeft = 5;
			}
			break;

		case kZ80StateRepNZ:
			if ((mF & 0x44) == 0x04) {
				mPC -= 2;
				mTStatesLeft = 5;
			}
			break;

		case kZ80StateSkipUnlessNZ:	if ( (mF & 0x40)) { mTStatesLeft = 4; mpNextState = &mReadOpcodeState; } break;
		case kZ80StateSkipUnlessZ:	if (!(mF & 0x40)) { mTStatesLeft = 4; mpNextState = &mReadOpcodeState; } break;
		case kZ80StateSkipUnlessNC:	if ( (mF & 0x01)) { mTStatesLeft = 4; mpNextState = &mReadOpcodeState; } break;
		case kZ80StateSkipUnlessC:	if (!(mF & 0x01)) { mTStatesLeft = 4; mpNextState = &mReadOpcodeState; } break;
		case kZ80StateSkipUnlessPO:	if ( (mF & 0x04)) { mTStatesLeft = 4; mpNextState = &mReadOpcodeState; } break;
		case kZ80StateSkipUnlessPE:	if (!(mF & 0x04)) { mTStatesLeft = 4; mpNextState = &mReadOpcodeState; } break;
		case kZ80StateSkipUnlessP:	if ( (mF & 0x80)) { mTStatesLeft = 4; mpNextState = &mReadOpcodeState; } break;
		case kZ80StateSkipUnlessM:	if (!(mF & 0x80)) { mTStatesLeft = 4; mpNextState = &mReadOpcodeState; } break;

		case kZ80StateDjnz:
			if (!--mB) {
				mTStatesLeft = 4;
				mpNextState = &mReadOpcodeState;
			}
			break;

#ifdef _DEBUG
		default:
			VDASSERT(!"Invalid CPU state detected.");
			break;
#else
		default:
			VDNEVERHERE;
#endif
	}
}

force_exit:
	;

///////////////////////////////////////////////////////////////////

#undef AT_SET_PARITY

#undef AT_CPU_READ_BYTE
#undef AT_CPU_READ_BYTE_ADDR16
#undef AT_CPU_READ_BYTE_HL
#undef AT_CPU_WRITE_BYTE
#undef AT_CPU_WRITE_BYTE_HL

#undef INSN_FETCH
#undef INSN_FETCH_TO
#undef INSN_FETCH_NOINC
