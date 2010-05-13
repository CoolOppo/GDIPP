#pragma once

using namespace std;

class gdimm_font_man
{
	/*
	there are two kinds of fonts: registered fonts and linked fonts

	registered fonts are created outside the font manager
	they are considered temporary, not managed by font manager
	there is only one registered font in each thread
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

		// used to retrieve font data from GetFontData
		DWORD table_header;
		DWORD face_index;

		FT_StreamRec stream;
		
		// from OS/2 table
		FT_Short xAvgCharWidth;
		FT_UShort usWeightClass;
		FT_UShort fsSelection;
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

	static unsigned long stream_io(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count);
	static void stream_close(FT_Stream stream);

	static DWORD get_font_size(HDC font_holder, DWORD &table_header);
	static DWORD get_ttc_face_index(HDC font_holder, DWORD ttc_file_size);
	static bool get_os2_info(HDC font_holder, font_info& info);

	HFONT create_linked_font(HDC font_holder, const LOGFONTW &font_attr, const wchar_t *font_family, wstring &font_face) const;

public:
	static DWORD tls_index;

	~gdimm_font_man();

	static void *create_font_holder();
	static void delete_font_holder();

	long register_font(HDC hdc, const wchar_t *font_face);
	long lookup_font(const LOGFONTW &font_attr, const wchar_t *font_family, wstring &font_face);

	void get_glyph_indices(long font_id, const wchar_t *str, int count, wchar_t *gi);
	
	FT_Stream get_stream(long font_id);
	DWORD get_face_index(long font_id);
	FT_Short get_xAvgCharWidth(long font_id);
	FT_UShort get_usWeightClass(long font_id);
	FT_UShort get_fsSelection(long font_id);
};