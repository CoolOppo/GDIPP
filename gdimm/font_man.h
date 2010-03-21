#pragma once

using namespace std;

class gdimm_font_man
{
	struct font_info
	{
		HDC font_holder;
		HFONT hfont;
		bool owned;
		DWORD table_header;
		FT_StreamRec stream;
		LPVOID mapping_start;
	};

	// face name -> font id
	map<wstring, long> _font_ids;
	// font id -> face info
	map<long, font_info> _loaded_fonts;

	static unsigned long stream_IoFunc(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count);
	static void stream_CloseFunc(FT_Stream stream) {};
	HFONT create_linked_font(HDC font_holder, const LOGFONTW &font_attr, const WCHAR *font_family, const WCHAR *&font_face);
	DWORD get_font_size(HDC font_holder, HFONT hfont, DWORD &table_header);
	void use_mapping(HDC font_holder, const WCHAR *font_face);

public:
	~gdimm_font_man();

	long register_font(HDC font_holder, HFONT new_font, const WCHAR *font_face);
	long lookup_font(HDC font_holder, const LOGFONTW &font_attr, const WCHAR *font_family, const WCHAR *&font_face);
	WORD get_glyph_index(HDC font_holder, WCHAR ch);
	FT_Stream get_font_stream(long font_id)
	{ return &_loaded_fonts[font_id].stream; }
};