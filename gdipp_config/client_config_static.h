#pragma once

#include "gdipp_config/config.h"
#include "gdipp_config/constant_client.h"

namespace gdipp
{

class GDIPP_API client_config_static
{
public:
	struct gamma_static
	{
		gamma_static();

		double red;
		double green;
		double blue;
	};

	struct shadow_static
	{
		shadow_static();

		int offset_x;
		int offset_y;
		unsigned char alpha;
	};

public:
	client_config_static();
	void parse(const config &cfg);

	gamma_static gamma;
	client_config::PAINTER_TYPE painter;
	client_config::PIXEL_GEOMETRY_TYPE pixel_geometry;
	shadow_static shadow;
};

}
