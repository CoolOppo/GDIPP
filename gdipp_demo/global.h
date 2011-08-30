#pragma once

#include "gdipp_config/demo_config.h"

namespace gdipp
{

extern config_file config_file_instance;
extern demo_config demo_conf;

// gdimm related
extern HMODULE h_gdimm;
extern WCHAR gdimm_path[MAX_PATH];
extern std::vector<HWND> paint_hwnd;

}

extern CAppModule _Module;
