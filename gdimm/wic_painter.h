#pragma once

#include "painter.h"
#include "wic_dib.h"

//using std::;

class gdimm_wic_painter : public gdimm_painter
{
public:
	gdimm_wic_painter();

	virtual bool begin(const dc_context *context, FT_Render_Mode render_mode);
	void end();
	bool paint(int x, int y, UINT options, CONST RECT *lprect, const void *text, UINT c, CONST INT *lpDx);

private:
	bool prepare(LPCWSTR lpString, UINT c, LONG &bbox_width, IDWriteFontFace **dw_font_face, DWRITE_GLYPH_RUN &dw_glyph_run);
	bool prepare(LPCWSTR lpString, UINT c, LONG &bbox_width, IDWriteTextLayout **dw_text_layout);
	void set_param(ID2D1RenderTarget *render_target);
	bool draw_text(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);

	static ID2D1Factory *_d2d_factory;
	static IDWriteFactory *_dw_factory;
	static IDWriteGdiInterop *_dw_gdi_interop;

	vector<FLOAT> _advances;
	DWRITE_MEASURING_MODE _dw_measuring_mode;
	HDC _hdc_canvas;
	FLOAT _pixels_per_dip;
	bool _use_gdi_natural;
	gdimm_wic_dib _wic_bitmap;

	FLOAT _em_size;
};
