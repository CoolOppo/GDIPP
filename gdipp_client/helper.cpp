#include "stdafx.h"
#include "helper.h"
#include "gdipp_client/gdipp_client.h"
#include "gdipp_support/helper.h"

namespace gdipp
{

bool dc_context::init(HDC hdc)
{
	outline_metrics = get_dc_metrics(hdc, _metric_buf);
	if (outline_metrics == NULL)
		return false;

	if (!get_dc_bmp_header(hdc, bmp_header))
		return false;

	this->hdc = hdc;

	log_font = get_log_font(hdc);
	log_font.lfWeight = get_gdi_weight_class(static_cast<unsigned short>(log_font.lfWeight));

	return true;
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

LOGFONTW get_log_font(HDC hdc)
{
	HFONT h_font = reinterpret_cast<HFONT>(GetCurrentObject(hdc, OBJ_FONT));
	assert(h_font != NULL);

	LOGFONTW font_attr;
	GetObject(h_font, sizeof(LOGFONTW), &font_attr);

	return font_attr;
}

bool get_render_mode(const font_config_cache *font_config, WORD dc_bmp_bpp, BYTE font_quality, FT_Render_Mode &render_mode)
{
	// return true if successfully find an appropriate render mode
	// otherwise return false

	if (font_config->render_mode.mono == 2)
	{
		render_mode = FT_RENDER_MODE_MONO;
		return true;
	}

	if (font_config->render_mode.gray == 2)
	{
		render_mode = FT_RENDER_MODE_NORMAL;
		return true;
	}

	if (font_config->render_mode.subpixel == 2)
	{
		render_mode = FT_RENDER_MODE_LCD;
		return true;
	}

	if (!font_config->render_mode.aliased_text && font_quality == NONANTIALIASED_QUALITY)
		return false;

	if (font_config->render_mode.mono == 1 && dc_bmp_bpp == 1)
	{
		render_mode = FT_RENDER_MODE_MONO;
		return true;
	}

	if (font_config->render_mode.gray == 1 && dc_bmp_bpp == 8)
	{
		render_mode = FT_RENDER_MODE_NORMAL;
		return true;
	}

	// we do not support 16 bpp currently

	if (font_config->render_mode.subpixel == 1 && dc_bmp_bpp >= 24)
	{
		render_mode = FT_RENDER_MODE_LCD;
		return true;
	}

	return false;
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

COLORREF parse_palette_color(HDC hdc, COLORREF color)
{
	// if input color is CLR_INVALID, return it unchanged.
	if (color == CLR_INVALID)
		return CLR_INVALID;

	COLORREF color_ret = color;

	// if the high-order byte is odd, use the selected palette whose index is specified in the low-order bytes
	// see PALETTEINDEX()
	if (!!(color_ret & 0x01000000))
	{
		const HPALETTE dc_palette = static_cast<const HPALETTE>(GetCurrentObject(hdc, OBJ_PAL));

		PALETTEENTRY pal_entry;
		const UINT entries = GetPaletteEntries(dc_palette, (color_ret & 0x00ffffff), 1, &pal_entry);

		// if the DC has no palette entry, this is an invalid color
		if (entries != 0)
			color_ret = RGB(pal_entry.peRed, pal_entry.peGreen, pal_entry.peBlue);
	}

	return color_ret;
}

}
