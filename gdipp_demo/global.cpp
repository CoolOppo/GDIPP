#include "stdafx.h"
#include "global.h"

namespace gdipp
{

config_file config_file_instance(L"demo.conf");
config config_instance(config_file_instance);

HMODULE gdipp::h_gdimm = NULL;
WCHAR gdimm_path[MAX_PATH];
std::vector<HWND> paint_hwnd;

}
