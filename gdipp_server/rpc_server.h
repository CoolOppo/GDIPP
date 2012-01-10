#pragma once

#include "gdipp_config/render_config_static.h"

namespace gdipp
{

class renderer;

// actual session handle structure
struct rpc_session
{
	unsigned short bits_per_pixel;
	HDC font_holder;
	void *font_id;
	/*
	LOGFONT is not directly mapped to a font
	instead, it is just a hint of how to create a font
	font links may affect the mapping result
	therefore, LOGFONT is part of session information
	*/
	LOGFONTW log_font;
	BYTE *outline_metrics_buf;
	unsigned long outline_metrics_size;
	const render_config_static *render_config;
	FT_Render_Mode render_mode;
	uint128_t render_trait;
	renderer *renderer;
};

DWORD WINAPI start_gdipp_rpc_server(LPVOID lpParameter);
bool stop_gdipp_rpc_server();

}
