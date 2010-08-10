#include "stdafx.h"
#include "font_store.h"
#include "helper_func.h"
#include "lock.h"

#define TTCF_TABLE_TAG mmioFOURCC('t', 't', 'c', 'f')

// little-endian <-> big-endian
#define SWAPWORD(x) MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x) MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

gdimm_font_store::gdimm_font_store()
{
	_font_man_tls_index = TlsAlloc();
	assert(_font_man_tls_index != TLS_OUT_OF_INDEXES);
}

gdimm_font_store::~gdimm_font_store()
{
	BOOL b_ret;

	for (map<long, font_info>::const_iterator iter = _id_to_info.begin(); iter != _id_to_info.upper_bound(-1); iter++)
	{
		// delete linked fonts
		DeleteObject(iter->second.linked_hfont);
	}

	b_ret = TlsFree(_font_man_tls_index);
	assert(b_ret);
}

DWORD gdimm_font_store::get_font_size(HDC font_holder, DWORD &table_header)
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

ULONG gdimm_font_store::get_ttc_face_index(HDC font_holder, DWORD ttc_file_size)
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

font_info *gdimm_font_store::lookup_font(long font_id)
{
	map<long, font_info>::iterator iter = _id_to_info.find(font_id);
	if (iter == _id_to_info.end())
		return NULL;

	return &iter->second;
}

long gdimm_font_store::register_font(HDC font_holder, const wchar_t *font_face)
{
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
			new_font_data.stream.read = gdimm_font_man::stream_io;
			new_font_data.stream.close = gdimm_font_man::stream_close;

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

long gdimm_font_store::link_font(HDC font_holder, HFONT linked_hfont, wstring &linked_font_face)
{
	vector<BYTE> metric_buf;
	OUTLINETEXTMETRICW *outline_metrics;
	outline_metrics = get_dc_metrics(font_holder, metric_buf);
	if (outline_metrics == NULL)
		return 0;

	linked_font_face = metric_face_name(outline_metrics);

	map<wstring, long>::const_iterator iter = _linked_name_to_id.find(linked_font_face);
	if (iter == _linked_name_to_id.end())
	{
		// double-check lock
		gdimm_lock lock(LOCK_LINKED_FONT);

		iter = _linked_name_to_id.find(linked_font_face);
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
			new_font_data.stream.read = gdimm_font_man::stream_io;
			new_font_data.stream.close = gdimm_font_man::stream_close;

			if (new_font_data.table_header != 0)
			{
				new_font_data.face_index = get_ttc_face_index(font_holder, new_font_data.stream.size);
				assert(new_font_data.face_index != ULONG_MAX);
			}

			new_font_data.os2_metrics.init(font_holder);

			_linked_name_to_id[linked_font_face] = new_font_id;
			_id_to_info[new_font_id] = new_font_data;

			return new_font_id;
		}
	}

	// font existed, use existing font
	DeleteObject(linked_hfont);

	return iter->second;
}

BOOL gdimm_font_store::register_thread_font_man(gdimm_font_man *font_man)
{
	return TlsSetValue(_font_man_tls_index, font_man);
}

const gdimm_font_man *gdimm_font_store::lookup_thread_font_man()
{
	return static_cast<const gdimm_font_man *>(TlsGetValue(_font_man_tls_index));
}