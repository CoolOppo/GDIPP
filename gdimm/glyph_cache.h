#pragma once

using namespace std;

class gdimm_glyph_cache
{
public:
	struct cache_trait
	{
		FTC_ScalerRec _scaler;
		FT_Render_Mode _render_mode;
		FT_ULong _load_flags;

		bool operator<(const gdimm_glyph_cache::cache_trait &trait) const
		{ return (memcmp(this, &trait, sizeof(gdimm_glyph_cache::cache_trait)) < 0); }
	};

private:
	typedef map<FT_UInt, const FT_BitmapGlyph> cache_node;
	typedef map<cache_trait, cache_node> cache_map;

	cache_map _cache;

	void remove_node(const cache_trait &trait);

public:
	~gdimm_glyph_cache();

	const FT_BitmapGlyph lookup(const cache_trait &trait, FT_UInt glyph_index) const;
	bool add(const cache_trait &trait, FT_UInt glyph_index, const FT_BitmapGlyph glyph);
};
