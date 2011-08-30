#pragma once

#include "gdipp_config/config.h"

namespace gdipp
{

class GDIPP_API freetype_config : public config
{
public:
	freetype_config();
	void load(const config_file &file);

	FT_UInt cache_max_faces;
	FT_UInt cache_max_sizes;
	FT_ULong cache_max_bytes;
	FT_LcdFilter lcd_filter;

private:
	void parse(const void *root);
};

}
