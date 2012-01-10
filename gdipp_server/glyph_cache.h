#pragma once

#include "gdipp_lib/lru.h"
#include "gdipp_server/glyph_run.h"

namespace gdipp
{

class glyph_cache
{
public:
	typedef uint128_t string_id_type;
	typedef uint128_t char_id_type;
	typedef std::pair<string_id_type, uint128_t> glyph_run_id_type;

	static string_id_type get_string_id(const wchar_t *string, unsigned int count, bool is_glyph_index);
	static char_id_type get_char_id(uint128_t render_trait, FT_UInt index, bool is_glyph_index);

	~glyph_cache();

	void initialize();

	const FT_Glyph lookup_glyph(char_id_type char_id);
	bool store_glyph(char_id_type char_id, const FT_Glyph glyph);
	const glyph_run *lookup_glyph_run(string_id_type string_id, uint128_t render_trait);
	bool store_glyph_run(string_id_type string_id, uint128_t render_trait, glyph_run *a_glyph_run);

private:
	// std::map from render trait to glyph run
	typedef std::map<uint128_t, INIT_ONCE> trait_to_run_map;

	void erase_glyph_run_cache_string(std::map<string_id_type, trait_to_run_map>::iterator str_iter);

	// std::map from character ID (including character index and render trait) to its glyph
	std::map<char_id_type, INIT_ONCE> _glyph_store;
	// std::map from string ID to glyph run
	// use hierarchical design so that when LRU string is evicted, all associated glyph runs are erased
	std::map<string_id_type, trait_to_run_map> _glyph_run_store;

	// least recently used glyph runs, indexed by string ID
	lru_list<string_id_type> _glyph_run_lru;
};

}
