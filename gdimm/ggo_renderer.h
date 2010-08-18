#pragma once

#include "renderer.h"

class gdimm_ggo_renderer : public gdimm_renderer
{
	static FT_Glyph empty_outline_glyph;

	UINT _ggo_format;
	MAT2 _matrix;

	bool get_glyph_metrics(wchar_t ch, GLYPHMETRICS &glyph_metrics) const;
	const FT_Glyph outline_to_bitmap(wchar_t ch, GLYPHMETRICS &glyph_metrics) const;
	int render(bool is_glyph_index, bool is_pdy, LPCWSTR lpString, UINT c, CONST INT *lpDx, glyph_run &new_glyph_run);

public:
	gdimm_ggo_renderer();
};