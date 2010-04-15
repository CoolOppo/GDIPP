#pragma once

using namespace std;

class gdimm_glyph_cache
{
public:
	struct cache_trait
	{
		FTC_FaceID face_id;
		FT_UInt width;
		FT_UInt height;
		FT_F26Dot6 embolden;
		bool is_oblique;
		FT_Render_Mode _render_mode;
		FT_ULong _load_flags;

		bool operator==(const gdimm_glyph_cache::cache_trait &trait) const
		{ return (memcmp(this, &trait, sizeof(gdimm_glyph_cache::cache_trait)) == 0); }
	};

private:
	typedef map<FT_UInt, const FT_BitmapGlyph> cache_map;
	
	struct cache_node
	{
		cache_trait trait;
		cache_map glyph_cache;
		bool using_cache_node;

		cache_node()
		{ using_cache_node = false; }
	};

	list<cache_node> _cache;
	size_t _cached_bytes;

	void erase_glyph_cache(const cache_map &glyph_cache);

public:
	gdimm_glyph_cache()
	{ _cached_bytes = 0; }

	const FT_BitmapGlyph lookup(const cache_trait &trait, FT_UInt glyph_index, bool *&using_cache_node);
	void add(const cache_trait &trait, FT_UInt glyph_index, const FT_BitmapGlyph glyph, bool *&using_cache_node);
	void clear();
};