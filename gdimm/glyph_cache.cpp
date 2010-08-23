#include "stdafx.h"
#include "glyph_cache.h"

const FT_Glyph gdimm_glyph_cache::lookup_glyph(uint64_t font_trait, FT_UInt index, bool is_glyph_index)
{
	map<uint64_t, index_to_glyph_map>::const_iterator trait_iter = _glyph_store.find(font_trait);
	if (trait_iter == _glyph_store.end())
		return NULL;

	const FT_Int internal_index = index * (is_glyph_index ? 1 : -1);
	index_to_glyph_map::const_iterator index_iter = trait_iter->second.find(internal_index);
	if (index_iter == trait_iter->second.end())
		return NULL;

	return index_iter->second;
}

bool gdimm_glyph_cache::store_glyph(uint64_t font_trait, FT_UInt index, bool is_glyph_index, const FT_Glyph glyph)
{
	// if the specified font trait does not exist in the map, insert new map
	// otherwise nothing is changed
	const pair<map<uint64_t, index_to_glyph_map>::iterator, bool> trait_insert_ret = _glyph_store.insert(pair<uint64_t, index_to_glyph_map>(font_trait, index_to_glyph_map()));

	const FT_Int internal_index = index * (is_glyph_index ? 1 : -1);
	const pair<index_to_glyph_map::const_iterator, bool> index_insert_ret = trait_insert_ret.first->second.insert(pair<FT_Int, const FT_Glyph>(internal_index, glyph));

	return index_insert_ret.second;
}

bool gdimm_glyph_cache::lookup_glyph_run(uint64_t font_trait, uint64_t str_hash, glyph_run &a_glyph_run)
{
	map<uint64_t, hash_to_run_map>::const_iterator trait_iter = _glyph_run_store.find(font_trait);
	if (trait_iter == _glyph_run_store.end())
		return false;

	hash_to_run_map::const_iterator hash_iter = trait_iter->second.find(str_hash);
	if (hash_iter == trait_iter->second.end())
		return false;

	a_glyph_run = hash_iter->second;

	return true;
}

bool gdimm_glyph_cache::store_glyph_run(uint64_t font_trait, uint64_t str_hash, const glyph_run &a_glyph_run)
{
	const pair<map<uint64_t, hash_to_run_map>::iterator, bool> trait_insert_ret = _glyph_run_store.insert(pair<uint64_t, hash_to_run_map>(font_trait, hash_to_run_map()));
	const pair<hash_to_run_map::const_iterator, bool> hash_insert_ret = trait_insert_ret.first->second.insert(pair<uint64_t, glyph_run>(str_hash, a_glyph_run));

	return hash_insert_ret.second;
}

bool gdimm_glyph_cache::erase_font_trait(uint64_t font_trait)
{
	// erase the glyph run from its store
	// then free and erase the contained glyphs
	
	const size_t del_count = _glyph_run_store.erase(font_trait);
	if (del_count == 0)
		return false;

	map<uint64_t, index_to_glyph_map>::iterator bmp_trait_iter = _glyph_store.find(font_trait);
	if (bmp_trait_iter != _glyph_store.end())
	{
		for (index_to_glyph_map::const_iterator index_iter = bmp_trait_iter->second.begin(); index_iter != bmp_trait_iter->second.end(); index_iter++)
			FT_Done_Glyph(index_iter->second);

		_glyph_store.erase(font_trait);
	}

	return true;
}