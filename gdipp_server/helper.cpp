#include "stdafx.h"
#include "helper.h"

namespace gdipp
{

FIXED fixed_from_26dot6(signed long x)
{
	signed long y = (x << 10);
	return *(reinterpret_cast<FIXED *>(&y));
}

signed long fixed_to_26dot6(const FIXED &x)
{
	return *(reinterpret_cast<const signed long *>(&x)) >> 10;
}

signed long float_to_16dot16(double x)
{
	return static_cast<FT_Pos>(x * 65536);
}

LONG int_from_16dot16(signed long x)
{
	const LONG ret = (x >> 16);

	if (ret == 0 && x > 0)
		return 1;
	else
		return ret;
}

LONG int_from_26dot6(signed long x)
{
	const LONG ret = (x >> 6);

	if (ret == 0 && x > 0)
		return 1;
	else
		return ret;
}

DWORD create_tls_index()
{
	DWORD new_tls_index = TlsAlloc();
	assert(new_tls_index != TLS_OUT_OF_INDEXES);

	return new_tls_index;
}

BOOL free_tls_index(DWORD tls_index)
{
	return TlsFree(tls_index);
}

BYTE division_by_255(short number, short numerator)
{
	// there are many approaches to approximate number * numerator / 255
	// it is a trade-off between efficiency and accuracy

	const int t = number * numerator;
	return (((t + 255) >> 8) + t) >> 8;
}

POINT get_baseline(UINT alignment, int x, int y, int width, int ascent, int descent)
{
	POINT baseline = {x, y};

	switch ((TA_LEFT | TA_RIGHT | TA_CENTER) & alignment)
	{
	case TA_LEFT:
		break;
	case TA_RIGHT:
		baseline.x -= width;
		break;
	case TA_CENTER:
		baseline.x -= width / 2;
		break;
	}

	switch ((TA_TOP | TA_BOTTOM | TA_BASELINE) & alignment)
	{
	case TA_TOP:
		baseline.y += ascent;
		break;
	case TA_BOTTOM:
		baseline.y -= descent;
		break;
	case TA_BASELINE:
		break;
	}

	return baseline;
}

int get_bmp_pitch(int width, WORD bpp)
{
#define FT_PAD_FLOOR( x, n )  ( (x) & ~((n)-1) )
#define FT_PAD_ROUND( x, n )  FT_PAD_FLOOR( (x) + ((n)/2), n )
#define FT_PAD_CEIL( x, n )   FT_PAD_FLOOR( (x) + ((n)-1), n )

	return FT_PAD_CEIL(static_cast<int>(ceil(static_cast<double>(width * bpp) / 8)), sizeof(LONG));
}

bool get_dc_bmp_header(HDC hdc, BITMAPINFOHEADER &dc_bmp_header)
{
	dc_bmp_header.biSize = sizeof(BITMAPINFOHEADER);

	const HBITMAP dc_bitmap = static_cast<const HBITMAP>(GetCurrentObject(hdc, OBJ_BITMAP));
	if (dc_bitmap == NULL)
	{
		// currently no selected bitmap
		// use DC capability

		dc_bmp_header.biWidth = GetDeviceCaps(hdc, HORZRES);
		dc_bmp_header.biHeight = GetDeviceCaps(hdc, VERTRES);
		dc_bmp_header.biPlanes = GetDeviceCaps(hdc, PLANES);
		dc_bmp_header.biBitCount = GetDeviceCaps(hdc, BITSPIXEL);

		return false;
	}
	else
	{
		// do not return the color table
		dc_bmp_header.biBitCount = 0;
		const int i_ret = GetDIBits(hdc, dc_bitmap, 0, 0, NULL, reinterpret_cast<LPBITMAPINFO>(&dc_bmp_header), DIB_RGB_COLORS);
		assert(i_ret != 0);

		return true;
	}
}

OUTLINETEXTMETRICW *get_dc_metrics(HDC hdc, std::vector<BYTE> &metric_buf)
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

int get_glyph_bmp_width(const FT_Bitmap &bitmap)
{
	if (bitmap.pixel_mode == FT_PIXEL_MODE_LCD)
		return bitmap.width / 3;
	else
		return bitmap.width;
}

/*LONG get_glyph_run_width(const glyph_run *a_glyph_run, bool is_control_width)
{
	assert(a_glyph_run != NULL);

	std::list<RECT>::const_iterator first_box_iter;
	std::list<RECT>::const_reverse_iterator last_box_iter;

	if (is_control_width)
	{
		// use control box metrics
		first_box_iter = a_glyph_run->ctrl_boxes.begin();
		last_box_iter = a_glyph_run->ctrl_boxes.rbegin();
	}
	else
	{
		// use black box metrics
		first_box_iter = a_glyph_run->black_boxes.begin();
		last_box_iter = a_glyph_run->black_boxes.rbegin();
	}

	if (a_glyph_run->ctrl_boxes.back().left >= a_glyph_run->ctrl_boxes.front().left)
		return last_box_iter->right - first_box_iter->left;
	else
		return first_box_iter->right - last_box_iter->left;
}*/

LOGFONTW get_log_font(HDC hdc)
{
	HFONT h_font = reinterpret_cast<HFONT>(GetCurrentObject(hdc, OBJ_FONT));
	assert(h_font != NULL);

	LOGFONTW font_attr;
	GetObject(h_font, sizeof(LOGFONTW), &font_attr);

	return font_attr;
}

bool get_render_mode(const render_config_static::render_mode_static &render_mode_conf, WORD dc_bmp_bpp, BYTE font_quality, FT_Render_Mode &render_mode)
{
	// return true if successfully find an appropriate render mode
	// otherwise return false

	if (render_mode_conf.mono == 2)
	{
		render_mode = FT_RENDER_MODE_MONO;
		return true;
	}

	if (render_mode_conf.gray == 2)
	{
		render_mode = FT_RENDER_MODE_NORMAL;
		return true;
	}

	if (render_mode_conf.subpixel == 2)
	{
		render_mode = FT_RENDER_MODE_LCD;
		return true;
	}

	if (!render_mode_conf.aliased_text && font_quality == NONANTIALIASED_QUALITY)
		return false;

	if (render_mode_conf.mono == 1 && dc_bmp_bpp == 1)
	{
		render_mode = FT_RENDER_MODE_MONO;
		return true;
	}

	if (render_mode_conf.gray == 1 && dc_bmp_bpp == 8)
	{
		render_mode = FT_RENDER_MODE_NORMAL;
		return true;
	}

	// we do not support 16 bpp currently

	if (render_mode_conf.subpixel == 1 && dc_bmp_bpp >= 24)
	{
		render_mode = FT_RENDER_MODE_LCD;
		return true;
	}

	return false;
}

bool operator<(const LOGFONTW &lf1, const LOGFONTW &lf2)
{
	return memcmp(&lf1, &lf2, sizeof(LOGFONTW)) < 0;
}

bool mb_to_wc(const char *multi_byte_str, int count, std::wstring &wide_char_str)
{
	int wc_str_len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, multi_byte_str, count, NULL, 0);
	if (wc_str_len == 0)
		return false;

	wide_char_str.resize(wc_str_len);
	wc_str_len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, multi_byte_str, count, &wide_char_str[0], wc_str_len);
	if (wc_str_len == 0)
		return false;

	return true;
}

BOOL paint_background(HDC hdc, const RECT *bg_rect, COLORREF bg_color)
{
	int i_ret;

	if (bg_color == CLR_INVALID)
		return FALSE;

	const HBRUSH bg_brush = CreateSolidBrush(bg_color);
	if (bg_brush == NULL)
		return FALSE;

	i_ret = FillRect(hdc, bg_rect, bg_brush);
	if (i_ret == 0)
		return FALSE;

	DeleteObject(bg_brush);
	return TRUE;
}

}
