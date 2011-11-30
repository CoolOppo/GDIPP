#pragma once

#include "gdipp_config/config.h"

namespace gdipp
{

extern config_file config_file_instance;
extern config config_instance;

// client related
extern HMODULE h_client;
extern WCHAR client_path[MAX_PATH];
extern std::vector<HWND> paint_hwnd;

}

extern CAppModule _Module;
