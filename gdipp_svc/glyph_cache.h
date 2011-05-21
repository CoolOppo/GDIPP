#pragma once

#include <map>
#include <MurmurHash3.h>
#include <MurmurHash/bigint.h>
#include "gdipp_support/gs_lru.h"
#include "gdipp_support/gs_rpc.h"

using std::map;

class gdipp_glyph_cache
{
public:
	gdipp_glyph_cache();

	const FT_Glyph lookup_glyph(unsigned int font_trait, FT_UInt index, bool is_glyph_index) const;
	bool store_glyph(unsigned int font_trait, FT_UInt index, bool is_glyph_index, const FT_Glyph glyph);
	bool lookup_glyph_run(unsigned int font_trait, uint128_t string_id, glyph_run *a_glyph_run) const;
	bool store_glyph_run(unsigned int font_trait, uint128_t string_id, glyph_run *a_glyph_run);

private:
	// map from string hash to glyph run
	typedef map<unsigned int, glyph_run *> hash_to_run_map;

	static uint64_t get_char_id(unsigned int font_trait, FT_UInt index, bool is_glyph_index);

	// map from character ID to its glyph
	map<uint64_t, const FT_Glyph> _glyph_store;
	map<uint128_t, hash_to_run_map> _glyph_run_store;

	// least recently used glyph runs, indexed by string ID
	lru_list<uint128_t> _glyph_run_lru;
};
