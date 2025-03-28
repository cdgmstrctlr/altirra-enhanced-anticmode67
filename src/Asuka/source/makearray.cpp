//	Asuka - VirtualDub Build/Post-Mortem Utility
//	Copyright (C) 2005-2006 Avery Lee
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

#include <stdafx.h>
#include <vd2/system/vdtypes.h>
#include <vd2/system/vdstl.h>
#include <vd2/system/filesys.h>
#include <string>
#include <vector>

#include "utils.h"

void tool_makearray(const vdfastvector<const char *>& args, const vdfastvector<const char *>& switches) {
	if (args.size() < 3) {
		printf("usage: makearray <binary file> <.cpp output file> <symbol name> [<len symbol name>]\n");
		exit(5);
	}

	FILE *f = fopen(args[0], "rb");
	if (!f)
		fail("    couldn't open: %s\n", args[0]);
	fseek(f, 0, SEEK_END);
	size_t l = ftell(f);
	vdfastvector<char> buf(l);
	fseek(f, 0, SEEK_SET);
	if (!buf.empty())
		fread(&buf[0], l, 1, f);
	fclose(f);

	f = fopen(args[1], "w");
	if (!f)
		fail("    couldn't open: %s\n", args[1]);

	fprintf(f, "// Automatically generated by Asuka from \"%s.\" DO NOT EDIT!\n\n", VDFileSplitPath(args[0]));

	fprintf(f, "extern const unsigned char %s[]={\n", args[2]);

	for(size_t i=0; i<l; i += 32) {
		size_t limit = i+32;
		if (limit > l)
			limit = l;

		for(size_t j=i; j<limit; ++j)
			fprintf(f, "0x%02x,", (uint8)buf[j]);
		fputc('\n', f);
	}

	fprintf(f, "};\n");

	if (args.size() >= 4)
		fprintf(f, "\nextern const size_t %s = %llu;\n", args[3], (unsigned long long)l);

	fclose(f);
}
