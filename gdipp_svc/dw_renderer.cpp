#include "stdafx.h"
#include "dw_renderer.h"
#include "helper_func.h"
#include "gdimm.h"

IDWriteFactory *gdimm_dw_renderer::_dw_factory = NULL;
IDWriteGdiInterop *gdimm_dw_renderer::_dw_gdi_interop = NULL;

gdimm_dw_renderer::gdimm_dw_renderer()
{
	HRESULT hr;

	if (_dw_factory == NULL)
	{
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown **>(&_dw_factory));
		assert(hr == S_OK);

		mem_man_instance.register_com_ptr(_dw_factory);
	}

	if (_dw_gdi_interop == NULL)
	{
		hr = _dw_factory->GetGdiInterop(&_dw_gdi_interop);
		assert(hr == S_OK);

		mem_man_instance.register_com_ptr(_dw_gdi_interop);
	}
}

bool gdimm_dw_renderer::begin(const dc_context *context, FT_Render_Mode render_mode)
{
	if (!gdimm_renderer::begin(context, render_mode))
		return false;

	// ignore rotated DC
	if (_context->log_font.lfEscapement % 3600 != 0)
		return false;

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
	_em_size = static_cast<FLOAT>(_context->outline_metrics->otmTextMetrics.tmHeight - _context->outline_metrics->otmTextMetrics.tmInternalLeading);
	_pixels_per_dip = GetDeviceCaps(_context->hdc, LOGPIXELSY) / 96.0f;

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool gdimm_dw_renderer::make_glyph_texture(FLOAT x, FLOAT y, const DWRITE_GLYPH_RUN *dw_glyph_run, glyph_run *a_glyph_run)
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
	if (_render_mode == FT_RENDER_MODE_MONO)
		return false;
	else if (_render_mode == FT_RENDER_MODE_LCD)
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

	const LONG texture_width = texture_rect.right - texture_rect.left;
	const LONG texture_height = texture_rect.bottom - texture_rect.top;

	FT_BitmapGlyph new_bmp_glyph = new FT_BitmapGlyphRec();
	new_bmp_glyph->left = texture_rect.left;
	new_bmp_glyph->top = -texture_rect.top;
	new_bmp_glyph->bitmap.rows = texture_rect.bottom - texture_rect.top;
	new_bmp_glyph->bitmap.width = texture_width * bytes_per_pixel;
	new_bmp_glyph->bitmap.pitch = new_bmp_glyph->bitmap.width;
	new_bmp_glyph->bitmap.pixel_mode = ft_pixel_mode;

	const int bmp_size = new_bmp_glyph->bitmap.pitch * new_bmp_glyph->bitmap.rows;
	new_bmp_glyph->bitmap.buffer = new BYTE[bmp_size];
	hr = dw_analysis->CreateAlphaTexture(dw_texture_type, &texture_rect, new_bmp_glyph->bitmap.buffer, bmp_size);
	assert(hr == S_OK);

	RECT ctrl_box, black_box;
	ctrl_box.left = static_cast<LONG>(x);
	ctrl_box.top = static_cast<LONG>(y);
	ctrl_box.right = texture_width;
	ctrl_box.bottom = ctrl_box.top;
	black_box.left = ctrl_box.left + texture_rect.left;
	black_box.top = ctrl_box.top;
	black_box.right = black_box.left + texture_width;
	black_box.bottom = ctrl_box.bottom;

	a_glyph_run->glyphs.push_back(reinterpret_cast<FT_Glyph>(new_bmp_glyph));
	a_glyph_run->ctrl_boxes.push_back(ctrl_box);
	a_glyph_run->black_boxes.push_back(black_box);

	return true;
}

bool gdimm_dw_renderer::render_glyph(LPCWSTR lpString, UINT c, glyph_run &new_glyph_run)
{
	HRESULT hr;

	CComPtr<IDWriteFontFace> dw_font_face;
	hr = _dw_gdi_interop->CreateFontFaceFromHdc(_context->hdc, &dw_font_face);
	assert(hr == S_OK);

	DWRITE_GLYPH_RUN dw_glyph_run;
	dw_glyph_run.fontFace = dw_font_face;
	dw_glyph_run.fontEmSize = _em_size;
	dw_glyph_run.glyphCount = c;
	dw_glyph_run.glyphIndices = reinterpret_cast<const UINT16 *>(lpString);
	dw_glyph_run.glyphAdvances = (_advances.empty() ? NULL : _advances.data());
	dw_glyph_run.glyphOffsets = NULL;
	dw_glyph_run.isSideways = FALSE;
	dw_glyph_run.bidiLevel = 0;

	return make_glyph_texture(0, 0, &dw_glyph_run, &new_glyph_run);
}

bool gdimm_dw_renderer::render_text(LPCWSTR lpString, UINT c, glyph_run &new_glyph_run)
{
	HRESULT hr;

	const long font_id = font_man_instance.register_font(_context->hdc, metric_face_name(_context->outline_metrics));
	assert(font_id < 0);
	const os2_metrics *os2_metrics = font_man_instance.lookup_os2_metrics(font_id);

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
		static_cast<DWRITE_FONT_WEIGHT>(_context->outline_metrics->otmTextMetrics.tmWeight),
		dw_font_style,
		static_cast<DWRITE_FONT_STRETCH>(os2_metrics->get_usWidthClass()),
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
			static_cast<FLOAT>(_context->bmp_header.biWidth),
			0,
			&dw_text_layout);
	}
	else
	{
		hr = _dw_factory->CreateGdiCompatibleTextLayout(lpString,
			c,
			dw_text_format,
			static_cast<FLOAT>(_context->bmp_header.biWidth),
			0,
			_pixels_per_dip,
			NULL,
			_use_gdi_natural,
			&dw_text_layout);
	}
	assert(hr == S_OK);

	UINT glyph_run_start = 0;
	bool draw_success;
	void *drawing_context[3] = {&new_glyph_run, &glyph_run_start, &draw_success};
	hr = dw_text_layout->Draw(drawing_context, this, 0, 0);
	assert(hr == S_OK);
	assert(glyph_run_start == c);

	return draw_success;
}

bool gdimm_dw_renderer::render(bool is_glyph_index, bool is_pdy, LPCWSTR lpString, UINT c, CONST INT *lpDx, glyph_run &new_glyph_run)
{
	if (lpDx != NULL)
	{
		const BYTE dx_skip = (is_pdy ? 2 : 1);
		_advances.resize(c);
		for (UINT i = 0; i < c; ++i)
			_advances[i] = static_cast<FLOAT>(lpDx[i * dx_skip]);// - 0.1f;	// small adjustment to emulate GDI metrics
	}

	if (is_glyph_index)
		return render_glyph(lpString, c, new_glyph_run);
	else
		return render_text(lpString, c, new_glyph_run);
}

//
// IUnknown methods
//
//      These methods are never called in this scenario so we just use stub
//      implementations.
//
IFACEMETHODIMP gdimm_dw_renderer::QueryInterface(
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject
	)
{
	return E_NOTIMPL;
}

IFACEMETHODIMP_(ULONG) gdimm_dw_renderer::AddRef( void)
{
	return 0;
}

IFACEMETHODIMP_(ULONG) gdimm_dw_renderer::Release( void)
{
	return 0;
}

/******************************************************************
*																 *
*  gdimm_dw_renderer::IsPixelSnappingDisabled					   *
*																 *
*  Determines whether pixel snapping is disabled. The recommended *
*  default is FALSE, unless doing animation that requires		 *
*  subpixel vertical placement.								   *
*																 *
******************************************************************/

IFACEMETHODIMP gdimm_dw_renderer::IsPixelSnappingDisabled(
	__maybenull void* clientDrawingContext,
	__out BOOL* isDisabled
	)
{
	*isDisabled = FALSE;

	return S_OK;
}

/******************************************************************
*																 *
*  gdimm_dw_renderer::GetCurrentTransform						   *
*																 *
*  Returns the current transform applied to the render target..   *
*																 *
******************************************************************/

IFACEMETHODIMP gdimm_dw_renderer::GetCurrentTransform(
	__maybenull void* clientDrawingContext,
	__out DWRITE_MATRIX* transform
	)
{
	transform = NULL;

	return S_OK;
}

/******************************************************************
*																 *
*  gdimm_dw_renderer::GetPixelsPerDip							   *
*																 *
*  This returns the number of pixels per DIP.					 *
*																 *
******************************************************************/

IFACEMETHODIMP gdimm_dw_renderer::GetPixelsPerDip(
	__maybenull void* clientDrawingContext,
	__out FLOAT* pixelsPerDip
	)
{
	*pixelsPerDip = _pixels_per_dip;

	return S_OK;
}

/******************************************************************
*																 *
*  gdimm_dw_renderer::DrawGlyphRun								  *
*																 *
*  Gets GlyphRun outlines via IDWriteFontFace::GetGlyphRunOutline *
*  and then draws and fills them using Direct2D path geometries   *
*																 *
******************************************************************/

IFACEMETHODIMP gdimm_dw_renderer::DrawGlyphRun(
	__maybenull void* clientDrawingContext,
	FLOAT baselineOriginX,
	FLOAT baselineOriginY,
	DWRITE_MEASURING_MODE measuringMode,
	__in DWRITE_GLYPH_RUN const* glyphRun,
	__in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
	__maybenull IUnknown* clientDrawingEffect
	)
{
	void **drawing_context = static_cast<void **>(clientDrawingContext);
	glyph_run *new_glyph_run = static_cast<glyph_run *>(drawing_context[0]);
	UINT *glyph_run_start = static_cast<UINT *>(drawing_context[1]);
	bool *draw_success = static_cast<bool *>(drawing_context[2]);

	if (_advances.empty())
		*draw_success &= make_glyph_texture(baselineOriginX, 0, glyphRun, new_glyph_run);
	else
	{
		DWRITE_GLYPH_RUN final_glyph_run = *glyphRun;
		final_glyph_run.glyphAdvances = &_advances[*glyph_run_start];

		*draw_success &= make_glyph_texture(baselineOriginX, 0, &final_glyph_run, new_glyph_run);
	}

	*glyph_run_start += glyphRunDescription->stringLength;

	return S_OK;
}

/******************************************************************
*																 *
*  gdimm_dw_renderer::DrawUnderline								 *
*																 *
*  This function is not implemented for the purposes of this	  *
*  sample.														*
*																 *
******************************************************************/

IFACEMETHODIMP gdimm_dw_renderer::DrawUnderline(
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
*  gdimm_dw_renderer::DrawStrikethrough							 *
*																 *
*  This function is not implemented for the purposes of this	  *
*  sample.														*
*																 *
******************************************************************/

IFACEMETHODIMP gdimm_dw_renderer::DrawStrikethrough(
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
*  gdimm_dw_renderer::DrawInlineObject							  *
*																 *
*  This function is not implemented for the purposes of this	  *
*  sample.														*
*																 *
******************************************************************/

IFACEMETHODIMP gdimm_dw_renderer::DrawInlineObject(
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
