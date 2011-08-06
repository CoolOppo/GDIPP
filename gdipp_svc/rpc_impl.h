#pragma once

#include "gdipp_lib/config_cache.h"
#include "gdipp_svc/dc_pool.h"
#include "gdipp_svc/font_man.h"
#include "gdipp_svc/glyph_cache.h"

namespace gdipp
{

class gdimm_renderer;

// actual session handle structure
struct rpc_session
{
	void *font_id;
	const font_setting_cache *font_setting;
	HDC hdc;
	FT_Render_Mode render_mode;
	unsigned int render_trait;
	gdimm_renderer *renderer;
};

extern dc_pool dc_pool_instance;
extern font_man font_man_instance;
extern glyph_cache glyph_cache_instance;
//extern sqlite3 *index_db_instance;

DWORD WINAPI start_gdipp_rpc_server(LPVOID lpParameter);
bool stop_gdipp_rpc_server();

}
