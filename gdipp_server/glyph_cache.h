#pragma once

#include <map>
#include <MurmurHash3.h>
#include "glyph_run.h"
#include "gdipp_support/lru.h"

namespace gdipp
{

typedef std::pair<uint64_t, uint64_t> uint128_t;

class glyph_cache
{
public:
	glyph_cache();

	const FT_Glyph lookup_glyph(unsigned int font_trait, FT_UInt index, bool is_glyph_index) const;
	bool store_glyph(unsigned int font_trait, FT_UInt index, bool is_glyph_index, const FT_Glyph glyph);
	const glyph_run *lookup_glyph_run(unsigned int font_trait, uint128_t string_id) const;
	bool store_glyph_run(unsigned int font_trait, uint128_t string_id, glyph_run *a_glyph_run);

private:
	// std::map from string hash to glyph run
	typedef std::map<unsigned int, glyph_run *> hash_to_run_map;

	static uint64_t get_char_id(unsigned int font_trait, FT_UInt index, bool is_glyph_index);

	// std::map from character ID to its glyph
	std::map<uint64_t, const FT_Glyph> _glyph_store;
	std::map<uint128_t, hash_to_run_map> _glyph_run_store;

	// least recently used glyph runs, indexed by string ID
	lru_list<uint128_t> _glyph_run_lru;
};

}
