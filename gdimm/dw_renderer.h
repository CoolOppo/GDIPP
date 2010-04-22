#pragma once

class dw_renderer : public IDWriteTextRenderer
{
	LONG _ref_count;
	IDWriteBitmapRenderTarget* _render_target;
	IDWriteRenderingParams* _render_params;
	COLORREF _text_color;

public:
	dw_renderer(
		IDWriteBitmapRenderTarget* render_target,
		IDWriteRenderingParams* render_params,
		COLORREF text_color
		);

	dw_renderer::~dw_renderer();

public:
	IFACEMETHOD(QueryInterface) (
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);
	IFACEMETHOD_(ULONG, AddRef) ();
	IFACEMETHOD_(ULONG, Release) ();

public:
	IFACEMETHOD(IsPixelSnappingDisabled)(
		__maybenull void* clientDrawingContext,
		__out BOOL* isDisabled
		);
	IFACEMETHOD(GetCurrentTransform)(
		__maybenull void* clientDrawingContext,
		__out DWRITE_MATRIX* transform
		);
	IFACEMETHOD(GetPixelsPerDip)(
		__maybenull void* clientDrawingContext,
		__out FLOAT* pixelsPerDip
		);

public:
	IFACEMETHOD(DrawGlyphRun)(
		__maybenull void* clientDrawingContext,
		FLOAT baselineOriginX,
		FLOAT baselineOriginY,
		DWRITE_MEASURING_MODE measuringMode,
		__in DWRITE_GLYPH_RUN const* glyphRun,
		__in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
		__maybenull IUnknown* clientDrawingEffect
		);
	IFACEMETHOD(DrawUnderline)(
		__maybenull void* clientDrawingContext,
		FLOAT baselineOriginX,
		FLOAT baselineOriginY,
		__in DWRITE_UNDERLINE const* underline,
		__maybenull IUnknown* clientDrawingEffect
		);
	IFACEMETHOD(DrawStrikethrough)(
		__maybenull void* clientDrawingContext,
		FLOAT baselineOriginX,
		FLOAT baselineOriginY,
		__in DWRITE_STRIKETHROUGH const* strikethrough,
		__maybenull IUnknown* clientDrawingEffect
		);
	IFACEMETHOD(DrawInlineObject)(
		__maybenull void* clientDrawingContext,
		FLOAT originX,
		FLOAT originY,
		IDWriteInlineObject* inlineObject,
		BOOL isSideways,
		BOOL isRightToLeft,
		__maybenull IUnknown* clientDrawingEffect
		);
};
