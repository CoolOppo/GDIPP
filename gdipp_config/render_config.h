#pragma once

#include "gdipp_config/config.h"

namespace gdipp
{

class GDIPP_API render_config : public config
{
	friend class render_config_cache;

	enum PIXEL_GEOMETRY_TYPE
	{
		PIXEL_GEOMETRY_RGB,
		PIXEL_GEOMETRY_BGR
	};

	enum RENDERER_TYPE
	{
		RENDERER_CLEARTYPE = 0,
		RENDERER_FREETYPE = 10,
		RENDERER_GETGLYPHOUTLINE = 20,
		RENDERER_DIRECTWRITE = 30,
		RENDERER_WIC = 31
	};

public:
	struct GDIPP_API render_mode_config
	{
		render_mode_config();

		unsigned char mono;
		unsigned char gray;
		unsigned char subpixel;
		PIXEL_GEOMETRY_TYPE pixel_geometry;
		bool aliased_text;
	};

public:
	render_config();
	void load(const config_file &file);

	unsigned char auto_hinting;
	bool embedded_bitmap;
	long embolden;
	unsigned char hinting;
	bool kerning;
	render_mode_config render_mode;
	RENDERER_TYPE renderer;

private:
	void parse(const void *root);
};

}
