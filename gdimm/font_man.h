#pragma once

#include "os2_metrics.h"

using namespace std;

class gdimm_font_man
{
	HDC _reg_font_holder;
	HDC _linked_font_holder;

	static unsigned long stream_io(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count);
	static void stream_close(FT_Stream stream);

	static DWORD get_font_size(HDC font_holder, DWORD &table_header);
	static ULONG get_ttc_face_index(HDC font_holder, DWORD ttc_file_size);

	static HFONT gdimm_font_man::create_linked_font(HDC font_holder,
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