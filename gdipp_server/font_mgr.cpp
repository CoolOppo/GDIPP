#include "stdafx.h"
#include "font_mgr.h"
#include "gdipp_lib/scoped_rw_lock.h"
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

font_mgr::font_mgr()
{
	_font_holder_tls_index = TlsAlloc();
	assert(_font_holder_tls_index != TLS_OUT_OF_INDEXES);
}

font_mgr::~font_mgr()
{
	for (std::map<std::wstring, font_entry>::const_iterator iter = _font_registry.begin(); iter != _font_registry.end(); ++iter)
		DeleteObject(iter->second.font_handle);

	TlsFree(_font_holder_tls_index);
}

void *font_mgr::register_font(HDC font_holder, const LOGFONTW *log_font, BYTE **outline_metrics_buf, unsigned long *outline_metrics_size)
{
	// create a font with supplied LOGFONT and retrieve related information

	bool b_ret;

	std::wstring font_face;
	std::map<std::wstring, font_entry>::iterator font_iter;

	const HFONT hfont = CreateFontIndirectW(log_font);
	if (hfont == NULL)
		return NULL;

	SelectObject(font_holder, hfont);

	*outline_metrics_size = get_dc_outline_metrics(font_holder, outline_metrics_buf);
	if (*outline_metrics_size == 0)
	{
		DeleteObject(hfont);
		return NULL;
	}

	const OUTLINETEXTMETRICW *outline_metrics = reinterpret_cast<const OUTLINETEXTMETRICW *>(*outline_metrics_buf);
	font_face = metric_face_name(outline_metrics);
	font_iter = _font_registry.find(font_face);
	if (font_iter == _font_registry.end())
	{
		const scoped_rw_lock lock_w(scoped_rw_lock::SERVER_FONT_MGR, false);
		font_iter = _font_registry.find(font_face);
		if (font_iter == _font_registry.end())
		{
			os2_metrics os2;
			b_ret = os2.init(font_holder);
			if (!b_ret)
			{
				DeleteObject(hfont);
				return NULL;
			}

			DWORD table_header;
			DWORD font_size = get_font_size(font_holder, &table_header);
			if (font_size == GDI_ERROR)
			{
				DeleteObject(hfont);
				return NULL;
			}

			DWORD face_index = 0;
			if (table_header != 0)
			{
				face_index = get_ttc_face_index(font_holder, font_size);
				if (face_index == ULONG_MAX)
				{
					DeleteObject(hfont);
					return NULL;
				}
			}

			const std::pair<std::map<std::wstring, font_entry>::iterator, bool> insert_ret = _font_registry.insert(std::pair<std::wstring, font_entry>(font_face, font_entry()));
			assert(insert_ret.second);
			font_iter = insert_ret.first;

			font_iter->second.font_handle = hfont;
			font_iter->second.os2 = os2;
			font_iter->second.face_index = face_index;
			font_iter->second.table_header = table_header;

			font_iter->second.stream.size = font_size;
			// need the table header to retrieve font data (see stream_io())
			font_iter->second.stream.descriptor.value = table_header;
			font_iter->second.stream.read = font_mgr::stream_io;
			font_iter->second.stream.close = font_mgr::stream_close;

			return &font_iter->second;
		}
	}

	// font has been created before
	DeleteObject(hfont);
	return &font_iter->second;
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

HDC font_mgr::get_thread_font_holder() const
{
	return reinterpret_cast<HDC>(TlsGetValue(_font_holder_tls_index));
}

BOOL font_mgr::set_thread_font_holder(HDC font_holder) const
{
	return TlsSetValue(_font_holder_tls_index, font_holder);
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

	const DWORD read_size = GetFontData(font_mgr_instance.get_thread_font_holder(), stream->descriptor.value, offset, buffer, count);
	assert(read_size != GDI_ERROR);
	assert(read_size == count);

	return read_size;
}

void font_mgr::stream_close(FT_Stream stream)
{
	// GetFontData() needs no close
}

}
