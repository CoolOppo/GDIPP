#pragma once

#include "gdipp_config/config.h"
#include "gdipp_config/constant_render.h"
#include "gdipp_lib/api.h"

namespace gdipp
{

class GDIPP_API render_config_static
{
public:
	struct GDIPP_API render_mode_static
	{
		render_mode_static();

		unsigned char mono;
		unsigned char gray;
		unsigned char subpixel;
		render_config::PIXEL_GEOMETRY_TYPE pixel_geometry;
		bool aliased_text;
	};

public:
	render_config_static();
	void parse(const config &cfg);

	unsigned char auto_hinting;
	bool embedded_bitmap;
	long embolden;
	unsigned char hinting;
	bool kerning;
	render_mode_static render_mode;
	render_config::RENDERER_TYPE renderer;
};

}
