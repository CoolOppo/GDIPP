#pragma once

#include "gdipp_lib/setting_cache.h"
#include "gdipp_server/renderer.h"

class ft_renderer : public renderer
{
public:
	const FT_OutlineGlyph get_outline_glyph(wchar_t glyph_char, bool is_glyph_index);

private:
	FT_F26Dot6 get_embolden(const font_config_cache *setting_cache, char font_weight_class, char text_weight_class);
	static void get_font_size(const OUTLINETEXTMETRICW *outline_metrics, FT_Short xAvgCharWidth, FT_UInt &font_width, FT_UInt &font_height);
	static FT_ULong make_load_flags(const font_config_cache *setting_cache, FT_Render_Mode render_mode);
	static void oblique_outline(const FT_Outline *outline, double slant_adv);

	bool generate_outline_glyph(FT_Glyph *glyph,
		WORD glyph_index,
		const FTC_Scaler scaler,
		FT_F26Dot6 embolden,
		FT_ULong load_flags,
		bool is_italic) const;
	const FT_Glyph generate_bitmap_glyph(WORD glyph_index,
		const FTC_Scaler scaler,
		FT_Render_Mode render_mode,
		FT_F26Dot6 embolden,
		FT_ULong load_flags,
		bool is_italic,
		bool request_outline,
		unsigned int font_trait) const;
	bool generate_glyph_run(bool is_glyph_index, LPCWSTR lpString, UINT c, glyph_run &new_glyph_run, bool request_outline);

	bool render(bool is_glyph_index, bool is_pdy, LPCWSTR lpString, UINT c, CONST INT *lpDx, glyph_run &new_glyph_run);
};
