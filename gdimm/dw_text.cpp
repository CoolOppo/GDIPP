#include "stdafx.h"
#include "dw_text.h"
#include "text_helper.h"
#include "gdimm.h"

CComPtr<IDWriteFactory> gdimm_dw_text::_dw_factory = NULL;
CComPtr<IDWriteGdiInterop> gdimm_dw_text::_dw_gdi_interop = NULL;
CComPtr<IDWriteTextAnalyzer> gdimm_dw_text::_dw_text_analyzer = NULL;
CComPtr<IDWriteRenderingParams> gdimm_dw_text::_dw_render_param = NULL;
CComPtr<gdimm_dw_renderer> gdimm_dw_text::_dw_renderer = NULL;

gdimm_dw_text::gdimm_dw_text(HDC hdc)
:
gdimm_text(hdc)
{
}

POINT gdimm_dw_text::get_adjusted_origin(int x, int y, int width, int height, int ascent) const
{
	POINT adj_origin = {x, y};

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
		break;
	case TA_BOTTOM:
		adj_origin.y -= height;
		break;
	case TA_BASELINE:
		adj_origin.y -= ascent;
		break;
	}

	return adj_origin;
}

UINT32 gdimm_dw_text::get_glyph_run_width(IDWriteFontFace *dw_font_face,
	const DWRITE_GLYPH_METRICS *glyph_metrics,
	UINT32 count,
	int point_size) const
{
	UINT32 glyph_run_width = 0;

	DWRITE_FONT_METRICS font_metrics;
	dw_font_face->GetMetrics(&font_metrics);

	for (UINT32 i = 0; i < count; i++)
		glyph_run_width += glyph_metrics[i].advanceWidth;

	return MulDiv(glyph_run_width, point_size, font_metrics.designUnitsPerEm);
}

bool gdimm_dw_text::draw_glyph(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	HRESULT hr;
	BOOL b_ret;

	CComPtr<IDWriteFontFace> dw_font_face;
	hr = _dw_gdi_interop->CreateFontFaceFromHdc(_hdc_text, &dw_font_face);
	assert(hr == S_OK);

	DWRITE_GLYPH_METRICS *glyph_metrics = new DWRITE_GLYPH_METRICS[c];
	hr = dw_font_face->GetDesignGlyphMetrics((UINT16*) lpString, c, glyph_metrics);
	assert(hr == S_OK);

	const LONG point_size = _outline_metrics->otmTextMetrics.tmHeight - _outline_metrics->otmTextMetrics.tmInternalLeading;
	const UINT32 glyph_run_width = get_glyph_run_width(dw_font_face, glyph_metrics, c, point_size);
	const UINT32 glyph_run_height = _outline_metrics->otmTextMetrics.tmHeight;

	DWRITE_GLYPH_RUN glyph_run = {dw_font_face,
		(FLOAT) point_size,
		c,
		(UINT16*) lpString,
		(_advances.empty() ? NULL : &_advances[0]),
		FALSE,
		0};

	const POINT dest_origin = get_adjusted_origin(x,
		y,
		glyph_run_width,
		glyph_run_height,
		_outline_metrics->otmTextMetrics.tmAscent);

	CComPtr<IDWriteBitmapRenderTarget> dw_render_target;
	hr = _dw_gdi_interop->CreateBitmapRenderTarget(_hdc_text, glyph_run_width, glyph_run_height, &dw_render_target);
	assert(hr == S_OK);
	HDC hdc_canvas = dw_render_target->GetMemoryDC();

	const COLORREF bg_color = GetBkColor(_hdc_text);

	if (options & ETO_OPAQUE)
		draw_background(_hdc_text, lprect, bg_color);

	const int bk_mode = GetBkMode(_hdc_text);
	if (bk_mode == OPAQUE)
	{
		const RECT bk_rect = {0, 0, glyph_run_width, glyph_run_height};
		b_ret = draw_background(hdc_canvas, &bk_rect, bg_color);
	}
	else if (bk_mode == TRANSPARENT)
	{
		b_ret = BitBlt(hdc_canvas, 0, 0, glyph_run_width, glyph_run_height, _hdc_text, dest_origin.x, dest_origin.y, SRCCOPY | NOMIRRORBITMAP);
	}
	else
		b_ret = FALSE;

	if (!b_ret)
		return false;

	// DirectWrite uses baseline as the reference origin Y
	hr = dw_render_target->DrawGlyphRun(0, (FLOAT) _outline_metrics->otmTextMetrics.tmAscent, DWRITE_MEASURING_MODE_NATURAL, &glyph_run, _dw_render_param, _text_color);
	assert(hr == S_OK);

	b_ret = BitBlt(_hdc_text, dest_origin.x, dest_origin.y, glyph_run_width, glyph_run_height, hdc_canvas, 0, 0, SRCCOPY | NOMIRRORBITMAP);
	assert(hr == S_OK);

	return true;
}

bool gdimm_dw_text::draw_text(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	HRESULT hr;
	BOOL b_ret;

	LOGFONT actual_lf = _font_attr;
	actual_lf.lfHeight = _outline_metrics->otmTextMetrics.tmHeight - _outline_metrics->otmTextMetrics.tmInternalLeading;
	actual_lf.lfWidth = 0;
	wcsncpy_s(actual_lf.lfFaceName, metric_family_name(_outline_metrics), LF_FACESIZE);
	
	CComPtr<IDWriteFont> dw_font;
	hr = _dw_gdi_interop->CreateFontFromLOGFONT(&actual_lf, &dw_font);
	assert(hr == S_OK);
	
	CComPtr<IDWriteTextFormat> dw_text_format;
	hr = _dw_factory->CreateTextFormat(actual_lf.lfFaceName,
		NULL,
		dw_font->GetWeight(),
		dw_font->GetStyle(),
		dw_font->GetStretch(),
		(FLOAT) actual_lf.lfHeight,
		L"",
		&dw_text_format);
	assert(hr == S_OK);

	switch ((TA_LEFT | TA_RIGHT | TA_CENTER) & _text_alignment)
	{
	case TA_LEFT:
		hr = dw_text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		break;
	case TA_RIGHT:
		hr = dw_text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
		break;
	case TA_CENTER:
		hr = dw_text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER); 
		break;
	}
	assert(hr == S_OK);

	switch ((TA_TOP | TA_BOTTOM | TA_BASELINE) & _text_alignment)
	{
	case TA_TOP:
		hr = dw_text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		break;
	case TA_BOTTOM:
		hr = dw_text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
		break;
	case TA_BASELINE:
		hr = dw_text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		break;
	}
	assert(hr == S_OK);
	
	CComPtr<IDWriteTextLayout> dw_text_layout;
	hr = _dw_factory->CreateTextLayout(lpString,
		c,
		dw_text_format,
		(FLOAT) _bmp_info.bmiHeader.biWidth,
		(FLOAT) _outline_metrics->otmTextMetrics.tmHeight,
		&dw_text_layout);
	assert(hr == S_OK);

	DWRITE_TEXT_METRICS text_metrics;
	hr = dw_text_layout->GetMetrics(&text_metrics);
	assert(hr == S_OK);

	CComPtr<IDWriteBitmapRenderTarget> dw_render_target;
	hr = _dw_gdi_interop->CreateBitmapRenderTarget(_hdc_text, text_metrics.width, _outline_metrics->otmTextMetrics.tmHeight, &dw_render_target);
	assert(hr == S_OK);
	HDC hdc_canvas = dw_render_target->GetMemoryDC();

	const COLORREF bg_color = GetBkColor(_hdc_text);

	if (options & ETO_OPAQUE)
		draw_background(_hdc_text, lprect, bg_color);

	const int bk_mode = GetBkMode(_hdc_text);
	if (bk_mode == OPAQUE)
	{
		const RECT bk_rect = {0, 0, text_metrics.width, _outline_metrics->otmTextMetrics.tmHeight};
		b_ret = draw_background(hdc_canvas, &bk_rect, bg_color);
	}
	else if (bk_mode == TRANSPARENT)
	{
		b_ret = BitBlt(hdc_canvas, 0, 0, text_metrics.width, _outline_metrics->otmTextMetrics.tmHeight, _hdc_text, x, y, SRCCOPY | NOMIRRORBITMAP);
	}
	else
		b_ret = FALSE;

	if (!b_ret)
		return false;

	gdimm_dw_renderer::drawing_context context = {dw_render_target,
		_text_color,
		(_advances.empty() ? NULL : &_advances[0])};

	hr = dw_text_layout->Draw(&context, _dw_renderer, 0, 0);
	assert(hr == S_OK);

	b_ret = BitBlt(_hdc_text, x, y, text_metrics.width, _outline_metrics->otmTextMetrics.tmHeight, hdc_canvas, 0, 0, SRCCOPY | NOMIRRORBITMAP);
	assert(hr == S_OK);

	return true;
}

bool gdimm_dw_text::init()
{
	if (!gdimm_text::init())
		return false;

	HRESULT hr;

	if (_dw_factory == NULL)
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**) &_dw_factory);
		assert(hr == S_OK);

		if (_dw_gdi_interop == NULL)
		{
			hr = _dw_factory->GetGdiInterop(&_dw_gdi_interop);
			assert(hr == S_OK);
		}

		if (_dw_render_param == NULL)
		{
			hr = _dw_factory->CreateRenderingParams(&_dw_render_param);
			assert(hr == S_OK);

			if (_dw_renderer == NULL)
				_dw_renderer = new gdimm_dw_renderer(_dw_render_param);
		}
	}

	return true;
}

bool gdimm_dw_text::text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	const int advance_factor = ((options & ETO_PDY) ? 2 : 1);

	if (lpDx != NULL)
	{
		_advances.resize(c);
		for (UINT i = 0; i < c; i++)
			_advances[i] = (FLOAT) lpDx[i * advance_factor];
	}

	if (options & ETO_GLYPH_INDEX)
		return draw_glyph(x, y, options, lprect, lpString, c, lpDx);
	else
		return draw_text(x, y, options, lprect, lpString, c, lpDx);
}