#pragma once

#include "renderer.h"

class ft_renderer: public gdimm_renderer
{
	FT_Render_Mode _render_mode;
	FT_ULong _load_flags;
	double _embolden;
	FTC_ScalerRec _ft_scaler;

	static FT_ULong get_load_flags(FT_Render_Mode render_mode, const WCHAR *font_name);
	static void oblique_outline(const FT_Outline *outline, double angle);

	void update_embolden(const TT_OS2 &os2_table, const WCHAR *font_name);
	const FT_BitmapGlyph render_glyph(WORD glyph_index, const WCHAR *font_face);
	void update_glyph_pos(UINT options, CONST INT *lpDx);

public:
	ft_renderer(gdimm_text *text);
	bool render(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx, FT_Render_Mode render_mode);
};