#pragma once

#include "text.h"
#include "font_man.h"
#include "wic_dib.h"

using namespace std;

class gdimm_wic_text : public gdimm_text
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

	gdimm_wic_dib _wic_bitmap;
	DWRITE_MEASURING_MODE _measuring_mode;
	bool _use_gdi_natural;
	vector<FLOAT> _advances;
	LONG _em_size;
	COLORREF _bg_color;
	POINT _cursor;
	int _char_extra;

	bool prepare(LPCWSTR lpString, UINT c, text_metrics &metrics, IDWriteFontFace **dw_font_face, DWRITE_GLYPH_RUN &dw_glyph_run);
	bool prepare(LPCWSTR lpString, UINT c, text_metrics &metrics, IDWriteTextFormat **dw_text_format, IDWriteTextLayout **dw_text_layout);
	bool draw_text(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);

public:
	gdimm_wic_text();

	virtual bool begin(const gdimm_text_context *context);
	bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
};