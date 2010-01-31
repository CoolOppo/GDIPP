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

class _gdimm_font_man
{
	// face name -> face id
	map<t_string, unsigned int> _font_indices;
	// face id -> face info
	map<unsigned int, font_info> _loaded_fonts;

	static DWORD get_font_size(HDC hdc, DWORD *table_header);
	static unsigned long stream_IoFunc(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count);
	static void stream_CloseFunc(FT_Stream stream);
	void use_mapping(const TCHAR *font_full_name);

public:
	unsigned int lookup_index(HDC hdc, const TCHAR *font_full_name);
	font_info &get_info(unsigned int id);
};

typedef singleton<_gdimm_font_man> gdimm_font_man;