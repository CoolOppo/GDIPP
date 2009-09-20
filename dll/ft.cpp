#include "stdafx.h"
#include "ft.h"

#include <cmath>
using namespace std;


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

bool operator<(const LOGFONT &font1, const LOGFONT &font2)
{
	char *font1_ptr = (char*) &font1;
	char *font2_ptr = (char*) &font2;
	return (strncmp(font1_ptr, font2_ptr, sizeof(LOGFONT)) < 0);
}

FT_Engine::FT_Engine()
{
	FT_Error ft_error;
	
	ft_error = FT_Init_FreeType(&lib);
	assert(ft_error == 0);

	// enable FreeType LCD filter
	ft_error = FT_Library_SetLcdFilter(lib, FT_LCD_FILTER_DEFAULT);
	assert(ft_error == 0);
}

FT_Engine::~FT_Engine()
{
	FT_Done_FreeType(lib);

	for (FontDataCache::const_iterator iter = font_cache.begin(); iter != font_cache.end(); iter++)
		delete[] iter->second.lpvBuffer;
}

void FT_Engine::GetTextInfo(HDC hdc)
{
	// get logical font structure
	HGDIOBJ gdiObj = GetCurrentObject(hdc, OBJ_FONT);
	assert(gdiObj != NULL);
	DWORD dwRet = GetObject(gdiObj, sizeof(LOGFONT), &curr_font);
	assert(dwRet != 0);

	px_width = abs(curr_font.lfWidth);
	px_height = abs(curr_font.lfHeight);

	// get foreground and background color
	fg_color = GetTextColor(hdc);
	assert(fg_color != CLR_INVALID);
	bg_color = GetBkColor(hdc);
	assert(bg_color != CLR_INVALID);
}

void FT_Engine::CacheFont(HDC hdc)
{
	// if the current font has not been cached, cache it
	if (font_cache.find(curr_font) == font_cache.end())
	{
		DWORD font_data_len = GetFontData(hdc, 0, 0, NULL, 0);
		BYTE *font_data = new BYTE[font_data_len];
		font_data_len = GetFontData(hdc, 0, 0, font_data, font_data_len);
		assert(font_data_len != GDI_ERROR);

		FontData fd = {font_data, font_data_len};
		font_cache[curr_font] = fd;
	}
}

void FT_Engine::DrawBitmapMono(HDC hdc, FT_Bitmap bitmap, FT_Vector pos)
{
	const int absPitch = abs(bitmap.pitch);

	if (bitmap.pitch > 0)
		pos.y += 11 - bitmap.rows;

	for (int j = 0; j < bitmap.rows; j++)
	{
		for (int i = 0; i < bitmap.width; i++)
		{
			// the left-most pixel is the highest bit
			BOOL mono = bitmap.buffer[j * absPitch + i / 8] & (0x80 >> (i % 8));

			if (mono == 0)
			{
				if (opaque)
					SetPixel(hdc, pos.x + i, pos.y + j, bg_color);
				else
					continue;
			}

			SetPixel(hdc, pos.x + i, pos.y + j, fg_color);
		}
	}
}

void FT_Engine::DrawBitmap256(HDC hdc, FT_Bitmap bitmap, FT_Vector pos)
{
	const int absPitch = abs(bitmap.pitch);
	const COLORRGB fgRGB = REFTORGB(fg_color);
	const COLORRGB bgRGB = REFTORGB(bg_color);

	// pitch > 0 means up flow, while Windows coordination is down flow
	if (bitmap.pitch > 0)
		pos.y += 11 - bitmap.rows;

	for (int j = 0; j < bitmap.rows; j++)
	{
		for (int i = 0; i < bitmap.width; i++)
		{
			// gray level
			BYTE gray = bitmap.buffer[j * absPitch + i];

			if (gray == 0)
			{
				if (opaque)
					SetPixel(hdc, pos.x + i, pos.y + j, bg_color);
				else
					continue;
			}

			// pixel color
			COLORREF c = RGB((gray * fgRGB.r + (255 - gray) * bgRGB.r) / 255, 
				(gray * fgRGB.g + (255 - gray) * bgRGB.g) / 255, 
				(gray * fgRGB.b + (255 - gray) * bgRGB.b) / 255);

			SetPixel(hdc, pos.x + i, pos.y + j, c);
		}
	}
}

void FT_Engine::DrawBitmapLCD(HDC hdc, FT_Bitmap bitmap, FT_Vector pos)
{
	const int absPitch = abs(bitmap.pitch);
	const COLORRGB fgRGB = REFTORGB(fg_color);
	const COLORRGB bgRGB = REFTORGB(bg_color);

	if (bitmap.pitch > 0)
		pos.y += 11 - bitmap.rows;

	for (int j = 0; j < bitmap.rows; j++)
	{
		for (int i = 0; i < bitmap.width / 3; i++)
		{
			BYTE r = bitmap.buffer[j * absPitch + i*3];
			BYTE g = bitmap.buffer[j * absPitch + i*3 + 1];
			BYTE b = bitmap.buffer[j * absPitch + i*3 + 2];

			if (RGB(r, g, b) == 0)
			{
				if (opaque)
					SetPixel(hdc, pos.x + i, pos.y + j, bg_color);
				else
					continue;
			}

			COLORREF c = RGB((r * fgRGB.r + (255 - r) * bgRGB.r) / 255, 
				(g * fgRGB.g + (255 - g) * bgRGB.g) / 255, 
				(b * fgRGB.b + (255 - b) * bgRGB.b) / 255);
			
			SetPixel(hdc, pos.x + i, pos.y + j, c);
		}
	}
}

void FT_Engine::DrawBitmap(HDC hdc, FT_Bitmap bitmap, FT_Vector pos)
{
	switch(bitmap.pixel_mode){
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

BOOL FT_Engine::TextOut(HDC hdc, const TCHAR *text, unsigned int count)
{
	FT_Error ft_error;
	FT_Face font_face;

	GetTextInfo(hdc);
	CacheFont(hdc);

	FontData fd = font_cache[curr_font];
	ft_error = FT_New_Memory_Face(lib, fd.lpvBuffer, fd.cbData, 0, &font_face);
	assert(ft_error == 0);

	ft_error = FT_Set_Pixel_Sizes(font_face, px_width, px_height);
	assert(ft_error == 0);

	FT_UInt glyphIndex, prev = 0;
	FT_Vector delta;
	//const FT_Long has_kern = FALSE;
	const FT_Long has_kern = FT_HAS_KERNING(font_face);

	for (unsigned int i = 0; i < count; i++)
	{
		glyphIndex = FT_Get_Char_Index(font_face, text[i]);

		// if font has kern info, use it
		if (has_kern)
		{
			FT_Get_Kerning(font_face, prev, glyphIndex, FT_KERNING_DEFAULT, &delta);
			cursor.x += delta.x >> 6; 
		}

		// load glyph
		ft_error = FT_Load_Glyph(font_face, glyphIndex, FT_LOAD_DEFAULT);
		assert(ft_error == 0);

		// render glyph
		//ft_error = FT_Render_Glyph(font_face->glyph, FT_RENDER_MODE_NORMAL);
		ft_error = FT_Render_Glyph(font_face->glyph, FT_RENDER_MODE_LCD);
		assert(ft_error == 0);

		// adjust cursor according to the glyph info
		FT_Vector glyphPen;
		glyphPen.x = cursor.x + font_face->glyph->bitmap_left;
		glyphPen.y = cursor.y + font_face->glyph->bitmap.rows - font_face->glyph->bitmap_top;
		DrawBitmap(hdc, font_face->glyph->bitmap, glyphPen);

		// advance cursor
		cursor.x += font_face->glyph->advance.x >> 6;
		prev = glyphIndex;
	}

	FT_Done_Face(font_face);
	return TRUE;
}