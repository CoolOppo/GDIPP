#pragma once

using namespace std;

  #define hook
  #define test

extern int total_count;
extern vector<const wstring> candidate_font;
extern bool random_text;

#define GDIPP_DEMO_MAX_STR_LEN 100
extern wchar_t window_title[GDIPP_DEMO_MAX_STR_LEN];

extern HMODULE h_gdimm;
extern WCHAR gdimm_path[MAX_PATH];