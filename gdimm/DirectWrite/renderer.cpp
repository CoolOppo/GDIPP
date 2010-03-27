#include "stdafx.h"
#include "renderer.h"

gdimm_renderer::gdimm_renderer(
	IDWriteBitmapRenderTarget* render_target,
	IDWriteRenderingParams* render_params,
	bool is_glyph_index
	)
:
_ref_count(0), 
_render_target(render_target),
_render_params(render_params),
_is_glyph_index(is_glyph_index)
{
	render_target->AddRef();
	render_params->AddRef();
}

/******************************************************************
*																 *
*  gdimm_renderer::gdimm_renderer							   *
*																 *
*  Destructor releases the interfaces passed when the class was   *
*  created.													   *
*																 *
******************************************************************/

gdimm_renderer::~gdimm_renderer()
{
	_render_target->Release();
	_render_params->Release();
}


/******************************************************************
*																 *
*  gdimm_renderer::DrawGlyphRun								  *
*																 *
*  Gets GlyphRun outlines via IDWriteFontFace::GetGlyphRunOutline *
*  and then draws and fills them using Direct2D path geometries   *
*																 *
******************************************************************/
STDMETHODIMP gdimm_renderer::DrawGlyphRun(
	__maybenull void* clientDrawingContext,
	FLOAT baselineOriginX,
	FLOAT baselineOriginY,
	DWRITE_MEASURING_MODE measuringMode,
	__in DWRITE_GLYPH_RUN const* glyphRun,
	__in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
	IUnknown* clientDrawingEffect
	)
{
	HRESULT hr;

	DWRITE_GLYPH_RUN final_glyphRun = *glyphRun;
	if (_is_glyph_index)
	{
		final_glyphRun.glyphIndices = (const UINT16*) glyphRunDescription->string;
	}

	hr = _render_target->DrawGlyphRun(
		baselineOriginX,
		baselineOriginY,
		measuringMode,
		&final_glyphRun,
		_render_params,
		RGB(0,0,0),
		NULL);

	return hr;
}

/******************************************************************
*																 *
*  gdimm_renderer::DrawUnderline								 *
*																 *
*  This function is not implemented for the purposes of this	  *
*  sample.														*
*																 *
******************************************************************/

STDMETHODIMP gdimm_renderer::DrawUnderline(
	__maybenull void* clientDrawingContext,
	FLOAT baselineOriginX,
	FLOAT baselineOriginY,
	__in DWRITE_UNDERLINE const* underline,
	IUnknown* clientDrawingEffect
	)
{
	// Not implemented
	return E_NOTIMPL;
}

/******************************************************************
*																 *
*  gdimm_renderer::DrawStrikethrough							 *
*																 *
*  This function is not implemented for the purposes of this	  *
*  sample.														*
*																 *
******************************************************************/

STDMETHODIMP gdimm_renderer::DrawStrikethrough(
	__maybenull void* clientDrawingContext,
	FLOAT baselineOriginX,
	FLOAT baselineOriginY,
	__in DWRITE_STRIKETHROUGH const* strikethrough,
	IUnknown* clientDrawingEffect
	)
{
	// Not implemented
	return E_NOTIMPL;
}

/******************************************************************
*																 *
*  gdimm_renderer::DrawInlineObject							  *
*																 *
*  This function is not implemented for the purposes of this	  *
*  sample.														*
*																 *
******************************************************************/

STDMETHODIMP gdimm_renderer::DrawInlineObject(
	__maybenull void* clientDrawingContext,
	FLOAT originX,
	FLOAT originY,
	IDWriteInlineObject* inlineObject,
	BOOL isSideways,
	BOOL isRightToLeft,
	IUnknown* clientDrawingEffect
	)
{
	// Not implemented
	return E_NOTIMPL;
}

/******************************************************************
*																 *
*  gdimm_renderer::AddRef										*
*																 *
*  Increments the ref count									   *
*																 *
******************************************************************/

STDMETHODIMP_(unsigned long) gdimm_renderer::AddRef()
{
	return InterlockedIncrement(&_ref_count);
}

/******************************************************************
*																 *
*  gdimm_renderer::Release									   *
*																 *
*  Decrements the ref count and deletes the instance if the ref   *
*  count becomes 0												*
*																 *
******************************************************************/

STDMETHODIMP_(unsigned long) gdimm_renderer::Release()
{
	long newCount = InterlockedDecrement(&_ref_count);

	if (newCount == 0)
	{
		delete this;
		return 0;
	}
	return newCount;
}

/******************************************************************
*																 *
*  gdimm_renderer::IsPixelSnappingDisabled					   *
*																 *
*  Determines whether pixel snapping is disabled. The recommended *
*  default is FALSE, unless doing animation that requires		 *
*  subpixel vertical placement.								   *
*																 *
******************************************************************/

STDMETHODIMP gdimm_renderer::IsPixelSnappingDisabled(
	__maybenull void* clientDrawingContext,
	__out BOOL* isDisabled
	)
{
	*isDisabled = FALSE;
	return S_OK;
}

/******************************************************************
*																 *
*  gdimm_renderer::GetCurrentTransform						   *
*																 *
*  Returns the current transform applied to the render target..   *
*																 *
******************************************************************/

STDMETHODIMP gdimm_renderer::GetCurrentTransform(
	__maybenull void* clientDrawingContext,
	__out DWRITE_MATRIX* transform
	)
{
	//forward the render target's transform
	_render_target->GetCurrentTransform(transform);
	return S_OK;
}

/******************************************************************
*																 *
*  gdimm_renderer::GetPixelsPerDip							   *
*																 *
*  This returns the number of pixels per DIP.					 *
*																 *
******************************************************************/

STDMETHODIMP gdimm_renderer::GetPixelsPerDip(
	__maybenull void* clientDrawingContext,
	__out FLOAT* pixelsPerDip
	)
{
	*pixelsPerDip = _render_target->GetPixelsPerDip();
	return S_OK;
}

/******************************************************************
*																 *
*  gdimm_renderer::QueryInterface								*
*																 *
*  Query interface implementation								 *
*																 *
******************************************************************/

STDMETHODIMP gdimm_renderer::QueryInterface(
	IID const& riid,
	void** ppvObject
	)
{
	if (__uuidof(IDWriteTextRenderer) == riid)
	{
		*ppvObject = this;
	}
	else if (__uuidof(IDWritePixelSnapping) == riid)
	{
		*ppvObject = this;
	}
	else if (__uuidof(IUnknown) == riid)
	{
		*ppvObject = this;
	}
	else
	{
		*ppvObject = NULL;
		return E_FAIL;
	}

	return S_OK;
}