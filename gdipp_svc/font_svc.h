#pragma once

#include "font_store.h"

extern gdipp_font_store font_store_instance;

DWORD WINAPI start_gdipp_rpc_server(LPVOID lpParameter);
bool stop_gdipp_rpc_server();