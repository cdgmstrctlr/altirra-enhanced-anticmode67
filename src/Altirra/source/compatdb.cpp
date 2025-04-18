//	Altirra - Atari 800/800XL/5200 emulator
//	Runtime compatibility database module
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
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include <unordered_map>
#include <vd2/system/binary.h>
#include "compatdb.h"

const char ATCompatDBHeader::kSignature[16] = {
	// Inspired by PNG.
	(char)0x89,'A','T','C','o','m','p','a','t','D','B',0x0D,0x0A,0x1A,0x0A,0x00
};

bool ATCompatDBHeader::Validate(size_t len) const {
	VDASSERT(len >= sizeof(*this));

	// check supported version
	if ((mVersion & 0xFFFFFF00) != 0x0200)
		return false;

	// validate top-level tables
	const void *tableBase = (const char *)this + sizeof(*this);
	const size_t tableLen = len - sizeof(*this);
	if (!mRuleSetTable.validate(tableBase, tableLen)) return false;
	if (!mRuleTable.validate(tableBase, tableLen)) return false;
	if (!mAliasTable.validate(tableBase, tableLen)) return false;
	if (!mTitleTable.validate(tableBase, tableLen)) return false;
	if (!mTagTable.validate(tableBase, tableLen)) return false;
	if (!mTagIdTable.validate(tableBase, tableLen)) return false;
	if (!mCharTable.validate(tableBase, tableLen)) return false;
	if (!mLargeRuleDataTable.validate(tableBase, tableLen)) return false;

	// validate data in each table
	const uint32 numAliases = mAliasTable.size();
	const uint32 numTitles = mTitleTable.size();
	const uint32 numRules = mRuleTable.size();
	const uint32 numTags = mTagTable.size();

	for(const auto& ruleSet : mRuleSetTable) {
		if (!ruleSet.mRules.validate(mRuleTable))
			return false;
	}

	for(const auto& rule : mRuleTable) {
		if (rule.mAliasId >= numAliases)
			return false;

		if (rule.mNextRuleId >= numRules)
			return false;
	}

	for(const auto& alias : mAliasTable) {
		if (alias.mTitleId >= numTitles)
			return false;

		if (alias.mRuleCount > numRules)
			return false;
	}

	for(const auto& title : mTitleTable) {
		if (!title.mName.validate(mCharTable))
			return false;

		if (!title.mTagIds.validate(mTagIdTable))
			return false;
	}

	for(const auto& tagId : mTagIdTable) {
		if (tagId >= numTags)
			return false;
	}

	for(const auto& tag : mTagTable) {
		if (!tag.mKey.validate(mCharTable))
			return false;
	}

	if (!mCharTable.empty() && mCharTable.back())
		return false;

	// all good
	return true;
}

///////////////////////////////////////////////////////////////////////////

ATCompatDBView::ATCompatDBView(const ATCompatDBHeader *hdr)
	: mpHeader(hdr)
{
}

std::pair<const ATCompatDBRule *, const ATCompatDBRule *> ATCompatDBView::FindMatchingRules(ATCompatRuleType type, uint64 value) const {
	for(const auto& ruleSet : mpHeader->mRuleSetTable) {
		if (ruleSet.mRuleType == type) {
			const auto& rules = ruleSet.mRules;

			// Use a custom binary search to ensure that we don't break if the array in the DB wasn't
			// sorted correctly.
			auto it = rules.begin();
			auto n = rules.size();

			while(n > 0) {
				uint32 n2 = n >> 1;

				auto it2 = it + n2;

				if (it2->GetValue() < value) {
					++n2;
					it += n2;
					n -= n2;
				} else {
					n = n2;
				}
			}

			if (it != rules.end() && it->GetValue() == value) {
				// walk forward and collect the range of matching rules (which may exist when multiple
				// aliases have the same rule)
				auto it2 = it + 1;

				while(it2 != rules.end() && it2->GetValue() == value)
					++it2;

				return {it, it2};
			}

			break;
		}
	}

	return {nullptr, nullptr};
}

bool ATCompatDBView::HasRelatedRuleOfType(const ATCompatDBRule *baseRule, ATCompatRuleType type) const {
	if (baseRule < mpHeader->mRuleTable.begin() || baseRule >= mpHeader->mRuleTable.end()) {
		VDFAIL("Invalid rule passed to HasRelatedRuleOfType().");
		return false;
	}

	for(const auto& ruleSet : mpHeader->mRuleSetTable) {
		if (ruleSet.mRuleType == type) {
			uint32 index = (uint32)(baseRule - mpHeader->mRuleTable.begin());
			const ATCompatDBAlias& alias = mpHeader->mAliasTable[baseRule->mAliasId];

			for(uint32 i = 1; i < alias.mRuleCount; ++i) {
				index = baseRule->mNextRuleId;
				baseRule = &mpHeader->mRuleTable[index];

				if (baseRule >= ruleSet.mRules.begin() && baseRule < ruleSet.mRules.end())
					return true;
			}

			break;
		}
	}

	return false;
}

const ATCompatDBTitle *ATCompatDBView::FindMatchingTitle(const ATCompatDBRule *const *rules, size_t numRules) const {
	std::unordered_map<uint32, uint32> pendingAliasTable;

	while(numRules--) {
		const ATCompatDBRule *rule = *rules++;

		auto r = pendingAliasTable.emplace(rule->mAliasId, 0);

		if (r.second)
			r.first->second = mpHeader->mAliasTable[rule->mAliasId].mRuleCount;

		if (!--r.first->second)
			return &mpHeader->mTitleTable[mpHeader->mAliasTable[rule->mAliasId].mTitleId];
	}

	return nullptr;
}

void ATCompatDBView::FindMatchingTitles(vdfastvector<const ATCompatDBTitle *>& titles, const ATCompatDBRule *const *rules, size_t numRules) const {
	std::unordered_map<uint32, uint32> pendingAliasTable;

	while(numRules--) {
		const ATCompatDBRule *rule = *rules++;

		auto r = pendingAliasTable.emplace(rule->mAliasId, 0);

		if (r.second)
			r.first->second = mpHeader->mAliasTable[rule->mAliasId].mRuleCount;

		if (!--r.first->second)
			titles.push_back(&mpHeader->mTitleTable[mpHeader->mAliasTable[rule->mAliasId].mTitleId]);
	}
}

ATCompatKnownTag ATCompatDBView::GetKnownTag(uint32 tagId) const {
	const auto *tag = &mpHeader->mTagTable[tagId];
	const char *tagKey = tag->mKey.c_str();

	return ATCompatGetKnownTagByKey(tagKey);
}

uint32 ATCompatDBView::FindLargeRuleBlob(const void *blob256) const {
	const uint32 numBlobs = (uint32)(mpHeader->mLargeRuleDataTable.size() >> 3);
	if (numBlobs) {
		uint32 key[8];
		memcpy(key, blob256, 32);

		// binary search for lower bound on the first dword (as big endian)
		const uint32 *table = mpHeader->mLargeRuleDataTable.data();
		const uint32 key0be = VDFromBE32(key[0]);
		uint32 base = 0;
		uint32 n = numBlobs;

		while(n > 0) {
			uint32 n2 = n >> 1;

			uint32 v = VDFromBE32(table[8 * (base + n2)]);

			if (v < key0be) {
				++n2;
				base += n2;
				n -= n2;
			} else {
				n = n2;
			}
		}

		if (base < numBlobs) {
			const uint32 *entry = table + (8 * base);
			do {
				if (entry[0] != key[0])
					break;

				if (!memcmp(entry, key, 32))
					return base * 32;
			} while(++base < numBlobs);
		}
	}

	return ~0;
}

///////////////////////////////////////////////////////////////////////////

bool ATCompatIsLargeRuleType(ATCompatRuleType type) {
	switch(type) {
		case kATCompatRuleType_CartFileSHA256:
		case kATCompatRuleType_DiskFileSHA256:
		case kATCompatRuleType_DOSBootFileSHA256:
		case kATCompatRuleType_ExeFileSHA256:
			return true;

		default:
			return false;
	}
}

namespace {
	const char *kKnownTagNames[] = {
		"basic",
		"basicreva",
		"basicrevb",
		"basicrevc",
		"nobasic",
		"osa",
		"osb",
		"xlos",
		"accuratedisktiming",
		"nociodevices",
		"noexpandedmem",
		"ctia",
		"noultimate1mb",
		"undoc6502",
		"no65c816highaddressing",
		"writabledisk",
		"nofloatingdatabus",
		"cart52008k",
		"cart520016konechip",
		"cart520016ktwochip",
		"cart520032k",
		"60hz",
		"50hz",
	};

	static_assert(vdcountof(kKnownTagNames) == kATCompatKnownTagCount - 1, "Known tag name list out of sync");
}

ATCompatKnownTag ATCompatGetKnownTagByKey(const char *key) {
	uint32 idx = 1;
	for(const char *s : kKnownTagNames) {
		if (!strcmp(key, s))
			return (ATCompatKnownTag)idx;

		++idx;
	}

	return kATCompatKnownTag_None;
}

const char *ATCompatGetKeyForKnownTag(ATCompatKnownTag knownTag) {
	return ((uint32)knownTag - 1) < kATCompatKnownTagCount ? kKnownTagNames[knownTag - 1] : nullptr;
}
