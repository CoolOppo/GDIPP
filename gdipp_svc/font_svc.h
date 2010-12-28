#pragma once

#include "font_man.h"

extern gdipp_font_man font_man_instance;

DWORD WINAPI start_gdipp_rpc_server(LPVOID lpParameter);
bool stop_gdipp_rpc_server();