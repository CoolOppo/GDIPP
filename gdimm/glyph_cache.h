#pragma once

#include "helper_def.h"
#include "lru.h"

using namespace std;

template <typename font_trait_type>
class gdimm_glyph_cache
{
	template <typename font_trait_type>
	friend class gdimm_renderer;

	/*
	positive index stands for glyph index
	negative index stands for Unicode code point
	0 is not used for either
	*/
	typedef map<FT_Int, const FT_BitmapGlyph> index_to_bmp_map;
	
	typedef map<uint64_t, glyph_run> hash_to_run_map;

	map<font_trait_type, index_to_bmp_map> _glyph_bmp_store;
	map<font_trait_type, hash_to_run_map> _glyph_run_store;
	lru_list<font_trait_type> _glyph_run_lru;

public:
	gdimm_glyph_cache()
	:
	_glyph_run_lru(0)
	{
	}

	const FT_BitmapGlyph lookup_glyph(const font_trait_type &font_trait, FT_UInt index, bool is_glyph_index)
	{
		map<font_trait_type, index_to_bmp_map>::const_iterator trait_iter = _glyph_bmp_store.find(font_trait);
		if (trait_iter == _glyph_bmp_store.end())
			return NULL;

		const FT_Int internal_index = index * (is_glyph_index ? 1 : -1);
		index_to_bmp_map::const_iterator index_iter = trait_iter->second.find(is_glyph_index);
		if (index_iter == trait_iter->second.end())
			return NULL;

		return index_iter->second;
	}

	bool store_glyph(const font_trait_type &font_trait, FT_UInt index, bool is_glyph_index, const FT_BitmapGlyph glyph_bmp)
	{
		const pair<map<font_trait_type, index_to_bmp_map>::iterator, bool> trait_insert_ret = _glyph_bmp_store.insert(pair<font_trait_type, index_to_bmp_map>(font_trait, index_to_bmp_map()));

		const FT_Int internal_index = index * (is_glyph_index ? 1 : -1);
		const pair<index_to_bmp_map::const_iterator, bool> index_insert_ret = trait_insert_ret.first->second.insert(pair<FT_Int, const FT_BitmapGlyph>(internal_index, glyph_bmp));

		return index_insert_ret.second;
	}

	const glyph_run *lookup_glyph_run(const font_trait_type &font_trait, uint64_t str_hash)
	{
		map<font_trait_type, hash_to_run_map>::const_iterator trait_iter = _glyph_run_store.find(font_trait);
		if (trait_iter == _glyph_run_store.end())
			return NULL;

		hash_to_run_map::const_iterator hash_iter = trait_iter->second.find(str_hash);
		if (hash_iter == trait_iter->second.end())
			return NULL;

		return &hash_iter->second;
	}
	
	const glyph_run *store_glyph_run(const font_trait_type &font_trait, uint64_t str_hash, const glyph_run &a_glyph_run)
	{
		const pair<map<font_trait_type, hash_to_run_map>::iterator, bool> trait_insert_ret = _glyph_run_store.insert(pair<font_trait_type, hash_to_run_map>(font_trait, hash_to_run_map()));
		const pair<hash_to_run_map::const_iterator, bool> hash_insert_ret = trait_insert_ret.first->second.insert(pair<uint64_t, glyph_run>(str_hash, a_glyph_run));

		return &hash_insert_ret.first->second;
	}

	bool erase_font_trait(const font_trait_type &font_trait)
	{
		map<font_trait_type, index_to_bmp_map>::iterator bmp_trait_iter = _glyph_bmp_store.find(font_trait);
		if (bmp_trait_iter == _glyph_bmp_store.end())
			return false;

		for (index_to_bmp_map::const_iterator index_iter = bmp_trait_iter->second.begin(); index_iter != bmp_trait_iter->second.end(); index_iter++)
			FT_Done_Glyph((FT_Glyph) index_iter->second);

		_glyph_bmp_store.erase(font_trait);
		_glyph_run_store.erase(font_trait);

		return true;
	}
};