#pragma once

#include "renderer.h"

class gdimm_ggo_renderer : public gdimm_renderer
{
	static FT_Glyph empty_outline_glyph;

	UINT _ggo_format;
	MAT2 _matrix;

	static void outline_ggo_to_ft(DWORD ggo_outline_buf_len, const BYTE *ggo_outline_buf, vector<FT_Vector> &curve_points, vector<char> &curve_tags, vector<short> &contour_indices);

	bool get_glyph_metrics(wchar_t ch, GLYPHMETRICS &glyph_metrics) const;
	const FT_Glyph outline_to_bitmap(wchar_t ch, GLYPHMETRICS &glyph_metrics) const;
	bool render(bool is_glyph_index, LPCWSTR lpString, UINT c, glyph_run &new_glyph_run);

public:
	gdimm_ggo_renderer(gdipp_rpc_session *render_session);
};