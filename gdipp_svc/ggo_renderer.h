#pragma once

#include <vector>
#include "gdipp_svc/renderer.h"

namespace gdipp
{

class gdimm_ggo_renderer : public gdimm_renderer
{
public:
	explicit gdimm_ggo_renderer(rpc_session *render_session);
	bool render(bool is_glyph_index, LPCWSTR lpString, UINT c, glyph_run *new_glyph_run);

private:
	static void outline_ggo_to_ft(DWORD ggo_outline_buf_len, const BYTE *ggo_outline_buf, std::vector<FT_Vector> &curve_points, std::vector<char> &curve_tags, std::vector<short> &contour_indices);

	bool get_glyph_metrics(wchar_t ch, GLYPHMETRICS &glyph_metrics) const;
	const FT_Glyph outline_to_bitmap(wchar_t ch, GLYPHMETRICS &glyph_metrics) const;

	static FT_Glyph empty_outline_glyph;

	UINT _ggo_format;
	MAT2 _matrix;
};

}
