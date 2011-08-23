#pragma once

#include "gdipp_support/config.h"

namespace gdipp
{

class freetype_config : public config
{
public:
	freetype_config();
	void load(const pugi::xml_node &root);

	FT_UInt cache_max_faces;
	FT_UInt cache_max_sizes;
	FT_ULong cache_max_bytes;
	FT_LcdFilter lcd_filter;
};

}
