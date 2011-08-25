#pragma once

#include "gdipp_config/config.h"
#include "gdipp_lib/type_enum.h"

namespace gdipp
{

class GDIPP_API render_config : public config
{
public:
	struct GDIPP_API gamma_config
	{
		gamma_config();

		double red;
		double green;
		double blue;
	};

	struct GDIPP_API render_mode_config
	{
		render_mode_config();

		unsigned char mono;
		unsigned char gray;
		unsigned char subpixel;
		PIXEL_GEOMETRY_TYPE pixel_geometry;
		bool aliased_text;
	};

	struct GDIPP_API shadow_config
	{
		shadow_config();

		int offset_x;
		int offset_y;
		unsigned char alpha;
	};

public:
	render_config();
	void load(const void *root);

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
