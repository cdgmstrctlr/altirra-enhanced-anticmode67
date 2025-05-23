//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2024 Avery Lee
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

#ifndef f_AT_PRINTERTTFENCODER_H
#define f_AT_PRINTERTTFENCODER_H

#include <vd2/system/date.h>
#include <vd2/system/vectors.h>

enum class ATTrueTypeGlyphIndex : uint16 {};

enum class ATTrueTypeName : uint16 {
	Copyright = 0,
	FontFamily = 1,
	FontSubfamily = 2,
	UniqueFontIdentifier = 3,
	FullFontName = 4,
	Version = 5,
	PostScriptName = 6
};

// TrueType font generator for PDF printer output.
//
// ATTrueTypeEncoder dynamically generates fonts from glyph outlines for direct embedding into
// PDF files. It is advantageous because PDF is both more compact and faster at rendering text
// than direct vector shapes. PDF supports creating fonts from vector shapes (Type 3), but
// that is not very well supported by readers, so we use a TrueType font instead.
//
// Fonts generated by TTE use an em square size of 1024 units and are currently always
// monospaced.
//
class ATTrueTypeEncoder {
	ATTrueTypeEncoder(const ATTrueTypeEncoder&) = delete;
	ATTrueTypeEncoder& operator=(const ATTrueTypeEncoder&) = delete;

public:
	ATTrueTypeEncoder();
	~ATTrueTypeEncoder();

	// Set the created and modified timestamps in the font. If not specified,
	// both timestamps are set to 1/1/2024 midnight UTC.
	void SetTimestamps(const VDDate& created, const VDDate& modified);

	// Set the advance width for all glyphs, in units. This is the distance that the text
	// origin moves after printing the glyph.
	void SetDefaultAdvanceWidth(sint32 advance);

	// Set the default character to use for missing characters.
	void SetDefaultChar(uint16 defaultCh);

	// Set the break (space) character.
	void SetBreakChar(uint16 breakCh);

	//==== Simple glyphs
	
	// Begin a simple glyph (outline-based), and return the glyph index.
	ATTrueTypeGlyphIndex BeginSimpleGlyph();

	// Add a glyph point, in glyph coordinates. If onCurve is set, the point is on the
	// curve; if unset, the point is a control point. Two on-curve adjacent on-curve
	// points form a line and two on-curve points with an off-curve point in between
	// form a quadratic Bezier curve.
	void AddGlyphPoint(sint32 x, sint32 y, bool onCurve);

	// End (close) a contour and start a new one. The last contour must be ended as well.
	void EndContour();

	// End a simple glyph and finalize it (compute bounding box).
	void EndSimpleGlyph();

	//==== composite glyphs

	// Begin a composite glyph, based on simple glyphs. Return the glyph index.
	ATTrueTypeGlyphIndex BeginCompositeGlyph();

	// Add a reference to a simple glyph to the current composite glyph. The offsets are
	// added to the points in the simple glyph.
	void AddGlyphReference(ATTrueTypeGlyphIndex index, sint32 offsetX, sint32 offsetY);

	// End the current composite glyph.
	void EndCompositeGlyph();

	//==== character mapping

	// Map a single character to a glyph.
	void MapCharacter(uint32 ch, ATTrueTypeGlyphIndex glyphIndex);

	// Map an ascending range of characters to an ascending range of glyph indices.
	void MapCharacterRange(uint32 chBase, ATTrueTypeGlyphIndex glyphIndexBase, uint32 numChars);

	//==== names

	// Set a name string in the font. Currently only ASCII strings are supported.
	void SetName(ATTrueTypeName name, const char *str);

	//==== finalization

	// Finalize everything in the font and serialize it to a binary .ttf file.
	vdspan<const uint8> Finalize();

private:
	struct TTFPoint {
		sint32 mX : 15;
		sint32 mY : 15;
		uint32 mbOnCurve : 1;
		uint32 mbEndContour : 1;
	};

	struct TTFGlyphInfo {
		sint32 mNumContours = 0;	// <0 for composite glyph
		uint32 mStartIndex = 0;		// starting point or reference index
		uint32 mCount = 0;			// count of points or references
		sint32 mBBoxX1 = 0;			// inclusive bounding box of points, including referenced points
		sint32 mBBoxY1 = 0;
		sint32 mBBoxX2 = 0;
		sint32 mBBoxY2 = 0;
	};

	struct TTFGlyphReference {
		ATTrueTypeGlyphIndex mSrcGlyph;
		sint32 mOffsetX;
		sint32 mOffsetY;
	};

	struct TTFCharMapping {
		uint32 mCh;
		ATTrueTypeGlyphIndex mGlyphIndex;

		uint32 GetOffset() const { return (uint16)((uint32)mGlyphIndex - mCh); }
	};

	struct TTFTableEntry {
		uint32 mId;
		uint32 mChecksum;
		uint32 mPos;
		uint32 mLength;
	};

	struct TTFTypeId {
		consteval TTFTypeId(const char *s);

		uint32 mId;
	};

	void WriteTableHead();
	void WriteTableHhea();
	void WriteTableHmtx();
	void WriteTableMaxp();
	void WriteTablePost();
	void WriteTableOS2();
	void WriteTableCmap();
	void WriteTableGlyfLoca();
	void WriteTableName();

	void BeginTable(TTFTypeId id);
	void EndTable();

	template<typename T> requires(!std::is_pointer_v<T>)
	void WriteRaw(const T& data) {
		WriteRaw(&data, sizeof data);
	}

	void WriteRaw(const void *p, size_t n);

	vdfastvector<TTFPoint> mGlyphPoints;
	vdfastvector<TTFGlyphReference> mGlyphReferences;
	vdfastvector<TTFGlyphInfo> mGlyphs;
	vdfastvector<TTFCharMapping> mCharMappings;
	vdfastvector<TTFTableEntry> mTableDirectory;
	vdfastvector<uint8> mTableBuffer;

	struct TTFNameEntry {
		uint16 mNameId;
		uint16 mOffset;
		uint16 mLength;
	};
	vdfastvector<TTFNameEntry> mNames;
	vdfastvector<uint16> mNameChars;

	sint32 mBBoxX1 = 0;
	sint32 mBBoxY1 = 0;
	sint32 mBBoxX2 = 0;
	sint32 mBBoxY2 = 0;

	sint32 mDefaultAdvanceWidth = 0;
	uint16 mBreakChar = 0x20;
	uint16 mDefaultChar = 0x20;

	VDDate mCreatedDate;
	VDDate mModifiedDate;

	uint32 mMaxPoints = 0;
	uint32 mMaxContours = 0;
	uint32 mMaxCompositePoints = 0;
	uint32 mMaxCompositeContours = 0;
	uint32 mMaxCompositeDepth = 0;
	uint32 mMaxCompositeElements = 0;
};

#endif

