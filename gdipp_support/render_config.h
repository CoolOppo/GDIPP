#pragma once

#include "gdipp_support/config.h"
#include "gdipp_support/type_enum.h"

namespace gdipp
{

class render_config : public config
{
public:
	struct gamma_config
	{
		gamma_config();

		double red;
		double green;
		double blue;
	};

	struct render_mode_config
	{
		render_mode_config();

		unsigned char mono;
		unsigned char gray;
		unsigned char subpixel;
		PIXEL_GEOMETRY_TYPE pixel_geometry;
		bool aliased_text;
	};

	struct shadow_config
	{
		shadow_config();

		int offset_x;
		int offset_y;
		unsigned char alpha;
	};

public:
	render_config();
	void load(const pugi::xml_node &root);

	unsigned char auto_hinting;
	bool embedded_bitmap;
	long embolden;
	gamma_config gamma;
	unsigned char hinting;
	bool kerning;
	render_mode_config render_mode;
	RENDERER_TYPE renderer;
	shadow_config shadow;
};

}
