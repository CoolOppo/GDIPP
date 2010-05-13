#pragma once

#include "dw_renderer.h"
#include "text.h"

using namespace std;

class gdimm_dw_text : public gdimm_text
{
	static CComPtr<IDWriteFactory> _dw_factory;
	static CComPtr<IDWriteGdiInterop> _dw_gdi_interop;
	static CComPtr<IDWriteTextAnalyzer> _dw_text_analyzer;
	static CComPtr<IDWriteRenderingParams> _dw_render_param;
	static CComPtr<gdimm_dw_renderer> _dw_renderer;

	vector<FLOAT> _advances;

	POINT get_adjusted_origin(int x, int y, int width, int height, int ascent) const;
	UINT32 get_glyph_run_width(IDWriteFontFace *dw_font_face, const DWRITE_GLYPH_METRICS *glyph_metrics, UINT32 count, int point_size) const;
	bool draw_glyph(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
	bool draw_text(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);

public:
	gdimm_dw_text(HDC hdc);

	virtual bool init();
	virtual bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
};