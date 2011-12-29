#include "stdafx.h"
#include "glyph_cache.h"
#include "gdipp_lib/lock.h"
#include "gdipp_server/global.h"

namespace gdipp
{

glyph_cache::string_id_type glyph_cache::get_string_id(const wchar_t *string, unsigned int count, bool is_glyph_index)
{
	string_id_type string_id;
#ifdef _M_X64
	MurmurHash3_x64_128(string, count * sizeof(wchar_t), is_glyph_index, &string_id);
#else
	MurmurHash3_x86_128(string, count * sizeof(wchar_t), is_glyph_index, &string_id);
#endif // _M_X64

	return string_id;
}

glyph_cache::char_id_type glyph_cache::get_char_id(uint128_t render_trait, FT_UInt index, bool is_glyph_index)
{
	/*
	character ID:
	* low 64 bits: low 64 bits of render_trait
	* high 64 bits:
	high								                low
	|             31             |       1        |  32   |
	| render_trait (65 - 96 bit) | is_glyph_index | index |
	*/
	char_id_type char_id = render_trait;
	char_id.second = (char_id.second << 33) | (static_cast<uint64_t>(is_glyph_index) << 32) | index;
	return char_id;
}

glyph_cache::~glyph_cache()
{
	for (std::map<char_id_type, const FT_Glyph>::const_iterator glyph_iter = _glyph_store.begin(); glyph_iter != _glyph_store.end(); ++glyph_iter)
		FT_Done_Glyph(glyph_iter->second);

	for (std::map<uint128_t, trait_to_run_map>::const_iterator str_iter = _glyph_run_store.begin(); str_iter != _glyph_run_store.end(); ++str_iter)
	{
		for (trait_to_run_map::const_iterator trait_iter = str_iter->second.begin(); trait_iter != str_iter->second.end(); ++trait_iter)
			delete trait_iter->second;
	}
}

void glyph_cache::initialize()
{
	_glyph_run_lru.resize(min(1 << server_cache_size, 16777216));
}

const FT_Glyph glyph_cache::lookup_glyph(char_id_type char_id) const
{
	std::map<char_id_type, const FT_Glyph>::const_iterator glyph_iter = _glyph_store.find(char_id);
	if (glyph_iter == _glyph_store.end())
	{
		lock l(lock::SERVER_GLYPH_CACHE);
		glyph_iter = _glyph_store.find(char_id);
		if (glyph_iter == _glyph_store.end())
			return NULL;
	}
	
	return glyph_iter->second;
}

bool glyph_cache::store_glyph(char_id_type char_id, const FT_Glyph glyph)
{
	lock l(lock::SERVER_GLYPH_CACHE);

	const std::pair<std::map<char_id_type, const FT_Glyph>::const_iterator, bool> glyph_insert_ret = _glyph_store.insert(std::pair<uint128_t, const FT_Glyph>(char_id, glyph));

	return glyph_insert_ret.second;
}

const glyph_run *glyph_cache::lookup_glyph_run(uint128_t string_id, uint128_t render_trait) const
{
	lock l(lock::SERVER_GLYPH_RUN_CACHE);

	std::map<uint128_t, trait_to_run_map>::const_iterator str_iter = _glyph_run_store.find(string_id);
	if (str_iter == _glyph_run_store.end())
		return NULL;

	trait_to_run_map::const_iterator trait_iter = str_iter->second.find(render_trait);
	if (trait_iter == str_iter->second.end())
		return NULL;

	return trait_iter->second;
}

bool glyph_cache::store_glyph_run(uint128_t string_id, uint128_t render_trait, glyph_run *a_glyph_run)
{
	lock l(lock::SERVER_GLYPH_RUN_CACHE);

	bool b_ret;
	uint128_t erased_str;

	b_ret = _glyph_run_lru.access(string_id, erased_str);
	if (b_ret)
	{
		// the string is evicted from LRU cache
		// erase all cached glyph run that is under the evicted string ID
		
		std::map<uint128_t, trait_to_run_map>::iterator str_iter = _glyph_run_store.find(erased_str);
		assert(str_iter != _glyph_run_store.end());
		
		for (trait_to_run_map::const_iterator trait_iter = str_iter->second.begin(); trait_iter != str_iter->second.end(); ++trait_iter)
			delete trait_iter->second;

		_glyph_run_store.erase(str_iter);
	}

	// after eviction, insert new glyph_run
	const std::pair<std::map<uint128_t, trait_to_run_map>::iterator, bool> str_insert_ret = _glyph_run_store.insert(std::pair<uint128_t, trait_to_run_map>(string_id, trait_to_run_map()));
	const std::pair<trait_to_run_map::const_iterator, bool> trait_insert_ret = str_insert_ret.first->second.insert(std::pair<uint128_t, glyph_run *>(render_trait, a_glyph_run));

	return trait_insert_ret.second;
}

}
