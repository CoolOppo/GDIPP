#include "stdafx.h"
#include "text.h"
#include "global.h"
#include "ft.h"
#include "font_man.h"
#include "font_chg.h"
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

gdimm_text::gdimm_text(HDC hdc, int x, int y, CONST RECT *lprect, CONST INT *lpDx)
{
	hdc_text = hdc;
	cursor.x = x;
	cursor.y = y;
	clip_rect = lprect;
	distances = lpDx;

	original_hfont = NULL;
	original_fontname[0] = '\0';
}

gdimm_text::~gdimm_text()
{
}

void gdimm_text::get_text_colors()
{
	// get foreground and background color
	fg_color = GetTextColor(hdc_text);
	assert(fg_color != CLR_INVALID);
	bg_color = GetBkColor(hdc_text);
	assert(bg_color != CLR_INVALID);
}

void gdimm_text::draw_bitmap_mono(FT_Bitmap bitmap, FT_Vector pos) const
{
	const int absPitch = abs(bitmap.pitch);

	if (bitmap.pitch > 0)
		pos.y += font_attr.lfHeight - bitmap.rows;

	for (int j = 0; j < bitmap.rows; j++)
	{
		for (int i = 0; i < bitmap.width; i++)
		{
			// the left-most pixel is the highest bit
			BOOL mono = bitmap.buffer[j * absPitch + i / 8] & (0x80 >> (i % 8));

			if (mono == 0)
				continue;

			SetPixel(hdc_text, pos.x + i, pos.y + j, fg_color);
		}
	}
}

void gdimm_text::draw_bitmap_256(FT_Bitmap bitmap, FT_Vector pos) const
{
	const int absPitch = abs(bitmap.pitch);
	const COLORRGB fgRGB = REFTORGB(fg_color);
	const COLORRGB bgRGB = REFTORGB(bg_color);

	// pitch > 0 means up flow, while Windows coordination is down flow
	if (bitmap.pitch > 0)
		pos.y += font_attr.lfHeight - bitmap.rows;

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

			SetPixel(hdc_text, pos.x + i, pos.y + j, c);
		}
	}
}

void gdimm_text::draw_bitmap_lcd(FT_Bitmap bitmap, FT_Vector pos) const
{
	const WORD src_byte_count = 3;	// 24-bit
	const WORD dest_bit_count = 24;
	const WORD dest_byte_count = dest_bit_count / 8;
	const LONG px_width = bitmap.width / src_byte_count;
	const LONG px_height = bitmap.rows;
	const int absPitch = abs(bitmap.pitch);

	const COLORRGB fgRGB = REFTORGB(fg_color);
	const COLORRGB bgRGB = REFTORGB(bg_color);

	if (bitmap.pitch > 0)
		pos.y += font_attr.lfHeight - px_height;

	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biBitCount = dest_bit_count;
	bmi.bmiHeader.biWidth = px_width;
	bmi.bmiHeader.biHeight = -px_height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;

	HDC hdc_paint = CreateCompatibleDC(hdc_text);
	assert(hdc_paint != NULL);
	
	BYTE *dest_bits;
	HBITMAP dest_bitmap = CreateDIBSection(hdc_text, &bmi, DIB_RGB_COLORS, (VOID**) &dest_bits, NULL, 0);
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
	BOOL b_ret = TransparentBlt(hdc_text, pos.x, pos.y, px_width, px_height, hdc_paint, 0, 0, px_width, px_height, bg_color);
	assert(b_ret == TRUE);
	
	DeleteObject(dest_bitmap);
	DeleteDC(hdc_paint);
}

void gdimm_text::draw_bitmap_dwm(FT_Bitmap bitmap, FT_Vector pos) const
{
	const WORD src_byte_count = 3;	// 24-bit
	const WORD dest_bit_count = 32;
	const WORD dest_byte_count = dest_bit_count / 8;
	const LONG px_width = bitmap.width / src_byte_count;
	const LONG padded_width = AlignUp(px_width * dest_byte_count, sizeof(DWORD));
	const LONG px_height = bitmap.rows;
	const int absPitch = abs(bitmap.pitch);

	const COLORRGB fgRGB = REFTORGB(fg_color);
	const COLORRGB bgRGB = REFTORGB(bg_color);

	if (bitmap.pitch > 0)
		pos.y += font_attr.lfHeight - px_height;

	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biBitCount = dest_bit_count;
	bmi.bmiHeader.biWidth = px_width;
	bmi.bmiHeader.biHeight = -px_height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;

	HDC hdc_paint = CreateCompatibleDC(hdc_text);
	assert(hdc_paint != NULL);
	
	BYTE *dest_bits;
	HBITMAP dest_bitmap = CreateDIBSection(hdc_text, &bmi, DIB_RGB_COLORS, (VOID**) &dest_bits, NULL, 0);
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
	BOOL b_ret = TransparentBlt(hdc_text, pos.x, pos.y, px_width, px_height, hdc_paint, 0, 0, px_width, px_height, bg_color);
	assert(b_ret == TRUE);
	
	DeleteObject(dest_bitmap);
	DeleteDC(hdc_paint);
}

void gdimm_text::draw_bitmap(FT_Bitmap bitmap, FT_Vector pos) const
{
	switch(bitmap.pixel_mode)
	{
	case FT_PIXEL_MODE_MONO:
		draw_bitmap_mono(bitmap, pos);
		break;
	case FT_PIXEL_MODE_GRAY:
		draw_bitmap_256(bitmap, pos);
		break;
	case FT_PIXEL_MODE_LCD:
		draw_bitmap_lcd(bitmap, pos);
		//draw_bitmap_dwm(bitmap, pos);
		break;
	}
}

void gdimm_text::draw_background() const
{
	// get background rect geometry
	const LONG rect_width = clip_rect->right - clip_rect->left;
	const LONG rect_height = clip_rect->bottom - clip_rect->top;

	// create brush with background color
	COLORREF bg_color = GetBkColor(hdc_text);
	assert(bg_color != CLR_INVALID);
	HBRUSH bg_brush = CreateSolidBrush(bg_color);
	assert(bg_brush != NULL);

	// select new brush, and store old brush
	HBRUSH old_brush = (HBRUSH) SelectObject(hdc_text, bg_brush);

	// paint rect with brush
	BOOL ret = PatBlt(hdc_text, clip_rect->left, clip_rect->top, rect_width, rect_height, PATCOPY);
	assert(ret == TRUE);
	DeleteObject(bg_brush);

	// restore old brush
	SelectObject(hdc_text, old_brush);
}

void gdimm_text::prepare()
{
	font_attr = gdimm_font_man::instance().get_font_attr(hdc_text);
	
	// change font face name to the substituted one
	const TCHAR *sub_font_name = gdimm_font_chg::instance().fs_lookup(font_attr.lfFaceName);
	if (sub_font_name != NULL)
		lstrcpyn(font_attr.lfFaceName, sub_font_name, LF_FACESIZE);
}

void gdimm_text::text_out(const WCHAR *glyph_indices, unsigned int count)
{
	t_string font_name = gdimm_font_man::instance().get_font_full_name(hdc_text);
	unsigned int font_index = gdimm_font_man::instance().lookup_index(font_name, hdc_text);

	FT_Error ft_error;
	FTC_FaceID face_id = (FTC_FaceID) font_index;

	FT_Face font_face;
	ft_error = FTC_Manager_LookupFace(ft_cache_man, face_id, &font_face);
	assert(ft_error == 0);

	FTC_ScalerRec cache_scale = {face_id, font_attr.lfWidth, font_attr.lfHeight, 1, 0, 0};
	FT_Size font_size;
	ft_error = FTC_Manager_LookupSize(ft_cache_man, &cache_scale, &font_size);
	assert(ft_error == 0);

	FT_UInt prev_glyph_index = 0;
	const FT_Long has_kern = FT_HAS_KERNING(font_face);
	//const FT_Long has_kern = FALSE;

	get_text_colors();

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

		if (clip_rect == NULL ||
			(cursor.x >= clip_rect->left && cursor.x <= clip_rect->right && cursor.y >= clip_rect->top && cursor.y <= clip_rect->bottom)
			)
		{
			// if font has kern info, use it
			if (has_kern)
			{
				FT_Vector kern;
				FT_Get_Kerning(font_face, prev_glyph_index, glyph_indices[i], FT_KERNING_DEFAULT, &kern);
				prev_glyph_index = glyph_indices[i];
				cursor.x += kern.x >> 6;
			}

			FT_Bitmap curr_bmp = bmp_glyph->bitmap;
			FT_Vector glyph_cursor;
			glyph_cursor.x = cursor.x + bmp_glyph->left;
			glyph_cursor.y = cursor.y + curr_bmp.rows - bmp_glyph->top;

			if (curr_bmp.width * curr_bmp.rows > 0)
				draw_bitmap(curr_bmp, glyph_cursor);
		}

		// advance cursor
		if (distances == NULL)
			cursor.x += (glyph->advance.x + 0x8000) >> 16;
		else
			cursor.x += distances[i];

		FT_Done_Glyph(glyph);
	}

	if (original_hfont != NULL)
	{
		// restore original font, which was replaced by fontlink
		SelectObject(hdc_text, original_hfont);
	}
}

bool gdimm_text::font_link(int font_link_index)
{
	// first-time font linking
	if (font_link_index == 0)
	{
		original_hfont = (HFONT) GetCurrentObject(hdc_text, OBJ_FONT);
		assert(original_hfont != NULL);

		lstrcpyn(original_fontname, font_attr.lfFaceName, LF_FACESIZE);
	}

	const TCHAR *linked_font_name = gdimm_font_chg::instance().fl_lookup(original_fontname, font_link_index);
	if (linked_font_name == NULL)
		return false;

	lstrcpyn(font_attr.lfFaceName, linked_font_name, LF_FACESIZE);
	HFONT linked_hfont = CreateFontIndirect(&font_attr);
	assert(linked_hfont != NULL);

	SelectObject(hdc_text, linked_hfont);

	return true;
}

bool gdimm_text::to_glyph_indices(const WCHAR *text, unsigned int count, WCHAR *glyph_indices)
{
	int fl_index = 0;

	while (true)
	{
		DWORD dw_ret = GetGlyphIndices(hdc_text, text, count, (WORD*) glyph_indices, GGI_MARK_NONEXISTING_GLYPHS);
		assert(dw_ret != GDI_ERROR);

		bool need_fl = false;
		for (unsigned int i = 0; i < count; i++)
		{
			if (glyph_indices[i] == 0xffff)
			{
				need_fl = true;
				break;
			}
		}
		
		if (!need_fl)
			break;
		else if (!font_link(fl_index++))
			return false;
	}

	return true;
}