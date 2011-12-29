#include "stdafx.h"
#include "font_mgr.h"
#include "gdipp_lib/lock.h"
#include "gdipp_server/freetype.h"
#include "gdipp_server/global.h"
#include "gdipp_server/helper.h"

#define TTCF_TABLE_TAG mmioFOURCC('t', 't', 'c', 'f')

// little-endian <-> big-endian
#define SWAPWORD(x) MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x) MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

namespace gdipp
{

DWORD font_mgr::get_font_size(HDC font_holder, DWORD *table_header)
{
	*table_header = TTCF_TABLE_TAG;

	/*
	try to get font file size with ttcf tag first
	if succeeds, the font face is part of a TTC file
	otherwise, the font face is a standalone TrueType font file
	*/
	DWORD font_size = GetFontData(font_holder, *table_header, 0, NULL, 0);
	if (font_size == GDI_ERROR)
	{
		*table_header = 0;
		font_size = GetFontData(font_holder, *table_header, 0, NULL, 0);
		assert(font_size != GDI_ERROR);
	}

	return font_size;
}

ULONG font_mgr::get_ttc_face_index(HDC font_holder, DWORD ttc_file_size)
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

void *font_mgr::register_font(const LOGFONTW *log_font, BYTE **outline_metrics_buf, unsigned long *outline_metrics_size, HDC hdc)
{
	// create a font with supplied LOGFONT and retrieve related information

	bool b_ret;
	font_entry *font_id = NULL;
	HDC font_holder;

	std::wstring font_face;
	std::map<std::wstring, font_entry>::iterator font_iter;

	// whether delete the created font when exiting (when anything failed or font existed)
	bool delete_font = true;

	const HFONT hfont = CreateFontIndirectW(log_font);
	if (hfont == NULL)
		return NULL;

	/*
	if an HDC is supplied, use the HDC to select font
	otherwise, claim a temporary HDC from DC pool and use it
	temporary HDC will be freed before exiting function
	*/
	if (hdc == NULL)
	{
		font_holder = dc_pool_instance.claim();
		assert(font_holder != NULL);
	}
	else
	{
		font_holder = hdc;
		SelectObject(font_holder, hfont);
	}

	*outline_metrics_size = get_dc_outline_metrics(font_holder, outline_metrics_buf);
	if (*outline_metrics_size == 0)
		goto clean_up_and_return;

	const OUTLINETEXTMETRICW *outline_metrics = reinterpret_cast<const OUTLINETEXTMETRICW *>(*outline_metrics_buf);
	font_face = metric_face_name(outline_metrics);
	font_iter = _font_registry.find(font_face);
	if (font_iter == _font_registry.end())
	{
		lock l(lock::SERVER_FONT_MGR);
		font_iter = _font_registry.find(font_face);
		if (font_iter == _font_registry.end())
		{
			os2_metrics os2;
			b_ret = os2.init(font_holder);
			if (!b_ret)
				goto clean_up_and_return;

			DWORD table_header;
			DWORD font_size = get_font_size(font_holder, &table_header);
			if (font_size == GDI_ERROR)
				goto clean_up_and_return;

			DWORD face_index = 0;
			if (table_header != 0)
			{
				face_index = get_ttc_face_index(font_holder, font_size);
				if (face_index == ULONG_MAX)
					goto clean_up_and_return;
			}

			const font_entry new_font_data = {};
			_font_registry[font_face] = new_font_data;
			font_id = &_font_registry[font_face];

			font_id->font_handle = hfont;
			font_id->os2 = os2;
			font_id->face_index = face_index;
			font_id->table_header = table_header;

			font_id->stream.size = font_size;
			font_id->stream.descriptor.pointer = font_id;
			font_id->stream.read = font_mgr::stream_io;
			font_id->stream.close = font_mgr::stream_close;

			delete_font = false;

			// goto is require for double checking lock
			// destructor of the lock will be called before jumping
			goto clean_up_and_return;
		}
	}

	font_id = &font_iter->second;

clean_up_and_return:
	// if anything failed, delete created font
	if (delete_font)
		DeleteObject(hfont);

	if (hdc == NULL)
		dc_pool_instance.free(font_holder);

	return font_id;
}

HFONT font_mgr::select_font(void *font_id, HDC hdc) const
{
	const font_entry *curr_font = reinterpret_cast<const font_entry *>(font_id);
	return reinterpret_cast<HFONT>(SelectObject(hdc, curr_font->font_handle));
}

ULONG font_mgr::lookup_face_index(void *font_id) const
{
	const font_entry *curr_font = reinterpret_cast<const font_entry *>(font_id);
	return curr_font->face_index;
}

DWORD font_mgr::lookup_font_data(void *font_id, DWORD table, DWORD offset, LPVOID data_buf, DWORD buf_size, HDC hdc) const
{
	const font_entry *curr_font = reinterpret_cast<const font_entry *>(font_id);
	HDC font_holder;

	if (hdc == NULL)
	{
		font_holder = dc_pool_instance.claim();
		assert(font_holder != NULL);
	}
	else
	{
		font_holder = hdc;
	}
	SelectObject(font_holder, curr_font->font_handle);

	const DWORD data_size = GetFontData(font_holder, table, offset, data_buf, buf_size);

	if (hdc == NULL)
		dc_pool_instance.free(font_holder);

	return data_size;
}

DWORD font_mgr::lookup_glyph_indices(void *font_id, const wchar_t *str, int count, unsigned short *gi, HDC hdc) const
{
	const font_entry *curr_font = reinterpret_cast<const font_entry *>(font_id);
	HDC font_holder;

	if (hdc == NULL)
	{
		font_holder = dc_pool_instance.claim();
		assert(font_holder != NULL);
	}
	else
	{
		font_holder = hdc;
	}
	SelectObject(font_holder, curr_font->font_handle);

	const DWORD converted = GetGlyphIndices(font_holder, str, count, gi, GGI_MARK_NONEXISTING_GLYPHS);

	if (hdc == NULL)
		dc_pool_instance.free(font_holder);

	return converted;
}

const os2_metrics *font_mgr::lookup_os2_metrics(void *font_id) const
{
	const font_entry *curr_font = reinterpret_cast<const font_entry *>(font_id);
	return &curr_font->os2;
}

FT_Stream font_mgr::lookup_stream(void *font_id) const
{
	font_entry *curr_font = reinterpret_cast<font_entry *>(font_id);
	return &curr_font->stream;
}

unsigned long font_mgr::get_dc_outline_metrics(HDC hdc, BYTE **outline_metrics_buf)
{
	// get outline metrics of the DC, which also include the text metrics

	unsigned long outline_metrics_size = GetOutlineTextMetricsW(hdc, 0, NULL);
	if (outline_metrics_size == 0)
		return outline_metrics_size;

	*outline_metrics_buf = new BYTE[outline_metrics_size];
	outline_metrics_size = GetOutlineTextMetricsW(hdc, outline_metrics_size, reinterpret_cast<OUTLINETEXTMETRICW *>(*outline_metrics_buf));
	assert(outline_metrics_size != 0);

	return outline_metrics_size;
}

unsigned long font_mgr::stream_io(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
{
	// callback function, called when freetype requests font data

	// count == 0 means seek operation
	if (count == 0)
		return 0;

	font_entry *font_id = reinterpret_cast<font_entry *>(stream->descriptor.pointer);
	const DWORD read_size = font_mgr_instance.lookup_font_data(font_id, font_id->table_header, offset, buffer, count);
	assert(read_size == count);

	return read_size;
}

void font_mgr::stream_close(FT_Stream stream)
{
	// GetFontData() needs no close
}

}
