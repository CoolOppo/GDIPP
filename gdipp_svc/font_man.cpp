#include "stdafx.h"
#include "font_man.h"

#define buf_family_name(metric_buf) (reinterpret_cast<const wchar_t *>(metric_buf + reinterpret_cast<const UINT>((reinterpret_cast<const OUTLINETEXTMETRICW *>(metric_buf)->otmpFamilyName))))
#define buf_face_name(metric_buf) (reinterpret_cast<const wchar_t *>(metric_buf + reinterpret_cast<const UINT>((reinterpret_cast<const OUTLINETEXTMETRICW *>(metric_buf)->otmpFaceName))))
#define buf_style_name(metric_buf) (reinterpret_cast<const wchar_t *>(metric_buf + reinterpret_cast<const UINT>((reinterpret_cast<const OUTLINETEXTMETRICW *>(metric_buf)->otmpStyleName))))
#define metric_family_name(outline_metric) (reinterpret_cast<const wchar_t *>(reinterpret_cast<const BYTE *>(outline_metric) + reinterpret_cast<const UINT>(outline_metric->otmpFamilyName)))
#define metric_face_name(outline_metric) (reinterpret_cast<const wchar_t *>(reinterpret_cast<const BYTE *>(outline_metric) + reinterpret_cast<const UINT>(outline_metric->otmpFaceName)))
#define metric_style_name(outline_metric) (reinterpret_cast<const wchar_t *>(reinterpret_cast<const BYTE *>(outline_metric) + reinterpret_cast<const UINT>(outline_metric->otmpStyleName)))

OUTLINETEXTMETRICW *gdipp_font_man::get_dc_metrics(HDC hdc, vector<BYTE> &metric_buf)
{
	// get outline metrics of the DC, which also include the text metrics

	UINT metric_size = GetOutlineTextMetricsW(hdc, 0, NULL);
	if (metric_size == 0)
		return NULL;

	metric_buf.resize(metric_size);
	OUTLINETEXTMETRICW *outline_metrics = reinterpret_cast<OUTLINETEXTMETRICW *>(&metric_buf[0]);
	metric_size = GetOutlineTextMetricsW(hdc, metric_size, outline_metrics);
	assert(metric_size != 0);

	return outline_metrics;
}

unsigned long gdipp_font_man::stream_io(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
{
	// callback function, called when freetype requests font data

	return 0;
}

void gdipp_font_man::stream_close(FT_Stream stream)
{
	// GetFontData() needs no close
}

unsigned long gdipp_font_man::register_font(const LOGFONTW *attr_buf, DWORD buf_size)
{
	const HFONT linked_hfont = CreateFontIndirectW(attr_buf);
	if (linked_hfont == NULL)
		return 0;

	HDC font_holder = CreateCompatibleDC(NULL);
	SelectObject(font_holder, linked_hfont);

	vector<BYTE> metric_buf;
	OUTLINETEXTMETRICW *outline_metrics;
	outline_metrics = get_dc_metrics(font_holder, metric_buf);
	if (outline_metrics == NULL)
		return 0;

	wstring font_face = metric_face_name(outline_metrics);
	map<wstring, unsigned long>::const_iterator iter = font_name_to_id.find(font_face);
	if (iter == font_name_to_id.end())
	{
		const unsigned long new_font_id = id_to_info.size();

		const font_info new_font_data = {linked_hfont, metric_buf};
		font_name_to_id[font_face] = new_font_id;
		id_to_info[new_font_id] = new_font_data;

		DeleteDC(font_holder);
		return new_font_id;
	}

	// font existed, use existing font
	DeleteObject(linked_hfont);
	DeleteDC(font_holder);

	return iter->second;
}

DWORD gdipp_font_man::get_font_raw_data(unsigned long font_id, DWORD table, DWORD offset, LPVOID data_buf, DWORD buf_size)
{
	HDC font_holder = CreateCompatibleDC(NULL);
	SelectObject(font_holder, id_to_info[font_id].font_handle);

	const DWORD data_size = GetFontData(font_holder, table, offset, data_buf, buf_size);
	
	DeleteDC(font_holder);
	
	return data_size;
}

DWORD gdipp_font_man::get_font_metrics(unsigned long font_id, LPVOID metrics_buf, DWORD buf_size)
{
	if (metrics_buf == NULL || buf_size == 0)
		return id_to_info[font_id].metric_buf.size();
	else
	{
		const DWORD copy_size = min(static_cast<DWORD>(id_to_info[font_id].metric_buf.size()), buf_size);
		CopyMemory(metrics_buf, &id_to_info[font_id].metric_buf[0], copy_size);
		return copy_size;
	}
}

DWORD gdipp_font_man::get_glyph_indices(unsigned long font_id, const wchar_t *str, int count, unsigned short *gi)
{
	HDC font_holder = CreateCompatibleDC(NULL);
	SelectObject(font_holder, id_to_info[font_id].font_handle);

	const DWORD converted = GetGlyphIndices(font_holder, str, count, gi, GGI_MARK_NONEXISTING_GLYPHS);

	DeleteDC(font_holder);

	return converted;
}

FT_Stream gdipp_font_man::lookup_stream(long font_id)
{
	return NULL;
}

ULONG gdipp_font_man::lookup_face_index(long font_id)
{
	return 0;
}