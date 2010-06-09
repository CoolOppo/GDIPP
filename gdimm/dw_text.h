#pragma once

#include "gdi_text.h"

class gdimm_dw_text : public gdimm_gdi_text, public IDWriteTextRenderer
{
	CComPtr<IDWriteFactory> _dw_factory;
	CComPtr<IDWriteGdiInterop> _dw_gdi_interop;

	vector<FLOAT> _advances;
	DWRITE_MEASURING_MODE _dw_measuring_mode;
	LONG _em_size;
	FLOAT _pixels_per_dip;
	bool _use_gdi_natural;

	bool make_glyph_texture(const DWRITE_GLYPH_RUN *dw_glyph_run);
	bool render_glyph(LPCWSTR lpString, UINT c);
	bool render_text(LPCWSTR lpString, UINT c);
	bool render(UINT options, LPCWSTR lpString, UINT c, CONST INT *lpDx);

	//////////////////////////////////////////////////////////////////////////

	IFACEMETHOD(QueryInterface)( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject);
	IFACEMETHOD_(ULONG, AddRef)( void);
	IFACEMETHOD_(ULONG, Release)( void);

	//////////////////////////////////////////////////////////////////////////

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

	//////////////////////////////////////////////////////////////////////////

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

public:
	gdimm_dw_text();

	bool begin(const gdimm_text_context *context);
	bool text_out(int x, int y, UINT options, CONST RECT *lprect, LPCWSTR lpString, UINT c, CONST INT *lpDx);
};