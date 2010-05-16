#pragma once

using namespace std;

enum RENDERER_TYPE
{
	CLEARTYPE,
	DIRECTWRITE,
	FREETYPE,
	GETGLYPHOUTLINE,

	// count of renderer types
	// never use this type
	_RENDERER_TYPE_COUNT_
};

struct font_setting_cache
{
	struct font_gamma
	{
		double gray;
		double red;
		double green;
		double blue;

		font_gamma();
	};

	struct font_shadow
	{
		LONG offset_x;
		LONG offset_y;
		WORD alpha;

		font_shadow();
	};

	bool auto_hinting;
	bool embedded_bitmap;
	FT_F26Dot6 embolden;
	font_gamma gamma;
	bool hinting;
	bool light_mode;
	LONG max_height;
	bool render_mono;
	bool render_non_aa;
	RENDERER_TYPE renderer;
	font_shadow shadow;
	bool subpixel_render;
	bool zero_alpha;

	font_setting_cache();
};

class gdimm_setting_cache
{
	typedef map<const wstring, font_setting_cache> cache_map;

	cache_map _cache;

public:
	const font_setting_cache *lookup(const wchar_t *font_name);
};