#include "stdafx.h"
#include "global.h"

namespace gdipp
{

config_file config_file_instance(L"demo.conf");
config config_instance(config_file_instance);

HMODULE gdipp::h_client = NULL;
WCHAR client_path[MAX_PATH];
std::vector<HWND> paint_hwnd;

}
