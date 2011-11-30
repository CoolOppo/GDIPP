#include "stdafx.h"
#include "freetype.h"
#include "gdipp_server/global.h"

namespace gdipp
{

FT_Library ft_lib;
FTC_Manager ft_cache_man;
FTC_ImageCache ft_glyph_cache;

void initialize_freetype()
{
	FT_Error ft_error;

	ft_error = FT_Init_FreeType(&ft_lib);
	assert(ft_error == 0);

	FT_LcdFilter lcd_filter = static_cast<FT_LcdFilter>(config_instance.get_number(L"/gdipp/freetype/lcd_filter/text()", static_cast<int>(FT_LCD_FILTER_DEFAULT)));
	ft_error = FT_Library_SetLcdFilter(ft_lib, lcd_filter);
	assert(ft_error == 0);

	ft_error = FTC_Manager_New(ft_lib,
		config_instance.get_number(L"/gdipp/freetype/cache_max_faces/text()", 16),
		config_instance.get_number(L"/gdipp/freetype/cache_max_sizes/text()", 32),
		config_instance.get_number(L"/gdipp/freetype/cache_max_bytes/text()", 2097152),
		face_requester,
		NULL,
		&ft_cache_man);
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
	FT_Open_Args args = {};
	args.flags = FT_OPEN_STREAM;
	args.stream = font_mgr_instance.lookup_stream(face_id);

	return FT_Open_Face(library, &args, font_mgr_instance.lookup_face_index(face_id), aface);
}

}
