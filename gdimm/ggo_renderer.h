#pragma once

#include "renderer.h"

class gdimm_ggo_renderer : public gdimm_renderer
{
	static FT_Glyph empty_glyph;
	static FT_Glyph empty_bmp_glyph;

	UINT _ggo_format;
	MAT2 _matrix;

	bool get_glyph_metrics(wchar_t ch, GLYPHMETRICS &glyph_metrics) const;
	const FT_BitmapGlyph outline_to_bitmap(wchar_t ch, GLYPHMETRICS &glyph_metrics) const;
	bool render(bool is_glyph_index, bool is_pdy, LPCWSTR lpString, UINT c, CONST INT *lpDx, glyph_run &new_glyph_run);

public:
	gdimm_ggo_renderer();

	bool begin(const dc_context *context);
};