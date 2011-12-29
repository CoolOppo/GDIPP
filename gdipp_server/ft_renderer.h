#pragma once

#include "gdipp_server/renderer.h"

namespace gdipp
{

class ft_renderer : public renderer
{
public:
	explicit ft_renderer(rpc_session *render_session);

private:
	static FT_F26Dot6 get_embolden_diff(char font_weight_class, char text_weight_class);
	static void get_font_width_height(const OUTLINETEXTMETRICW *outline_metrics, FT_Short xAvgCharWidth, FT_UInt &font_width, FT_UInt &font_height);
	static FT_ULong make_load_flags(const render_config_static *render_config, FT_Render_Mode render_mode);
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
		uint128_t render_trait) const;
	bool generate_glyph_run(bool is_glyph_index, LPCWSTR lpString, UINT c, glyph_run *new_glyph_run, bool request_outline);

	bool render(bool is_glyph_index, LPCWSTR lpString, UINT c, glyph_run *new_glyph_run);
};

}
