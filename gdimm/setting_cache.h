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
	LONG max_height;
	font_render_mode render_mode;
	RENDERER_TYPE renderer;
	font_shadow shadow;
	bool use_alpha;

	font_setting_cache();
};

class gdimm_setting_cache
{
	typedef map<const wstring, font_setting_cache> cache_map;

	cache_map _cache;

public:
	const font_setting_cache *lookup(const wchar_t *font_name);
};