#include "stdafx.h"
#include "text_helper.h"
#include "gdimm.h"

FT_F26Dot6 to_26dot6(const FIXED &x)
{
	return *((FT_F26Dot6 *)&x) >> 10;
}

FT_F26Dot6 to_26dot6(FLOAT x)
{
	return (FT_F26Dot6)(x * 64);
}

LONG from_26dot6(FT_Pos x)
{
	return x >> 6;
}

LONG from_16dot16(FT_Pos x)
{
	return x >> 16;
}

// convert floating point to 16.16 format
FT_Pos to_16dot16(double x)
{
	return (FT_Pos)(x * 65536);
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

BOOL draw_background(HDC hdc, const RECT *bg_rect, COLORREF bg_color)
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

	return FT_PAD_CEIL((int) ceil((double)(width * bpp) / 8), sizeof(LONG));
}

bool get_dc_dc_bmp_header(HDC hdc, BITMAPINFOHEADER &dc_dc_bmp_header)
{
	dc_dc_bmp_header.biSize = sizeof(BITMAPINFOHEADER);

	const HBITMAP dc_bitmap = (HBITMAP) GetCurrentObject(hdc, OBJ_BITMAP);
	if (dc_bitmap == NULL)
	{
		// currently no selected bitmap
		// use DC capability

		dc_dc_bmp_header.biWidth = GetDeviceCaps(hdc, HORZRES);
		dc_dc_bmp_header.biHeight = GetDeviceCaps(hdc, VERTRES);
		dc_dc_bmp_header.biPlanes = GetDeviceCaps(hdc, PLANES);
		dc_dc_bmp_header.biBitCount = GetDeviceCaps(hdc, BITSPIXEL);

		return false;
	}

	dc_dc_bmp_header.biBitCount = 0;
	int i_ret = GetDIBits(hdc, dc_bitmap, 0, 0, NULL, (BITMAPINFO *)&dc_dc_bmp_header, DIB_RGB_COLORS);
	assert(i_ret != 0);

	return true;
}

bool get_dc_metrics(HDC hdc, vector<BYTE> &metric_buf, OUTLINETEXTMETRICW *&outline_metrics)
{
	// get outline metrics of the DC, which also include the text metrics
	UINT metric_size = GetOutlineTextMetricsW(hdc, 0, NULL);
	if (metric_size == 0)
		return false;

	metric_buf.resize(metric_size);
	outline_metrics = (OUTLINETEXTMETRICW *)&metric_buf[0];
	metric_size = GetOutlineTextMetricsW(hdc, metric_size, outline_metrics);
	assert(metric_size != 0);

	return true;
}

int get_ft_bmp_width(const FT_Bitmap &bitmap)
{
	if (bitmap.pixel_mode == FT_PIXEL_MODE_LCD)
		return bitmap.width / 3;
	else
		return bitmap.width;
}

RECT get_ft_glyph_run_rect(const vector<const FT_BitmapGlyph> &glyphs, const vector<POINT> &glyph_pos)
{
	RECT glyph_run_rect;

	const size_t last_index = glyph_pos.size() - 1;

	// condition checks because glyph might be place right-to-left

	if (glyph_pos[last_index].x >= glyph_pos[0].x)
	{
		// left to right
		glyph_run_rect.left = min(glyph_pos[0].x, 0);
		glyph_run_rect.right = glyph_pos[last_index].x + get_ft_bmp_width(glyphs[last_index]->bitmap);
	}
	else
	{
		// right to left
		glyph_run_rect.left = min(glyph_pos[last_index].x, 0);
		glyph_run_rect.right = glyph_pos[0].x + get_ft_bmp_width(glyphs[0]->bitmap);
	}

	if (glyph_pos[last_index].y >= glyph_pos[0].y)
	{
		// top to bottom
		glyph_run_rect.top = glyph_pos[0].y;
		glyph_run_rect.bottom = glyph_pos[last_index].y + glyphs[last_index]->bitmap.rows;
	}
	else
	{
		// bottom to top
		glyph_run_rect.top = glyph_pos[last_index].y;
		glyph_run_rect.bottom = glyph_pos[0].y + glyphs[0]->bitmap.rows;
	}

	return glyph_run_rect;
}

unsigned char get_gdi_weight_class(unsigned short weight)
{
	/*
	emulate GDI behavior:
	weight 1 to 550 are rendered as Regular
	551 to 611 are Semibold
	612 to infinity are Bold
	*/

	const LONG weight_class_max[] = {550, 611};
	const unsigned char max_weight_class = sizeof(weight_class_max) / sizeof(LONG);

	for (unsigned char i = 0; i < max_weight_class; i++)
	{
		if (weight <= weight_class_max[i])
			return i;
	}

	return max_weight_class;
}

LOGFONTW get_logfont(HDC hdc)
{
	HFONT h_font = (HFONT) GetCurrentObject(hdc, OBJ_FONT);
	assert(h_font != NULL);

	LOGFONTW font_attr;
	GetObject(h_font, sizeof(LOGFONTW), &font_attr);

	return font_attr;
}

bool get_render_mode(const font_setting_cache *font_setting, WORD dc_bmp_bpp, BYTE font_quality, FT_Render_Mode &render_mode)
{
	if ((font_setting->render_mode.mono == 1 && dc_bmp_bpp == 1) ||
		(font_setting->render_mode.mono == 2) ||
		// non-antialiased font
		(font_quality == NONANTIALIASED_QUALITY && font_setting->render_mode.subpixel < 2))
	{
		render_mode = FT_RENDER_MODE_MONO;
		return true;
	}

	if ((font_setting->render_mode.gray == 1 && dc_bmp_bpp == 8) ||
		(font_setting->render_mode.gray == 2))
	{
		render_mode = FT_RENDER_MODE_NORMAL;
		return true;
	}

	// we do not support 16 bpp currently
	if ((font_setting->render_mode.subpixel == 1 && dc_bmp_bpp >= 24) ||
		(font_setting->render_mode.subpixel == 2))
	{
		render_mode = FT_RENDER_MODE_LCD;
		return true;
	}

	return false;
}

const wchar_t *metric_family_name(const OUTLINETEXTMETRICW *outline_metric)
{
	return (const wchar_t *)((BYTE *)outline_metric + (UINT) outline_metric->otmpFamilyName);
}

const wchar_t *metric_face_name(const OUTLINETEXTMETRICW *outline_metric)
{
	return (const wchar_t *)((BYTE *)outline_metric + (UINT) outline_metric->otmpFaceName);
}

const wchar_t *metric_style_name(const OUTLINETEXTMETRICW *outline_metric)
{
	return (const wchar_t *)((BYTE *)outline_metric + (UINT) outline_metric->otmpStyleName);
}