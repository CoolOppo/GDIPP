#include "stdafx.h"
#include "wic_text.h"
#include "text_helper.h"
#include "gdimm.h"

gdimm_wic_text::gdimm_wic_text()
:
_d2d_factory(NULL),
_wic_render_target(NULL),
_dw_factory(NULL)
{
}

bool gdimm_wic_text::prepare(LPCWSTR lpString, UINT c, text_metrics &metrics, IDWriteFontFace **dw_font_face, DWRITE_GLYPH_RUN &dw_glyph_run)
{
	HRESULT hr;

	hr = _dw_gdi_interop->CreateFontFaceFromHdc(_context->hdc, dw_font_face);
	assert(hr == S_OK);

	vector<DWRITE_GLYPH_METRICS> glyph_metrics(c);
	hr = (*dw_font_face)->GetDesignGlyphMetrics((UINT16 *)lpString, c, &glyph_metrics[0]);
	assert(hr == S_OK);

	UINT32 glyph_run_width = 0;
	for (UINT i = 0; i < c; i++)
		glyph_run_width += glyph_metrics[i].advanceWidth;
	
	glyph_run_width = MulDiv(glyph_run_width, _em_size, _context->outline_metrics->otmEMSquare);
	metrics.width = max(metrics.width, glyph_run_width);

	dw_glyph_run.fontFace = *dw_font_face;
	dw_glyph_run.fontEmSize = (FLOAT) _em_size;
	dw_glyph_run.glyphCount = c;
	dw_glyph_run.glyphIndices = (UINT16 *)lpString;
	dw_glyph_run.glyphAdvances = &_advances[0];
	dw_glyph_run.glyphOffsets = NULL;
	dw_glyph_run.isSideways = FALSE;
	dw_glyph_run.bidiLevel = 0;

	return true;
}

bool gdimm_wic_text::prepare(LPCWSTR lpString, UINT c, text_metrics &metrics, IDWriteTextFormat **dw_text_format, IDWriteTextLayout **dw_text_layout)
{
	HRESULT hr;

	LOGFONTW actual_lf = _font_attr;
	wcsncpy_s(actual_lf.lfFaceName, metric_family_name(_context->outline_metrics), LF_FACESIZE);

	CComPtr<IDWriteFont> dw_font;
	hr = _dw_gdi_interop->CreateFontFromLOGFONT(&actual_lf, &dw_font);
	assert(hr == S_OK);

	hr = _dw_factory->CreateTextFormat(metric_family_name(_context->outline_metrics),
		NULL,
		dw_font->GetWeight(),
		dw_font->GetStyle(),
		dw_font->GetStretch(),
		(FLOAT) _em_size,
		L"",
		dw_text_format);
	assert(hr == S_OK);

	if (_measuring_mode == DWRITE_MEASURING_MODE_NATURAL)
	{
		hr = _dw_factory->CreateTextLayout(lpString,
			c,
			*dw_text_format,
			(FLOAT) _dc_bmp_header.biWidth,
			0,
			dw_text_layout);
	}
	else
	{
		hr = _dw_factory->CreateGdiCompatibleTextLayout(lpString,
			c,
			*dw_text_format,
			(FLOAT) _dc_bmp_header.biWidth,
			0,
			GetDeviceCaps(_context->hdc, LOGPIXELSY) / 96.0f,
			NULL,
			true,
			dw_text_layout);
	}
	assert(hr == S_OK);

	DWRITE_TEXT_METRICS text_metrics;
	hr = (*dw_text_layout)->GetMetrics(&text_metrics);
	assert(hr == S_OK);
	metrics.width = max(metrics.width, (UINT32) ceil(text_metrics.width));

	return true;
}

bool gdimm_wic_text::draw_text(UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	HRESULT hr;
	BOOL b_ret, draw_success;

	text_metrics metrics = {};
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
	CComPtr<IDWriteTextFormat> dw_text_format;
	CComPtr<IDWriteTextLayout> dw_text_layout;

	if (options & ETO_GLYPH_INDEX)
		b_ret = prepare(lpString, c, metrics, &dw_font_face, dw_glyph_run);
	else
		b_ret = prepare(lpString, c, metrics, &dw_text_format, &dw_text_layout);
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

	BITMAPINFOHEADER bmp_header = {};
	bmp_header.biSize = sizeof(BITMAPINFOHEADER);
	bmp_header.biWidth = metrics.width;
	bmp_header.biHeight = -metrics.height;
	bmp_header.biPlanes = 1;
	bmp_header.biBitCount = 32;
	bmp_header.biCompression = BI_RGB;

	BYTE *text_bits;
	HBITMAP text_bitmap = CreateDIBSection(_context->hdc, (BITMAPINFO *)&bmp_header, DIB_RGB_COLORS, (VOID* *)&text_bits, NULL, 0);
	assert(text_bitmap != NULL);

	HDC hdc_canvas = CreateCompatibleDC(_context->hdc);
	assert(hdc_canvas != NULL);
	SelectObject(hdc_canvas, text_bitmap);

	_wic_bitmap.initialize(&bmp_header, text_bits);

	CComPtr<ID2D1RenderTarget> wic_render_target;
	hr = _d2d_factory->CreateWicBitmapRenderTarget(&_wic_bitmap, D2D1::RenderTargetProperties(), &wic_render_target);
	assert(hr == S_OK);

	CComPtr<IDWriteRenderingParams> dw_render_params;
	hr = _dw_factory->CreateCustomRenderingParams(1.0f, 0.0f, 1.0f, DWRITE_PIXEL_GEOMETRY_RGB, DWRITE_RENDERING_MODE_DEFAULT, &dw_render_params);
	assert(hr == S_OK);
	wic_render_target->SetTextRenderingParams(dw_render_params);

	wic_render_target->BeginDraw();
	
	if (options & ETO_CLIPPED)
	{
		wic_render_target->PushAxisAlignedClip(D2D1::RectF((FLOAT) lprect->left,
			(FLOAT) lprect->top,
			(FLOAT) lprect->right,
			(FLOAT) lprect->bottom),
			D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	}

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
		draw_success = BitBlt(hdc_canvas,
			0,
			0,
			metrics.width,
			metrics.height,
			_context->hdc,
			metrics.origin.x,
			metrics.origin.y,
			SRCCOPY);
	}
	draw_success = TRUE;
	if (draw_success)
	{
		CComPtr<ID2D1SolidColorBrush> text_brush;
		hr = wic_render_target->CreateSolidColorBrush(D2D1::ColorF(_text_color), &text_brush);
		assert(hr == S_OK);

		if (options & ETO_GLYPH_INDEX)
			wic_render_target->DrawGlyphRun(D2D1::Point2F(0, (FLOAT) metrics.ascent), &dw_glyph_run, text_brush);
		else
			wic_render_target->DrawTextLayout(D2D1::Point2F(), dw_text_layout, text_brush);
	}

	if (options & ETO_CLIPPED)
		wic_render_target->PopAxisAlignedClip();
	
	hr = wic_render_target->EndDraw();
	
	draw_success = (hr == S_OK);
	if (draw_success)
	{
		draw_success = BitBlt(_context->hdc,
			metrics.origin.x,
			metrics.origin.y,
			metrics.width,
			metrics.height,
			hdc_canvas,
			0,
			0,
			SRCCOPY);
	}

	b_ret = DeleteObject(text_bitmap);
	assert(b_ret);
	b_ret = DeleteDC(hdc_canvas);
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

	_char_extra = GetTextCharacterExtra(_context->hdc);
	assert(_char_extra != 0x8000000);

	if (_d2d_factory == NULL)
	{
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &_d2d_factory);
		assert(hr == S_OK);
	}

	if (_dw_factory == NULL)
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **) &_dw_factory);
		assert(hr == S_OK);
	}

	if (_dw_gdi_interop == NULL)
	{
		hr = _dw_factory->GetGdiInterop(&_dw_gdi_interop);
		assert(hr == S_OK);
	}

	_em_size = _context->outline_metrics->otmTextMetrics.tmHeight - _context->outline_metrics->otmTextMetrics.tmInternalLeading;
	_bg_color = GetBkColor(_context->hdc);

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