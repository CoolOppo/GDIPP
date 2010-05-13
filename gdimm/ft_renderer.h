#pragma once

#include "gdi_text.h"

class ft_renderer : public gdimm_gdi_text
{
	FTC_ScalerRec _ft_scaler;
	FT_Render_Mode _render_mode;
	FT_ULong _load_flags;
	FT_F26Dot6 _embolden;
	bool _is_italic;
	const void *_cache_node_ptr;

	static FT_ULong get_load_flags(FT_Render_Mode render_mode, const wchar_t *font_name);
	static void oblique_outline(const FT_Outline *outline, double angle);

	void update_embolden(FT_UShort weight_class, const wchar_t *font_name);
	const FT_BitmapGlyph render_glyph(WORD glyph_index, const wchar_t *font_face);
	void update_glyph_pos(UINT options, CONST INT *lpDx);
	bool render(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx, FT_Render_Mode render_mode);

public:
	ft_renderer(HDC hdc);
	~ft_renderer();

	virtual bool init();
};