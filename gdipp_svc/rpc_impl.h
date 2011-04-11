#pragma once

#include "font_man.h"
#include "glyph_cache.h"
#include <setting_cache.h>
#include <support_pool.h>

class gdimm_renderer;

// actual session handle structure
struct gdipp_rpc_session
{
	void *font_id;
	const font_setting_cache *font_setting;
	HDC hdc;
	FT_Render_Mode render_mode;
	unsigned int render_trait;
	gdimm_renderer *renderer;
};

extern gdipp_pool<HDC> dc_pool_instance;
extern gdipp_font_man font_man_instance;
extern gdipp_glyph_cache glyph_cache_instance;
//extern sqlite3 *index_db_instance;

DWORD WINAPI start_gdipp_rpc_server(LPVOID lpParameter);
bool stop_gdipp_rpc_server();