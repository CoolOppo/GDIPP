#pragma once

using namespace std;

enum RENDERER_TYPE
{
	RENDERER_CLEARTYPE,
	RENDERER_STUB,
	RENDERER_FREETYPE,
	RENDERER_GETGLYPHOUTLINE,
	RENDERER_DIRECTWRITE,
	RENDERER_WIC,

	// count of renderer types
	// never use this type
	_RENDERER_TYPE_COUNT_
};

enum PIXEL_GEOMETRY_TYPE
{
	PIXEL_GEOMETRY_RGB,
	PIXEL_GEOMETRY_BGR
};

struct font_setting_cache
{
	struct font_gamma
	{
		double red;
		double green;
		double blue;

		font_gamma();
	};

	struct font_render_mode
	{
		WORD mono;
		WORD gray;
		WORD subpixel;
		PIXEL_GEOMETRY_TYPE pixel_geometry;

		font_render_mode();
	};

	struct font_shadow
	{
		LONG offset_x;
		LONG offset_y;
		WORD alpha;

		font_shadow();
	};

	WORD auto_hinting;
	bool embedded_bitmap;
	FT_F26Dot6 embolden;
	font_gamma gamma;
	WORD hinting;
	bool kerning;
	LONG max_height;
	font_render_mode render_mode;
	RENDERER_TYPE renderer;
	font_shadow shadow;

	font_setting_cache();
};

struct gdimm_font_trait
{
	const wchar_t *font_name;
	unsigned char weight_class;
	bool italic;
};

class gdimm_setting_cache
{
	struct cache_trait
	{
		wstring font_name;
		unsigned char weight_class;
		bool italic;

		cache_trait(const gdimm_font_trait &font_trait);
		bool operator<(const cache_trait &trait) const;
	};

	typedef map<cache_trait, font_setting_cache> cache_map;

	cache_map _cache;

public:
	const font_setting_cache *lookup(const gdimm_font_trait &font_trait);
};