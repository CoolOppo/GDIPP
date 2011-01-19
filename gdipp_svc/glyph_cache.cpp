#include "stdafx.h"
#include "glyph_cache.h"

unsigned __int64 gdipp_glyph_cache::get_char_id(unsigned int font_trait, FT_UInt index, bool is_glyph_index)
{
	return (((static_cast<unsigned __int64>(font_trait) << 1) | static_cast<char>(is_glyph_index)) << 31) | index;
}

gdipp_glyph_cache::gdipp_glyph_cache()
	: _glyph_run_lru(10)
{
}

const FT_Glyph gdipp_glyph_cache::lookup_glyph(unsigned int font_trait, FT_UInt index, bool is_glyph_index) const
{
	const unsigned __int64 char_id = get_char_id(font_trait, index, is_glyph_index);

	map<unsigned __int64, const FT_Glyph>::const_iterator glyph_iter = _glyph_store.find(char_id);
	if (glyph_iter == _glyph_store.end())
		return NULL;
	else
		return glyph_iter->second;
}

bool gdipp_glyph_cache::store_glyph(unsigned int font_trait, FT_UInt index, bool is_glyph_index, const FT_Glyph glyph)
{
	const unsigned __int64 char_id = get_char_id(font_trait, index, is_glyph_index);

	const pair<map<unsigned __int64, const FT_Glyph>::const_iterator, bool> glyph_insert_ret = _glyph_store.insert(pair<unsigned __int64, const FT_Glyph>(char_id, glyph));

	return glyph_insert_ret.second;
}

bool gdipp_glyph_cache::lookup_glyph_run(unsigned int font_trait, unsigned __int64 string_id, glyph_run &a_glyph_run) const
{
	gdipp_lock lock("glyph_run_cache");

	map<unsigned __int64, hash_to_run_map>::const_iterator str_iter = _glyph_run_store.find(string_id);
	if (str_iter == _glyph_run_store.end())
		return false;

	hash_to_run_map::const_iterator trait_iter = str_iter->second.find(font_trait);
	if (trait_iter == str_iter->second.end())
		return false;

	a_glyph_run = trait_iter->second;

	return true;
}

bool gdipp_glyph_cache::store_glyph_run(unsigned int font_trait, unsigned __int64 string_id, const glyph_run &a_glyph_run)
{
	bool b_ret;
	unsigned __int64 erased_str;

	gdipp_lock lock("glyph_run_cache");

	b_ret = _glyph_run_lru.access(string_id, erased_str);
	if (b_ret)
		_glyph_run_store.erase(erased_str);

	const pair<map<unsigned __int64, hash_to_run_map>::iterator, bool> str_insert_ret = _glyph_run_store.insert(pair<unsigned __int64, hash_to_run_map>(string_id, hash_to_run_map()));
	const pair<hash_to_run_map::const_iterator, bool> trait_insert_ret = str_insert_ret.first->second.insert(pair<unsigned int, glyph_run>(font_trait, a_glyph_run));

	return trait_insert_ret.second;
}