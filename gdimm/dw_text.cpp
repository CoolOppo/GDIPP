#include "stdafx.h"
#include "dw_text.h"
#include "text_helper.h"
#include "gdimm.h"

gdimm_dw_text::gdimm_dw_text()
:
_dw_factory(NULL),
_dw_gdi_interop(NULL)
{
}

bool gdimm_dw_text::make_glyph_texture(const DWRITE_GLYPH_RUN *dw_glyph_run)
{
	HRESULT hr;

	DWRITE_RENDERING_MODE dw_render_mode;
	if (_render_mode == FT_RENDER_MODE_LCD)
	{
		switch (_context->setting_cache->hinting)
		{
		case 0:
			dw_render_mode = DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL;
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
	else
		dw_render_mode = DWRITE_RENDERING_MODE_ALIASED;

	DWRITE_TEXTURE_TYPE dw_texture_type;
	int bytes_per_pixel;
	char ft_pixel_mode;
	if (_render_mode == FT_RENDER_MODE_LCD)
	{
		dw_texture_type = DWRITE_TEXTURE_CLEARTYPE_3x1;
		bytes_per_pixel = 3;
		ft_pixel_mode = FT_PIXEL_MODE_LCD;
	}
	else
	{
		dw_texture_type = DWRITE_TEXTURE_ALIASED_1x1;
		bytes_per_pixel = 1;
		ft_pixel_mode = FT_PIXEL_MODE_GRAY;
	}

	CComPtr<IDWriteGlyphRunAnalysis> dw_analysis;
	hr = _dw_factory->CreateGlyphRunAnalysis(dw_glyph_run,
		_pixels_per_dip,
		NULL,
		dw_render_mode,
		_dw_measuring_mode,
		0,
		0,
		&dw_analysis);
	assert(hr == S_OK);

	RECT texture_rect;
	hr = dw_analysis->GetAlphaTextureBounds(dw_texture_type, &texture_rect);
	assert(hr == S_OK);

	if (IsRectEmpty(&texture_rect))
		return false;

	_glyphs.resize(1);
	_glyph_pos.resize(1);

	_glyphs[0] = new FT_BitmapGlyphRec();
	_glyphs[0]->left = texture_rect.left;
	_glyphs[0]->top = -texture_rect.top;
	_glyphs[0]->bitmap.rows = texture_rect.bottom - texture_rect.top;
	_glyphs[0]->bitmap.width = (texture_rect.right - texture_rect.left) * bytes_per_pixel;
	_glyphs[0]->bitmap.pitch = _glyphs[0]->bitmap.width;
	_glyphs[0]->bitmap.pixel_mode = ft_pixel_mode;

	const int bmp_size = _glyphs[0]->bitmap.pitch * _glyphs[0]->bitmap.rows;
	_glyphs[0]->bitmap.buffer = new BYTE[bmp_size];
	hr = dw_analysis->CreateAlphaTexture(dw_texture_type, &texture_rect, _glyphs[0]->bitmap.buffer, bmp_size);
	assert(hr == S_OK);

	return true;
}

bool gdimm_dw_text::render_glyph(LPCWSTR lpString, UINT c)
{
	HRESULT hr;

	CComPtr<IDWriteFontFace> dw_font_face;
	hr = _dw_gdi_interop->CreateFontFaceFromHdc(_context->hdc, &dw_font_face);
	assert(hr == S_OK);

	DWRITE_GLYPH_RUN dw_glyph_run;
	dw_glyph_run.fontFace = dw_font_face;
	dw_glyph_run.fontEmSize = (FLOAT) _em_size;
	dw_glyph_run.glyphCount = c;
	dw_glyph_run.glyphIndices = (UINT16 *)lpString;
	dw_glyph_run.glyphAdvances = (_advances.empty() ? NULL : &_advances[0]);
	dw_glyph_run.glyphOffsets = NULL;
	dw_glyph_run.isSideways = FALSE;
	dw_glyph_run.bidiLevel = 0;

	return make_glyph_texture(&dw_glyph_run);
}

bool gdimm_dw_text::render_text(LPCWSTR lpString, UINT c)
{
	HRESULT hr;

	LOGFONTW actual_lf = _font_attr;
	wcsncpy_s(actual_lf.lfFaceName, metric_family_name(_context->outline_metrics), LF_FACESIZE);

	CComPtr<IDWriteFont> dw_font;
	hr = _dw_gdi_interop->CreateFontFromLOGFONT(&actual_lf, &dw_font);
	assert(hr == S_OK);

	CComPtr<IDWriteTextFormat> dw_text_format;
	hr = _dw_factory->CreateTextFormat(metric_family_name(_context->outline_metrics),
		NULL,
		dw_font->GetWeight(),
		dw_font->GetStyle(),
		dw_font->GetStretch(),
		(FLOAT) _em_size,
		L"",
		&dw_text_format);
	assert(hr == S_OK);

	CComPtr<IDWriteTextLayout> dw_text_layout;
	if (_dw_measuring_mode == DWRITE_MEASURING_MODE_NATURAL)
	{
		hr = _dw_factory->CreateTextLayout(lpString,
			c,
			dw_text_format,
			(FLOAT) _dc_bmp_header.biWidth,
			0,
			&dw_text_layout);
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
			&dw_text_layout);
	}
	assert(hr == S_OK);

	hr = dw_text_layout->Draw(NULL, this, 0, 0);
	assert(hr == S_OK);

	return true;
}

bool gdimm_dw_text::render(UINT options, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	const int advance_factor = ((options & ETO_PDY) ? 2 : 1);
	if (lpDx != NULL)
	{
		_advances.resize(c);
		for (UINT i = 0; i < c; i++)
			_advances[i] = (FLOAT) lpDx[i * advance_factor];
	}

	if (options & ETO_GLYPH_INDEX)
		return render_glyph(lpString, c);
	else
		return render_text(lpString, c);
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
	transform = NULL;

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
	*pixelsPerDip = _pixels_per_dip;

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
		make_glyph_texture(glyphRun);
	else
	{
		DWRITE_GLYPH_RUN final_glyph_run = *glyphRun;
		final_glyph_run.glyphAdvances = &_advances[0];
		
		make_glyph_texture(&final_glyph_run);
	}

	return S_OK;
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
	return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////

bool gdimm_dw_text::begin(const gdimm_text_context *context)
{
	HRESULT hr;

	if (!gdimm_gdi_text::begin(context))
		return false;

	// ignore rotated DC
	if (_font_attr.lfEscapement % 3600 != 0)
		return false;

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

	switch (_context->setting_cache->hinting)
	{
	case 0:
	case 1:
		_dw_measuring_mode = DWRITE_MEASURING_MODE_NATURAL;
		break;
	case 2:
		_dw_measuring_mode = DWRITE_MEASURING_MODE_GDI_NATURAL;
		break;
	default:
		_dw_measuring_mode = DWRITE_MEASURING_MODE_GDI_CLASSIC;
		break;
	}
	_use_gdi_natural = (_dw_measuring_mode != DWRITE_MEASURING_MODE_GDI_CLASSIC);

	_advances.clear();
	_em_size = _context->outline_metrics->otmTextMetrics.tmHeight - _context->outline_metrics->otmTextMetrics.tmInternalLeading;
	_pixels_per_dip = GetDeviceCaps(_context->hdc, LOGPIXELSY) / 96.0f;

	return true;
}

bool gdimm_dw_text::text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	bool b_ret = gdimm_gdi_text::text_out(x, y, options, lprect, lpString, c, lpDx);

	if (!_glyphs.empty())
	{
		delete[] _glyphs[0]->bitmap.buffer;
		delete _glyphs[0];
	}

	return b_ret;
}