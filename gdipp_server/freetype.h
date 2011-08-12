#pragma once

namespace gdipp
{

extern FT_Library ft_lib;
extern FTC_Manager ft_cache_man;
extern FTC_ImageCache ft_glyph_cache;

extern FT_UInt ft_cache_max_faces;
extern FT_UInt ft_cache_max_sizes;
extern FT_ULong ft_cache_max_bytes;

void initialize_freetype();
void destroy_freetype();
FT_Error face_requester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface);

}
