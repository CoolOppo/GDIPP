#include "stdafx.h"
#include "ft.h"

FT_Library ft_lib;
FTC_Manager ft_cache_man;
FTC_CMapCache ft_cmap_cache;
FTC_ImageCache ft_glyph_cache;
FontTrait ft_font_trait;

void InitializeFreeType()
{
	FT_Error ft_error;

	ft_error = FT_Init_FreeType(&ft_lib);
	assert(ft_error == 0);

	// enable FreeType LCD filter
	ft_error = FT_Library_SetLcdFilter(ft_lib, FT_LCD_FILTER_DEFAULT);
	assert(ft_error == 0);

	ft_error = FTC_Manager_New(ft_lib, 0, 0, 0, Face_Requester, NULL, &ft_cache_man);
	assert(ft_error == 0);

	ft_error = FTC_CMapCache_New(ft_cache_man, &ft_cmap_cache);
	assert(ft_error == 0);

	ft_error = FTC_ImageCache_New(ft_cache_man, &ft_glyph_cache);
	assert(ft_error == 0);
}

void DestroyFreeType()
{
	FTC_Manager_Done(ft_cache_man);
	FT_Error ft_error = FT_Done_FreeType(ft_lib);
	assert(ft_error == 0);
}

FT_Error Face_Requester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface)
{
	HDC hdc_mem = CreateCompatibleDC(NULL);
	assert(hdc_mem != NULL);
	HFONT hfont = CreateFont(-11, 0, 0, 0,
		ft_font_trait.lfWeight,
		ft_font_trait.lfItalic,
		FALSE,
		FALSE,
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		ft_font_trait.lfFaceName);
	assert(hfont != NULL);
	SelectObject(hdc_mem, hfont);

	DWORD needed = GetFontData(hdc_mem, 0, 0, NULL, 0);
	assert(needed != GDI_ERROR);
	BYTE *font_data = new BYTE[needed];
	needed = GetFontData(hdc_mem, 0, 0, font_data, needed);
	assert(needed != GDI_ERROR);

	DeleteObject(hfont);
	DeleteDC(hdc_mem);

	FT_Error error = FT_New_Memory_Face(library, font_data, needed, 0, aface);
	assert(error == 0);

	return 0;
}