#pragma once

using namespace std;

class gdimm_font_man
{
	/*
	there are two kinds of fonts: registered fonts and linked fonts

	registered fonts are created outside the font manager
	they are considered temporary, not managed by font manager
	there is only one registered font for each thread
	registered fonts are stored in TLS
	registered fonts have non-negative font id

	linked fonts are created by font manager, for font linking
	every linked font are kept alive until the font manager is destructed
	linked fonts are shared by all threads
	linked fonts have negative font id
	*/

	struct font_info
	{
		// handle of the linked font for destructor
		// NULL if registered font
		HFONT hfont;
		DWORD table_header;
		FT_StreamRec stream;
	};

	/*
	HDC that holds font per thread

	registered font holder is created outside the font manager
	font manager simply use the passed HDC to get font data

	linked font holder is created by font manager
	linked fonts are selected into linked font holder before retrieving font data
	*/
	struct tls_font_holder
	{
		HDC registered;
		HDC linked;
	};

	// for registered fonts
	// face name -> font id
	// we use this map because FreeType callback only have face id
	map<wstring, long> _reg_ids;
	// font id -> font info
	// we use this map because vector internally free and re-allocate existing entries
	// pointers become invalid
	map<long, font_info> _reg_fonts;

	// for linked fonts
	map<wstring, long> _linked_ids;
	map<long, font_info> _linked_fonts;

	static unsigned long stream_IoFunc(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count);
	static void stream_CloseFunc(FT_Stream stream) {}

	DWORD get_font_size(HDC font_holder, DWORD &table_header) const;
	HFONT create_linked_font(HDC font_holder, const LOGFONTW &font_attr, const WCHAR *font_family, wstring &font_face) const;

public:
	static DWORD tls_index;

	~gdimm_font_man();

	static void *create_linked_font_holder();
	static void delete_linked_font_holder();

	long register_font(HDC hdc, const WCHAR *font_face);
	long lookup_font(const LOGFONTW &font_attr, const WCHAR *font_family, wstring &font_face);
	void get_glyph_indices(long font_id, const WCHAR *str, int count, WCHAR *gi);

	FT_Stream get_font_stream(long font_id);
};