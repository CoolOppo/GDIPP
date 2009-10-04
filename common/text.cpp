#include "stdafx.h"
#include <cmath>
#include "text.h"
#include "ft.h"

void DebugOutput(TCHAR *str)
{
	FILE *f = fopen("C:\\ft_debug.txt", "a+");
	fwprintf(f, TEXT("%s\n"), str);
	fclose(f);
}

void DebugOutput(DWORD num)
{
	FILE *f = fopen("C:\\ft_debug.txt", "a+");
	fwprintf(f, TEXT("%u\n"), num);
	fclose(f);
}

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

gdimm_Text::gdimm_Text()
{
	InitializeFreeType();
}

gdimm_Text::~gdimm_Text()
{
	DestroyFreeType();
}

void gdimm_Text::GetTextInfo(HDC hdc)
{
	// get logical font structure
	HGDIOBJ gdiObj = GetCurrentObject(hdc, OBJ_FONT);
	assert(gdiObj != NULL);
	DWORD dwRet = GetObject(gdiObj, sizeof(LOGFONT), &curr_font.font_attr);
	assert(dwRet != 0);

	curr_font.font_attr.lfHeight = abs(curr_font.font_attr.lfHeight);
	if (curr_font.font_attr.lfWidth == 0)
		curr_font.font_attr.lfWidth = curr_font.font_attr.lfHeight;

	// get foreground and background color
	fg_color = GetTextColor(hdc);
	assert(fg_color != CLR_INVALID);
	bg_color = GetBkColor(hdc);
	assert(bg_color != CLR_INVALID);

	ft_font_trait = curr_font.GetTrait();
}

void gdimm_Text::DrawBitmapMono(HDC hdc, FT_Bitmap bitmap, FT_Vector pos)
{
	const int absPitch = abs(bitmap.pitch);

	if (bitmap.pitch > 0)
		pos.y += curr_font.font_attr.lfHeight - bitmap.rows;

	for (int j = 0; j < bitmap.rows; j++)
	{
		for (int i = 0; i < bitmap.width; i++)
		{
			// the left-most pixel is the highest bit
			BOOL mono = bitmap.buffer[j * absPitch + i / 8] & (0x80 >> (i % 8));

			if (mono == 0)
				continue;

			SetPixel(hdc, pos.x + i, pos.y + j, fg_color);
		}
	}
}

void gdimm_Text::DrawBitmap256(HDC hdc, FT_Bitmap bitmap, FT_Vector pos)
{
	const int absPitch = abs(bitmap.pitch);
	const COLORRGB fgRGB = REFTORGB(fg_color);
	const COLORRGB bgRGB = REFTORGB(bg_color);

	// pitch > 0 means up flow, while Windows coordination is down flow
	if (bitmap.pitch > 0)
		pos.y += curr_font.font_attr.lfHeight - bitmap.rows;

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

			SetPixel(hdc, pos.x + i, pos.y + j, c);
		}
	}
}

void gdimm_Text::DrawBitmapLCD(HDC hdc, FT_Bitmap bitmap, FT_Vector pos)
{
	const int absPitch = abs(bitmap.pitch);
	const COLORRGB fgRGB = REFTORGB(fg_color);
	const COLORRGB bgRGB = REFTORGB(bg_color);

	if (bitmap.pitch > 0)
		pos.y += curr_font.font_attr.lfHeight - bitmap.rows;

	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biWidth = bitmap.width / (bmi.bmiHeader.biBitCount / 8);
	bmi.bmiHeader.biHeight = -bitmap.rows;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;

	// assume bitmap.pitch is DWORD aligned
	int bits_size = absPitch * bitmap.rows;
	unsigned char *bmp_bits = new unsigned char[bits_size];
	memset(bmp_bits, 0, bits_size);

	// one bit for one pixel, new byte at each beginning of scanlines
	int mask_bits_width = AlignUp(absPitch / 3 / 8, sizeof(WORD));
	int mask_size = mask_bits_width * bitmap.rows;
	unsigned char *mask_bits = new unsigned char[mask_size];
	memset(mask_bits, 0, mask_size);

	for (int j = 0; j < bitmap.rows; j++)
	{
		for (int i = 0; i < bitmap.width / 3; i++)
		{
			int bmp_ptr = j * absPitch + i*3;

			BYTE r = bitmap.buffer[bmp_ptr];
			BYTE g = bitmap.buffer[bmp_ptr+1];
			BYTE b = bitmap.buffer[bmp_ptr+2];

			bmp_bits[bmp_ptr] = (b * fgRGB.b + (255 - b) * bgRGB.b) / 255;
			bmp_bits[bmp_ptr+1] = (g * fgRGB.g + (255 - g) * bgRGB.g) / 255;
			bmp_bits[bmp_ptr+2] = (r * fgRGB.r + (255 - r) * bgRGB.r) / 255;

			if (r != 0 && g != 0 && b != 0)
			{
				int mask_ptr = j * mask_bits_width + i / 8;
				int mask_pos = 7 - i % 8;
				mask_bits[mask_ptr] |= (1 << mask_pos);
			}
		}
	}

	HDC hdc_mem = CreateCompatibleDC(hdc);
	HBITMAP hbmp = CreateDIBitmap(hdc, &bmi.bmiHeader, CBM_INIT, bmp_bits, &bmi, DIB_RGB_COLORS);
	SelectObject(hdc_mem, hbmp);

	HBITMAP hmask = CreateBitmap(bmi.bmiHeader.biWidth, bitmap.rows, 1, 1, mask_bits);
	BOOL b_ret = MaskBlt(hdc, pos.x, pos.y, bmi.bmiHeader.biWidth, bitmap.rows, hdc_mem, 0, 0, hmask, 0, 0, MAKEROP4(SRCCOPY, 0x00AA0029));
	assert(b_ret == TRUE);

	DeleteObject(hmask);
	DeleteObject(hbmp);
	DeleteDC(hdc_mem);
	delete[] mask_bits;
	delete[] bmp_bits;
}

void gdimm_Text::DrawBitmap(HDC hdc, FT_Bitmap bitmap, FT_Vector pos)
{
	switch(bitmap.pixel_mode)
	{
	case FT_PIXEL_MODE_MONO:
		DrawBitmapMono(hdc, bitmap, pos);
		break;
	case FT_PIXEL_MODE_GRAY:
		DrawBitmap256(hdc, bitmap, pos);
		break;
	case FT_PIXEL_MODE_LCD:
		DrawBitmapLCD(hdc, bitmap, pos);
		break;
	}
}

BOOL gdimm_Text::TextOut(HDC hdc, CONST INT * lpDx, const TCHAR *text, unsigned int count)
{
	if (lstrcmp(text, TEXT("Restore")) == 0)
		int a = 0;

	GetTextInfo(hdc);

	FT_Error ft_error;
	FontTrait font_trait = curr_font.GetTrait();
	uint32_t face_id = curr_font.GetFaceID(&font_trait);
	FT_Face font_face;

	ft_error = FTC_Manager_LookupFace(ft_cache_man, (FTC_FaceID)face_id, &font_face);
	assert(ft_error == 0);

	FT_Size font_size;
	FTC_ScalerRec cache_scale = {(FTC_FaceID)face_id, curr_font.font_attr.lfWidth, curr_font.font_attr.lfHeight, 1, 0, 0};
	ft_error = FTC_Manager_LookupSize(ft_cache_man, &cache_scale, &font_size);
	assert(ft_error == 0);

	FT_UInt glyph_index, prev = 0;
	const FT_Long has_kern = FT_HAS_KERNING(font_face);
	//const FT_Long has_kern = FALSE;

	for (unsigned int i = 0; i < count; i++)
	{
		TCHAR a = text[i];
		glyph_index = FTC_CMapCache_Lookup(ft_cmap_cache, (FTC_FaceID)face_id, -1, text[i]);
		if (glyph_index == 0)
			continue;//assert(glyph_index != 0);

		FT_Glyph glyph;
		FTC_Node node;
		ft_error = FTC_ImageCache_LookupScaler(ft_glyph_cache, &cache_scale, FT_LOAD_DEFAULT, glyph_index, &glyph, &node);
		assert(ft_error == 0);

		ft_error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_LCD, 0, 0);
		assert(ft_error == 0);
		FT_BitmapGlyph bmp_glyph = (FT_BitmapGlyph) glyph;

		if (clip_rect == NULL ||
			(cursor.x >= clip_rect->left && cursor.x <= clip_rect->right && cursor.y >= clip_rect->top && cursor.y <= clip_rect->bottom)
			)
		{
			// if font has kern info, use it
			if (has_kern)
			{
				FT_Vector delta;
				FT_Get_Kerning(font_face, prev, glyph_index, FT_KERNING_DEFAULT, &delta);
				prev = glyph_index;
				cursor.x += delta.x >> 6;
			}

			FT_Bitmap curr_bmp = bmp_glyph->bitmap;
			FT_Vector glyph_cursor;
			glyph_cursor.x = cursor.x + bmp_glyph->left;
			glyph_cursor.y = cursor.y + curr_bmp.rows - bmp_glyph->top;

			if (curr_bmp.width * curr_bmp.rows > 0)
				DrawBitmap(hdc, curr_bmp, glyph_cursor);
		}

		// advance cursor
		if (lpDx == NULL)
			cursor.x += glyph->advance.x >> 16;
		else
			cursor.x += lpDx[i];

		FT_Done_Glyph(glyph);
	}

	return TRUE;
}