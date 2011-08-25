#pragma once

#include "gdipp_server/dc_pool.h"
#include "gdipp_server/font_mgr.h"
#include "gdipp_server/glyph_cache.h"
#include "gdipp_config/config_mgr.h"

namespace gdipp
{

class renderer;

// actual session handle structure
struct rpc_session
{
	void *font_id;
	render_config render_config;
	HDC hdc;
	FT_Render_Mode render_mode;
	unsigned int render_trait;
	renderer *renderer;
};

extern config_mgr config_mgr_instance;
extern dc_pool dc_pool_instance;
extern font_mgr font_mgr_instance;
extern glyph_cache glyph_cache_instance;
//extern sqlite3 *index_db_instance;

DWORD WINAPI start_gdipp_rpc_server(LPVOID lpParameter);
bool stop_gdipp_rpc_server();

}
