#include "stdafx.h"
#include "d2d_text.h"
#include "text_helper.h"
#include "gdimm.h"

gdimm_d2d_text::gdimm_d2d_text()
:
_d2d1_factory(NULL),
_dc_render_target(NULL),
_d2d1_brush(NULL),
_dw_factory(NULL),
_dw_gdi_interop(NULL)
{
}

void gdimm_d2d_text::get_text_geometry(int x, int y, int width, int height, int ascent, D2D1_POINT_2F &origin, RECT &text_rect) const
{
	origin = D2D1::Point2F(0, ascent);

	text_rect.left = x;
	text_rect.top = y;

	switch ((TA_LEFT | TA_RIGHT | TA_CENTER) & _text_alignment)
	{
	case TA_LEFT:
		break;
	case TA_RIGHT:
		text_rect.left -= width;
		break;
	case TA_CENTER:
		text_rect.left -= width / 2;
		break;
	}

	text_rect.right = text_rect.left + width;

	switch ((TA_TOP | TA_BOTTOM | TA_BASELINE) & _text_alignment)
	{
	case TA_TOP:
		break;
	case TA_BOTTOM:
		text_rect.top -= height;
		break;
	case TA_BASELINE:
		text_rect.top -= ascent;
		break;
	}

	text_rect.bottom = text_rect.top + height;
}

SIZE gdimm_d2d_text::get_glyph_run_bbox(IDWriteFontFace *dw_font_face,
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

void gdimm_d2d_text::get_float_advances(UINT options, UINT c, CONST INT *lpDx, FLOAT *out_advances)
{
	const int advance_factor = ((options & ETO_PDY) ? 2 : 1);

	for (UINT i = 0; i < c; i++)
		out_advances[i] = (FLOAT) lpDx[i * advance_factor];
}

bool gdimm_d2d_text::draw_glyph(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	HRESULT hr;

	CComPtr<IDWriteFontFace> dw_font_face;
	hr = _dw_gdi_interop->CreateFontFaceFromHdc(_context->hdc, &dw_font_face);
	assert(hr == S_OK);

	DWRITE_GLYPH_METRICS *glyph_metrics = new DWRITE_GLYPH_METRICS[c];
	hr = dw_font_face->GetDesignGlyphMetrics((UINT16 *)lpString, c, glyph_metrics);
	assert(hr == S_OK);

	const LONG point_size = _context->outline_metrics->otmTextMetrics.tmHeight - _context->outline_metrics->otmTextMetrics.tmInternalLeading;
	SIZE bbox = get_glyph_run_bbox(dw_font_face, glyph_metrics, c, point_size);
	bbox.cy = max(bbox.cy, _context->outline_metrics->otmTextMetrics.tmHeight);

	DWRITE_GLYPH_RUN glyph_run = {};
	glyph_run.fontFace = dw_font_face;
	glyph_run.fontEmSize = (FLOAT) point_size;
	glyph_run.glyphCount = c;
	glyph_run.glyphIndices = (UINT16 *)lpString;

	vector<FLOAT> glyph_advances;
	if (lpDx != NULL)
	{
		glyph_advances.resize(c);
		get_float_advances(options, c, lpDx, &glyph_advances[0]);
		glyph_run.glyphAdvances = &glyph_advances[0];
	}

	D2D1_POINT_2F dest_origin;
	RECT text_rect;
	get_text_geometry(x,
		y,
		bbox.cx, 
		_context->outline_metrics->otmTextMetrics.tmHeight,
		_context->outline_metrics->otmTextMetrics.tmAscent,
		dest_origin,
		text_rect);

	const RECT dc_rect = {0, 0, _dc_bmp_header.biWidth, _dc_bmp_header.biHeight};
	hr = _dc_render_target->BindDC(_context->hdc, &dc_rect);
	assert(hr == S_OK);

	_dc_render_target->BeginDraw();
	_dc_render_target->DrawGlyphRun(dest_origin, &glyph_run, _d2d1_brush);
	hr = _dc_render_target->EndDraw();
	assert(hr == S_OK);

	return true;
}

bool gdimm_d2d_text::draw_text(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	HRESULT hr;
	
	CComPtr<IDWriteFont> dw_font;
	wcsncpy_s(_context->font_attr.lfFaceName, metric_family_name(_context->outline_metrics), LF_FACESIZE);
	hr = _dw_gdi_interop->CreateFontFromLOGFONT(&_context->font_attr, &dw_font);
	assert(hr == S_OK);
	
	CComPtr<IDWriteTextFormat> dw_text_format;
	hr = _dw_factory->CreateTextFormat(metric_family_name(_context->outline_metrics),
		NULL,
		dw_font->GetWeight(),
		dw_font->GetStyle(),
		dw_font->GetStretch(),
		(FLOAT)(_context->outline_metrics->otmTextMetrics.tmHeight - _context->outline_metrics->otmTextMetrics.tmInternalLeading),
		L"",
		&dw_text_format);
	assert(hr == S_OK);
	
	CComPtr<IDWriteTextLayout> dw_text_layout;
	hr = _dw_factory->CreateTextLayout(
		lpString,
		c,
		dw_text_format,
		(FLOAT) _dc_bmp_header.biWidth,
		(FLOAT) _dc_bmp_header.biHeight,
		&dw_text_layout);
	assert(hr == S_OK);
	

	DWRITE_TEXT_METRICS text_metrics;
	hr = dw_text_layout->GetMetrics(&text_metrics);
	assert(hr == S_OK);

	const UINT32 text_width = (UINT32) ceil(text_metrics.width);
	const UINT32 text_height = (UINT32) ceil(text_metrics.height);

	D2D1_POINT_2F dest_origin;
	RECT r = {0, 0, _dc_bmp_header.biWidth, _dc_bmp_header.biHeight};
	RECT dc_rect;
	get_text_geometry(x,
		y,
		text_width, 
		text_height,
		_context->outline_metrics->otmTextMetrics.tmAscent,
		dest_origin,
		dc_rect);

	dest_origin.x += dc_rect.left;
	dest_origin.y += dc_rect.top - _context->outline_metrics->otmTextMetrics.tmAscent;

	if (_context->hdc != _last_hdc)
	{
		hr = _dc_render_target->BindDC(_context->hdc, &r);
		assert(hr == S_OK);
	}

	_dc_render_target->BeginDraw();
	_dc_render_target->DrawTextLayout(dest_origin, dw_text_layout, _d2d1_brush);
	hr = _dc_render_target->EndDraw();
	assert(hr == S_OK);

	return true;
}

bool gdimm_d2d_text::begin(const gdimm_text_context *context)
{
	if (!gdimm_text::begin(context))
		return false;

	HRESULT hr;

	if (_d2d1_factory == NULL)
	{
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &_d2d1_factory);
		assert(hr == S_OK);
	}

	if (_dc_render_target == NULL)
	{
		const D2D1_RENDER_TARGET_PROPERTIES target_prop = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			0,
			0,
			D2D1_RENDER_TARGET_USAGE_NONE,
			D2D1_FEATURE_LEVEL_DEFAULT);
		hr = _d2d1_factory->CreateDCRenderTarget(&target_prop, &_dc_render_target);
		assert(hr == S_OK);
	}

	if (_d2d1_brush == NULL)
	{
		hr = _dc_render_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &_d2d1_brush);
		assert(hr == S_OK);
	}

	if (_dw_factory == NULL)
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **)&_dw_factory);
		assert(hr == S_OK);

		if (_dw_gdi_interop == NULL)
		{
			hr = _dw_factory->GetGdiInterop(&_dw_gdi_interop);
			assert(hr == S_OK);
		}
	}

	_d2d1_brush->SetColor(D2D1::ColorF(_text_color));

	return true;
}

bool gdimm_d2d_text::text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	bool b_ret;
	if (options & ETO_GLYPH_INDEX)
		return true;//draw_glyph(x, y, options, lprect, lpString, c, lpDx);
	else
		b_ret = draw_text(x, y, options, lprect, lpString, c, lpDx);

	_last_hdc = _context->hdc;

	return b_ret;
}