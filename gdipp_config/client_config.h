#pragma once

#include "gdipp_config/config.h"

namespace gdipp
{

class GDIPP_API client_config : public config
{
	enum PAINTER_TYPE
	{
		PAINTER_GDI = 10,
		PAINTER_D2D = 20
	};

public:
	struct GDIPP_API gamma_config
	{
		gamma_config();

		double red;
		double green;
		double blue;
	};


	struct GDIPP_API shadow_config
	{
		shadow_config();

		int offset_x;
		int offset_y;
		unsigned char alpha;
	};

public:
	client_config();
	void load(const config_file &file);

	gamma_config gamma;
	PAINTER_TYPE painter;
	shadow_config shadow;

private:
	void parse(const void *root);
};

}
