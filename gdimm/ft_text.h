#pragma once

#include "gdi_text.h"

class gdimm_ft_text : public gdimm_gdi_text
{
	list<const void *> _cache_node_ptrs;

	static FTC_ScalerRec get_scaler(const OUTLINETEXTMETRICW *outline_metrics, bool width_specified = false, FT_Short xAvgCharWidth = 0);
	static FT_ULong get_load_flags(const font_setting_cache *setting_cache, FT_Render_Mode render_mode);
	static void oblique_outline(const FT_Outline *outline, double slant_adv);

	FT_F26Dot6 get_embolden(const font_setting_cache *setting_cache, unsigned char font_weight_class);
	const FT_BitmapGlyph render_glyph(WORD glyph_index,
		const FTC_Scaler scaler,
		FT_F26Dot6 embolden,
		FT_Render_Mode render_mode,
		FT_ULong load_flags,
		bool is_italic,
		const void *&cache_node_ptr);
	void update_glyph_pos(UINT options, CONST INT *lpDx);
	bool render(UINT options, LPCWSTR lpString, UINT c, CONST INT *lpDx);

public:
	bool begin(const gdimm_text_context *context);
	void end();
};