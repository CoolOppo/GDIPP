#include "stdafx.h"
#include "text.h"
#include "global.h"
#include "ft.h"
#include "font_man.h"
#include "font_link.h"
#include <cmath>

struct COLORRGB
{
	BYTE r;
	BYTE g;
	BYTE b;
	BYTE pad;
};
#define REFTORGB(ref) (*((COLORRGB*) &ref))

#define AlignDown(num, alignment) (num - num & (alignment -	1))

inline int AlignUp(int num, int alignment)
{
	if (num == 0)
		return alignment;
	else if (num % alignment == 0)
		return num;
	else
		return (num / alignment + 1) * alignment;
}

gdimm_text::gdimm_text(HDC hdc, int x, int y)
{
	_hdc_text = hdc;
	_cursor.x = x;
	_cursor.y = y;

	// get foreground and background color
	_fg_color = GetTextColor(_hdc_text);
	assert(_fg_color != CLR_INVALID);
	_bg_color = GetBkColor(_hdc_text);
	assert(_bg_color != CLR_INVALID);

	_metric_buf = NULL;
	_original_hfont = NULL;

	load_font_info();
}

gdimm_text::~gdimm_text()
{
	delete[] _metric_buf;
}

/*void gdimm_text::draw_bitmap_mono(FT_Bitmap bitmap, FT_Vector pos) const
{
	const int absPitch = abs(bitmap.pitch);

	if (bitmap.pitch > 0)
		pos.y += _font_attr.lfHeight - bitmap.rows;

	for (int j = 0; j < bitmap.rows; j++)
	{
		for (int i = 0; i < bitmap.width; i++)
		{
			// the left-most pixel is the highest bit
			BOOL mono = bitmap.buffer[j * absPitch + i / 8] & (0x80 >> (i % 8));

			if (mono == 0)
				continue;

			SetPixel(_hdc_text, pos.x + i, pos.y + j, _fg_color);
		}
	}
}

void gdimm_text::draw_bitmap_256(FT_Bitmap bitmap, FT_Vector pos) const
{
	const int absPitch = abs(bitmap.pitch);
	const COLORRGB fgRGB = REFTORGB(_fg_color);
	const COLORRGB bgRGB = REFTORGB(_bg_color);

	// pitch > 0 means up flow, while Windows coordination is down flow
	if (bitmap.pitch > 0)
		pos.y += _font_attr.lfHeight - bitmap.rows;

	for (int j = 0; j < bitmap.rows; j++)
	{
		for (int i = 0; i < bitmap.width; i++)
		{
			// gray level
			BYTE gray = bitmap.buffer[j * absPitch + i];

			if (gray == 0)
				continue;

			// pixel color
			COLORREF c = RGB((gray * fgRGB.r + (255 - gray) * bgRGB.r) / 255, 
				(gray * fgRGB.g + (255 - gray) * bgRGB.g) / 255, 
				(gray * fgRGB.b + (255 - gray) * bgRGB.b) / 255);

			SetPixel(_hdc_text, pos.x + i, pos.y + j, c);
		}
	}
}*/

void gdimm_text::draw_bitmap_lcd(FT_Bitmap bitmap, FT_Vector pos) const
{
	const WORD src_byte_count = 3;	// 24-bit
	const WORD dest_bit_count = 24;
	const WORD dest_byte_count = dest_bit_count / 8;
	const LONG px_width = bitmap.width / src_byte_count;
	const LONG px_height = bitmap.rows;
	const int absPitch = abs(bitmap.pitch);

	const COLORRGB fgRGB = REFTORGB(_fg_color);
	const COLORRGB bgRGB = REFTORGB(_bg_color);

	if (bitmap.pitch > 0)
		pos.y += _font_height - px_height;

	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biBitCount = dest_bit_count;
	bmi.bmiHeader.biWidth = px_width;
	bmi.bmiHeader.biHeight = -px_height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;

	HDC hdc_paint = CreateCompatibleDC(_hdc_text);
	assert(hdc_paint != NULL);
	
	BYTE *dest_bits;
	HBITMAP dest_bitmap = CreateDIBSection(_hdc_text, &bmi, DIB_RGB_COLORS, (VOID**) &dest_bits, NULL, 0);
	assert(dest_bitmap != NULL);

	for (int j = 0; j < px_height; j++)
	{
		for (int i = 0; i < px_width; i++)
		{
			int src_ptr = j * absPitch + i * src_byte_count;
			int dest_ptr = src_ptr;

			BYTE r = bitmap.buffer[src_ptr];
			BYTE g = bitmap.buffer[src_ptr+1];
			BYTE b = bitmap.buffer[src_ptr+2];
			
			dest_bits[dest_ptr] = (b * fgRGB.b + (255 - b) * bgRGB.b) / 255;
			dest_bits[dest_ptr+1] = (g * fgRGB.g + (255 - g) * bgRGB.g) / 255;
			dest_bits[dest_ptr+2] = (r * fgRGB.r + (255 - r) * bgRGB.r) / 255;
		}
	}

	SelectObject(hdc_paint, dest_bitmap);
	BOOL b_ret = TransparentBlt(_hdc_text, pos.x, pos.y, px_width, px_height, hdc_paint, 0, 0, px_width, px_height, _bg_color);
	assert(b_ret == TRUE);
	
	DeleteObject(dest_bitmap);
	DeleteDC(hdc_paint);
}

/*void gdimm_text::draw_bitmap_dwm(FT_Bitmap bitmap, FT_Vector pos) const
{
	const WORD src_byte_count = 3;	// 24-bit
	const WORD dest_bit_count = 32;
	const WORD dest_byte_count = dest_bit_count / 8;
	const LONG px_width = bitmap.width / src_byte_count;
	const LONG padded_width = AlignUp(px_width * dest_byte_count, sizeof(DWORD));
	const LONG px_height = bitmap.rows;
	const int absPitch = abs(bitmap.pitch);

	const COLORRGB fgRGB = REFTORGB(_fg_color);
	const COLORRGB bgRGB = REFTORGB(_bg_color);

	if (bitmap.pitch > 0)
		pos.y += _font_attr.otmAscent - px_height;

	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biBitCount = dest_bit_count;
	bmi.bmiHeader.biWidth = px_width;
	bmi.bmiHeader.biHeight = -px_height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;

	HDC hdc_paint = CreateCompatibleDC(_hdc_text);
	assert(hdc_paint != NULL);
	
	BYTE *dest_bits;
	HBITMAP dest_bitmap = CreateDIBSection(_hdc_text, &bmi, DIB_RGB_COLORS, (VOID**) &dest_bits, NULL, 0);
	assert(dest_bitmap != NULL);

	for (int j = 0; j < px_height; j++)
	{
		for (int i = 0; i < px_width; i++)
		{
			int src_ptr = j * absPitch + i * src_byte_count;
			int dest_ptr = j * padded_width + i * dest_byte_count;

			BYTE r = bitmap.buffer[src_ptr];
			BYTE g = bitmap.buffer[src_ptr+1];
			BYTE b = bitmap.buffer[src_ptr+2];
			
			dest_bits[dest_ptr] = (b * fgRGB.b + (255 - b) * bgRGB.b) / 255;
			dest_bits[dest_ptr+1] = (g * fgRGB.g + (255 - g) * bgRGB.g) / 255;
			dest_bits[dest_ptr+2] = (r * fgRGB.r + (255 - r) * bgRGB.r) / 255;
			dest_bits[dest_ptr+3] = 0xff;
		}
	}

	SelectObject(hdc_paint, dest_bitmap);
	BOOL b_ret = TransparentBlt(_hdc_text, pos.x, pos.y, px_width, px_height, hdc_paint, 0, 0, px_width, px_height, _bg_color);
	assert(b_ret);
	
	DeleteObject(dest_bitmap);
	DeleteDC(hdc_paint);
}*/

void gdimm_text::draw_bitmap(FT_Bitmap bitmap, FT_Vector pos) const
{
	switch (bitmap.pixel_mode)
	{
	case FT_PIXEL_MODE_MONO:
		//draw_bitmap_mono(bitmap, pos);
		break;
	case FT_PIXEL_MODE_GRAY:
		//draw_bitmap_256(bitmap, pos);
		break;
	case FT_PIXEL_MODE_LCD:
		draw_bitmap_lcd(bitmap, pos);
		//draw_bitmap_dwm(bitmap, pos);
		break;
	}
}

TCHAR *gdimm_text::get_family_name() const
{
	return (TCHAR*) (_metric_buf + (UINT) _font_metric->otmpFamilyName);
}

TCHAR *gdimm_text::get_full_name() const
{
	return (TCHAR*) (_metric_buf + (UINT) _font_metric->otmpFullName);
}

void gdimm_text::load_font_info()
{
	int metric_size = GetOutlineTextMetrics(_hdc_text, 0, NULL);
	assert(metric_size != 0);

	if (_metric_buf != NULL)
		delete[] _metric_buf;

	_metric_buf = new BYTE[metric_size];
	_font_metric = (OUTLINETEXTMETRIC*) _metric_buf;
	metric_size = GetOutlineTextMetrics(_hdc_text, metric_size, _font_metric);
	assert(metric_size != 0);

	HFONT hfont = (HFONT) GetCurrentObject(_hdc_text, OBJ_FONT);
	GetObject(hfont, sizeof(LOGFONT), &_font_attr);
	_font_height = _font_attr.lfHeight;
}

bool gdimm_text::load_next_font(int font_link_index)
{
	// first-time font linking
	if (font_link_index == 0)
	{
		_original_hfont = (HFONT) GetCurrentObject(_hdc_text, OBJ_FONT);
		assert(_original_hfont != NULL);
	}

	const TCHAR *linked_font_name = gdimm_font_link::instance().lookup(get_family_name(), font_link_index);
	if (linked_font_name == NULL)
		return false;

	HFONT linked_hfont = CreateFont(
		_font_metric->otmTextMetrics.tmHeight,
		0,
		0,
		0,
		_font_metric->otmTextMetrics.tmWeight,
		_font_metric->otmTextMetrics.tmItalic,
		_font_metric->otmTextMetrics.tmUnderlined,
		_font_metric->otmTextMetrics.tmStruckOut,
		_font_metric->otmTextMetrics.tmCharSet,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		_font_metric->otmTextMetrics.tmPitchAndFamily,
		linked_font_name);
	assert(linked_hfont != NULL);

	SelectObject(_hdc_text, linked_hfont);
	load_font_info();

	return true;
}

bool gdimm_text::restore_dc_font()
{
	if (_original_hfont == NULL)
		return false;

	// restore original font, which was replaced by font link
	SelectObject(_hdc_text, _original_hfont);
	load_font_info();
	_original_hfont = NULL;

	return true;
}

BOOL gdimm_text::is_font_true_type() const
{
	return (_font_metric->otmTextMetrics.tmPitchAndFamily & TMPF_TRUETYPE);
}

unsigned int gdimm_text::to_glyph_indices(const WCHAR *text, unsigned int start, unsigned int count, WCHAR *glyph_indices)
{
	DWORD dw_ret;

	if (start == 0)
	{
		dw_ret = GetGlyphIndices(_hdc_text, text, count, (WORD*) glyph_indices, GGI_MARK_NONEXISTING_GLYPHS);
		assert(dw_ret != GDI_ERROR);
	}

	wchar_t bad_glyph_index[2] = {0xffff, 0};
	size_t first_good = wcsspn(glyph_indices + start, bad_glyph_index);
	size_t first_bad = wcscspn(glyph_indices + start, bad_glyph_index);
	unsigned int processed = min(abs((int)(first_good - first_bad)), (int)(count - start));

	int fl_index = 0;
	while (glyph_indices[start] == 0xffff)
	{
		if (!load_next_font(fl_index++))
			return 0;

		dw_ret = GetGlyphIndices(_hdc_text, text + start, processed, ((WORD*) glyph_indices) + start, GGI_MARK_NONEXISTING_GLYPHS);
		assert(dw_ret != GDI_ERROR);
	}

	return processed;
}

void gdimm_text::draw_background(CONST RECT *lprect) const
{
	// get background rect geometry
	const LONG rect_width = lprect->right - lprect->left;
	const LONG rect_height = lprect->bottom - lprect->top;

	// create brush with background color
	COLORREF _bg_color = GetBkColor(_hdc_text);
	assert(_bg_color != CLR_INVALID);
	HBRUSH bg_brush = CreateSolidBrush(_bg_color);
	assert(bg_brush != NULL);

	// select new brush, and store old brush
	HBRUSH old_brush = (HBRUSH) SelectObject(_hdc_text, bg_brush);

	// paint rect with brush
	BOOL ret = PatBlt(_hdc_text, lprect->left, lprect->top, rect_width, rect_height, PATCOPY);
	assert(ret == TRUE);
	DeleteObject(bg_brush);

	// restore old brush
	SelectObject(_hdc_text, old_brush);
}

void gdimm_text::text_out(const WCHAR *glyph_indices, unsigned int count, CONST RECT *lprect, CONST INT *lpDx)
{
	TCHAR *font_full_name = get_full_name();
	unsigned int font_index = gdimm_font_man::instance().lookup_index(_hdc_text, font_full_name);

	FT_Error ft_error;
	FTC_FaceID face_id = (FTC_FaceID) font_index;

	FT_Face font_face;
	ft_error = FTC_Manager_LookupFace(ft_cache_man, face_id, &font_face);
	assert(ft_error == 0);

	if (_font_height > 0)
		_font_height = MulDiv(_font_height, font_face->units_per_EM, font_face->height);
	else
		_font_height = -_font_height;

	FTC_ScalerRec cache_scale = {face_id, _font_height, _font_height, 1, 0, 0};
	FT_Size font_size;
	ft_error = FTC_Manager_LookupSize(ft_cache_man, &cache_scale, &font_size);
	assert(ft_error == 0);

	FT_UInt prev_glyph_index = 0;
	const FT_Long has_kern = FT_HAS_KERNING(font_face);
	//const FT_Long has_kern = FALSE;

	// collect glyph bitmaps of all characters
	for (unsigned int i = 0; i < count; i++)
	{
		FT_Glyph glyph;
		ft_error = FTC_ImageCache_LookupScaler(ft_glyph_cache, &cache_scale, FT_LOAD_DEFAULT, glyph_indices[i], &glyph, NULL);
		assert(ft_error == 0);

		if (glyph->format < 0)
			continue;

		ft_error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_LCD, NULL, 0);
		assert(ft_error == 0);
		FT_BitmapGlyph bmp_glyph = (FT_BitmapGlyph) glyph;

		if (lprect == NULL ||
			(_cursor.x >= lprect->left && _cursor.x <= lprect->right && _cursor.y >= lprect->top && _cursor.y <= lprect->bottom)
			)
		{
			// if font has kern info, use it
			if (has_kern)
			{
				FT_Vector kern;
				FT_Get_Kerning(font_face, prev_glyph_index, glyph_indices[i], FT_KERNING_DEFAULT, &kern);
				prev_glyph_index = glyph_indices[i];
				_cursor.x += kern.x >> 6;
			}

			FT_Bitmap curr_bmp = bmp_glyph->bitmap;
			FT_Vector glyph_cursor;
			glyph_cursor.x = _cursor.x + bmp_glyph->left;
			glyph_cursor.y = _cursor.y + curr_bmp.rows - bmp_glyph->top;

			if (curr_bmp.width * curr_bmp.rows > 0)
				draw_bitmap(curr_bmp, glyph_cursor);

			/*GLYPHMETRICS gm;
			MAT2 mat2 = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
			DWORD buf_len = GetGlyphOutline(_hdc_text, glyph_indices[i], GGO_NATIVE | GGO_GLYPH_INDEX, &gm, 0, NULL, &mat2);
			assert(buf_len != GDI_ERROR);
			BYTE *buf = new BYTE[buf_len];
			buf_len = GetGlyphOutline(_hdc_text, glyph_indices[i], GGO_NATIVE | GGO_GLYPH_INDEX, &gm, buf_len, buf, &mat2);
			assert(buf_len != GDI_ERROR);
			TTPOLYGONHEADER h = *(TTPOLYGONHEADER*)buf;
			TTPOLYCURVE *c = (TTPOLYCURVE*)(buf + sizeof(h));
			delete[] buf;*/
		}

		// advance _cursor
		if (lpDx == NULL)
			_cursor.x += (glyph->advance.x + 0x8000) >> 16;
		else
			_cursor.x += lpDx[i];

		FT_Done_Glyph(glyph);
	}

	restore_dc_font();
}