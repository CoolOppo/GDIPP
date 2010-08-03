#pragma once

#include "helper_def.h"
#include "helper_func.h"
#include "lru.h"

using namespace std;

class gdimm_glyph_cache
{
	friend class gdimm_renderer;

	/*
	positive index stands for glyph index
	negative index stands for Unicode code point
	0 is not used for either
	*/
	typedef map<FT_Int, const FT_BitmapGlyph> index_to_bmp_map;
	
	typedef map<uint64_t, glyph_run> hash_to_run_map;

	map<LOGFONTW, index_to_bmp_map> _glyph_bmp_store;
	map<LOGFONTW, hash_to_run_map> _glyph_run_store;
	lru_list<LOGFONTW> _glyph_run_lru;

public:
	gdimm_glyph_cache();

	const FT_BitmapGlyph lookup_glyph(const LOGFONTW &font_trait, FT_UInt index, bool is_glyph_index);
	bool store_glyph(const LOGFONTW &font_trait, FT_UInt index, bool is_glyph_index, const FT_BitmapGlyph glyph);
	bool lookup_glyph_run(const LOGFONTW &font_trait, uint64_t str_hash, glyph_run &a_glyph_run);
	bool store_glyph_run(const LOGFONTW &font_trait, uint64_t str_hash, const glyph_run &a_glyph_run);
	bool erase_font_trait(const LOGFONTW &font_trait);
};