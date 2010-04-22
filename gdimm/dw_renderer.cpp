#include "stdafx.h"
#include "dw_renderer.h"

dw_renderer::dw_renderer(
	IDWriteBitmapRenderTarget* render_target,
	IDWriteRenderingParams* render_params,
	COLORREF text_color
	)
:
_ref_count(0), 
_render_target(render_target),
_render_params(render_params),
_text_color(text_color)
{
	render_target->AddRef();
	render_params->AddRef();
}

/******************************************************************
*																 *
*  dw_renderer::~dw_renderer							   *
*																 *
*  Destructor releases the interfaces passed when the class was   *
*  created.													   *
*																 *
******************************************************************/

dw_renderer::~dw_renderer()
{
	_render_target->Release();
	_render_params->Release();
}

/******************************************************************
*																 *
*  dw_renderer::QueryInterface								*
*																 *
*  Query interface implementation								 *
*																 *
******************************************************************/

IFACEMETHODIMP dw_renderer::QueryInterface(
	/* [in] */ REFIID riid,
	/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject
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

/******************************************************************
*																 *
*  dw_renderer::AddRef										*
*																 *
*  Increments the ref count									   *
*																 *
******************************************************************/

IFACEMETHODIMP_(ULONG) dw_renderer::AddRef()
{
	return InterlockedIncrement(&_ref_count);
}

/******************************************************************
*																 *
*  dw_renderer::Release									   *
*																 *
*  Decrements the ref count and deletes the instance if the ref   *
*  count becomes 0												*
*																 *
******************************************************************/

IFACEMETHODIMP_(ULONG) dw_renderer::Release()
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
*  dw_renderer::IsPixelSnappingDisabled					   *
*																 *
*  Determines whether pixel snapping is disabled. The recommended *
*  default is FALSE, unless doing animation that requires		 *
*  subpixel vertical placement.								   *
*																 *
******************************************************************/

IFACEMETHODIMP dw_renderer::IsPixelSnappingDisabled(
	__maybenull void* clientDrawingContext,
	__out BOOL* isDisabled
	)
{
	*isDisabled = FALSE;
	return S_OK;
}

/******************************************************************
*																 *
*  dw_renderer::GetCurrentTransform						   *
*																 *
*  Returns the current transform applied to the render target..   *
*																 *
******************************************************************/

IFACEMETHODIMP dw_renderer::GetCurrentTransform(
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
*  dw_renderer::GetPixelsPerDip							   *
*																 *
*  This returns the number of pixels per DIP.					 *
*																 *
******************************************************************/

IFACEMETHODIMP dw_renderer::GetPixelsPerDip(
	__maybenull void* clientDrawingContext,
	__out FLOAT* pixelsPerDip
	)
{
	*pixelsPerDip = _render_target->GetPixelsPerDip();
	return S_OK;
}

/******************************************************************
*																 *
*  dw_renderer::DrawGlyphRun								  *
*																 *
*  Gets GlyphRun outlines via IDWriteFontFace::GetGlyphRunOutline *
*  and then draws and fills them using Direct2D path geometries   *
*																 *
******************************************************************/

IFACEMETHODIMP dw_renderer::DrawGlyphRun(
	__maybenull void* clientDrawingContext,
	FLOAT baselineOriginX,
	FLOAT baselineOriginY,
	DWRITE_MEASURING_MODE measuringMode,
	__in DWRITE_GLYPH_RUN const* glyphRun,
	__in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
	__maybenull IUnknown* clientDrawingEffect
	)
{
	HRESULT hr = _render_target->DrawGlyphRun(
		baselineOriginX,
		baselineOriginY,
		measuringMode,
		glyphRun,
		_render_params,
		_text_color);

	return hr;
}

/******************************************************************
*																 *
*  dw_renderer::DrawUnderline								 *
*																 *
*  This function is not implemented for the purposes of this	  *
*  sample.														*
*																 *
******************************************************************/

IFACEMETHODIMP dw_renderer::DrawUnderline(
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
*  dw_renderer::DrawStrikethrough							 *
*																 *
*  This function is not implemented for the purposes of this	  *
*  sample.														*
*																 *
******************************************************************/

IFACEMETHODIMP dw_renderer::DrawStrikethrough(
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
*  dw_renderer::DrawInlineObject							  *
*																 *
*  This function is not implemented for the purposes of this	  *
*  sample.														*
*																 *
******************************************************************/

IFACEMETHODIMP dw_renderer::DrawInlineObject(
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