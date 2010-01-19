#include "stdafx.h"
#include "ft.h"
#include "font.h"

FT_Library ft_lib;
FTC_Manager ft_cache_man;
FTC_CMapCache ft_cmap_cache;
FTC_ImageCache ft_glyph_cache;

void InitializeFreeType()
{
	FT_Error ft_error;

	ft_error = FT_Init_FreeType(&ft_lib);
	assert(ft_error == 0);

#ifdef NDEBUG
	// enable FreeType LCD filter
	ft_error = FT_Library_SetLcdFilter(ft_lib, FT_LCD_FILTER_DEFAULT);
	assert(ft_error == 0);
#endif

	ft_error = FTC_Manager_New(ft_lib, 1, 0, 0, Face_Requester, NULL, &ft_cache_man);
	assert(ft_error == 0);

	ft_error = FTC_CMapCache_New(ft_cache_man, &ft_cmap_cache);
	assert(ft_error == 0);

	ft_error = FTC_ImageCache_New(ft_cache_man, &ft_glyph_cache);
	assert(ft_error == 0);
}

void DestroyFreeType()
{
	//FTC_Manager_Done(ft_cache_man);
	FT_Error ft_error = FT_Done_FreeType(ft_lib);
	assert(ft_error == 0);
}

FT_Error Face_Requester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface)
{
	unsigned int font_index = (unsigned int)face_id;

	FT_Open_Args args;
	args.flags = FT_OPEN_STREAM;
	args.stream = &(gdimm_font::instance().get_info(font_index).stream);
	
	return FT_Open_Face(library, &args, 0, aface);
}