#pragma once

#include <support_lru.h>
#include <support_rpc.h>

using namespace std;

class gdipp_glyph_cache
{	
	// map from string hash to glyph run
	typedef map<unsigned int, glyph_run> hash_to_run_map;

	// map from character ID to its glyph
	map<unsigned __int64, const FT_Glyph> _glyph_store;
	map<unsigned __int64, hash_to_run_map> _glyph_run_store;

	// least recently used glyph runs, indexed by string ID
	lru_list<unsigned __int64> _glyph_run_lru;

	static unsigned __int64 get_char_id(unsigned int font_trait, FT_UInt index, bool is_glyph_index);

public:
	gdipp_glyph_cache();

	const FT_Glyph lookup_glyph(unsigned int font_trait, FT_UInt index, bool is_glyph_index) const;
	bool store_glyph(unsigned int font_trait, FT_UInt index, bool is_glyph_index, const FT_Glyph glyph);
	bool lookup_glyph_run(unsigned int font_trait, unsigned __int64 string_id, glyph_run &a_glyph_run) const;
	bool store_glyph_run(unsigned int font_trait, unsigned __int64 string_id, const glyph_run &a_glyph_run);
};