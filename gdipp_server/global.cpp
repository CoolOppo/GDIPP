#include "stdafx.h"
#include "global.h"

namespace gdipp
{

config_file config_file_instance(L"server.conf");
config config_instance(config_file_instance);
dc_pool dc_pool_instance;
font_mgr font_mgr_instance;
glyph_cache glyph_cache_instance;
unsigned int server_cache_size;

}
