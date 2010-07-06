#pragma once

#include "os2_metrics.h"
using namespace std;

class gdimm_font_man
{
	/*
	font manager sees two kinds of fonts: registered fonts and linked fonts

	registered fonts are created outside the font manager
	they are considered temporary, not managed by font manager
	registered fonts have non-negative font id
	the font information are shared by all threads, however the font holder is stored in TLS

	linked fonts are created by font manager, for font linking
	every linked font are kept alive until the font manager is destructed
	linked fonts have negative font id
	the handle and information of linked fonts are shared by all threads, however the font holder is stored in TLS
	*/

	struct font_info
	{
		// handle of the linked font for destruction
		// NULL for registered fonts
		HFONT linked_hfont;

		// the buffer of the outline metrics of the linked font
		// empty for registered fonts
		vector<BYTE> metric_buf;

		// used to retrieve font data from GetFontData
		DWORD table_header;
		DWORD face_index;

		FT_StreamRec stream;
		gdimm_os2_metrics os2_metrics;
	};

	// face name -> font id
	// we use this map because FreeType callback only have face id
	static map<wstring, long> _reg_name_to_id;
	static map<wstring, long> _linked_name_to_id;
	// font id -> font info
	// we use this map because vector internally free and re-allocate existing entries
	// pointers become invalid
	static map<long, font_info> _id_to_info;
	
	// number of font manager instances
	// data shared by all threads are not deleted until all font manager instances are deleted
	static DWORD _ref_count;

	// TLS indices of the font holders
	// one font holder per thread
	static DWORD _reg_tls_index;
	static DWORD _linked_tls_index;

	static unsigned long stream_io(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count);
	static void stream_close(FT_Stream stream);

	static DWORD get_font_size(HDC font_holder, DWORD &table_header);
	static ULONG get_ttc_face_index(HDC font_holder, DWORD ttc_file_size);

	HFONT gdimm_font_man::create_linked_font(HDC font_holder,
		const LOGFONTW &font_attr,
		const wchar_t *font_family,
		vector<BYTE> &metric_buf,
		OUTLINETEXTMETRICW *&outline_metrics);

public:
	static FT_Stream lookup_stream(long font_id);
	static ULONG lookup_face_index(long font_id);
	static const gdimm_os2_metrics *lookup_os2_metrics(long font_id);
	static const OUTLINETEXTMETRICW *lookup_outline_metrics(long font_id);
	static int lookup_kern(const FTC_Scaler scaler, WORD left_glyph, WORD right_glyph);

	gdimm_font_man();
	~gdimm_font_man();

	long register_font(HDC font_holder, const wchar_t *font_face);
	long lookup_font(const LOGFONTW &font_attr, const wchar_t *font_family, wstring &font_face);
	void get_glyph_indices(long font_id, const wchar_t *str, int count, wchar_t *gi);
};