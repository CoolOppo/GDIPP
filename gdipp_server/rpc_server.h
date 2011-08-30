#pragma once

#include "gdipp_config/render_config.h"

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

DWORD WINAPI start_gdipp_rpc_server(LPVOID lpParameter);
bool stop_gdipp_rpc_server();

}
