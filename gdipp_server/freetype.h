#pragma once

namespace gdipp
{

extern FT_Library ft_lib;
extern FTC_Manager ft_cache_man;
extern FTC_ImageCache ft_glyph_cache;
extern FT_Glyph empty_outline_glyph;

void initialize_freetype();
void destroy_freetype();
FT_Error face_requester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface);
int get_freetype_kern(const FTC_Scaler scaler, WORD left_glyph, WORD right_glyph);
FT_Glyph make_empty_outline_glyph();

}
