#include "stdafx.h"
#include "dw_text.h"
#include "text_helper.h"
#include "dw_renderer.h"
#include "gdimm.h"

CComPtr<IDWriteFactory> dw_factory = NULL;
CComPtr<IDWriteGdiInterop> dw_gdi_interop = NULL;
CComPtr<IDWriteRenderingParams> dw_rendering_params = NULL;

POINT gdimm_dw_text::get_adjusted_origin(const POINT &origin, int width) const
{
	POINT adj_origin = origin;

	switch ((TA_LEFT | TA_RIGHT | TA_CENTER) & _text_alignment)
	{
	case TA_LEFT:
		break;
	case TA_RIGHT:
		adj_origin.x -= width;
		break;
	case TA_CENTER:
		adj_origin.x -= width / 2;
		break;
	}

	switch ((TA_TOP | TA_BOTTOM | TA_BASELINE) & _text_alignment)
	{
	case TA_TOP:
		adj_origin.y += _outline_metrics->otmTextMetrics.tmAscent;
		break;
	case TA_BOTTOM:
		adj_origin.y -= _outline_metrics->otmTextMetrics.tmDescent;
		break;
	case TA_BASELINE:
		break;
	}

	return adj_origin;
}

SIZE gdimm_dw_text::get_glyph_run_bbox(IDWriteFontFace *dw_font_face,
	const DWRITE_GLYPH_METRICS *glyph_metrics,
	UINT32 count,
	int point_size) const
{
	DWRITE_FONT_METRICS font_metrics;
	dw_font_face->GetMetrics(&font_metrics);

	SIZE bbox = {};

	for (UINT32 i = 0; i < count; i++)
	{
		bbox.cx += glyph_metrics[i].advanceWidth;
		bbox.cy = max(bbox.cy, glyph_metrics[i].advanceHeight);
	}

	bbox.cx = MulDiv(bbox.cx, point_size, font_metrics.designUnitsPerEm);
	bbox.cy = MulDiv(bbox.cy, point_size, font_metrics.designUnitsPerEm);

	return bbox;
}

void gdimm_dw_text::get_float_dx(UINT options, UINT c, CONST INT *lpDx, FLOAT *outDx)
{
	const int dx_factor = ((options & ETO_PDY) ? 2 : 1);

	for (UINT i = 0; i < c; i++)
		outDx[i] = (FLOAT) lpDx[i * dx_factor];
}

bool gdimm_dw_text::draw_glyph(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	HRESULT hr;
	BOOL b_ret;

	CComPtr<IDWriteFontFace> dw_font_face;
	hr = dw_gdi_interop->CreateFontFaceFromHdc(_hdc_text, &dw_font_face);
	assert(hr == S_OK);

	DWRITE_GLYPH_METRICS *glyph_metrics = new DWRITE_GLYPH_METRICS[c];
	hr = dw_font_face->GetDesignGlyphMetrics((UINT16*) lpString, c, glyph_metrics);
	assert(hr == S_OK);

	const LONG point_size = _outline_metrics->otmTextMetrics.tmHeight - _outline_metrics->otmTextMetrics.tmInternalLeading;
	SIZE bbox = get_glyph_run_bbox(dw_font_face, glyph_metrics, c, point_size);
	bbox.cy = max(bbox.cy, _outline_metrics->otmTextMetrics.tmHeight);

	vector<FLOAT> glyph_advances;
	if (lpDx != NULL)
	{
		glyph_advances.resize(c);
		get_float_dx(options, c, lpDx, &glyph_advances[0]);
	}

	POINT dest_origin = {x, y};
	//dest_origin = get_adjusted_origin(dest_origin, 

	DWRITE_GLYPH_RUN glyph_run = {};
	glyph_run.fontFace = dw_font_face;
	glyph_run.fontEmSize = (FLOAT) point_size;
	glyph_run.glyphCount = c;
	glyph_run.glyphIndices = (UINT16*) lpString;

	if (!glyph_advances.empty())
		glyph_run.glyphAdvances = &glyph_advances[0];

	CComPtr<IDWriteBitmapRenderTarget> dw_bmp_render_target;
	hr = dw_gdi_interop->CreateBitmapRenderTarget(_hdc_text, bbox.cx, bbox.cy, &dw_bmp_render_target);
	assert(hr == S_OK);

	const HDC hdc_canvas = dw_bmp_render_target->GetMemoryDC();
	const COLORREF bg_color = GetBkColor(_hdc_text);

	if (options & ETO_OPAQUE)
		draw_background(_hdc_text, lprect, bg_color);

	const int bk_mode = GetBkMode(_hdc_text);
	if (bk_mode == OPAQUE)
	{
		const RECT bk_rect = {0, 0, bbox.cx, bbox.cy};
		b_ret = draw_background(hdc_canvas, &bk_rect, bg_color);
	}
	else if (bk_mode == TRANSPARENT)
		b_ret = BitBlt(hdc_canvas, 0, 0, bbox.cx, bbox.cy, _hdc_text, x, y, SRCCOPY | NOMIRRORBITMAP);
	else
		b_ret = FALSE;

	if (!b_ret)
		return false;

	hr = dw_bmp_render_target->DrawGlyphRun(0,
		0,
		DWRITE_MEASURING_MODE_GDI_NATURAL,
		&glyph_run,
		dw_rendering_params,
		_text_color);
	assert(hr == S_OK);

/*
	if (options & ETO_CLIPPED)
	{
		RECT dest_rect = {dest_origin.x, dest_origin.y, dest_origin.x + bmp_width, dest_origin.y + cell_height};
		IntersectRect(&dest_rect, &dest_rect, lprect);
		const LONG dest_width = dest_rect.right - dest_rect.left;
		const LONG dest_height = dest_rect.bottom - dest_rect.top;
		const LONG dest_x = dest_rect.left - dest_origin.x;
		const LONG dest_y = dest_rect.top - dest_origin.y;

		b_ret = BitBlt(_hdc_text,
			dest_rect.left,
			dest_rect.top,
			dest_width,
			dest_height,
			hdc_canvas,
			dest_x,
			dest_y,
			SRCCOPY | NOMIRRORBITMAP);
		assert(b_ret);
	}
	else
	{*/
		b_ret = BitBlt(_hdc_text,
			x, y,
			bbox.cx, bbox.cy,
			hdc_canvas,
			0, 0,
			SRCCOPY | NOMIRRORBITMAP);
		assert(b_ret);
	//}

	return true;
}

bool gdimm_dw_text::draw_text(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	HRESULT hr;
	BOOL b_ret;

	CComPtr<IDWriteFont> dw_font;
	wcsncpy_s(_font_attr.lfFaceName, metric_family_name(_outline_metrics), LF_FACESIZE);
	hr = dw_gdi_interop->CreateFontFromLOGFONT(&_font_attr, &dw_font);
	assert(hr == S_OK);

	CComPtr<IDWriteTextFormat> dw_text_format;
	hr = dw_factory->CreateTextFormat(metric_family_name(_outline_metrics),
		NULL,
		dw_font->GetWeight(),
		dw_font->GetStyle(),
		dw_font->GetStretch(),
		(FLOAT)(_outline_metrics->otmTextMetrics.tmHeight - _outline_metrics->otmTextMetrics.tmInternalLeading),
		L"",
		&dw_text_format);
	assert(hr == S_OK);

	CComPtr<IDWriteTextLayout> dw_text_layout;
	hr = dw_factory->CreateTextLayout(
		lpString,
		c,
		dw_text_format,
		(FLOAT) _bmp_info.bmiHeader.biWidth,
		(FLOAT) _bmp_info.bmiHeader.biHeight,
		&dw_text_layout);
	assert(hr == S_OK);

	DWRITE_TEXT_METRICS text_metrics;
	hr = dw_text_layout->GetMetrics(&text_metrics);
	assert(hr == S_OK);

	const UINT32 text_width = (UINT32) ceil(text_metrics.width);
	const UINT32 text_height = (UINT32) ceil(text_metrics.height);

	CComPtr<IDWriteBitmapRenderTarget> dw_bmp_render_target;
	hr = dw_gdi_interop->CreateBitmapRenderTarget(_hdc_text, 
		text_width, text_height,
		&dw_bmp_render_target);
	assert(hr == S_OK);

	const HDC hdc_canvas = dw_bmp_render_target->GetMemoryDC();
	const COLORREF bg_color = GetBkColor(_hdc_text);

	if (options & ETO_OPAQUE)
		draw_background(_hdc_text, lprect, bg_color);

	const int bk_mode = GetBkMode(_hdc_text);
	if (bk_mode == OPAQUE)
	{
		const RECT bk_rect = {0, 0, text_width, text_height};
		b_ret = draw_background(hdc_canvas, &bk_rect, bg_color);
	}
	else if (bk_mode == TRANSPARENT)
	{
		// "If a rotation or shear transformation is in effect in the source device context, BitBlt returns an error"
		b_ret = BitBlt(hdc_canvas, 0, 0, text_width, text_height, _hdc_text, x, y, SRCCOPY | NOMIRRORBITMAP);
	}
	else
		b_ret = FALSE;

	if (!b_ret)
		return false;

	const CComPtr<IDWriteTextRenderer> gdi_text_renderer = new dw_renderer(dw_bmp_render_target, dw_rendering_params, _text_color);

	hr = dw_text_layout->Draw(NULL, gdi_text_renderer, 0, 0);
	assert(hr == S_OK);

	BitBlt(_hdc_text,
		x, y,
		text_width, text_height,
		hdc_canvas,
		0, 0, 
		SRCCOPY | NOMIRRORBITMAP);

	return true;
}

bool gdimm_dw_text::init(HDC hdc)
{
	if (!gdimm_text::init(hdc))
		return false;

	HRESULT hr;

	if (dw_factory == NULL)
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**) &dw_factory);
		assert(hr == S_OK);
	}

	if (dw_gdi_interop == NULL)
	{
		hr = dw_factory->GetGdiInterop(&dw_gdi_interop);
		assert(hr == S_OK);
	}

	if (dw_rendering_params == NULL)
	{
		hr = dw_factory->CreateRenderingParams(&dw_rendering_params);
		assert(hr == S_OK);
	}

	return true;
}

bool gdimm_dw_text::text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	if (options & ETO_GLYPH_INDEX)
		return draw_glyph(x, y, options, lprect, lpString, c, lpDx);
	else
		return draw_text(x, y, options, lprect, lpString, c, lpDx);
}