#include "stdafx.h"
#include "glyph_cache.h"

gdimm_glyph_cache::~gdimm_glyph_cache()
{
	//for (cache_map::iterator iter = _cache.begin(); iter != _cache.end(); iter++)
	//	remove_node(iter->first);
}

void gdimm_glyph_cache::remove_node(const cache_trait &trait)
{
	for (cache_node::const_iterator iter = _cache[trait].begin(); iter != _cache[trait].end(); iter++)
		FT_Done_Glyph((FT_Glyph) iter->second);

	_cache.erase(trait);
}

const FT_BitmapGlyph gdimm_glyph_cache::lookup(const cache_trait &trait, FT_UInt glyph_index) const
{
	cache_map::const_iterator map_iter = _cache.find(trait);
	if (map_iter == _cache.end())
		return NULL;

	cache_node::const_iterator node_iter = map_iter->second.find(glyph_index);
	if (node_iter == map_iter->second.end())
		return NULL;

	return node_iter->second;
}

bool gdimm_glyph_cache::add(const cache_trait &trait, FT_UInt glyph_index, const FT_BitmapGlyph glyph)
{
	cache_map::iterator map_iter = _cache.find(trait);
	if (map_iter == _cache.end())
		_cache[trait].insert(pair<FT_UInt, const FT_BitmapGlyph>(glyph_index, glyph));
	else
	{
		cache_node::const_iterator node_iter = map_iter->second.find(glyph_index);
		if (node_iter == map_iter->second.end())
			map_iter->second.insert(pair<FT_UInt, const FT_BitmapGlyph>(glyph_index, glyph));
		else
			return false;
	}
	
	return true;
}