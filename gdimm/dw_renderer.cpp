#include "stdafx.h"
#include "dw_renderer.h"

gdimm_dw_renderer::gdimm_dw_renderer(IDWriteRenderingParams* render_params)
:
_ref_count(0), 
_render_params(render_params)
{
	render_params->AddRef();
}

/******************************************************************
*																 *
*  gdimm_dw_renderer::~gdimm_dw_renderer							   *
*																 *
*  Destructor releases the interfaces passed when the class was   *
*  created.													   *
*																 *
******************************************************************/

gdimm_dw_renderer::~gdimm_dw_renderer()
{
	_render_params->Release();
}

/******************************************************************
*																 *
*  gdimm_dw_renderer::QueryInterface								*
*																 *
*  Query interface implementation								 *
*																 *
******************************************************************/

IFACEMETHODIMP gdimm_dw_renderer::QueryInterface(
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
*  gdimm_dw_renderer::AddRef										*
*																 *
*  Increments the ref count									   *
*																 *
******************************************************************/

IFACEMETHODIMP_(ULONG) gdimm_dw_renderer::AddRef()
{
	return InterlockedIncrement(&_ref_count);
}

/******************************************************************
*																 *
*  gdimm_dw_renderer::Release									   *
*																 *
*  Decrements the ref count and deletes the instance if the ref   *
*  count becomes 0												*
*																 *
******************************************************************/

IFACEMETHODIMP_(ULONG) gdimm_dw_renderer::Release()
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
	//forward the render target's transform

	assert(clientDrawingContext != NULL);
	drawing_context *context = (drawing_context*) clientDrawingContext;

	context->render_target->GetCurrentTransform(transform);
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
	assert(clientDrawingContext != NULL);
	drawing_context *context = (drawing_context*) clientDrawingContext;

	*pixelsPerDip = context->render_target->GetPixelsPerDip();
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
	assert(clientDrawingContext != NULL);
	drawing_context *context = (drawing_context*) clientDrawingContext;

	if (context->advances == NULL)
	{
		return context->render_target->DrawGlyphRun(baselineOriginX,
			baselineOriginY,
			measuringMode,
			glyphRun,
			_render_params,
			context->text_color);
	}
	else
	{
		DWRITE_GLYPH_RUN final_glyph_run = *glyphRun;
		final_glyph_run.glyphAdvances = context->advances;

		return context->render_target->DrawGlyphRun(baselineOriginX,
			baselineOriginY,
			measuringMode,
			&final_glyph_run,
			_render_params,
			context->text_color);
	}
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
	// Not implemented
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
	// Not implemented
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
	// Not implemented
	return E_NOTIMPL;
}