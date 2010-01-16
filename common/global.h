#pragma once

#include <map>
#include <string>
using namespace std;

// font mapping information
struct font_mapping
{
	void *data_start;
	DWORD data_length;
	HANDLE file_mapping;
};

// the font table maps the File Mapping name to the mapping information
typedef basic_string<TCHAR> t_string;
typedef pair<t_string, font_mapping> string_to_mapping;
typedef map<t_string, font_mapping> font_table;

extern font_table loaded_fonts;

BOOL is_font_true_type(HDC hdc);

void debug_output(const TCHAR *str);
void debug_output(DWORD num);