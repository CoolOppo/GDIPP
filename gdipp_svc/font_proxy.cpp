#include "stdafx.h"
#include "font_man.h"
#include "freetype.h"

#define TTCF_TABLE_TAG mmioFOURCC('t', 't', 'c', 'f')

// little-endian <-> big-endian
#define SWAPWORD(x) MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x) MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

DWORD font_holder_tls_index = 0;

gdimm_font_man::gdimm_font_man()
{
	font_holder_tls_index = TlsAlloc();
	assert(font_holder_tls_index != TLS_OUT_OF_INDEXES);
}

gdimm_font_man::~gdimm_font_man()
{
	BOOL b_ret;

	b_ret = TlsFree(font_holder_tls_index);
	assert(b_ret);
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

DWORD gdimm_font_man::get_font_size(long font_id, DWORD &table_header)
{
	error_status_t rpc_error;
	table_header = TTCF_TABLE_TAG;
	DWORD font_size;

	/*
	try to get font file size with ttcf tag first
	if succeeds, the font face is part of a TTC file
	otherwise, the font face is a standalone TrueType font file
	*/
	rpc_error = gdipp_rpc_get_font_data_size(h_gdipp_rpc, font_id, table_header, 0, &font_size);
	if (rpc_error != 0)
		return 0;

	if (font_size == GDI_ERROR)
	{
		table_header = 0;
		rpc_error = gdipp_rpc_get_font_data_size(h_gdipp_rpc, font_id, table_header, 0, &font_size);
		assert(rpc_error == 0 && font_size != GDI_ERROR);
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

ULONG gdimm_font_man::get_ttc_face_index(long font_id, DWORD ttc_file_size)
{
	// get the index of the current face in its TTC file
	// by comparing its start offset retrieved from GetFontData and from the TTC header

	// pre-condition: the font file contains TTC header

	error_status_t rpc_error;
	DWORD face_start, read_size;

	// start offset of the current face
	rpc_error = gdipp_rpc_get_font_data_size(h_gdipp_rpc, font_id, 0, 0, &face_start);
	assert(rpc_error == 0 && face_start != GDI_ERROR);
	face_start = ttc_file_size - face_start;

	DWORD read_offset = sizeof(DWORD) + sizeof(FIXED);
	ULONG face_count;
	DWORD buffer_len = sizeof(face_count);

	// number of face records in the TTC header
	rpc_error = gdipp_rpc_get_font_data(h_gdipp_rpc, font_id, TTCF_TABLE_TAG, read_offset, reinterpret_cast<byte *>(&face_count), buffer_len, &read_size);
	assert(rpc_error == 0 && read_size == buffer_len);

	// TrueType font data uses big-endian, while mainstream Windows uses little-endian platforms
	face_count = SWAPLONG(face_count);
	read_offset += buffer_len;

	for (ULONG i = 0; i < face_count; i++)
	{
		// start offset of the current record
		DWORD curr_start;
		buffer_len = sizeof(curr_start);
		rpc_error = gdipp_rpc_get_font_data(h_gdipp_rpc, font_id, TTCF_TABLE_TAG, read_offset, reinterpret_cast<byte *>(&curr_start), buffer_len, &read_size);
		assert(rpc_error == 0 && read_size == buffer_len);
		curr_start = SWAPLONG(curr_start);

		if (curr_start == face_start)
			return i;

		read_offset += buffer_len;
	}

	return ULONG_MAX;
}

unsigned long gdimm_font_man::stream_io(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
{
	// callback function, called when freetype requests font data

	error_status_t rpc_error;

	// count == 0 means seek operation
	if (count == 0)
		return 0;

	const long font_id = stream->descriptor.value;

	const font_info *info = font_man_instance.get_font_info(font_id);
	assert(info != NULL);

	DWORD read_size;
	if (font_id < 0)
	{
		HDC font_holder = reinterpret_cast<HDC>(TlsGetValue(font_holder_tls_index));
		assert(font_holder != NULL);
		read_size = GetFontData(font_holder, info->table_header, offset, buffer, count);
	}
	else
	{
		rpc_error = gdipp_rpc_get_font_data(h_gdipp_rpc, font_id, info->table_header, offset, buffer, count, &read_size);
		assert(rpc_error == 0);
	}
	assert(read_size == count);

	return read_size;
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

	return int_from_26dot6(delta.x);
}

void gdimm_font_man::stream_close(FT_Stream stream)
{
	// GetFontData() needs no close
}

const font_info * gdimm_font_man::get_font_info(long font_id)
{
	return &_id_to_info[font_id];
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

long gdimm_font_man::register_font(HDC font_holder, const wchar_t *font_face)
{
	TlsSetValue(font_holder_tls_index, font_holder);

	map<wstring, long>::const_iterator iter = _name_to_id.find(font_face);
	if (iter == _name_to_id.end())
	{
		// double-check lock
		gdimm_lock lock(LOCK_REG_FONT);
		iter = _name_to_id.find(font_face);
		if (iter == _name_to_id.end())
		{
			long new_font_id;
			if (_id_to_info.empty())
				new_font_id = -1;
			else
				new_font_id = (long) _id_to_info.begin()->first - 1;

			font_info new_font_data = {};
			new_font_data.stream.size = get_font_size(font_holder, new_font_data.table_header);
			new_font_data.stream.descriptor.value = new_font_id;
			new_font_data.stream.read = gdimm_font_man::stream_io;
			new_font_data.stream.close = gdimm_font_man::stream_close;

			if (new_font_data.table_header != 0)
			{
				new_font_data.face_index = get_ttc_face_index(font_holder, new_font_data.stream.size);
				assert(new_font_data.face_index != ULONG_MAX);
			}

			new_font_data.os2_metrics.init(font_holder);

			_name_to_id[font_face] = new_font_id;
			_id_to_info[new_font_id] = new_font_data;

			return new_font_id;
		}
	}

	return iter->second;
}

long gdimm_font_man::link_font(const LOGFONTW &linked_font_attr, wstring &linked_font_face)
{
	error_status_t rpc_error;
	long font_id;

	rpc_error = gdipp_rpc_register_font(h_gdipp_rpc, reinterpret_cast<const byte *>(&linked_font_attr), sizeof(linked_font_attr), reinterpret_cast<unsigned long *>(&font_id));
	assert(rpc_error == 0 && font_id >= 0);

	map<long, font_info>::const_iterator iter = _id_to_info.find(font_id);
	if (iter == _id_to_info.end())
	{
		// double-check lock
		gdimm_lock lock(LOCK_REG_FONT);
		iter = _id_to_info.find(font_id);
		if (iter == _id_to_info.end())
		{
			font_info new_font_data = {};
			new_font_data.stream.size = get_font_size(font_id, new_font_data.table_header);
			new_font_data.stream.descriptor.value = font_id;
			new_font_data.stream.read = gdimm_font_man::stream_io;
			new_font_data.stream.close = gdimm_font_man::stream_close;

			if (new_font_data.table_header != 0)
			{
				new_font_data.face_index = get_ttc_face_index(font_id, new_font_data.stream.size);
				assert(new_font_data.face_index != ULONG_MAX);
			}

			new_font_data.os2_metrics.init(font_id);

			_id_to_info[font_id] = new_font_data;
		}
	}

	DWORD metric_buf_size;

	rpc_error = gdipp_rpc_get_font_metrics_size(h_gdipp_rpc, font_id, &metric_buf_size);
	assert(rpc_error == 0 && metric_buf_size > 0);

	byte *font_metrics = new byte[metric_buf_size];

	rpc_error = gdipp_rpc_get_font_metrics_data(h_gdipp_rpc, font_id, font_metrics, metric_buf_size, &metric_buf_size);
	assert(rpc_error == 0 && metric_buf_size > 0);

	linked_font_face = metric_face_name(font_metrics);

	delete[] font_metrics;

	return font_id;
}

void gdimm_font_man::get_glyph_indices(long font_id, const wchar_t *str, int count, wchar_t *gi)
{
	error_status_t rpc_error;
	DWORD converted;

	if (font_id < 0)
	{
		HDC font_holder = reinterpret_cast<HDC>(TlsGetValue(font_holder_tls_index));
		assert(font_holder != NULL);
		converted = GetGlyphIndices(font_holder, str, count, reinterpret_cast<LPWORD>(gi), GGI_MARK_NONEXISTING_GLYPHS);
	}
	else
	{
		rpc_error = gdipp_rpc_get_glyph_indices(h_gdipp_rpc, font_id, str, count, reinterpret_cast<unsigned short *>(gi), &converted);
		assert(rpc_error == 0);
	}
	assert(converted == count);
}