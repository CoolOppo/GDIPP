#pragma once

#include "font_man.h"
#include "painter.h"
#include "wic_dib.h"

using namespace std;

class gdimm_wic_painter : public gdimm_painter
{
	struct text_metrics
	{
		UINT32 width;
		LONG height;
		LONG ascent;
		LONG descent;
		POINT origin;
		POINT baseline;
	};

	CComPtr<ID2D1Factory> _d2d_factory;
	CComPtr<ID2D1RenderTarget> _wic_render_target;
	CComPtr<IDWriteFactory> _dw_factory;
	CComPtr<IDWriteGdiInterop> _dw_gdi_interop;

	vector<FLOAT> _advances;
	DWRITE_MEASURING_MODE _dw_measuring_mode;
	HDC _hdc_canvas;
	FLOAT _pixels_per_dip;
	bool _use_gdi_natural;
	gdimm_wic_dib _wic_bitmap;

	FLOAT _em_size;
	POINT _cursor;

	bool prepare(LPCWSTR lpString, UINT c, text_metrics &metrics, IDWriteFontFace **dw_font_face, DWRITE_GLYPH_RUN &dw_glyph_run);
	bool prepare(LPCWSTR lpString, UINT c, text_metrics &metrics, IDWriteTextLayout **dw_text_layout);
	void set_param(ID2D1RenderTarget *render_target);
	bool draw_text(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);

public:
	gdimm_wic_painter();

	virtual bool begin(const dc_context *context);
	bool paint(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
};