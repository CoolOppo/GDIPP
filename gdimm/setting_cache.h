#pragma once

using namespace std;

enum RENDERER_TYPE
{
	CLEARTYPE,
	GETGLYPHOUTLINE,
	FREETYPE,
	DIRECTWRITE
};

struct font_setting_cache
{
	struct font_gamma
	{
		double gray;
		double red;
		double green;
		double blue;

		font_gamma()
		:
		gray(1.0),
		red(1.0),
		green(1.0),
		blue(1.0)
		{}
	};

	struct font_shadow
	{
		LONG offset_x;
		LONG offset_y;
		WORD alpha;

		font_shadow()
		:
		offset_x(0),
		offset_y(0),
		alpha(8)
		{}
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

	font_setting_cache()
	:
	auto_hinting(false),
	embedded_bitmap(false),
	embolden(0),
	hinting(true),
	light_mode(true),
	max_height(72),
	render_mono(false),
	render_non_aa(false),
	renderer(FREETYPE),
	subpixel_render(true),
	zero_alpha(false)
	{}
};

class gdimm_setting_cache
{
	typedef map<const wstring, font_setting_cache> cache_map;

	cache_map _cache;

public:
	const font_setting_cache *lookup(const wchar_t *font_name);
};