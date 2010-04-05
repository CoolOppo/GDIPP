#include "stdafx.h"
#include "gdimm.h"

// table name for GetFontData() to get whole ttc file data
#define TTCF_FONT_TABLE mmioFOURCC('t', 't', 'c', 'f')

DWORD gdimm_font_man::tls_index;

gdimm_font_man::~gdimm_font_man()
{
	BOOL b_ret;

	// delete linked fonts
	for (map<long, font_info>::const_iterator iter = _linked_fonts.begin(); iter != _linked_fonts.end(); iter++)
	{
		b_ret = DeleteObject(iter->second.hfont);
		assert(b_ret);
	}
}

unsigned long gdimm_font_man::stream_IoFunc(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
{
	// callback function, called when freetype requests font data

	// count == 0 means seek operation
	if (count == 0)
		return 0;

	const long font_id = stream->descriptor.value;
	DWORD read_size;

	const tls_font_holder *font_holder = (const tls_font_holder*) TlsGetValue(tls_index);
	assert(font_holder != NULL);

	if (font_id >= 0)
	{
		const DWORD table_header = font_man_instance._reg_fonts[font_id].table_header;
		read_size = GetFontData(font_holder->registered, table_header, offset, buffer, count);
	}
	else
	{
		const DWORD table_header = font_man_instance._linked_fonts[font_id].table_header;
		read_size = GetFontData(font_holder->linked, table_header, offset, buffer, count);
	}
	assert(read_size == count);

	return read_size;
}

DWORD gdimm_font_man::get_font_size(HDC font_holder, DWORD &table_header) const
{
	table_header = TTCF_FONT_TABLE;

	// specify 0 as table index to retrieve font data from ttf files
	// specify ttcf to retrieve from ttc files
	DWORD font_size = GetFontData(font_holder, table_header, 0, NULL, 0);
	if (font_size == GDI_ERROR)
	{
		table_header = 0;
		font_size = GetFontData(font_holder, table_header, 0, NULL, 0);
		assert(font_size != GDI_ERROR);
	}

	return font_size;
}

HFONT gdimm_font_man::create_linked_font(HDC font_holder, const LOGFONTW &font_attr, const WCHAR *font_family, wstring &font_face) const
{
	LOGFONTW new_font_attr = font_attr;
	
	/*
	this reset is essential to make GetGlyphIndices work correctly
	for example, lfOutPrecision might be OUT_PS_ONLY_PRECIS for Myriad Pro
	if create HFONT of Microsoft YaHei with such lfOutPrecision, GetGlyphIndices always fails
	*/
	new_font_attr.lfOutPrecision = OUT_DEFAULT_PRECIS;
	wcsncpy_s(new_font_attr.lfFaceName, font_family, LF_FACESIZE);

	HFONT new_hfont = CreateFontIndirectW(&new_font_attr);
	assert(new_hfont != NULL);
	
	SelectObject(font_holder, new_hfont);
	UINT metric_size = GetOutlineTextMetricsW(font_holder, 0, NULL);
	if (metric_size == 0)
		return NULL;

	vector<BYTE> metric_buf;
	metric_buf.resize(metric_size);
	metric_size = GetOutlineTextMetricsW(font_holder, metric_size, (OUTLINETEXTMETRICW*) &metric_buf[0]);
	assert(metric_size != 0);

	font_face = metric_face_name(&metric_buf[0]);

	return new_hfont;
}

void *gdimm_font_man::create_linked_font_holder()
{
	BOOL b_ret;

	tls_font_holder *font_holder = (tls_font_holder*) TlsGetValue(tls_index);
	if (font_holder == NULL)
	{
		font_holder = (tls_font_holder*) LocalAlloc(LPTR, sizeof(tls_font_holder));
		assert(font_holder != NULL);

		font_holder->linked = CreateCompatibleDC(NULL);
		assert(font_holder->linked != NULL);

		b_ret = TlsSetValue(tls_index, font_holder);
		assert(b_ret);
	}

	return font_holder;
}

void gdimm_font_man::delete_linked_font_holder()
{
	BOOL b_ret;

	tls_font_holder *font_holder = (tls_font_holder*) TlsGetValue(tls_index);
	if (font_holder == NULL)
		assert(GetLastError() == ERROR_SUCCESS);
	else
	{
		b_ret = DeleteDC(font_holder->linked);
		assert(b_ret);
	}

	font_holder = (tls_font_holder*) LocalFree((HLOCAL) font_holder);
	assert(font_holder == NULL);
}

long gdimm_font_man::register_font(HDC hdc, const WCHAR *font_face)
{
	tls_font_holder *font_holder = (tls_font_holder*) create_linked_font_holder();
	assert(font_holder != NULL);

	// use passed HDC as registered font holder
	font_holder->registered = hdc;

	map<wstring, long>::const_iterator iter = _reg_ids.find(font_face);
	if (iter == _reg_ids.end())
	{
		// double-check interlock
		critical_section interlock(CS_REG_FONT);

		iter = _reg_ids.find(font_face);
		if (iter == _reg_ids.end())
		{
			const long new_font_id = (long) _reg_ids.size();
			font_info new_font_data = {0};

			new_font_data.hfont = NULL;
			new_font_data.stream.size = get_font_size(hdc, new_font_data.table_header);
			new_font_data.stream.descriptor.value = new_font_id;
			new_font_data.stream.read = stream_IoFunc;
			new_font_data.stream.close = stream_CloseFunc;

			_reg_ids[font_face] = new_font_id;
			_reg_fonts[new_font_id] = new_font_data;

			return new_font_id;
		}
	}

	// font existed, use old
	return iter->second;
}

long gdimm_font_man::lookup_font(const LOGFONTW &font_attr, const WCHAR *font_family, wstring &font_face)
{
	// gdimm may be attached to a process which already has multi threads
	// always check if the current thread has linked font holder
	const tls_font_holder *font_holder = (const tls_font_holder*) create_linked_font_holder();

	HFONT linked_font = create_linked_font(font_holder->linked, font_attr, font_family, font_face);
	if (linked_font == NULL)
		return 0;

	map<wstring, long>::const_iterator iter = _linked_ids.find(font_face);
	if (iter == _linked_ids.end())
	{
		// double-check interlock
		critical_section interlock(CS_LINKED_FONT);

		iter = _linked_ids.find(font_face);
		if (iter == _linked_ids.end())
		{
			// negative font id
			const long new_font_id = -((long) _linked_ids.size()) - 1;
			font_info new_font_data = {0};

			new_font_data.hfont = linked_font;
			new_font_data.stream.size = get_font_size(font_holder->linked, new_font_data.table_header);
			new_font_data.stream.descriptor.value = new_font_id;
			new_font_data.stream.read = stream_IoFunc;
			new_font_data.stream.close = stream_CloseFunc;

			_linked_ids[font_face] = new_font_id;
			_linked_fonts[new_font_id] = new_font_data;

			return new_font_id;
		}
	}

	// font existed, use old one

	DeleteObject(linked_font);
	const long font_id = iter->second;
	SelectObject(font_holder->linked, _linked_fonts[font_id].hfont);

	return font_id;
}

void gdimm_font_man::get_glyph_indices(long font_id, const WCHAR *str, int count, WCHAR *gi)
{
	DWORD converted;

	const tls_font_holder *font_holder = (const tls_font_holder*) TlsGetValue(tls_index);
	assert(font_holder != NULL);

	if (font_id >= 0)
		converted = GetGlyphIndices(font_holder->registered, str, count, (LPWORD) gi, GGI_MARK_NONEXISTING_GLYPHS);
	else
		converted = GetGlyphIndices(font_holder->linked, str, count, (LPWORD) gi, GGI_MARK_NONEXISTING_GLYPHS);
	assert(converted == count);
}

FT_Stream gdimm_font_man::get_font_stream(long font_id)
{
	if (font_id >= 0)
		return &_reg_fonts[font_id].stream;
	else
		return &_linked_fonts[font_id].stream;
}