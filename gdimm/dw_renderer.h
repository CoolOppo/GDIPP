#pragma once

class gdimm_dw_renderer : public IDWriteTextRenderer
{
	LONG _ref_count;
	IDWriteRenderingParams* _render_params;

public:
	struct drawing_context
	{
		IDWriteBitmapRenderTarget* render_target;
		COLORREF text_color;
		const FLOAT *advances;
	};

	gdimm_dw_renderer(IDWriteRenderingParams* render_params);
	gdimm_dw_renderer::~gdimm_dw_renderer();

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
