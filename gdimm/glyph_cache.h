#pragma once

#include "helper_def.h"
#include "helper_func.h"
#include "lru.h"

using namespace std;

class gdimm_glyph_cache
{
	friend class gdimm_renderer;

	/*
	map from character to its glyph
	positive key stands for glyph index
	negative key stands for Unicode code point
	0 is not used for either
	*/
	typedef map<FT_Int, const FT_Glyph> index_to_glyph_map;

	// map from string hash to glyph run
	typedef map<uint64_t, glyph_run> hash_to_run_map;

	// map from font trait to data
	map<uint64_t, index_to_glyph_map> _glyph_store;
	map<uint64_t, hash_to_run_map> _glyph_run_store;

	// least recently used list font trait
	lru_list<uint64_t> _glyph_run_lru;

public:
	const FT_Glyph lookup_glyph(uint64_t font_trait, FT_UInt index, bool is_glyph_index);
	bool store_glyph(uint64_t font_trait, FT_UInt index, bool is_glyph_index, const FT_Glyph glyph);
	bool lookup_glyph_run(uint64_t font_trait, uint64_t str_hash, glyph_run &a_glyph_run);
	bool store_glyph_run(uint64_t font_trait, uint64_t str_hash, const glyph_run &a_glyph_run);
	bool erase_font_trait(uint64_t font_trait);
};