#include "stdafx.h"
#include "glyph_cache.h"
#include "gdipp_lib/scoped_rw_lock.h"
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
	for (std::map<char_id_type, INIT_ONCE>::iterator glyph_iter = _glyph_store.begin(); glyph_iter != _glyph_store.end(); ++glyph_iter)
	{
		BOOL pending;
		FT_Glyph glyph;
		InitOnceBeginInitialize(&glyph_iter->second, INIT_ONCE_CHECK_ONLY, &pending, reinterpret_cast<void **>(&glyph));
		assert(!pending);
		FT_Done_Glyph(glyph);
	}

	for (std::map<string_id_type, trait_to_run_map>::iterator str_iter = _glyph_run_store.begin(); str_iter != _glyph_run_store.end(); ++str_iter)
		erase_glyph_run_cache_string(str_iter);
}

void glyph_cache::initialize()
{
	_glyph_run_lru.resize(min(1 << server_cache_size, 16777216));
}

const FT_Glyph glyph_cache::lookup_glyph(char_id_type char_id)
{
	std::map<char_id_type, INIT_ONCE>::iterator glyph_iter;
	
	{
		const scoped_rw_lock lock_w(scoped_rw_lock::SERVER_GLYPH_CACHE, false);

		glyph_iter = _glyph_store.find(char_id);
		if (glyph_iter == _glyph_store.end())
		{
			const std::pair<std::map<char_id_type, INIT_ONCE>::iterator, bool> insert_ret = _glyph_store.insert(std::pair<char_id_type, INIT_ONCE>(char_id, INIT_ONCE()));
			assert(insert_ret.second);
			glyph_iter = insert_ret.first;
			InitOnceInitialize(&glyph_iter->second);
		}
	}

	FT_Glyph glyph = NULL;
	BOOL pending;
	InitOnceBeginInitialize(&glyph_iter->second, 0, &pending, reinterpret_cast<void **>(&glyph));
	assert((glyph == NULL) == pending);

	return glyph;
}

bool glyph_cache::store_glyph(char_id_type char_id, const FT_Glyph glyph)
{
	std::map<char_id_type, INIT_ONCE>::iterator glyph_iter;

	{
		const scoped_rw_lock lock_w(scoped_rw_lock::SERVER_GLYPH_CACHE, false);

		glyph_iter = _glyph_store.find(char_id);
	}

	InitOnceComplete(&glyph_iter->second, (glyph == NULL ? INIT_ONCE_INIT_FAILED : 0), glyph);
	return glyph != NULL;
}

const glyph_run *glyph_cache::lookup_glyph_run(string_id_type string_id, uint128_t render_trait)
{
	trait_to_run_map::iterator trait_iter;

	{
		const scoped_rw_lock lock_w(scoped_rw_lock::SERVER_GLYPH_RUN_CACHE, false);

		std::map<uint128_t, trait_to_run_map>::iterator str_iter = _glyph_run_store.find(string_id);
		if (str_iter == _glyph_run_store.end())
		{
			const std::pair<std::map<string_id_type, trait_to_run_map>::iterator, bool> str_insert_ret = _glyph_run_store.insert(std::pair<string_id_type, trait_to_run_map>(string_id, trait_to_run_map()));
			assert(str_insert_ret.second);
			str_iter = str_insert_ret.first;
			trait_iter = str_iter->second.end();
		}
		else
		{
			trait_iter = str_iter->second.find(render_trait);
		}

		if (trait_iter == str_iter->second.end())
		{
			const std::pair<trait_to_run_map::iterator, bool> trait_insert_ret = str_iter->second.insert(std::pair<uint128_t, INIT_ONCE>(render_trait, INIT_ONCE()));
			assert(trait_insert_ret.second);
			trait_iter = trait_insert_ret.first;
			InitOnceInitialize(&trait_iter->second);
		}
	}

	glyph_run *a_glyph_run = NULL;
	BOOL pending;
	InitOnceBeginInitialize(&trait_iter->second, 0, &pending, reinterpret_cast<void **>(&a_glyph_run));
	assert((a_glyph_run == NULL) == pending);

	return a_glyph_run;
}

bool glyph_cache::store_glyph_run(string_id_type string_id, uint128_t render_trait, glyph_run *a_glyph_run)
{
	trait_to_run_map::iterator trait_iter;

	{
		bool b_ret;
		string_id_type erased_str;
		std::map<string_id_type, trait_to_run_map>::iterator str_iter;

		const scoped_rw_lock lock_w(scoped_rw_lock::SERVER_GLYPH_RUN_CACHE, false);

		b_ret = _glyph_run_lru.access(string_id, &erased_str);
		if (b_ret)
		{
			// the string is evicted from LRU cache
			// erase all cached glyph run that is under the evicted string ID

			str_iter = _glyph_run_store.find(erased_str);
			assert(str_iter != _glyph_run_store.end());
			erase_glyph_run_cache_string(str_iter);
			_glyph_run_store.erase(str_iter);
		}

		str_iter = _glyph_run_store.find(string_id);
		assert(str_iter != _glyph_run_store.end());
		trait_iter = str_iter->second.find(render_trait);
	}

	InitOnceComplete(&trait_iter->second, (a_glyph_run == NULL ? INIT_ONCE_INIT_FAILED : 0), a_glyph_run);
	return a_glyph_run != NULL;
}

void glyph_cache::erase_glyph_run_cache_string(std::map<string_id_type, trait_to_run_map>::iterator str_iter)
{
	for (trait_to_run_map::iterator trait_iter = str_iter->second.begin(); trait_iter != str_iter->second.end(); ++trait_iter)
	{
		BOOL pending;
		glyph_run *a_glyph_run;
		InitOnceBeginInitialize(&trait_iter->second, INIT_ONCE_CHECK_ONLY, &pending, reinterpret_cast<void **>(&a_glyph_run));
		assert(!pending);
		delete a_glyph_run;
	}
}

}
