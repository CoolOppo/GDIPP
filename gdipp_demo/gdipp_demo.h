#pragma once

#include <vector>

// demo settings
extern int total_count;
extern int thread_count;
extern bool random_text;
extern std::vector<const std::wstring> paint_fonts;

// gdimm related
extern HMODULE h_gdimm;
extern WCHAR gdimm_path[MAX_PATH];
extern std::vector<HWND> paint_hwnd;
