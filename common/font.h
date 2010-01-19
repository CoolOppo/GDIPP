#pragma once

#include "global.h"
#include <map>

using namespace std;

struct font_info
{
	HDC hdc;
	DWORD table_header;
	FT_StreamRec stream;
	LPVOID mapping_start;
};

class _gdimm_font
{
	// face name -> face id
	map<t_string, unsigned int> font_indices;
	// face id -> face info
	map<unsigned int, font_info> loaded_fonts;

	static DWORD get_font_size(HDC hdc, DWORD *table_header);
	static unsigned long stream_IoFunc(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count);
	static void stream_CloseFunc(FT_Stream stream);
	void use_mapping(const t_string &font_full_name);

public:
	static LOGFONT get_font_attr(HDC hdc);
	static t_string get_font_full_name(HDC hdc);
	unsigned int lookup_index(const t_string &font_full_name, HDC hdc);
	font_info &get_info(unsigned int id);
};

typedef singleton<_gdimm_font> gdimm_font;