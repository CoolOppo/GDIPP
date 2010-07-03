#include "stdafx.h"
#include "font_man.h"
#include "ft.h"
#include "lock.h"
#include "text_helper.h"

#define TTCF_TABLE_TAG mmioFOURCC('t', 't', 'c', 'f')

// little-endian <-> big-endian
#define SWAPWORD(x) MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x) MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

map<wstring, long> gdimm_font_man::_reg_name_to_id;
map<wstring, long> gdimm_font_man::_linked_name_to_id;
map<long, gdimm_font_man::font_info> gdimm_font_man::_id_to_info;
DWORD gdimm_font_man::_ref_count = 0;
DWORD gdimm_font_man::_reg_tls_index = 0;
DWORD gdimm_font_man::_linked_tls_index = 0;
set<HDC> all_linked_font_holder;

gdimm_font_man::gdimm_font_man()
{
	_ref_count += 1;

	if (_reg_tls_index == 0)
		_reg_tls_index = create_tls_index();

	if (_linked_tls_index == 0)
		_linked_tls_index = create_tls_index();

	const HDC linked_font_holder = CreateCompatibleDC(NULL);
	TlsSetValue(_linked_tls_index, linked_font_holder);
	all_linked_font_holder.insert(linked_font_holder);
}

gdimm_font_man::~gdimm_font_man()
{
	// delete current thread's font holder for linked fonts
	const HDC linked_font_holder = (HDC) TlsGetValue(_linked_tls_index);
	all_linked_font_holder.erase(linked_font_holder);
	DeleteDC(linked_font_holder);

	_ref_count -= 1;
	if (_ref_count == 0)
	{
		for (map<long, font_info>::const_iterator iter = _id_to_info.begin(); iter != _id_to_info.upper_bound(-1); iter++)
		{
			// delete linked fonts
			DeleteObject(iter->second.linked_hfont);
		}

		for (set<HDC>::const_iterator iter = all_linked_font_holder.begin(); iter != all_linked_font_holder.end(); iter++)
		{

			DeleteDC(*iter);
		}

		// delete TLS indices
		TlsFree(_reg_tls_index);
		TlsFree(_linked_tls_index);
	}
}

unsigned long gdimm_font_man::stream_io(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
{
	// callback function, called when freetype requests font data

	// count == 0 means seek operation
	if (count == 0)
		return 0;

	const long font_id = stream->descriptor.value;
	DWORD tls_index;

	if (font_id >= 0)
		tls_index = _reg_tls_index;
	else
		tls_index = _linked_tls_index;
	
	HDC font_holder = (HDC) TlsGetValue(tls_index);
	const font_info &info = gdimm_font_man::_id_to_info[font_id];
	const DWORD read_size = GetFontData(font_holder, info.table_header, offset, buffer, count);
	assert(read_size == count);

	return read_size;
}

void gdimm_font_man::stream_close(FT_Stream stream)
{
	// GetFontData() needs no close
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

ULONG gdimm_font_man::get_ttc_face_index(HDC font_holder, DWORD ttc_file_size)
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
	ULONG face_count;
	DWORD buffer_len = sizeof(face_count);

	// number of face records in the TTC header
	read_size = GetFontData(font_holder, TTCF_TABLE_TAG, read_offset, &face_count, buffer_len);
	assert(read_size == buffer_len);

	// TrueType font data uses big-endian, while mainstream Windows uses little-endian platforms
	face_count = SWAPLONG(face_count);
	read_offset += buffer_len;

	for (ULONG i = 0; i < face_count; i++)
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

	return ULONG_MAX;
}

HFONT gdimm_font_man::create_linked_font(HDC font_holder, const LOGFONTW &font_attr, const wchar_t *font_family, wstring &font_face)
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

	vector<BYTE> metric_buf;
	OUTLINETEXTMETRICW *outline_metrics;
	if (!get_dc_metrics(font_holder, metric_buf, outline_metrics))
		return NULL;

	font_face = metric_face_name(outline_metrics);

	return new_hfont;
}

FT_Stream gdimm_font_man::lookup_stream(long font_id)
{
	return &_id_to_info[font_id].stream;
}

ULONG gdimm_font_man::lookup_face_index(long font_id)
{
	return _id_to_info[font_id].face_index;
}

const gdimm_os2_metrics *gdimm_font_man::lookup_os2_metrics(long font_id)
{
	return &_id_to_info[font_id].os2_metrics;
}

int gdimm_font_man::lookup_kern(const FTC_Scaler scaler, WORD left_glyph, WORD right_glyph)
{
	FT_Error ft_error;

	FT_Size size;
	ft_error = FTC_Manager_LookupSize(ft_cache_man, scaler, &size);
	assert(ft_error == 0);

	FT_Vector delta;
	ft_error = FT_Get_Kerning(size->face, left_glyph, right_glyph, FT_KERNING_DEFAULT, &delta);
	assert(ft_error == 0);

	return from_26dot6(delta.x);
}

long gdimm_font_man::register_font(HDC font_holder, const wchar_t *font_face)
{
	BOOL b_ret;

	b_ret = TlsSetValue(_reg_tls_index, font_holder);
	assert(b_ret);

	map<wstring, long>::const_iterator iter = _reg_name_to_id.find(font_face);
	if (iter == _reg_name_to_id.end())
	{
		// double-check lock
		gdimm_lock lock(LOCK_REG_FONT);

		iter = _reg_name_to_id.find(font_face);
		if (iter == _reg_name_to_id.end())
		{
			long new_font_id;
			if (_id_to_info.empty())
				new_font_id = 0;
			else
				new_font_id  = (long) _id_to_info.rbegin()->first + 1;

			font_info new_font_data = {};
			new_font_data.stream.size = get_font_size(font_holder, new_font_data.table_header);
			new_font_data.stream.descriptor.value = new_font_id;
			new_font_data.stream.read = stream_io;
			new_font_data.stream.close = stream_close;

			if (new_font_data.table_header != 0)
			{
				new_font_data.face_index = get_ttc_face_index(font_holder, new_font_data.stream.size);
				assert(new_font_data.face_index != ULONG_MAX);
			}

			new_font_data.os2_metrics.init(font_holder);

			_reg_name_to_id[font_face] = new_font_id;
			_id_to_info[new_font_id] = new_font_data;

			return new_font_id;
		}
	}

	return iter->second;
}

long gdimm_font_man::lookup_font(const LOGFONTW &font_attr, const wchar_t *font_family, wstring &font_face)
{
	HDC font_holder = (HDC) TlsGetValue(_linked_tls_index);
	const HFONT linked_hfont = create_linked_font(font_holder, font_attr, font_family, font_face);
	if (linked_hfont == NULL)
		return 0;

	map<wstring, long>::const_iterator iter = _linked_name_to_id.find(font_face);
	if (iter == _linked_name_to_id.end())
	{
		// double-check lock
		gdimm_lock lock(LOCK_LINKED_FONT);

		iter = _linked_name_to_id.find(font_face);
		if (iter == _linked_name_to_id.end())
		{
			// negative font id
			long new_font_id;
			if (_id_to_info.empty())
				new_font_id = -1;
			else
				new_font_id = (long) _id_to_info.begin()->first - 1;

			font_info new_font_data = {};
			new_font_data.linked_hfont = linked_hfont;
			new_font_data.stream.size = get_font_size(font_holder, new_font_data.table_header);
			new_font_data.stream.descriptor.value = new_font_id;
			new_font_data.stream.read = stream_io;
			new_font_data.stream.close = stream_close;

			if (new_font_data.table_header != 0)
			{
				new_font_data.face_index = get_ttc_face_index(font_holder, new_font_data.stream.size);
				assert(new_font_data.face_index != ULONG_MAX);
			}

			new_font_data.os2_metrics.init(font_holder);

			_linked_name_to_id[font_face] = new_font_id;
			_id_to_info[new_font_id] = new_font_data;

			return new_font_id;
		}
	}

	// font existed, use existing font
	DeleteObject(linked_hfont);

	return iter->second;
}

void gdimm_font_man::get_glyph_indices(long font_id, const wchar_t *str, int count, wchar_t *gi)
{
	DWORD tls_index;

	if (font_id >= 0)
		tls_index = _reg_tls_index;
	else
		tls_index = _linked_tls_index;

	HDC font_holder = (HDC) TlsGetValue(tls_index);
	DWORD converted = GetGlyphIndices(font_holder, str, count, (LPWORD) gi, GGI_MARK_NONEXISTING_GLYPHS);
	assert(converted == count);
}