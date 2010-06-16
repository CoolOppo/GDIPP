#include "stdafx.h"
#include "wic_text.h"
#include "text_helper.h"
#include "gdimm.h"

gdimm_wic_text::gdimm_wic_text()
:
_d2d_factory(NULL),
_wic_render_target(NULL),
_dw_factory(NULL),
_dw_gdi_interop(NULL),
_hdc_canvas(NULL)
{
}

bool gdimm_wic_text::prepare(LPCWSTR lpString, UINT c, text_metrics &metrics, IDWriteFontFace **dw_font_face, DWRITE_GLYPH_RUN &dw_glyph_run)
{
	HRESULT hr;
	BOOL b_ret;

	hr = _dw_gdi_interop->CreateFontFaceFromHdc(_context->hdc, dw_font_face);
	assert(hr == S_OK);

	dw_glyph_run.fontFace = *dw_font_face;
	dw_glyph_run.fontEmSize = _em_size;
	dw_glyph_run.glyphCount = c;
	dw_glyph_run.glyphIndices = (UINT16 *)lpString;
	dw_glyph_run.glyphAdvances = (_advances.empty() ? NULL : &_advances[0]);
	dw_glyph_run.glyphOffsets = NULL;
	dw_glyph_run.isSideways = FALSE;
	dw_glyph_run.bidiLevel = 0;

	/*vector<DWRITE_GLYPH_METRICS> glyph_metrics(c);
	if (_dw_measuring_mode == DWRITE_MEASURING_MODE_NATURAL)
		hr = (*dw_font_face)->GetDesignGlyphMetrics((UINT16 *)lpString, c, &glyph_metrics[0]);
	else
		hr = (*dw_font_face)->GetGdiCompatibleGlyphMetrics(_em_size,
			_pixels_per_dip,
			NULL,
			_use_gdi_natural,
			(UINT16 *)lpString,
			c,
			&glyph_metrics[0]);
	assert(hr == S_OK);

	UINT32 glyph_run_width = 0;
	for (UINT i = 0; i < c; i++)
		glyph_run_width += glyph_metrics[i].advanceWidth;
	metrics.width = max(metrics.width, glyph_run_width * _em_size / _context->outline_metrics->otmEMSquare);*/

	// more accurate width than DirectWrite functions
	SIZE text_extent;
	b_ret = GetTextExtentPointI(_context->hdc, (LPWORD) lpString, c, &text_extent);
	assert(b_ret);
	metrics.width = max(metrics.width, (UINT32) text_extent.cx);

	return true;
}

bool gdimm_wic_text::prepare(LPCWSTR lpString, UINT c, text_metrics &metrics, IDWriteTextLayout **dw_text_layout)
{
	HRESULT hr;
	bool b_ret;

	gdimm_os2_metrics os2_metrics;
	b_ret = os2_metrics.init(_context->hdc);
	assert(b_ret);

	DWRITE_FONT_STYLE dw_font_style;
	if (!_context->outline_metrics->otmTextMetrics.tmItalic)
		dw_font_style = DWRITE_FONT_STYLE_NORMAL;
	else if (os2_metrics.is_italic())
		dw_font_style = DWRITE_FONT_STYLE_ITALIC;
	else
		dw_font_style = DWRITE_FONT_STYLE_OBLIQUE;

	CComPtr<IDWriteTextFormat> dw_text_format;
	hr = _dw_factory->CreateTextFormat(_context->font_family,
		NULL,
		(DWRITE_FONT_WEIGHT) _context->outline_metrics->otmTextMetrics.tmWeight,
		dw_font_style,
		(DWRITE_FONT_STRETCH) os2_metrics.get_usWidthClass(),
		_em_size,
		L"",
		&dw_text_format);
	assert(hr == S_OK);

	hr = dw_text_format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	assert(hr == S_OK);

	if (_dw_measuring_mode == DWRITE_MEASURING_MODE_NATURAL)
	{
		hr = _dw_factory->CreateTextLayout(lpString,
			c,
			dw_text_format,
			(FLOAT) _dc_bmp_header.biWidth,
			0,
			dw_text_layout);
	}
	else
	{
		hr = _dw_factory->CreateGdiCompatibleTextLayout(lpString,
			c,
			dw_text_format,
			(FLOAT) _dc_bmp_header.biWidth,
			0,
			_pixels_per_dip,
			NULL,
			_use_gdi_natural,
			dw_text_layout);
	}
	assert(hr == S_OK);

// 	// more accurate width than DirectWrite functions
// 	SIZE text_extent;
// 	b_ret = GetTextExtentPoint32W(_context->hdc, lpString, c, &text_extent);
// 	assert(b_ret);
// 	metrics.width = max(metrics.width, (UINT32) text_extent.cx);

	DWRITE_TEXT_METRICS text_metrics;
	hr = (*dw_text_layout)->GetMetrics(&text_metrics);
	assert(hr == S_OK);
	metrics.width = max(metrics.width, (UINT32) ceil(text_metrics.width));

	return true;
}

void gdimm_wic_text::set_param(ID2D1RenderTarget *render_target)
{
	HRESULT hr;

	DWRITE_RENDERING_MODE dw_render_mode;
	if (_render_mode == FT_RENDER_MODE_MONO)
		dw_render_mode = DWRITE_RENDERING_MODE_ALIASED;
	else
	{
		switch (_context->setting_cache->hinting)
		{
		case 0:
			dw_render_mode = DWRITE_RENDERING_MODE_DEFAULT;
			break;
		case 1:
			dw_render_mode = DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL_SYMMETRIC;
			break;
		case 2:
			dw_render_mode = DWRITE_RENDERING_MODE_CLEARTYPE_GDI_NATURAL;
			break;
		default:
			dw_render_mode = DWRITE_RENDERING_MODE_CLEARTYPE_GDI_CLASSIC;
			break;
		}
	}

	D2D1_TEXT_ANTIALIAS_MODE text_aa_mode;
	switch (_render_mode)
	{
	case FT_RENDER_MODE_NORMAL:
	case FT_RENDER_MODE_LIGHT:
		text_aa_mode = D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE;
		break;
	case FT_RENDER_MODE_MONO:
		text_aa_mode = D2D1_TEXT_ANTIALIAS_MODE_ALIASED;
		break;
	default:
		text_aa_mode = D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE;
		break;
	}

	DWRITE_PIXEL_GEOMETRY pixel_geometry;
	switch (_context->setting_cache->render_mode.pixel_geometry)
	{
	case PIXEL_GEOMETRY_BGR:
		pixel_geometry = DWRITE_PIXEL_GEOMETRY_BGR;
	default:
		pixel_geometry = DWRITE_PIXEL_GEOMETRY_RGB;
	}

	// use average gamma
	const FLOAT avg_gamma = (FLOAT)(_context->setting_cache->gamma.red + _context->setting_cache->gamma.green + _context->setting_cache->gamma.blue) / 3;

	CComPtr<IDWriteRenderingParams> dw_render_params;
	hr = _dw_factory->CreateCustomRenderingParams(avg_gamma, 0.0f, 1.0f, pixel_geometry, dw_render_mode, &dw_render_params);
	assert(hr == S_OK);

	render_target->SetTextRenderingParams(dw_render_params);
	render_target->SetTextAntialiasMode(text_aa_mode);
}

bool gdimm_wic_text::draw_text(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	HRESULT hr;
	BOOL b_ret, draw_success;

	text_metrics metrics = {};
	POINT canvas_origin = {};

	metrics.height = _context->outline_metrics->otmTextMetrics.tmHeight;
	metrics.ascent = _context->outline_metrics->otmTextMetrics.tmAscent;
	metrics.descent = _context->outline_metrics->otmTextMetrics.tmDescent;

	const int advance_factor = ((options & ETO_PDY) ? 2 : 1);
	if (lpDx != NULL)
	{
		_advances.resize(c);
		for (UINT i = 0; i < c; i++)
		{
			_advances[i] = (FLOAT) lpDx[i * advance_factor];
			metrics.width += lpDx[i * advance_factor];
		}
	}

	CComPtr<IDWriteFontFace> dw_font_face;
	DWRITE_GLYPH_RUN dw_glyph_run;
	CComPtr<IDWriteTextLayout> dw_text_layout;

	if (options & ETO_GLYPH_INDEX)
		b_ret = prepare(lpString, c, metrics, &dw_font_face, dw_glyph_run);
	else
		b_ret = prepare(lpString, c, metrics, &dw_text_layout);
	assert(b_ret);

	metrics.baseline = get_baseline(_text_alignment,
		_cursor.x,
		_cursor.y,
		metrics.width,
		metrics.ascent,
		metrics.descent);

	metrics.origin.x = metrics.baseline.x;
	metrics.origin.y = metrics.baseline.y - metrics.ascent;

	_cursor.x += metrics.width;

	// calculate metrics after clipping
	if (options & ETO_CLIPPED)
	{
		RECT bmp_rect = {metrics.origin.x,
			metrics.origin.y,
			metrics.origin.x + metrics.width,
			metrics.origin.y + metrics.height};
		if (!IntersectRect(&bmp_rect, &bmp_rect, lprect))
			bmp_rect = *lprect;

		metrics.width = bmp_rect.right - bmp_rect.left;
		metrics.height = bmp_rect.bottom - bmp_rect.top;
		canvas_origin.x = metrics.origin.x - bmp_rect.left;
		canvas_origin.y = metrics.origin.y - bmp_rect.top;
		metrics.origin.x = bmp_rect.left;
		metrics.origin.y = bmp_rect.top;
	}

	BITMAPINFOHEADER bmp_header = {};
	bmp_header.biSize = sizeof(BITMAPINFOHEADER);
	bmp_header.biWidth = metrics.width;
	bmp_header.biHeight = -metrics.height;
	bmp_header.biPlanes = 1;
	bmp_header.biBitCount = _dc_bmp_header.biBitCount;
	bmp_header.biCompression = BI_RGB;

	BYTE *text_bits;
	HBITMAP text_bitmap = CreateDIBSection(_context->hdc, (BITMAPINFO *)&bmp_header, DIB_RGB_COLORS, (VOID **)&text_bits, NULL, 0);
	assert(text_bitmap != NULL);
	SelectObject(_hdc_canvas, text_bitmap);

	_wic_bitmap.initialize(&bmp_header, text_bits);

	CComPtr<ID2D1RenderTarget> wic_render_target;
	hr = _d2d_factory->CreateWicBitmapRenderTarget(&_wic_bitmap, D2D1::RenderTargetProperties(), &wic_render_target);
	assert(hr == S_OK);

	set_param(wic_render_target);

	wic_render_target->BeginDraw();

	if (options & ETO_OPAQUE)
		draw_background(_context->hdc, lprect, _bg_color);

	const int bk_mode = GetBkMode(_context->hdc);
	if (bk_mode == OPAQUE)
	{
		wic_render_target->Clear(D2D1::ColorF(_bg_color));
		draw_success = TRUE;
	}
	else if (bk_mode == TRANSPARENT)
	{
		// "If a rotation or shear transformation is in effect in the source device context, BitBlt returns an error"
		draw_success = BitBlt(_hdc_canvas,
			0,
			0,
			metrics.width,
			metrics.height,
			_context->hdc,
			metrics.origin.x,
			metrics.origin.y,
			SRCCOPY);
	}
	
	if (draw_success)
	{
		CComPtr<ID2D1SolidColorBrush> text_brush;
		hr = wic_render_target->CreateSolidColorBrush(D2D1::ColorF(_text_color), &text_brush);
		assert(hr == S_OK);

		if (options & ETO_GLYPH_INDEX)
			wic_render_target->DrawGlyphRun(D2D1::Point2F((FLOAT) canvas_origin.x, (FLOAT)(canvas_origin.y + metrics.ascent)), &dw_glyph_run, text_brush, _dw_measuring_mode);
		else
			wic_render_target->DrawTextLayout(D2D1::Point2F((FLOAT) canvas_origin.x, (FLOAT) canvas_origin.y), dw_text_layout, text_brush);
	}
	
	hr = wic_render_target->EndDraw();
	
	draw_success = (hr == S_OK);
	if (draw_success)
	{
		draw_success = BitBlt(_context->hdc,
			metrics.origin.x,
			metrics.origin.y,
			metrics.width,
			metrics.height,
			_hdc_canvas,
			0,
			0,
			SRCCOPY);
	}

	b_ret = DeleteObject(text_bitmap);
	assert(b_ret);

	return !!draw_success;
}

bool gdimm_wic_text::begin(const gdimm_text_context *context)
{
	HRESULT hr;

	if (!gdimm_text::begin(context))
		return false;

	// ignore rotated DC
	if (_font_attr.lfEscapement % 3600 != 0)
		return false;

	if (_d2d_factory == NULL)
	{
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &_d2d_factory);
		assert(hr == S_OK);
	}

	if (_dw_factory == NULL)
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **)(&_dw_factory));
		assert(hr == S_OK);
	}

	if (_dw_gdi_interop == NULL)
	{
		hr = _dw_factory->GetGdiInterop(&_dw_gdi_interop);
		assert(hr == S_OK);
	}

	if (_hdc_canvas == NULL)
	{
		_hdc_canvas = CreateCompatibleDC(NULL);
		assert(_hdc_canvas != NULL);
	}

	switch (_context->setting_cache->hinting)
	{
	case 2:
		_dw_measuring_mode = DWRITE_MEASURING_MODE_GDI_NATURAL;
		break;
	case 3:
		_dw_measuring_mode = DWRITE_MEASURING_MODE_GDI_CLASSIC;
		break;
	default:
		_dw_measuring_mode = DWRITE_MEASURING_MODE_NATURAL;
		break;
	}
	_use_gdi_natural = (_dw_measuring_mode != DWRITE_MEASURING_MODE_GDI_CLASSIC);

	// BGR -> RGB
	_text_color = RGB(GetBValue(_text_color), GetGValue(_text_color), GetRValue(_text_color));
	_bg_color = RGB(GetBValue(_bg_color), GetGValue(_bg_color), GetRValue(_bg_color));

	_advances.clear();
	_em_size = (FLOAT)(_context->outline_metrics->otmTextMetrics.tmHeight - _context->outline_metrics->otmTextMetrics.tmInternalLeading) - 0.3f;	// small adjustment to emulate GDI metrics
	_pixels_per_dip = GetDeviceCaps(_context->hdc, LOGPIXELSY) / 96.0f;

	return true;
}

bool gdimm_wic_text::text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	BOOL b_ret;

	BOOL update_cursor;
	if (((TA_NOUPDATECP | TA_UPDATECP) & _text_alignment) == TA_UPDATECP)
	{
		POINT cp;
		b_ret = GetCurrentPositionEx(_context->hdc, &cp);
		assert(b_ret);

		_cursor.x = cp.x;
		_cursor.y = cp.y;
		update_cursor = true;
	}
	else
	{
		_cursor.x = x;
		_cursor.y = y;
		update_cursor = false;
	}

	const bool draw_success = draw_text(options, lprect, lpString, c, lpDx);

	// if TA_UPDATECP is set, update current position after text out
	if (update_cursor && draw_success)
	{
		b_ret = MoveToEx(_context->hdc, _cursor.x, _cursor.y, NULL);
		assert(b_ret);
	}

	return draw_success;
}