#pragma once

#include <map>
#include "gdipp_support/config_trait.h"
#include "gdipp_lib/type_enum.h"

namespace gdipp
{

struct font_setting_cache
{
	struct font_gamma
	{
		font_gamma();

		double red;
		double green;
		double blue;
	};

	struct font_render_mode
	{
		font_render_mode();

		BYTE mono;
		BYTE gray;
		BYTE subpixel;
		PIXEL_GEOMETRY_TYPE pixel_geometry;
		bool aliased_text;
	};

	struct font_shadow
	{
		font_shadow();

		LONG offset_x;
		LONG offset_y;
		BYTE alpha;
	};

	font_setting_cache();

	BYTE auto_hinting;
	bool embedded_bitmap;
	FT_F26Dot6 embolden;
	font_gamma gamma;
	BYTE hinting;
	bool kerning;
	font_render_mode render_mode;
	RENDERER_TYPE renderer;
	font_shadow shadow;
};

class config_cache
{
public:
	const font_setting_cache *lookup(const config_trait *setting_trait);

private:
	// setting ID : setting cache
	std::map<unsigned int, font_setting_cache> _cache;
};

}
