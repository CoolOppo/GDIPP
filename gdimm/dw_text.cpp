#include "stdafx.h"
#include "dw_text.h"
#include "text_helper.h"
#include "gdimm.h"

gdimm_dw_text::gdimm_dw_text()
:
_dw_factory(NULL),
_dw_gdi_interop(NULL),
_dw_render_param(NULL)
{
}

bool gdimm_dw_text::prepare_glyph(LPCWSTR lpString, UINT c,	IDWriteFontFace **dw_font_face)
{
	HRESULT hr;

	hr = _dw_gdi_interop->CreateFontFaceFromHdc(_context->hdc, dw_font_face);
	assert(hr == S_OK);

	vector<DWRITE_GLYPH_METRICS> glyph_metrics;
	glyph_metrics.resize(c);
	hr = (*dw_font_face)->GetDesignGlyphMetrics((UINT16 *)lpString, c, &glyph_metrics[0]);
	assert(hr == S_OK);

	UINT32 glyph_run_width = 0;
	UINT32 glyph_run_height = 0;
	for (UINT i = 0; i < c; i++)
	{
		glyph_run_width += (UINT32) ceil((FLOAT) glyph_metrics[i].advanceWidth * _em_height / _context->outline_metrics->otmEMSquare);
		glyph_run_height = max(glyph_run_height, glyph_metrics[i].advanceHeight);
	}
	glyph_run_height = (UINT32) ceil((FLOAT) glyph_run_height * _em_height / _context->outline_metrics->otmEMSquare);

	_cell_width = max(_cell_width, glyph_run_width);
	_extra_height = max((LONG) glyph_run_height - _cell_height, 0);

	hr = _dw_gdi_interop->CreateBitmapRenderTarget(_context->hdc, _cell_width, _cell_height, &_dw_render_target);
	assert(hr == S_OK);

	return true;
}

bool gdimm_dw_text::prepare_text(LPCWSTR lpString, UINT c, IDWriteTextFormat **dw_text_format, IDWriteTextLayout **dw_text_layout)
{
	HRESULT hr;

	LOGFONT actual_lf = _font_attr;
	wcsncpy_s(actual_lf.lfFaceName, metric_family_name(_context->outline_metrics), LF_FACESIZE);
	
	CComPtr<IDWriteFont> dw_font;
	hr = _dw_gdi_interop->CreateFontFromLOGFONT(&actual_lf, &dw_font);
	assert(hr == S_OK);
	
	hr = _dw_factory->CreateTextFormat(actual_lf.lfFaceName,
		NULL,
		dw_font->GetWeight(),
		dw_font->GetStyle(),
		dw_font->GetStretch(),
		(FLOAT) _em_height,
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
			_pixels_per_dip,
			NULL,
			_use_gdi_natural,
			dw_text_layout);
	}
	assert(hr == S_OK);

	DWRITE_TEXT_METRICS dw_text_metrics;
	hr = (*dw_text_layout)->GetMetrics(&dw_text_metrics);
	if (hr != S_OK)
		return false;
	assert(hr == S_OK);

	_cell_width = max(_cell_width, (UINT32) ceil(dw_text_metrics.width));
	_extra_height = (LONG) max(ceil(dw_text_metrics.height) - _cell_height, 0);

	hr = _dw_gdi_interop->CreateBitmapRenderTarget(_context->hdc, _cell_width, _cell_height, &_dw_render_target);
	assert(hr == S_OK);

	return true;
}

//
// IUnknown methods
//
//      These methods are never called in this scenario so we just use stub
//      implementations.
//
IFACEMETHODIMP gdimm_dw_text::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject
	)
{
	*ppvObject = NULL;
	return E_NOTIMPL;
}

IFACEMETHODIMP_(ULONG) gdimm_dw_text::AddRef( void)
{
	return 0;
}

IFACEMETHODIMP_(ULONG) gdimm_dw_text::Release( void)
{
	return 0;
}

/******************************************************************
*																 *
*  gdimm_dw_text::IsPixelSnappingDisabled					   *
*																 *
*  Determines whether pixel snapping is disabled. The recommended *
*  default is FALSE, unless doing animation that requires		 *
*  subpixel vertical placement.								   *
*																 *
******************************************************************/

IFACEMETHODIMP gdimm_dw_text::IsPixelSnappingDisabled(
	__maybenull void* clientDrawingContext,
	__out BOOL* isDisabled
	)
{
	*isDisabled = FALSE;
	return S_OK;
}

/******************************************************************
*																 *
*  gdimm_dw_text::GetCurrentTransform						   *
*																 *
*  Returns the current transform applied to the render target..   *
*																 *
******************************************************************/

IFACEMETHODIMP gdimm_dw_text::GetCurrentTransform(
	__maybenull void* clientDrawingContext,
	__out DWRITE_MATRIX* transform
	)
{
	_dw_render_target->GetCurrentTransform(transform);
	return S_OK;
}

/******************************************************************
*																 *
*  gdimm_dw_text::GetPixelsPerDip							   *
*																 *
*  This returns the number of pixels per DIP.					 *
*																 *
******************************************************************/

IFACEMETHODIMP gdimm_dw_text::GetPixelsPerDip(
	__maybenull void* clientDrawingContext,
	__out FLOAT* pixelsPerDip
	)
{
	*pixelsPerDip = _dw_render_target->GetPixelsPerDip();
	return S_OK;
}

/******************************************************************
*																 *
*  gdimm_dw_text::DrawGlyphRun								  *
*																 *
*  Gets GlyphRun outlines via IDWriteFontFace::GetGlyphRunOutline *
*  and then draws and fills them using Direct2D path geometries   *
*																 *
******************************************************************/

IFACEMETHODIMP gdimm_dw_text::DrawGlyphRun(
	__maybenull void* clientDrawingContext,
	FLOAT baselineOriginX,
	FLOAT baselineOriginY,
	DWRITE_MEASURING_MODE measuringMode,
	__in DWRITE_GLYPH_RUN const* glyphRun,
	__in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
	__maybenull IUnknown* clientDrawingEffect
	)
{
	if (_advances.empty())
	{
		return _dw_render_target->DrawGlyphRun(baselineOriginX,
			baselineOriginY,
			_measuring_mode,
			glyphRun,
			_dw_render_param,
			_text_color);
	}
	else
	{
		DWRITE_GLYPH_RUN final_glyph_run = *glyphRun;
		final_glyph_run.glyphAdvances = &_advances[0];

		return _dw_render_target->DrawGlyphRun(baselineOriginX,
			baselineOriginY,
			_measuring_mode,
			&final_glyph_run,
			_dw_render_param,
			_text_color);
	}
}

/******************************************************************
*																 *
*  gdimm_dw_text::DrawUnderline								 *
*																 *
*  This function is not implemented for the purposes of this	  *
*  sample.														*
*																 *
******************************************************************/

IFACEMETHODIMP gdimm_dw_text::DrawUnderline(
	__maybenull void* clientDrawingContext,
	FLOAT baselineOriginX,
	FLOAT baselineOriginY,
	__in DWRITE_UNDERLINE const* underline,
	__maybenull IUnknown* clientDrawingEffect
	)
{
	// Not implemented
	return E_NOTIMPL;
}

/******************************************************************
*																 *
*  gdimm_dw_text::DrawStrikethrough							 *
*																 *
*  This function is not implemented for the purposes of this	  *
*  sample.														*
*																 *
******************************************************************/

IFACEMETHODIMP gdimm_dw_text::DrawStrikethrough(
	__maybenull void* clientDrawingContext,
	FLOAT baselineOriginX,
	FLOAT baselineOriginY,
	__in DWRITE_STRIKETHROUGH const* strikethrough,
	__maybenull IUnknown* clientDrawingEffect
	)
{
	// Not implemented
	return E_NOTIMPL;
}

/******************************************************************
*																 *
*  gdimm_dw_text::DrawInlineObject							  *
*																 *
*  This function is not implemented for the purposes of this	  *
*  sample.														*
*																 *
******************************************************************/

IFACEMETHODIMP gdimm_dw_text::DrawInlineObject(
	__maybenull void* clientDrawingContext,
	FLOAT originX,
	FLOAT originY,
	IDWriteInlineObject* inlineObject,
	BOOL isSideways,
	BOOL isRightToLeft,
	__maybenull IUnknown* clientDrawingEffect
	)
{
	// Not implemented
	return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////

bool gdimm_dw_text::begin(const gdimm_text_context *context)
{
	if (!gdimm_text::begin(context))
		return false;

	HRESULT hr;

	if (_dw_factory == NULL)
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **)&_dw_factory);
		assert(hr == S_OK);

		if (_dw_gdi_interop == NULL)
		{
			hr = _dw_factory->GetGdiInterop(&_dw_gdi_interop);
			assert(hr == S_OK);
		}

		if (_dw_render_param == NULL)
		{
			hr = _dw_factory->CreateCustomRenderingParams(0.8f, 0.0f, 1.0f, DWRITE_PIXEL_GEOMETRY_RGB, DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL_SYMMETRIC, &_dw_render_param);
			assert(hr == S_OK);
		}
	}

	/*bool is_mono = false;
	if (is_non_aa(_font_attr.lfQuality, _context->setting_cache) || 
		(_dc_bmp_header.biBitCount == 1))
		is_mono = true;

	if (is_mono && !_context->setting_cache->render_mono)
		return false;*/

	switch (_context->setting_cache->hinting)
	{
	case 2:
		_measuring_mode = DWRITE_MEASURING_MODE_GDI_CLASSIC;
		break;
	case 3:
		_measuring_mode = DWRITE_MEASURING_MODE_GDI_NATURAL;
		break;
	default:
		_measuring_mode = DWRITE_MEASURING_MODE_NATURAL;
		break;
	}
	_use_gdi_natural = (_measuring_mode == DWRITE_MEASURING_MODE_GDI_NATURAL);
	_advances.clear();
	_pixels_per_dip = GetDeviceCaps(_context->hdc, LOGPIXELSY) / 96.0f;

	_cell_width = 0;
	_cell_height = _context->outline_metrics->otmTextMetrics.tmHeight;
	_em_height = _context->outline_metrics->otmTextMetrics.tmHeight - _context->outline_metrics->otmTextMetrics.tmInternalLeading;

	return true;
}

bool gdimm_dw_text::text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	BOOL b_ret;

	const int advance_factor = ((options & ETO_PDY) ? 2 : 1);
	if (lpDx != NULL)
	{
		_advances.resize(c);
		for (UINT i = 0; i < c; i++)
		{
			_advances[i] = (FLOAT) lpDx[i * advance_factor];
			_cell_width += (UINT32) _advances[i];
		}
	}

	CComPtr<IDWriteFontFace> dw_font_face;
	CComPtr<IDWriteTextFormat> dw_text_format;
	CComPtr<IDWriteTextLayout> dw_text_layout;

	bool success;
	if (options & ETO_GLYPH_INDEX)
		success = prepare_glyph(lpString, c, &dw_font_face);
	else
		success = prepare_text(lpString, c, &dw_text_format, &dw_text_layout);

	if (!success)
		return false;

	_extra_height = min(_extra_height, 2);
	LONG cell_ascent = _context->outline_metrics->otmTextMetrics.tmAscent;
	LONG cell_descent = _context->outline_metrics->otmTextMetrics.tmDescent;
	const POINT baseline = get_baseline(_text_alignment,
		x,
		y,
		_cell_width,
		cell_ascent,
		cell_descent);
	RECT bmp_rect = {baseline.x,
		baseline.y - cell_ascent,
		baseline.x + _cell_width,
		baseline.y + cell_descent};

	// apply clipping
	if (options & ETO_CLIPPED)
	{
		if (!IntersectRect(&bmp_rect, &bmp_rect, lprect))
			bmp_rect = *lprect;

		_cell_width = bmp_rect.right - bmp_rect.left;
		_cell_height = bmp_rect.bottom - bmp_rect.top;
		cell_ascent = baseline.y - bmp_rect.top;
		cell_descent = bmp_rect.bottom - baseline.y;
	}

	HDC hdc_canvas = _dw_render_target->GetMemoryDC();

	const COLORREF bg_color = GetBkColor(_context->hdc);

	if (options & ETO_OPAQUE)
		draw_background(_context->hdc, lprect, bg_color);

	const int bk_mode = GetBkMode(_context->hdc);
	if (bk_mode == OPAQUE)
	{
		const RECT bk_rect = {0, 0, _cell_width, _cell_height};
		b_ret = draw_background(hdc_canvas, &bk_rect, bg_color);
	}
	else if (bk_mode == TRANSPARENT)
	{
		b_ret = BitBlt(hdc_canvas, 0, 0, _cell_width, _cell_height, _context->hdc, bmp_rect.left, bmp_rect.top, SRCCOPY);
	}
	else
		b_ret = FALSE;

	if (!b_ret)
	{
		_dw_render_target->Release();
		return false;
	}

	if (options & ETO_GLYPH_INDEX)
	{
		DWRITE_GLYPH_RUN dw_glyph_run = {dw_font_face,
			(FLOAT) _em_height,
			c,
			(UINT16 *)lpString,
			(_advances.empty() ? NULL : &_advances[0]),
			FALSE,
			0};

		HRESULT hr = _dw_render_target->DrawGlyphRun(0, (FLOAT)(cell_ascent - _extra_height), _measuring_mode, &dw_glyph_run, _dw_render_param, _text_color);
		assert(hr == S_OK);
	}
	else
	{
		HRESULT hr = dw_text_layout->Draw(NULL, this, 0, (FLOAT) -_extra_height);
		assert(hr == S_OK);
	}

	if (!success)
	{
		_dw_render_target->Release();
		return false;
	}

	b_ret = BitBlt(_context->hdc, bmp_rect.left, bmp_rect.top, _cell_width, _cell_height, hdc_canvas, 0, 0, SRCCOPY);
	assert(b_ret);

	_dw_render_target->Release();
	return true;
}