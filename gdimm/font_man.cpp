#include "stdafx.h"
#include "gdimm.h"
#include "ft.h"
#include "lock.h"

#define TTCF_TABLE_TAG mmioFOURCC('t', 't', 'c', 'f')
#define OS2_TABLE_TAG mmioFOURCC('O', 'S', '/', '2')

// little-endian <-> big-endian
#define SWAPWORD(x) MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x) MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

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

unsigned long gdimm_font_man::stream_io(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
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

void gdimm_font_man::stream_close(FT_Stream stream)
{
	// GetFontData needs no close
}

DWORD gdimm_font_man::get_font_size(HDC font_holder, DWORD &table_header)
{
	table_header = TTCF_TABLE_TAG;

	/*
	try to get font file size with ttcf tag first
	if succeeds, the font face is part of a TTC file
	otherwise, the font face is a standalone TrueType font file
	*/
	DWORD font_size = GetFontData(font_holder, table_header, 0, NULL, 0);
	if (font_size == GDI_ERROR)
	{
		table_header = 0;
		font_size = GetFontData(font_holder, table_header, 0, NULL, 0);
		assert(font_size != GDI_ERROR);
	}

	return font_size;
}

DWORD gdimm_font_man::get_ttc_face_index(HDC font_holder, DWORD ttc_file_size)
{
	// get the index of the current face in its TTC file
	// by comparing its start offset retrieved from GetFontData and from the TTC header

	// pre-condition: the font file contains TTC header

	DWORD read_size;

	// start offset of the current face
	DWORD face_start = GetFontData(font_holder, 0, 0, NULL, 0);
	assert(face_start != GDI_ERROR);
	face_start = ttc_file_size - face_start;

	DWORD read_offset = sizeof(DWORD) + sizeof(FIXED);
	DWORD face_count;
	DWORD buffer_len = sizeof(face_count);

	// number of face records in the TTC header
	read_size = GetFontData(font_holder, TTCF_TABLE_TAG, read_offset, &face_count, buffer_len);
	assert(read_size == buffer_len);

	// TrueType font data uses big-endian, while mainstream Windows uses little-endian platforms
	face_count = SWAPLONG(face_count);
	read_offset += buffer_len;

	for (DWORD i = 0; i < face_count; i++)
	{
		// start offset of the current record
		DWORD curr_start;
		buffer_len = sizeof(curr_start);
		read_size = GetFontData(font_holder, TTCF_TABLE_TAG, read_offset, &curr_start, buffer_len);
		assert(read_size == buffer_len);
		curr_start = SWAPLONG(curr_start);

		if (curr_start == face_start)
			return i;

		read_offset += buffer_len;
	}

	return -1;
}

bool gdimm_font_man::get_os2_info(HDC font_holder, font_info& info)
{
	DWORD font_data_size;

	font_data_size = GetFontData(font_holder,
		OS2_TABLE_TAG,
		offsetof(TT_OS2, xAvgCharWidth),
		&info.xAvgCharWidth,
		sizeof(FT_Short));
	if (font_data_size == GDI_ERROR)
		return false;

	font_data_size = GetFontData(font_holder,
		OS2_TABLE_TAG,
		offsetof(TT_OS2, usWeightClass),
		&info.usWeightClass,
		sizeof(FT_UShort));
	if (font_data_size == GDI_ERROR)
		return false;

	font_data_size = GetFontData(font_holder,
		OS2_TABLE_TAG,
		offsetof(TT_OS2, fsSelection),
		&info.fsSelection,
		sizeof(FT_UShort));
	if (font_data_size == GDI_ERROR)
		return false;

	info.xAvgCharWidth = SWAPWORD(info.xAvgCharWidth);
	info.usWeightClass = SWAPWORD(info.usWeightClass);
	info.fsSelection = SWAPWORD(info.fsSelection);

	return true;
}

HFONT gdimm_font_man::create_linked_font(HDC font_holder, const LOGFONTW &font_attr, const wchar_t *font_family, wstring &font_face) const
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

	font_face = metric_face_name((OUTLINETEXTMETRICW*) &metric_buf[0]);

	return new_hfont;
}

void *gdimm_font_man::create_font_holder()
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

void gdimm_font_man::delete_font_holder()
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

long gdimm_font_man::register_font(HDC hdc, const wchar_t *font_face)
{
	bool b_ret;

	tls_font_holder *font_holder = (tls_font_holder*) create_font_holder();
	assert(font_holder != NULL);

	// use passed HDC as registered font holder
	font_holder->registered = hdc;

	map<wstring, long>::const_iterator iter = _reg_ids.find(font_face);
	if (iter == _reg_ids.end())
	{
		// double-check lock
		gdimm_lock lock(LOCK_REG_FONT);

		iter = _reg_ids.find(font_face);
		if (iter == _reg_ids.end())
		{
			const long new_font_id = (long) _reg_ids.size();
			font_info new_font_data = {};

			new_font_data.hfont = NULL;
			new_font_data.stream.size = get_font_size(font_holder->registered, new_font_data.table_header);
			new_font_data.stream.descriptor.value = new_font_id;
			new_font_data.stream.read = stream_io;
			new_font_data.stream.close = stream_close;

			if (new_font_data.table_header != 0)
			{
				new_font_data.face_index = get_ttc_face_index(font_holder->registered, new_font_data.stream.size);
				assert(new_font_data.face_index != -1);
			}

			b_ret = get_os2_info(font_holder->registered, new_font_data);
			assert(b_ret);

			_reg_ids[font_face] = new_font_id;
			_reg_fonts[new_font_id] = new_font_data;

			return new_font_id;
		}
	}

	// font existed, use old
	return iter->second;
}

long gdimm_font_man::lookup_font(const LOGFONTW &font_attr, const wchar_t *font_family, wstring &font_face)
{
	bool b_ret;

	// gdimm may be attached to a process which already has multi threads
	// always check if the current thread has linked font holder
	const tls_font_holder *font_holder = (const tls_font_holder*) create_font_holder();

	HFONT linked_font = create_linked_font(font_holder->linked, font_attr, font_family, font_face);
	if (linked_font == NULL)
		return 0;

	map<wstring, long>::const_iterator iter = _linked_ids.find(font_face);
	if (iter == _linked_ids.end())
	{
		// double-check lock
		gdimm_lock lock(LOCK_LINKED_FONT);

		iter = _linked_ids.find(font_face);
		if (iter == _linked_ids.end())
		{
			// negative font id
			const long new_font_id = -((long) _linked_ids.size()) - 1;
			font_info new_font_data = {};

			new_font_data.hfont = linked_font;
			new_font_data.stream.size = get_font_size(font_holder->linked, new_font_data.table_header);
			new_font_data.stream.descriptor.value = new_font_id;
			new_font_data.stream.read = stream_io;
			new_font_data.stream.close = stream_close;

			if (new_font_data.table_header != 0)
			{
				new_font_data.face_index = get_ttc_face_index(font_holder->linked, new_font_data.stream.size);
				assert(new_font_data.face_index != -1);
			}

			b_ret = get_os2_info(font_holder->linked, new_font_data);
			assert(b_ret);

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

void gdimm_font_man::get_glyph_indices(long font_id, const wchar_t *str, int count, wchar_t *gi)
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

FT_Stream gdimm_font_man::get_stream(long font_id)
{
	if (font_id >= 0)
		return &_reg_fonts[font_id].stream;
	else
		return &_linked_fonts[font_id].stream;
}

ULONG gdimm_font_man::get_face_index(long font_id)
{
	if (font_id >= 0)
		return _reg_fonts[font_id].face_index;
	else
		return _linked_fonts[font_id].face_index;
}

FT_Short gdimm_font_man::get_xAvgCharWidth(long font_id)
{
	if (font_id >= 0)
		return _reg_fonts[font_id].xAvgCharWidth;
	else
		return _linked_fonts[font_id].xAvgCharWidth;
}

FT_UShort gdimm_font_man::get_usWeightClass(long font_id)
{
	if (font_id >= 0)
		return _reg_fonts[font_id].usWeightClass;
	else
		return _linked_fonts[font_id].usWeightClass;
}

FT_UShort gdimm_font_man::get_fsSelection(long font_id)
{
	if (font_id >= 0)
		return _reg_fonts[font_id].fsSelection;
	else
		return _linked_fonts[font_id].fsSelection;
}