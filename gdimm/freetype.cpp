#include "stdafx.h"
#include "freetype.h"
#include "font_man.h"
#include "gdimm.h"

FT_Library ft_lib;
FTC_Manager ft_cache_man;
FTC_ImageCache ft_glyph_cache;

FT_UInt ft_cache_max_faces = 2;
FT_UInt ft_cache_max_sizes = 4;
FT_ULong ft_cache_max_bytes = 200000;

void initialize_freetype()
{
	FT_Error ft_error;

	wcs_convert(gdipp_get_gdimm_setting(L"freetype/cache_max_faces", L"", 0, false), &ft_cache_max_faces);
	wcs_convert(gdipp_get_gdimm_setting(L"freetype/cache_max_sizes", L"", 0, false), &ft_cache_max_sizes);
	wcs_convert(gdipp_get_gdimm_setting(L"freetype/cache_max_bytes", L"", 0, false), &ft_cache_max_bytes);
	FT_LcdFilter lcd_filter = FT_LCD_FILTER_DEFAULT;
	wcs_convert(gdipp_get_gdimm_setting(L"freetype/lcd_filter", L"", 0, false), (int *)&lcd_filter);

	ft_error = FT_Init_FreeType(&ft_lib);
	assert(ft_error == 0);

	ft_error = FT_Library_SetLcdFilter(ft_lib, lcd_filter);
	assert(ft_error == 0);

	ft_error = FTC_Manager_New(ft_lib, ft_cache_max_faces, ft_cache_max_sizes, ft_cache_max_bytes, face_requester, NULL, &ft_cache_man);
	assert(ft_error == 0);

	ft_error = FTC_ImageCache_New(ft_cache_man, &ft_glyph_cache);
	assert(ft_error == 0);
}

void destroy_freetype()
{
	FT_Error ft_error;

	FTC_Manager_Done(ft_cache_man);
	
	ft_error = FT_Done_FreeType(ft_lib);
	assert(ft_error == 0);
}

FT_Error face_requester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface)
{
	const long font_id = (long) face_id;

	FT_Open_Args args = {};
	args.flags = FT_OPEN_STREAM;
	args.stream = gdimm_font_man::lookup_stream(font_id);

	return FT_Open_Face(library, &args, gdimm_font_man::lookup_face_index(font_id), aface);
}