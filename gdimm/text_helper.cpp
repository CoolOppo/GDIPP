#include "stdafx.h"
#include "text_helper.h"
#include "gdimm.h"

#define FT_PAD_FLOOR( x, n )  ( (x) & ~((n)-1) )
#define FT_PAD_ROUND( x, n )  FT_PAD_FLOOR( (x) + ((n)/2), n )
#define FT_PAD_CEIL( x, n )   FT_PAD_FLOOR( (x) + ((n)-1), n )

FT_F26Dot6 to_26dot6(const FIXED &fixed)
{
	return *((FT_F26Dot6*) &fixed) >> 10;
}

LONG from_16dot16(FT_Pos fixed)
{
	return fixed >> 16;
}

// convert floating point to 16.16 format
FT_Pos to_16dot16(double x)
{
	return (FT_Pos)(x * 65536);
}

BOOL draw_background(HDC hdc, const RECT *bg_rect, COLORREF bg_color)
{
	int i_ret;

	assert(bg_color != CLR_INVALID);

	const HBRUSH bg_brush = CreateSolidBrush(bg_color);
	if (bg_brush == NULL)
		return FALSE;

	i_ret = FillRect(hdc, bg_rect, bg_brush);
	if (i_ret == 0)
		return FALSE;

	DeleteObject(bg_brush);
	return TRUE;
}

int get_bmp_pitch(int width, WORD bpp)
{
	return FT_PAD_CEIL((int) ceil((double)(width * bpp) / 8), sizeof(LONG));
}

BITMAPINFO get_dc_bmp_info(HDC hdc)
{
	int i_ret;

	const HBITMAP dc_bitmap = (HBITMAP) GetCurrentObject(hdc, OBJ_BITMAP);
	assert(dc_bitmap != NULL);

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	i_ret = GetDIBits(hdc, dc_bitmap, 0, 0, NULL, &bmi, DIB_RGB_COLORS);
	assert(i_ret != 0);
	assert(bmi.bmiHeader.biBitCount != 0);

	return bmi;
}

bool get_dc_metrics(HDC hdc, vector<BYTE> &metric_buf, OUTLINETEXTMETRICW *&outline_metrics)
{
	// get outline metrics of the DC, which also include the text metrics
	UINT metric_size = GetOutlineTextMetricsW(hdc, 0, NULL);
	if (metric_size == 0)
		return false;

	metric_buf.resize(metric_size);
	outline_metrics = (OUTLINETEXTMETRICW*) &metric_buf[0];
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

RECT get_glyph_bmp_rect(const vector<const FT_BitmapGlyph> &glyphs, const vector<POINT> &glyph_pos, POINT cursor)
{
	RECT bmp_rect = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};

	const size_t last_pos = glyph_pos.size() - 1;

	// condition checks because glyph might be place right-to-left

	if (glyph_pos[last_pos].x >= glyph_pos[0].x)
	{
		// left to right
		bmp_rect.left = glyph_pos[0].x;
		bmp_rect.right = max(glyph_pos[last_pos].x + get_ft_bmp_width(glyphs[last_pos]->bitmap), cursor.x);
	}
	else
	{
		// right to left
		bmp_rect.left = min(glyph_pos[last_pos].x, cursor.x);
		bmp_rect.right = glyph_pos[0].x + get_ft_bmp_width(glyphs[0]->bitmap);
	}

	/*
	vertical text is not supported yet

	if (glyph_pos[last_pos].y >= glyph_pos[0].y)
	{
		bmp_rect.top = glyph_pos[0].y;
		bmp_rect.bottom = glyph_pos[last_pos].y + glyphs[last_pos]->bitmap.rows;
	}
	else
	{
		bmp_rect.top = glyph_pos[last_pos].y;
		bmp_rect.bottom = glyph_pos[0].y + glyphs[0]->bitmap.rows;
	}
	*/

	return bmp_rect;
}

LOGFONTW get_logfont(HDC hdc)
{
	HFONT h_font = (HFONT) GetCurrentObject(hdc, OBJ_FONT);
	assert(h_font != NULL);

	LOGFONTW font_attr;
	GetObject(h_font, sizeof(LOGFONTW), &font_attr);

	return font_attr;
}

bool get_render_mode(const wchar_t *font_name, BYTE font_quality, WORD dc_bmp_bpp, FT_Render_Mode &render_mode)
{
	const font_setting_cache *setting_cache = setting_cache_instance.lookup(font_name);

	// non-antialiased font
	// draw with monochrome mode
	if (font_quality == NONANTIALIASED_QUALITY && !setting_cache->render_non_aa)
	{
		render_mode = FT_RENDER_MODE_MONO;
		return true;
	}

	switch (dc_bmp_bpp)
	{
	case 1:
		render_mode = FT_RENDER_MODE_MONO;
		break;
	case 8:
		render_mode = FT_RENDER_MODE_NORMAL;
		break;
	case 24:
	case 32:
		render_mode = (setting_cache->subpixel_render ? FT_RENDER_MODE_LCD : FT_RENDER_MODE_NORMAL);
		break;
	default:
		// we do not support 16 bpp currently
		return false;
	}

	return true;
}