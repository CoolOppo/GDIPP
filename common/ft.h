#pragma once

#include "font_info.h"

extern FT_Library ft_lib;
extern FTC_Manager ft_cache_man;
extern FTC_CMapCache ft_cmap_cache;
extern FTC_ImageCache ft_glyph_cache;
extern FontTrait ft_font_trait;

void InitializeFreeType();
void DestroyFreeType();
FT_Error Face_Requester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface);