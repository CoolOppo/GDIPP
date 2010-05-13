#include "stdafx.h"
#include "d2d_text.h"
#include "text_helper.h"
#include "gdimm.h"

CComPtr<ID2D1Factory> gdimm_d2d_text::_d2d1_factory = NULL;
CComPtr<ID2D1DCRenderTarget > gdimm_d2d_text::_d2d1_render_target = NULL;

CComPtr<IDWriteFactory> gdimm_d2d_text::_dw_factory = NULL;
CComPtr<IDWriteGdiInterop> gdimm_d2d_text::_dw_gdi_interop = NULL;

gdimm_d2d_text::gdimm_d2d_text(HDC hdc)
:
gdimm_text(hdc)
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
	hr = _dw_gdi_interop->CreateFontFaceFromHdc(_hdc_text, &dw_font_face);
	assert(hr == S_OK);

	DWRITE_GLYPH_METRICS *glyph_metrics = new DWRITE_GLYPH_METRICS[c];
	hr = dw_font_face->GetDesignGlyphMetrics((UINT16*) lpString, c, glyph_metrics);
	assert(hr == S_OK);

	const LONG point_size = _outline_metrics->otmTextMetrics.tmHeight - _outline_metrics->otmTextMetrics.tmInternalLeading;
	SIZE bbox = get_glyph_run_bbox(dw_font_face, glyph_metrics, c, point_size);
	bbox.cy = max(bbox.cy, _outline_metrics->otmTextMetrics.tmHeight);

	DWRITE_GLYPH_RUN glyph_run = {};
	glyph_run.fontFace = dw_font_face;
	glyph_run.fontEmSize = (FLOAT) point_size;
	glyph_run.glyphCount = c;
	glyph_run.glyphIndices = (UINT16*) lpString;

	vector<FLOAT> glyph_advances;
	if (lpDx != NULL)
	{
		glyph_advances.resize(c);
		get_float_advances(options, c, lpDx, &glyph_advances[0]);
		glyph_run.glyphAdvances = &glyph_advances[0];
	}

	D2D1_POINT_2F dest_origin;
	RECT dc_rect;
	get_text_geometry(x,
		y,
		bbox.cx, 
		_outline_metrics->otmTextMetrics.tmHeight,
		_outline_metrics->otmTextMetrics.tmAscent,
		dest_origin,
		dc_rect);

	CComPtr<ID2D1SolidColorBrush> fg_brush;
	hr = _d2d1_render_target->CreateSolidColorBrush(_fg_color, &fg_brush);
	assert(hr == S_OK);

	hr = _d2d1_render_target->BindDC(_hdc_text, &dc_rect);
	assert(hr == S_OK);

	_d2d1_render_target->BeginDraw();
	_d2d1_render_target->DrawGlyphRun(dest_origin, &glyph_run, fg_brush);
	hr = _d2d1_render_target->EndDraw();
	assert(hr == S_OK);

	return true;
}

bool gdimm_d2d_text::draw_text(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	HRESULT hr;
	
	CComPtr<IDWriteFont> dw_font;
	wcsncpy_s(_font_attr.lfFaceName, metric_family_name(_outline_metrics), LF_FACESIZE);
	hr = _dw_gdi_interop->CreateFontFromLOGFONT(&_font_attr, &dw_font);
	assert(hr == S_OK);
	
	CComPtr<IDWriteTextFormat> dw_text_format;
	hr = _dw_factory->CreateTextFormat(metric_family_name(_outline_metrics),
		NULL,
		dw_font->GetWeight(),
		dw_font->GetStyle(),
		dw_font->GetStretch(),
		(FLOAT)(_outline_metrics->otmTextMetrics.tmHeight - _outline_metrics->otmTextMetrics.tmInternalLeading),
		L"",
		&dw_text_format);
	assert(hr == S_OK);
	
	CComPtr<IDWriteTextLayout> dw_text_layout;
	hr = _dw_factory->CreateTextLayout(
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

	D2D1_POINT_2F dest_origin;
	RECT dc_rect;
	get_text_geometry(x,
		y,
		text_width, 
		text_height,
		_outline_metrics->otmTextMetrics.tmAscent,
		dest_origin,
		dc_rect);
	const D2D1_RECT_F d2d1_rect = D2D1::RectF(0,
		0,
		text_width,
		text_height);

	CComPtr<ID2D1SolidColorBrush> fg_brush;
	hr = _d2d1_render_target->CreateSolidColorBrush(_fg_color, &fg_brush);
	assert(hr == S_OK);

	hr = _d2d1_render_target->BindDC(_hdc_text, &dc_rect);
	assert(hr == S_OK);

	_d2d1_render_target->BeginDraw();
	_d2d1_render_target->DrawText(lpString, c, dw_text_format, d2d1_rect, fg_brush, D2D1_DRAW_TEXT_OPTIONS_NONE, DWRITE_MEASURING_MODE_GDI_NATURAL);
	hr = _d2d1_render_target->EndDraw();
	assert(hr == S_OK);

	return true;
}

bool gdimm_d2d_text::init()
{
	if (!gdimm_text::init())
		return false;

	HRESULT hr;

	if (_d2d1_factory == NULL)
	{
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &_d2d1_factory);
		assert(hr == S_OK);

		if (_d2d1_render_target == NULL)
		{
			const D2D1_RENDER_TARGET_PROPERTIES target_prop = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
				0,
				0,
				D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE,
				D2D1_FEATURE_LEVEL_DEFAULT);
			hr = _d2d1_factory->CreateDCRenderTarget(&target_prop, &_d2d1_render_target);
			assert(hr == S_OK);
		}
	}

	if (_dw_factory == NULL)
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**) &_dw_factory);
		assert(hr == S_OK);

		if (_dw_gdi_interop == NULL)
		{
			hr = _dw_factory->GetGdiInterop(&_dw_gdi_interop);
			assert(hr == S_OK);
		}
	}

	_fg_color = D2D1::ColorF((FLOAT) GetRValue(_text_color) / 255,
		(FLOAT) GetGValue(_text_color) / 255,
		(FLOAT) GetBValue(_text_color) / 255);

	return true;
}

bool gdimm_d2d_text::text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	if (options & ETO_GLYPH_INDEX)
		return draw_glyph(x, y, options, lprect, lpString, c, lpDx);
	else
		return draw_text(x, y, options, lprect, lpString, c, lpDx);
}