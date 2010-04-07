#pragma once

#include "renderer.h"
using namespace std;

class ft_renderer: public gdimm_renderer
{
	FT_Render_Mode _render_mode;
	FT_ULong _load_flags;
	FTC_ScalerRec _ft_scaler;
	TT_OS2 _os2_table;

	static FT_ULong get_load_flags(FT_Render_Mode render_mode, const WCHAR *font_name);
	static void oblique_outline(const FT_Outline *outline, double angle);

	const FT_BitmapGlyph render_glyph(WORD glyph_index, const WCHAR *font_face);
	void update_glyph_pos(UINT options, CONST INT *lpDx);

public:
	ft_renderer(gdimm_text *text);
	bool render(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx, FT_Render_Mode render_mode);
};