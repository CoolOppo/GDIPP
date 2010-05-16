#include "stdafx.h"
#include "dw_text.h"
#include "text_helper.h"
#include "gdimm.h"

CComPtr<IDWriteFactory> gdimm_dw_text::_dw_factory = NULL;
CComPtr<IDWriteGdiInterop> gdimm_dw_text::_dw_gdi_interop = NULL;
CComPtr<IDWriteRenderingParams> gdimm_dw_text::_dw_render_param = NULL;
CComPtr<gdimm_dw_renderer> gdimm_dw_text::_dw_renderer = NULL;

bool gdimm_dw_text::draw_glyph(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c)
{
	HRESULT hr;
	BOOL b_ret;

	CComPtr<IDWriteFontFace> dw_font_face;
	hr = _dw_gdi_interop->CreateFontFaceFromHdc(_hdc_text, &dw_font_face);
	assert(hr == S_OK);

	vector<DWRITE_GLYPH_METRICS> glyph_metrics;
	glyph_metrics.resize(c);
	hr = dw_font_face->GetDesignGlyphMetrics((UINT16*) lpString, c, &glyph_metrics[0]);
	assert(hr == S_OK);

	const LONG em_height = _outline_metrics->otmTextMetrics.tmHeight - _outline_metrics->otmTextMetrics.tmInternalLeading;
	const LONG cell_height = _outline_metrics->otmTextMetrics.tmHeight;
	const LONG cell_ascent = _outline_metrics->otmTextMetrics.tmAscent;
	const LONG cell_descent = _outline_metrics->otmTextMetrics.tmDescent;

	UINT32 cell_width = 0;
	for (UINT i = 0; i < c; i++)
		cell_width += (UINT32) ceil((FLOAT) glyph_metrics[i].advanceWidth * em_height / _outline_metrics->otmEMSquare);

	DWRITE_GLYPH_RUN glyph_run = {dw_font_face,
		(FLOAT) em_height,
		c,
		(UINT16*) lpString,
		(_advances.empty() ? NULL : &_advances[0]),
		FALSE,
		0};

	const POINT baseline = get_baseline(_text_alignment,
		x,
		y,
		cell_width,
		cell_ascent,
		cell_descent);

	RECT bmp_rect = {baseline.x,
		baseline.y - cell_ascent,
		baseline.x + cell_width,
		baseline.y + cell_descent};

	CComPtr<IDWriteBitmapRenderTarget> dw_render_target;
	hr = _dw_gdi_interop->CreateBitmapRenderTarget(_hdc_text, cell_width, cell_height, &dw_render_target);
	assert(hr == S_OK);
	HDC hdc_canvas = dw_render_target->GetMemoryDC();

	const COLORREF bg_color = GetBkColor(_hdc_text);

	if (options & ETO_OPAQUE)
		draw_background(_hdc_text, lprect, bg_color);

	const int bk_mode = GetBkMode(_hdc_text);
	if (bk_mode == OPAQUE)
	{
		const RECT bk_rect = {0, 0, cell_width, cell_height};
		b_ret = draw_background(hdc_canvas, &bk_rect, bg_color);
	}
	else if (bk_mode == TRANSPARENT)
	{
		// "If a rotation or shear transformation is in effect in the source device context, BitBlt returns an error"
		b_ret = BitBlt(hdc_canvas, 0, 0, cell_width, cell_height, _hdc_text, bmp_rect.left, bmp_rect.top, SRCCOPY);
	}
	else
		b_ret = FALSE;

	if (!b_ret)
		return false;

	// DirectWrite uses baseline as the reference point
	hr = dw_render_target->DrawGlyphRun(0, (FLOAT) cell_ascent, _measuring_mode, &glyph_run, _dw_render_param, _text_color);
	assert(hr == S_OK);

	b_ret = BitBlt(_hdc_text, bmp_rect.left, bmp_rect.top, cell_width, cell_height, hdc_canvas, 0, 0, SRCCOPY);
	assert(hr == S_OK);

	return true;
}

bool gdimm_dw_text::draw_text(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c)
{
	HRESULT hr;
	BOOL b_ret;

	const LONG em_height = _outline_metrics->otmTextMetrics.tmHeight - _outline_metrics->otmTextMetrics.tmInternalLeading;
	const LONG cell_height = _outline_metrics->otmTextMetrics.tmHeight;
	const LONG cell_ascent = _outline_metrics->otmTextMetrics.tmAscent;
	const LONG cell_descent = _outline_metrics->otmTextMetrics.tmDescent;

	LOGFONT actual_lf = _font_attr;
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
		(FLOAT) em_height,
		L"",
		&dw_text_format);
	assert(hr == S_OK);
	
	CComPtr<IDWriteTextLayout> dw_text_layout;
	hr = _dw_factory->CreateTextLayout(lpString,
		c,
		dw_text_format,
		(FLOAT) _bmp_info.bmiHeader.biWidth,
		(FLOAT) cell_height,
		&dw_text_layout);
	assert(hr == S_OK);

	DWRITE_TEXT_METRICS text_metrics;
	hr = dw_text_layout->GetMetrics(&text_metrics);
	if (hr != S_OK)
		return false;
	assert(hr == S_OK);
	const UINT32 cell_width = (UINT32) ceil(text_metrics.width);

	const POINT baseline = get_baseline(_text_alignment,
		x,
		y,
		cell_width,
		cell_ascent,
		cell_descent);

	RECT bmp_rect = {baseline.x,
		baseline.y - cell_ascent,
		baseline.x + cell_width,
		baseline.y + cell_descent};

	CComPtr<IDWriteBitmapRenderTarget> dw_render_target;
	hr = _dw_gdi_interop->CreateBitmapRenderTarget(_hdc_text, cell_width, cell_height, &dw_render_target);
	assert(hr == S_OK);
	HDC hdc_canvas = dw_render_target->GetMemoryDC();

	const COLORREF bg_color = GetBkColor(_hdc_text);

	if (options & ETO_OPAQUE)
		draw_background(_hdc_text, lprect, bg_color);

	const int bk_mode = GetBkMode(_hdc_text);
	if (bk_mode == OPAQUE)
	{
		const RECT bk_rect = {0, 0, cell_width, cell_height};
		b_ret = draw_background(hdc_canvas, &bk_rect, bg_color);
	}
	else if (bk_mode == TRANSPARENT)
	{
		b_ret = BitBlt(hdc_canvas, 0, 0, cell_width, cell_height, _hdc_text, bmp_rect.left, bmp_rect.top, SRCCOPY);
	}
	else
		b_ret = FALSE;

	if (!b_ret)
		return false;

	gdimm_dw_renderer::drawing_context context = {dw_render_target,
		_measuring_mode,
		_text_color,
		(_advances.empty() ? NULL : &_advances[0])};

	hr = dw_text_layout->Draw(&context, _dw_renderer, 0, 0);
	assert(hr == S_OK);

	b_ret = BitBlt(_hdc_text, bmp_rect.left, bmp_rect.top, cell_width, cell_height, hdc_canvas, 0, 0, SRCCOPY);
	assert(hr == S_OK);

	return true;
}

bool gdimm_dw_text::begin(HDC hdc, const OUTLINETEXTMETRICW *outline_metrics, const wchar_t *font_face, const font_setting_cache *setting_cache)
{
	if (!gdimm_text::begin(hdc, outline_metrics, font_face, setting_cache))
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

	bool is_mono = false;
	if (is_non_aa(_font_attr.lfQuality, _setting_cache) || 
		(_bmp_info.bmiHeader.biBitCount == 1))
		is_mono = true;

	if (is_mono && !_setting_cache->render_mono)
		return false;

	_measuring_mode = DWRITE_MEASURING_MODE_NATURAL;//_setting_cache->hinting ? DWRITE_MEASURING_MODE_NATURAL : DWRITE_MEASURING_MODE_NATURAL;
	_advances.clear();

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
		return draw_glyph(x, y, options, lprect, lpString, c);
	else
		return draw_text(x, y, options, lprect, lpString, c);
}