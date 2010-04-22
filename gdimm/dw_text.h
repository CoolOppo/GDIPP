#pragma once

#include "text.h"

class gdimm_dw_text : public gdimm_text
{
	POINT get_adjusted_origin(const POINT &origin, int width) const;
	SIZE get_glyph_run_bbox(IDWriteFontFace *dw_font_face, const DWRITE_GLYPH_METRICS *glyph_metrics, UINT32 count, int point_size) const;
	void get_float_dx(UINT options, UINT c, CONST INT *lpDx, FLOAT *outDx);
	bool draw_glyph(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
	bool draw_text(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);

public:
	bool init(HDC hdc);
	bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
};