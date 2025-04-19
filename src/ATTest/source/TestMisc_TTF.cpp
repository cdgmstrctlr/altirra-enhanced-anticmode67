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

#include <stdafx.h>
#include <vd2/system/file.h>
#include <vd2/system/vdstl.h>
#include "printerttfencoder.h"
#include "test.h"

AT_DEFINE_TEST_NONAUTO(Misc_MakeTTF) {
	ATTrueTypeEncoder ttfEncoder;

	ttfEncoder.SetDefaultAdvanceWidth(250);

	ttfEncoder.BeginSimpleGlyph();
	ttfEncoder.EndSimpleGlyph();

	ttfEncoder.MapCharacter(0x20, ttfEncoder.BeginSimpleGlyph());
	ttfEncoder.EndSimpleGlyph();

	auto dotGlyph = ttfEncoder.BeginSimpleGlyph();
	ttfEncoder.MapCharacter(0x31, dotGlyph);
	ttfEncoder.AddGlyphPoint(125, 0, true);
	ttfEncoder.AddGlyphPoint(37, 37, true);
	ttfEncoder.AddGlyphPoint(0, 125, true);
	ttfEncoder.AddGlyphPoint(37, 213, true);
	ttfEncoder.AddGlyphPoint(125, 250, true);
	ttfEncoder.AddGlyphPoint(213, 213, true);
	ttfEncoder.AddGlyphPoint(250, 125, true);
	ttfEncoder.AddGlyphPoint(215, 37, true);
	ttfEncoder.EndContour();
	ttfEncoder.EndSimpleGlyph();

	for(int i=2; i<16; ++i) {
		ttfEncoder.MapCharacter(0x30+i, ttfEncoder.BeginCompositeGlyph());

		for(int j=0; j<4; ++j) {
			if (i & (1<<j))
				ttfEncoder.AddGlyphReference(dotGlyph, 0, 250*j);
		}
		ttfEncoder.EndCompositeGlyph();
	}

	ttfEncoder.SetName(ATTrueTypeName::Copyright, "None - autogenerated");
	ttfEncoder.SetName(ATTrueTypeName::FontFamily, "Altirra Print");
	ttfEncoder.SetName(ATTrueTypeName::FontSubfamily, "Normal");
	ttfEncoder.SetName(ATTrueTypeName::FullFontName, "Altirra Print Normal");
	ttfEncoder.SetName(ATTrueTypeName::UniqueFontIdentifier, "Altirra Print Normal");
	ttfEncoder.SetName(ATTrueTypeName::Version, "Version 1.0");

	const wchar_t *filename = ATTestGetArguments();
	auto data = ttfEncoder.Finalize();

	VDFile f(filename, nsVDFile::kWrite | nsVDFile::kCreateAlways);
	f.write(data.data(), data.size());

	return 0;
}
