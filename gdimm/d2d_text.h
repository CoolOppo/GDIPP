#pragma once

#include "text.h"

class gdimm_d2d_text : public gdimm_text
{
	CComPtr<ID2D1Factory> _d2d1_factory;
	CComPtr<ID2D1DCRenderTarget> _dc_render_target;
	CComPtr<ID2D1SolidColorBrush> _d2d1_brush;
	CComPtr<IDWriteFactory> _dw_factory;
	CComPtr<IDWriteGdiInterop> _dw_gdi_interop;

	HDC _last_hdc;

	void get_text_geometry(int x, int y, int width, int height, int ascent, D2D1_POINT_2F &origin, RECT &text_rect) const;
	SIZE get_glyph_run_bbox(IDWriteFontFace *dw_font_face, const DWRITE_GLYPH_METRICS *glyph_metrics, UINT32 count, int point_size) const;
	void get_float_advances(UINT options, UINT c, CONST INT *lpDx, FLOAT *out_advances);
	bool draw_glyph(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
	bool draw_text(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);

public:
	gdimm_d2d_text();

	bool begin(const gdimm_text_context *context);
	bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
};