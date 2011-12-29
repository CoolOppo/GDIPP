#pragma once

#include "gdipp_config/config.h"
#include "gdipp_config/render_config_cache.h"
#include "gdipp_server/dc_pool.h"
#include "gdipp_server/font_link.h"
#include "gdipp_server/font_mgr.h"
#include "gdipp_server/glyph_cache.h"

namespace gdipp
{

extern config_file config_file_instance;
extern config config_instance;
extern dc_pool dc_pool_instance;
extern font_link font_link_instance;
extern font_mgr font_mgr_instance;
extern render_config_cache font_render_config_cache_instance;
extern glyph_cache glyph_cache_instance;
extern unsigned int server_cache_size;
//extern sqlite3 *index_db_instance;

}
