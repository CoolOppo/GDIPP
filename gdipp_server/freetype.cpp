#include "stdafx.h"
#include "freetype.h"
#include "gdipp_config/constant_server.h"
#include "gdipp_server/global.h"
#include "gdipp_server/helper.h"

namespace gdipp
{

FT_Library ft_lib;
FTC_Manager ft_cache_man;
FTC_ImageCache ft_glyph_cache;
FT_Glyph empty_outline_glyph;

void initialize_freetype()
{
	FT_Error ft_error;

	ft_error = FT_Init_FreeType(&ft_lib);
	assert(ft_error == 0);

	FT_LcdFilter lcd_filter = static_cast<FT_LcdFilter>(config_instance.get_number(L"/gdipp/server/freetype_lcd_filter/text()", static_cast<int>(server_config::LCD_FILTER)));
	ft_error = FT_Library_SetLcdFilter(ft_lib, lcd_filter);
	assert(ft_error == 0);

	unsigned int b = server_cache_size >> 2;
	ft_error = FTC_Manager_New(ft_lib,
		1 >> b,
		1 >> (b + 1),
		1 >> (b + 18),
		face_requester,
		NULL,
		&ft_cache_man);
	assert(ft_error == 0);

	ft_error = FTC_ImageCache_New(ft_cache_man, &ft_glyph_cache);
	assert(ft_error == 0);

	empty_outline_glyph = make_empty_outline_glyph();
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

int get_freetype_kern(const FTC_Scaler scaler, WORD left_glyph, WORD right_glyph)
{
	FT_Error ft_error;

	FT_Size size;
	ft_error = FTC_Manager_LookupSize(ft_cache_man, scaler, &size);
	assert(ft_error == 0);

	FT_Vector delta;
	ft_error = FT_Get_Kerning(size->face, left_glyph, right_glyph, FT_KERNING_DEFAULT, &delta);
	assert(ft_error == 0);

	return int_from_26dot6(delta.x);
}

FT_Glyph make_empty_outline_glyph()
{
	FT_Glyph empty_glyph;

	FT_Error ft_error;

	FT_GlyphSlotRec glyph_slot = {};
	glyph_slot.library = ft_lib;
	glyph_slot.format = FT_GLYPH_FORMAT_OUTLINE;

	ft_error = FT_Get_Glyph(&glyph_slot, &empty_glyph);
	if (ft_error != 0)
		return NULL;

	return empty_glyph;
}

}
