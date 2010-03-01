#pragma once

#include <map>
using namespace std;

struct font_info
{
	HFONT hfont;
	bool owned;
	DWORD table_header;
	FT_StreamRec stream;
	LPVOID mapping_start;
};

class _gdimm_font_man
{
	// face name -> font id
	map<wstring, long> _font_ids;
	// font id -> face info
	map<long, font_info> _loaded_fonts;

	HDC _font_holder;

	static unsigned long stream_IoFunc(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count);
	static void stream_CloseFunc(FT_Stream stream) {};
	DWORD get_font_size(HFONT hfont, DWORD *table_header);
	void use_mapping(const WCHAR *font_full_name);

public:
	_gdimm_font_man()
	{ _font_holder = CreateCompatibleDC(NULL); }
	~_gdimm_font_man();

	long register_font(HFONT hfont, const WCHAR *font_family, const WCHAR *font_style);
	long lookup_font(const LOGFONTW &font_attr, const WCHAR *font_family, const WCHAR *font_style);
	WORD get_glyph_index(WCHAR ch);
	FT_Stream get_font_stream(long font_id)
	{ return &_loaded_fonts[font_id].stream; }
};

typedef singleton<_gdimm_font_man> gdimm_font_man;