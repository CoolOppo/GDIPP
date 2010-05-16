#pragma once

#include "text.h"

class gdimm_d2d_text : public gdimm_text
{
	static CComPtr<ID2D1Factory> _d2d1_factory;
	static CComPtr<ID2D1DCRenderTarget > _d2d1_render_target;

	static CComPtr<IDWriteFactory> _dw_factory;
	static CComPtr<IDWriteGdiInterop> _dw_gdi_interop;

	D2D1_COLOR_F _fg_color;

	void get_text_geometry(int x, int y, int width, int height, int ascent, D2D1_POINT_2F &origin, RECT &text_rect) const;
	SIZE get_glyph_run_bbox(IDWriteFontFace *dw_font_face, const DWRITE_GLYPH_METRICS *glyph_metrics, UINT32 count, int point_size) const;
	void get_float_advances(UINT options, UINT c, CONST INT *lpDx, FLOAT *out_advances);
	bool draw_glyph(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
	bool draw_text(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);

public:
	bool begin(HDC hdc, const OUTLINETEXTMETRICW *outline_metrics, const wchar_t *font_face, const font_setting_cache *setting_cache);
	bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
};