#include "stdafx.h"
#include "dw_renderer.h"
#include "helper_func.h"
#include "gdimm.h"

gdimm_dw_text::gdimm_dw_text()
:
_dw_factory(NULL),
_dw_gdi_interop(NULL)
{
}

bool gdimm_dw_text::make_glyph_texture(FLOAT x, FLOAT y, const DWRITE_GLYPH_RUN *dw_glyph_run)
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
		case 2:
			dw_render_mode = DWRITE_RENDERING_MODE_CLEARTYPE_GDI_NATURAL;
			break;
		case 3:
			dw_render_mode = DWRITE_RENDERING_MODE_CLEARTYPE_GDI_CLASSIC;
			break;
		default:
			dw_render_mode = DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL_SYMMETRIC;
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

	FT_BitmapGlyph new_glyph = new FT_BitmapGlyphRec();
	new_glyph->left = texture_rect.left;
	new_glyph->top = -texture_rect.top;
	new_glyph->bitmap.rows = texture_rect.bottom - texture_rect.top;
	new_glyph->bitmap.width = (texture_rect.right - texture_rect.left) * bytes_per_pixel;
	new_glyph->bitmap.pitch = new_glyph->bitmap.width;
	new_glyph->bitmap.pixel_mode = ft_pixel_mode;

	const int bmp_size = new_glyph->bitmap.pitch * new_glyph->bitmap.rows;
	new_glyph->bitmap.buffer = new BYTE[bmp_size];
	hr = dw_analysis->CreateAlphaTexture(dw_texture_type, &texture_rect, new_glyph->bitmap.buffer, bmp_size);
	assert(hr == S_OK);

	const POINT pen_pos = {(LONG) x, (LONG) y};

	_glyphs.push_back(new_glyph);
	_glyph_pos.push_back(pen_pos);

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
	dw_glyph_run.fontEmSize = _em_size;
	dw_glyph_run.glyphCount = c;
	dw_glyph_run.glyphIndices = (UINT16 *)lpString;
	dw_glyph_run.glyphAdvances = (_advances.empty() ? NULL : &_advances[0]);
	dw_glyph_run.glyphOffsets = NULL;
	dw_glyph_run.isSideways = FALSE;
	dw_glyph_run.bidiLevel = 0;

	return make_glyph_texture(0, 0, &dw_glyph_run);
}

bool gdimm_dw_text::render_text(LPCWSTR lpString, UINT c)
{
	HRESULT hr;

	const long font_id = _font_man.register_font(_context->hdc, metric_face_name(_context->outline_metrics));
	const gdimm_os2_metrics *os2_metrics = _font_man.lookup_os2_metrics(font_id);

	DWRITE_FONT_STYLE dw_font_style;
	if (!_context->outline_metrics->otmTextMetrics.tmItalic)
		dw_font_style = DWRITE_FONT_STYLE_NORMAL;
	else if (os2_metrics->is_italic())
		dw_font_style = DWRITE_FONT_STYLE_ITALIC;
	else
		dw_font_style = DWRITE_FONT_STYLE_OBLIQUE;

	CComPtr<IDWriteTextFormat> dw_text_format;
	hr = _dw_factory->CreateTextFormat(metric_family_name(_context->outline_metrics),
		NULL,
		(DWRITE_FONT_WEIGHT) _context->outline_metrics->otmTextMetrics.tmWeight,
		dw_font_style,
		(DWRITE_FONT_STRETCH) os2_metrics->get_usWidthClass(),
		_em_size,
		L"",
		&dw_text_format);
	assert(hr == S_OK);
	
	hr = dw_text_format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
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

	UINT glyph_run_start = 0;
	hr = dw_text_layout->Draw(&glyph_run_start, this, 0, 0);
	assert(glyph_run_start == c);
	
	return (hr == S_OK);
}

bool gdimm_dw_text::render(UINT options, LPCWSTR lpString, UINT c, CONST INT *lpDx)
{
	const int advance_factor = ((options & ETO_PDY) ? 2 : 1);
	if (lpDx != NULL)
	{
		_advances.resize(c);
		for (UINT i = 0; i < c; i++)
			_advances[i] = (FLOAT) lpDx[i * advance_factor] - 0.1f;	// small adjustment to emulate GDI metrics
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
	bool b_ret;
	UINT *glyph_run_start = (UINT *)clientDrawingContext;

	if (_advances.empty())
		b_ret = make_glyph_texture(baselineOriginX, 0, glyphRun);
	else
	{
		DWRITE_GLYPH_RUN final_glyph_run = *glyphRun;
		final_glyph_run.glyphAdvances = &_advances[*glyph_run_start];
		
		b_ret = make_glyph_texture(baselineOriginX, 0, &final_glyph_run);
	}

	*glyph_run_start += glyphRunDescription->stringLength;

	if (b_ret)
		return S_OK;
	else
		return E_FAIL;
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

bool gdimm_dw_text::begin(const dc_context *context)
{
	HRESULT hr;

	if (!gdimm_gdi_painter::begin(context))
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

	_advances.clear();
	_em_size = (FLOAT)(_context->outline_metrics->otmTextMetrics.tmHeight - _context->outline_metrics->otmTextMetrics.tmInternalLeading);
	_pixels_per_dip = GetDeviceCaps(_context->hdc, LOGPIXELSY) / 96.0f;

	return true;
}

void gdimm_dw_text::end()
{
	for (vector<const FT_BitmapGlyph>::iterator iter = _glyphs.begin(); iter != _glyphs.end(); iter++)
	{
		delete[] (*iter)->bitmap.buffer;
		delete *iter;
	}
}