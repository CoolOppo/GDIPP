#pragma once

extern FT_Library ft_lib;
extern FTC_Manager ft_cache_man;
extern FTC_CMapCache ft_cmap_cache;
extern FTC_ImageCache ft_glyph_cache;

void initialize_freetype();
void destroy_freetype();
FT_Error face_requester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface);