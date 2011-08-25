#include "stdafx.h"
#include "glyph_cache.h"
#include "gdipp_lib/lock.h"

namespace gdipp
{

glyph_cache::glyph_cache()
	: _glyph_run_lru(10)
{
}

const FT_Glyph glyph_cache::lookup_glyph(unsigned int font_trait, FT_UInt index, bool is_glyph_index) const
{
	const uint64_t char_id = get_char_id(font_trait, index, is_glyph_index);

	std::map<uint64_t, const FT_Glyph>::const_iterator glyph_iter = _glyph_store.find(char_id);
	if (glyph_iter == _glyph_store.end())
		return NULL;
	else
		return glyph_iter->second;
}

bool glyph_cache::store_glyph(unsigned int font_trait, FT_UInt index, bool is_glyph_index, const FT_Glyph glyph)
{
	const uint64_t char_id = get_char_id(font_trait, index, is_glyph_index);

	const std::pair<std::map<uint64_t, const FT_Glyph>::const_iterator, bool> glyph_insert_ret = _glyph_store.insert(std::pair<uint64_t, const FT_Glyph>(char_id, glyph));

	return glyph_insert_ret.second;
}

const glyph_run *glyph_cache::lookup_glyph_run(unsigned int font_trait, uint128_t string_id) const
{
	lock l("glyph_run_cache");

	std::map<uint128_t, hash_to_run_map>::const_iterator str_iter = _glyph_run_store.find(string_id);
	if (str_iter == _glyph_run_store.end())
		return NULL;

	hash_to_run_map::const_iterator trait_iter = str_iter->second.find(font_trait);
	if (trait_iter == str_iter->second.end())
		return NULL;

	return trait_iter->second;
}

bool glyph_cache::store_glyph_run(unsigned int font_trait, uint128_t string_id, glyph_run *a_glyph_run)
{
	bool b_ret;
	uint128_t erased_str;

	lock l("glyph_run_cache");

	b_ret = _glyph_run_lru.access(string_id, erased_str);
	if (b_ret)
		_glyph_run_store.erase(erased_str);

	const std::pair<std::map<uint128_t, hash_to_run_map>::iterator, bool> str_insert_ret = _glyph_run_store.insert(std::pair<uint128_t, hash_to_run_map>(string_id, hash_to_run_map()));
	const std::pair<hash_to_run_map::const_iterator, bool> trait_insert_ret = str_insert_ret.first->second.insert(std::pair<unsigned int, glyph_run *>(font_trait, a_glyph_run));

	return trait_insert_ret.second;
}

uint64_t glyph_cache::get_char_id(unsigned int font_trait, FT_UInt index, bool is_glyph_index)
{
	return (((static_cast<uint64_t>(font_trait) << 1) | static_cast<char>(is_glyph_index)) << 31) | index;
}

}
