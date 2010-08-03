#pragma once

#include "renderer.h"

class gdimm_ft_renderer : public gdimm_renderer
{
	static FT_BitmapGlyphRec empty_glyph;

	FT_F26Dot6 get_embolden(const font_setting_cache *setting_cache, unsigned char font_weight_class, unsigned char text_weight_class);
	static void get_font_size(const OUTLINETEXTMETRICW *outline_metrics, FT_Short xAvgCharWidth, FT_UInt &font_width, FT_UInt &font_height);
	static FT_ULong get_load_flags(const font_setting_cache *setting_cache, FT_Render_Mode render_mode);
	static void oblique_outline(const FT_Outline *outline, double slant_adv);

	const FT_BitmapGlyph render_glyph(WORD glyph_index,
		const FTC_Scaler scaler,
		FT_F26Dot6 embolden,
		FT_ULong load_flags,
		bool is_italic,
		const LOGFONTW &font_trait);
	void update_glyph_pos(glyph_run &new_glyph_run);
	bool render(LPCWSTR lpString, UINT c, bool is_glyph_index, CONST INT *lpDx, bool is_pdy, glyph_run &new_glyph_run);
};